
/*
#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t rank;
    uint8_t suit;
} Card;

typedef enum {
    GAME_STATE_START,
    GAME_STATE_BETTING,
    GAME_STATE_PLAYER_TURN,
    GAME_STATE_DEALER_TURN,
    GAME_STATE_RESOLVE
} GameState;

// Core game API
void BJ_Init(void);
void BJ_StartGame(void);
void BJ_IncreaseBet(void);
void BJ_DecreaseBet(voi);
void BJ_Deal(void);
void BJ_Hit(void);
void BJ_Stand(void);
void BJ_DoubleDown(void);
void BJ_TakeInsurance(void);
void BJ_PlayAgain(void);
void BJ_ResetGame(void);

// Getters used by display bridge
GameState BJ_GetState(void);
int32_t BJ_GetBank(void);
int32_t BJ_GetBet(void);
bool BJ_DoubleDownAvailable(void);

uint8_t BJ_GetDealerCardCount(void);
uint8_t BJ_GetPlayerCardCount(void);
Card BJ_GetDealerCard(uint8_t index);
Card BJ_GetPlayerCard(uint8_t index);

#endif

*/

#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t rank;
    uint8_t suit;
} Card;

/* Changed for LED integration: this now matches the GameState enum already used in game_logic.c. */
typedef enum {
    GAME_STATE_IDLE = 0,
    GAME_STATE_BETTING,
    GAME_STATE_DEALING,
    GAME_STATE_PLAYER_TURN,
    GAME_STATE_DEALER_TURN,
    GAME_STATE_RESOLVE,
    GAME_STATE_GAME_OVER
} GameState;

/* Changed for LED integration: this result enum already exists in game_logic.c, and HAL/display code needs it in the header too. */
typedef enum {
    RESULT_NONE = 0,
    RESULT_PLAYER_WIN,
    RESULT_PLAYER_BLACKJACK,
    RESULT_DEALER_WIN,
    RESULT_PUSH,
    RESULT_PLAYER_BUST,
    RESULT_DEALER_BUST
} RoundResult;

/* Core game API */
void BJ_Init(void);
void BJ_StartGame(void);
void BJ_IncreaseBet(void);
void BJ_DecreaseBet(void);
void BJ_Deal(void);
void BJ_Hit(void);
void BJ_Stand(void);
void BJ_DoubleDown(void);
void BJ_TakeInsurance(void);
void BJ_PlayAgain(void);
void BJ_ResetGame(void);

/* Getters used by display bridge */
GameState BJ_GetState(void);
int32_t BJ_GetBank(void);
int32_t BJ_GetBet(void);
bool BJ_DoubleDownAvailable(void);

uint8_t BJ_GetDealerCardCount(void);
uint8_t BJ_GetPlayerCardCount(void);
Card BJ_GetDealerCard(uint8_t index);
Card BJ_GetPlayerCard(uint8_t index);

/* Changed for LED integration: these getters are already implemented in game_logic.c. */
RoundResult BJ_GetLastResult(void);
int BJ_GetPlayerValue(void);
int BJ_GetDealerValue(void);
bool BJ_InsuranceAvailable(void);

#endif