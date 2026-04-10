/*
 * blackjack_logic.c
 *
 * Core game logic for embedded Blackjack (no display/IO code).
 * Dealer stands on 17. 52-card deck. Standard blackjack rules.
 *
 * Intended for use with a microcontroller; all display, PWM audio,
 * LED, and joystick input calls are left as stubs (prefixed HAL_).
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ──────────────────────────────────────────────
   HAL STUBS  –  replace with your hardware calls
   ────────────────────────────────────────────── */
extern void     HAL_Display_ShowStartScreen(void);
extern void     HAL_Display_ShowGame(void);          /* re-render cards/bank */
extern void     HAL_Display_ShowWin(int32_t amount);
extern void     HAL_Display_ShowLoss(int32_t amount);
extern void     HAL_Display_ShowBlackjack(void);
extern void     HAL_Display_ShowBust(void);
extern void     HAL_Display_ShowPush(void);
extern void     HAL_Display_ShowDealerCards(void);
extern void     HAL_Audio_PlayWin(void);
extern void     HAL_Audio_PlayLoss(void);
extern void     HAL_LED_Flash(bool winning);
extern uint32_t HAL_Random_GetSeed(void);           /* hardware RNG / timer */

/* ──────────────────────────────────────────────
   CONSTANTS
   ────────────────────────────────────────────── */
#define DECK_SIZE           52
#define MAX_HAND_CARDS      11   /* max cards before bust (all Aces) */
#define DEALER_STAND_VALUE  17
#define BET_UNIT            5    /* $5 per tick */
#define MAX_BET_TICKS       10
#define STARTING_BANK       100  /* starting bank in dollars */

/* ──────────────────────────────────────────────
   TYPES
   ────────────────────────────────────────────── */
typedef enum {
    SUIT_CLUBS = 0,
    SUIT_DIAMONDS,
    SUIT_HEARTS,
    SUIT_SPADES
} Suit;

typedef struct {
    uint8_t rank;   /* 1=Ace, 2-10, 11=Jack, 12=Queen, 13=King */
    Suit    suit;
} Card;

typedef struct {
    Card    cards[MAX_HAND_CARDS];
    uint8_t count;
} Hand;

typedef enum {
    GAME_STATE_IDLE = 0,
    GAME_STATE_BETTING,
    GAME_STATE_DEALING,
    GAME_STATE_PLAYER_TURN,
    GAME_STATE_DEALER_TURN,
    GAME_STATE_RESOLVE,
    GAME_STATE_GAME_OVER
} GameState;

typedef enum {
    RESULT_NONE = 0,
    RESULT_PLAYER_WIN,
    RESULT_PLAYER_BLACKJACK,
    RESULT_DEALER_WIN,
    RESULT_PUSH,
    RESULT_PLAYER_BUST,
    RESULT_DEALER_BUST
} RoundResult;

/* ──────────────────────────────────────────────
   GAME CONTEXT  (single global for embedded)
   ────────────────────────────────────────────── */
typedef struct {
    Card        deck[DECK_SIZE];
    uint8_t     deck_index;         /* next card to deal */

    Hand        player_hand;
    Hand        dealer_hand;

    int32_t     bank;               /* dollars */
    int32_t     current_bet;        /* dollars */
    uint8_t     bet_ticks;          /* joystick up ticks */
    bool        doubled_down;
    bool        insurance_taken;

    GameState   state;
    RoundResult last_result;
} BlackjackGame;

static BlackjackGame g;             /* single game instance */

/* ──────────────────────────────────────────────
   INTERNAL: PRNG (simple LCG for embedded)
   ────────────────────────────────────────────── */
static uint32_t prng_state;

static void prng_seed(uint32_t seed) {
    prng_state = seed ? seed : 12345u; //check
}

static uint32_t prng_next(void) {
    prng_state = prng_state * 1664525u + 1013904223u; //recheck
    return prng_state;
}

/* ──────────────────────────────────────────────
   INTERNAL: DECK OPERATIONS
   ────────────────────────────────────────────── */
static void deck_init_and_shuffle(void) {
    /* Build ordered deck */
    for (uint8_t s = 0; s < 4; s++) {
        for (uint8_t r = 1; r <= 13; r++) {
            g.deck[s * 13 + r - 1].rank = r;
            g.deck[s * 13 + r - 1].suit = (Suit)s;
        }
    }
    /* Fisher-Yates shuffle */
    for (int i = DECK_SIZE - 1; i > 0; i--) {
        int j = (int)(prng_next() % (uint32_t)(i + 1));
        Card tmp   = g.deck[i];
        g.deck[i]  = g.deck[j];
        g.deck[j]  = tmp;
    }
    g.deck_index = 0;
}

