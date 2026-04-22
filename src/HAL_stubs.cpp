/*

//bridge file for all the HAL stubs

#include <stdint.h>
#include <string>
#include "display.h"
#include <vector>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

extern "C" {
    #include "game_logic.h"
}

static int g_startMenuSelection = 0;
static int g_gameSelection = 0;

extern "C" void DisplayBridge_SetStartSelection(int selectedIndex) {
    g_startMenuSelection = selectedIndex;
}

extern "C" void DisplayBridge_SetGameSelection(int selectedIndex) {
    g_gameSelection = selectedIndex;
}

static std::string card_to_string(Card c) {
    switch (c.rank) {
        case 1:  return "A";
        case 11: return "J";
        case 12: return "Q";
        case 13: return "K";
        default: return std::to_string((int)c.rank);
    }
}

static std::vector<std::string> build_dealer_hand(bool hideHoleCard) {
    std::vector<std::string> dealerHand;

    for (uint8_t i = 0; i < BJ_GetDealerCardCount(); i++) {
        if (hideHoleCard && i == 1) {
            dealerHand.push_back("?");
        } else {
            dealerHand.push_back(card_to_string(BJ_GetDealerCard(i)));
        }
    }

    return dealerHand;
}

static std::vector<std::string> build_player_hand(void) {
    std::vector<std::string> playerHand;

    for (uint8_t i = 0; i < BJ_GetPlayerCardCount(); i++) {
        playerHand.push_back(card_to_string(BJ_GetPlayerCard(i)));
    }

    return playerHand;
}

extern "C" void HAL_Display_ShowStartScreen(void) {
    startscreen(g_startMenuSelection);
}

extern "C" void HAL_Display_ShowGame(void) {
    GameState state = BJ_GetState();

    // Betting phase uses the separate bet screen
    if (state == GAME_STATE_BETTING) {
        bet_screen((int)BJ_GetBank(), (int)BJ_GetBet());
        return;
    }

    // Normal gameplay screen
    // Hide dealer hole card during player turn
    bool hideDealerHoleCard = (state == GAME_STATE_PLAYER_TURN);

    std::vector<std::string> dealerHand = build_dealer_hand(hideDealerHoleCard);
    std::vector<std::string> playerHand = build_player_hand();

    game_screen(
        dealerHand,
        playerHand,
        {},                         // split hand 1 not implemented yet
        {},                         // split hand 2 not implemented yet
        (int)BJ_GetBank(),
        false,                      // canSplit: set true later when split exists in logic
        BJ_DoubleDownAvailable(),   // canDoubleDown
        false,                      // doubleDownActive display flag
        false,                      // splitActive
        true,                       // leftHandActive (unused until split exists)
        g_gameSelection             // current highlighted action
    );
}

extern "C" void HAL_Display_ShowDealerCards(void) {
    std::vector<std::string> dealerHand = build_dealer_hand(false); // reveal all
    std::vector<std::string> playerHand = build_player_hand();

    game_screen(
        dealerHand,
        playerHand,
        {},
        {},
        (int)BJ_GetBank(),
        false,
        BJ_DoubleDownAvailable(),
        false,
        false,
        true,
        g_gameSelection
    );
}

extern "C" void HAL_Display_ShowWin(int32_t amount) {
    winning((int)amount);
}

extern "C" void HAL_Display_ShowLoss(int32_t amount) {
    losing((int)amount);
}

extern "C" void HAL_Display_ShowPush(void) {
    push_screen();
}

// If you do not yet have separate blackjack/bust screens,
// map them to existing result screens for now.
extern "C" void HAL_Display_ShowBlackjack(void) {
    int payout = (int)((BJ_GetBet() * 3) / 2);
    winning(payout);
}

extern "C" void HAL_Display_ShowBust(void) {
    losing((int)BJ_GetBet());
}

/*extern "C" uint32_t HAL_Random_GetSeed(void) {
    // Simple temporary seed source for testing
    // Mix current time with an ADC reading if ADC is available
    uint32_t t = to_ms_since_boot(get_absolute_time());

    adc_init();
    adc_select_input(0);   // change if needed
    uint16_t noise = adc_read();

    return t ^ ((uint32_t)noise << 16) ^ 0xA5A5A5A5u;
}

extern "C" uint32_t HAL_Random_GetSeed(void) {
    return 0x12345678u;
}

extern "C" void HAL_LED_Flash(int pattern) {
    // Temporary stub
    // Replace later with WS2812 / PIO animation code
    (void)pattern;
}

*/

