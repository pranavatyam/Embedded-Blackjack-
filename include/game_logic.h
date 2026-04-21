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
void BJ_DecreaseBet(void);
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