/* Reshuffle when fewer than 10 cards remain */
static void deck_check_reshuffle(void) {
    if (g.deck_index >= DECK_SIZE - 10) {
        deck_init_and_shuffle();
    }
}

static Card deck_deal_card(void) {
    deck_check_reshuffle();
    return g.deck[g.deck_index++];
}

/* ──────────────────────────────────────────────
   INTERNAL: HAND SCORING
   ────────────────────────────────────────────── */
static int hand_value(const Hand *h) {
    int total    = 0;
    int ace_count = 0;

    for (uint8_t i = 0; i < h->count; i++) {
        uint8_t r = h->cards[i].rank;
        if (r == 1) {
            ace_count++;
            total += 11;        /* optimistic Ace */
        } else if (r >= 10) {
            total += 10;        /* J, Q, K */
        } else {
            total += r;
        }
    }
    /* Convert Aces from 11 → 1 to avoid bust */
    while (total > 21 && ace_count > 0) {
        total    -= 10;
        ace_count--;
    }
    return total;
}

static bool hand_is_blackjack(const Hand *h) {
    if (h->count != 2) return false;
    int v = hand_value(h);
    return (v == 21);
}

static bool hand_is_bust(const Hand *h) {
    return (hand_value(h) > 21);
}

static void hand_add_card(Hand *h, Card c) {
    if (h->count < MAX_HAND_CARDS) {
        h->cards[h->count++] = c;
    }
}

static void hand_clear(Hand *h) {
    h->count = 0;
}

/* ──────────────────────────────────────────────
   INTERNAL: BET HELPERS
   ────────────────────────────────────────────── */
/*
 * Exponential bet scaling:
 *   tick 0 → $5, tick 1 → $10, tick 2 → $20, ...
 *   bet = BET_UNIT * 2^ticks  (capped at bank balance)
 */
static int32_t ticks_to_bet(uint8_t ticks) {
    int32_t bet = BET_UNIT;
    for (uint8_t i = 0; i < ticks; i++) {
        bet *= 2;
    }
    if (bet > g.bank) bet = g.bank;
    return bet;
}

/* ──────────────────────────────────────────────
   INTERNAL: RESOLVE ROUND
   ────────────────────────────────────────────── */
static void resolve_round(void) {
    int player_val = hand_value(&g.player_hand);
    int dealer_val = hand_value(&g.dealer_hand);

    bool player_bj = hand_is_blackjack(&g.player_hand);
    bool dealer_bj = hand_is_blackjack(&g.dealer_hand);
    bool player_bust = (player_val > 21);
    bool dealer_bust  = (dealer_val > 21);

    int32_t payout = 0;

    if (player_bust) {
        g.last_result = RESULT_PLAYER_BUST;
        payout = -g.current_bet;

    } else if (player_bj && dealer_bj) {
        /* Both blackjack → push */
        g.last_result = RESULT_PUSH;
        payout = 0;

    } else if (player_bj) {
        /* Blackjack pays 3:2 */
        g.last_result = RESULT_PLAYER_BLACKJACK;
        payout = (g.current_bet * 3) / 2;

    } else if (dealer_bj || dealer_val > player_val && !dealer_bust) {
        g.last_result = RESULT_DEALER_WIN;
        payout = -g.current_bet;

    } else if (dealer_bust || player_val > dealer_val) {
        g.last_result = RESULT_PLAYER_WIN;
        payout = g.current_bet;
        /* Double down doubles the payout */
        if (g.doubled_down) payout *= 2;

    } else {
        /* Equal values → push */
        g.last_result = RESULT_PUSH;
        payout = 0;
    }

    /* Insurance side-bet: pays 2:1 if dealer has blackjack */
    if (g.insurance_taken) {
        if (dealer_bj) {
            payout += g.current_bet;   /* insurance wins (bet/2 * 2 = original bet) */
        } else {
            payout -= g.current_bet / 2;  /* insurance lost */
        }
    }

    g.bank += payout;

    /* Trigger HAL output */
    if (payout > 0) {
        HAL_Audio_PlayWin();
        HAL_LED_Flash(true);
        if (g.last_result == RESULT_PLAYER_BLACKJACK) {
            HAL_Display_ShowBlackjack();
        } else {
            HAL_Display_ShowWin(payout);
        }
    } else if (payout < 0) {
        HAL_Audio_PlayLoss();
        HAL_LED_Flash(false);
        if (g.last_result == RESULT_PLAYER_BUST) {
            HAL_Display_ShowBust();
        } else {
            HAL_Display_ShowLoss(-payout);
        }
    } else {
        HAL_Display_ShowPush();
    }

    /* Check if player is out of money */
    if (g.bank <= 0) {
        g.bank  = 0;
        g.state = GAME_STATE_GAME_OVER;
    } else {
        g.state = GAME_STATE_RESOLVE;
    }
}