// bridge file for all the HAL stubs

#include <stdint.h>
#include <string>
#include "display.h"
#include <vector>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

extern "C" {
    #include "game_logic.h"
}

/* Changed for LED integration: this bridge now calls the real WS2812 functions. */
#include "leds.h"

static int g_startMenuSelection = 0;
static int g_gameSelection = 0;

extern "C" void DisplayBridge_SetStartSelection(int selectedIndex) {
    g_startMenuSelection = selectedIndex;
}

extern "C" void DisplayBridge_SetGameSelection(int selectedIndex) {
    g_gameSelection = selectedIndex;
}

static std::string card_to_string(Card c) {
    switch (c.rank) {
        case 1:  return "A";
        case 11: return "J";
        case 12: return "Q";
        case 13: return "K";
        default: return std::to_string((int)c.rank);
    }
}

static std::vector<std::string> build_dealer_hand(bool hideHoleCard) {
    std::vector<std::string> dealerHand;

    for (uint8_t i = 0; i < BJ_GetDealerCardCount(); i++) {
        if (hideHoleCard && i == 1) {
            dealerHand.push_back("?");
        } else {
            dealerHand.push_back(card_to_string(BJ_GetDealerCard(i)));
        }
    }

    return dealerHand;
}

static std::vector<std::string> build_player_hand(void) {
    std::vector<std::string> playerHand;

    for (uint8_t i = 0; i < BJ_GetPlayerCardCount(); i++) {
        playerHand.push_back(card_to_string(BJ_GetPlayerCard(i)));
    }

    return playerHand;
}

extern "C" void HAL_Display_ShowStartScreen(void) {
    startscreen(g_startMenuSelection);
}

extern "C" void HAL_Display_ShowGame(void) {
    GameState state = BJ_GetState();

    if (state == GAME_STATE_BETTING) {
        bet_screen((int)BJ_GetBank(), (int)BJ_GetBet());
        return;
    }

    bool hideDealerHoleCard = (state == GAME_STATE_PLAYER_TURN);

    std::vector<std::string> dealerHand = build_dealer_hand(hideDealerHoleCard);
    std::vector<std::string> playerHand = build_player_hand();

    game_screen(
        dealerHand,
        playerHand,
        {},
        {},
        (int)BJ_GetBank(),
        false,
        BJ_DoubleDownAvailable(),
        false,
        false,
        true,
        g_gameSelection
    );
}

extern "C" void HAL_Display_ShowDealerCards(void) {
    std::vector<std::string> dealerHand = build_dealer_hand(false);
    std::vector<std::string> playerHand = build_player_hand();

    game_screen(
        dealerHand,
        playerHand,
        {},
        {},
        (int)BJ_GetBank(),
        false,
        BJ_DoubleDownAvailable(),
        false,
        false,
        true,
        g_gameSelection
    );
}

extern "C" void HAL_Display_ShowWin(int32_t amount) {
    winning((int)amount);
}

extern "C" void HAL_Display_ShowLoss(int32_t amount) {
    losing((int)amount);
}

extern "C" void HAL_Display_ShowPush(void) {
    push_screen();
}

extern "C" void HAL_Display_ShowBlackjack(void) {
    int payout = (int)((BJ_GetBet() * 3) / 2);
    winning(payout);
}

extern "C" void HAL_Display_ShowBust(void) {
    losing((int)BJ_GetBet());
}

/*
extern "C" uint32_t HAL_Random_GetSeed(void) {
    uint32_t t = to_ms_since_boot(get_absolute_time());

    adc_init();
    adc_select_input(0);
    uint16_t noise = adc_read();

    return t ^ ((uint32_t)noise << 16) ^ 0xA5A5A5A5u;
}
*/

extern "C" uint32_t HAL_Random_GetSeed(void) {
    return 0x12345678u;
}

// win/loss LED animation bridge
extern "C" void HAL_LED_Flash(bool winning) {
    uint32_t start = to_ms_since_boot(get_absolute_time());

    if (winning) {
        uint8_t step = 0;
        while (to_ms_since_boot(get_absolute_time()) - start < 3000) {
            leds_show_win_frame(step);
            step += 4;
            sleep_ms(80);
        }
    } else {
        bool on = false;
        while (to_ms_since_boot(get_absolute_time()) - start < 3000) {
            on = !on;
            leds_show_loss_frame(on);
            sleep_ms(150);
        }
    }

    leds_clear();
}