/* ──────────────────────────────────────────────
   PUBLIC API
   ────────────────────────────────────────────── */

/*
 * BJ_Init – call once at boot.
 */
void BJ_Init(void) {
    prng_seed(HAL_Random_GetSeed());
    memset(&g, 0, sizeof(g));
    g.bank  = STARTING_BANK;
    g.state = GAME_STATE_IDLE;
    deck_init_and_shuffle();
    HAL_Display_ShowStartScreen();
}

/*
 * BJ_StartGame – joystick press from IDLE / GAME_OVER.
 * Transitions to BETTING state.
 */
void BJ_StartGame(void) {
    if (g.state != GAME_STATE_IDLE && g.state != GAME_STATE_GAME_OVER) return;
    if (g.state == GAME_STATE_GAME_OVER) {
        /* Full reset */
        g.bank = STARTING_BANK;
        deck_init_and_shuffle();
    }
    g.bet_ticks   = 0;
    g.current_bet = ticks_to_bet(0);
    g.state       = GAME_STATE_BETTING;
    HAL_Display_ShowGame();
}

/*
 * BJ_IncreaseBet – joystick UP during BETTING.
 * Exponential: each tick doubles. Holding repeats.
 */
void BJ_IncreaseBet(void) {
    if (g.state != GAME_STATE_BETTING) return;
    if (g.bet_ticks < MAX_BET_TICKS) {
        g.bet_ticks++;
    }
    g.current_bet = ticks_to_bet(g.bet_ticks);
    HAL_Display_ShowGame();
}

/*
 * BJ_DecreaseBet – joystick DOWN during BETTING (reverse).
 */
void BJ_DecreaseBet(void) {
    if (g.state != GAME_STATE_BETTING) return;
    if (g.bet_ticks > 0) {
        g.bet_ticks--;
    }
    g.current_bet = ticks_to_bet(g.bet_ticks);
    HAL_Display_ShowGame();
}

/*
 * BJ_Deal – joystick press confirms bet and deals opening hand.
 */
void BJ_Deal(void) {
    if (g.state != GAME_STATE_BETTING) return;
    if (g.current_bet > g.bank) g.current_bet = g.bank;

    g.doubled_down    = false;
    g.insurance_taken = false;

    hand_clear(&g.player_hand);
    hand_clear(&g.dealer_hand);

    /* Standard deal order: player, dealer, player, dealer */
    hand_add_card(&g.player_hand, deck_deal_card());
    hand_add_card(&g.dealer_hand, deck_deal_card());
    hand_add_card(&g.player_hand, deck_deal_card());
    hand_add_card(&g.dealer_hand, deck_deal_card());

    g.state = GAME_STATE_PLAYER_TURN;
    HAL_Display_ShowGame();

    /* Check for immediate blackjack */
    if (hand_is_blackjack(&g.player_hand)) {
        HAL_Display_ShowDealerCards();   /* reveal dealer */
        resolve_round();
    }
}

/*
 * BJ_Hit – joystick swipe right + confirm during PLAYER_TURN.
 */
void BJ_Hit(void) {
    if (g.state != GAME_STATE_PLAYER_TURN) return;
    hand_add_card(&g.player_hand, deck_deal_card());
    HAL_Display_ShowGame();

    if (hand_is_bust(&g.player_hand)) {
        HAL_Display_ShowDealerCards();
        resolve_round();
    }
}

/*
 * BJ_Stand – joystick swipe left + confirm during PLAYER_TURN.
 * Dealer auto-draws until >= 17.
 */
void BJ_Stand(void) {
    if (g.state != GAME_STATE_PLAYER_TURN) return;
    g.state = GAME_STATE_DEALER_TURN;

    while (hand_value(&g.dealer_hand) < DEALER_STAND_VALUE) {
        hand_add_card(&g.dealer_hand, deck_deal_card());
    }

    HAL_Display_ShowDealerCards();
    resolve_round();
}

/*
 * BJ_DoubleDown – joystick swipe down + confirm during PLAYER_TURN.
 * Doubles the bet, deals exactly one more card, then stands.
 */
void BJ_DoubleDown(void) {
    if (g.state != GAME_STATE_PLAYER_TURN) return;
    if (g.player_hand.count != 2)          return; /* only on first two cards */

    int32_t extra = g.current_bet;
    if (extra > (g.bank - g.current_bet)) {
        extra = g.bank - g.current_bet;    /* cap to available funds */
    }
    g.current_bet  += extra;
    g.doubled_down  = true;

    hand_add_card(&g.player_hand, deck_deal_card());
    HAL_Display_ShowGame();

    if (hand_is_bust(&g.player_hand)) {
        HAL_Display_ShowDealerCards();
        resolve_round();
    } else {
        BJ_Stand();   /* double-down always stands after one card */
    }
}

/*
 * BJ_TakeInsurance – called when dealer's up-card is an Ace.
 * Side bet = half the current bet.
 */
void BJ_TakeInsurance(void) {
    if (g.state != GAME_STATE_PLAYER_TURN) return;
    /* Only offer insurance when dealer's first visible card is Ace */
    if (g.dealer_hand.cards[0].rank != 1)  return;
    if (g.insurance_taken)                 return;

    int32_t insurance_cost = g.current_bet / 2;
    if (insurance_cost < 1) insurance_cost = 1;
    if (insurance_cost > g.bank - g.current_bet) return; /* can't afford */

    g.insurance_taken = true;
    g.bank           -= insurance_cost;  /* deduct now; refunded on win */
    HAL_Display_ShowGame();
}

/*
 * BJ_PlayAgain – after RESOLVE, reset for a new round.
 */
void BJ_PlayAgain(void) {
    if (g.state != GAME_STATE_RESOLVE) return;
    g.bet_ticks   = 0;
    g.current_bet = ticks_to_bet(0);
    g.state       = GAME_STATE_BETTING;
    HAL_Display_ShowGame();
}

/*
 * BJ_ResetGame – full reset from GAME_OVER or any state.
 * Call when player holds joystick down to reset.
 */
void BJ_ResetGame(void) {
    g.bank = STARTING_BANK;
    deck_init_and_shuffle();
    g.bet_ticks   = 0;
    g.current_bet = ticks_to_bet(0);
    g.state       = GAME_STATE_BETTING;
    hand_clear(&g.player_hand);
    hand_clear(&g.dealer_hand);
    g.doubled_down    = false;
    g.insurance_taken = false;
    HAL_Display_ShowGame();
}

/* ──────────────────────────────────────────────
   GETTERS  (for display layer)
   ────────────────────────────────────────────── */

int        BJ_GetPlayerValue(void)  { return hand_value(&g.player_hand);  }
int        BJ_GetDealerValue(void)  { return hand_value(&g.dealer_hand);  }
int32_t    BJ_GetBank(void)         { return g.bank;                      }
int32_t    BJ_GetBet(void)          { return g.current_bet;               }
GameState  BJ_GetState(void)        { return g.state;                     }
RoundResult BJ_GetLastResult(void)  { return g.last_result;               }

uint8_t    BJ_GetPlayerCardCount(void)       { return g.player_hand.count; }
uint8_t    BJ_GetDealerCardCount(void)       { return g.dealer_hand.count; }
Card       BJ_GetPlayerCard(uint8_t idx)     { return g.player_hand.cards[idx]; }
Card       BJ_GetDealerCard(uint8_t idx)     { return g.dealer_hand.cards[idx]; }

bool       BJ_InsuranceAvailable(void) {
    return (g.state == GAME_STATE_PLAYER_TURN &&
            g.player_hand.count == 2         &&
            g.dealer_hand.cards[0].rank == 1 &&
            !g.insurance_taken);
}
bool       BJ_DoubleDownAvailable(void) {
    return (g.state == GAME_STATE_PLAYER_TURN &&
            g.player_hand.count == 2);
}
