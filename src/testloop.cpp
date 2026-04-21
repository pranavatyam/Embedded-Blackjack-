#include <stdio.h>
#include "pico/stdlib.h"

extern "C" {
#include "game_logic.h"   // change if your logic header has a different name
}

#include "HAL_stubs.h"

// Optional if these HAL functions are not already in a header:
extern "C" void HAL_Display_ShowStartScreen(void);
extern "C" void HAL_Display_ShowGame(void);
extern "C" void HAL_Display_ShowDealerCards(void);
extern "C" void HAL_Display_ShowWin(int32_t amount);
extern "C" void HAL_Display_ShowLoss(int32_t amount);
extern "C" void HAL_Display_ShowPush(void);

static void print_test_help() {
    printf("\n=== Blackjack Serial Test ===\n");
    printf("m : show this help\n");
    printf("0 : start screen, YES selected\n");
    printf("1 : start screen, NO selected\n");
    printf("s : BJ_StartGame()\n");
    printf("u : BJ_IncreaseBet()\n");
    printf("d : BJ_DecreaseBet()\n");
    printf("e : BJ_Deal()\n");
    printf("h : BJ_Hit()\n");
    printf("t : BJ_Stand()\n");
    printf("x : BJ_DoubleDown()\n");
    printf("i : BJ_TakeInsurance()\n");
    printf("p : BJ_PlayAgain()\n");
    printf("r : BJ_ResetGame()\n");
    printf("g : redraw current game/bet screen\n");
    printf("a : highlight HIT\n");
    printf("b : highlight STAND\n");
    printf("c : highlight SPLIT\n");
    printf("f : highlight DOUBLE\n");
    printf("v : reveal dealer cards\n");
    printf("w : test win screen\n");
    printf("l : test loss screen\n");
    printf("q : test push screen\n");
    printf("=============================\n");
}

extern "C" void serial_test_loop() {
    print_test_help();

    // Start on the start screen with YES selected
    DisplayBridge_SetStartSelection(0);
    HAL_Display_ShowStartScreen();

    while (true) {
        int ch = getchar_timeout_us(0);

        if (ch == PICO_ERROR_TIMEOUT) {
            sleep_ms(20);
            continue;
        }

        switch (ch) {
            case 'm':
            case 'M':
                print_test_help();
                break;

            case '0':
                DisplayBridge_SetStartSelection(0);
                HAL_Display_ShowStartScreen();
                printf("Start screen: YES selected\n");
                break;

            case '1':
                DisplayBridge_SetStartSelection(1);
                HAL_Display_ShowStartScreen();
                printf("Start screen: NO selected\n");
                break;

            case 's':
            case 'S':
                BJ_StartGame();
                printf("BJ_StartGame()\n");
                break;

            case 'u':
            case 'U':
                BJ_IncreaseBet();
                printf("BJ_IncreaseBet()\n");
                break;

            case 'd':
            case 'D':
                BJ_DecreaseBet();
                printf("BJ_DecreaseBet()\n");
                break;

            case 'e':
            case 'E':
                BJ_Deal();
                printf("BJ_Deal()\n");
                break;

            case 'h':
            case 'H':
                BJ_Hit();
                printf("BJ_Hit()\n");
                break;

            case 't':
            case 'T':
                BJ_Stand();
                printf("BJ_Stand()\n");
                break;

            case 'x':
            case 'X':
                BJ_DoubleDown();
                printf("BJ_DoubleDown()\n");
                break;

            case 'i':
            case 'I':
                BJ_TakeInsurance();
                printf("BJ_TakeInsurance()\n");
                break;

            case 'p':
            case 'P':
                BJ_PlayAgain();
                printf("BJ_PlayAgain()\n");
                break;

            case 'r':
            case 'R':
                BJ_ResetGame();
                printf("BJ_ResetGame()\n");
                break;

            case 'g':
            case 'G':
                HAL_Display_ShowGame();
                printf("HAL_Display_ShowGame()\n");
                break;

            case 'a':
            case 'A':
                DisplayBridge_SetGameSelection(0);   // HIT
                HAL_Display_ShowGame();
                printf("Cursor -> HIT\n");
                break;

            case 'b':
            case 'B':
                DisplayBridge_SetGameSelection(1);   // STAND
                HAL_Display_ShowGame();
                printf("Cursor -> STAND\n");
                break;

            case 'c':
            case 'C':
                DisplayBridge_SetGameSelection(2);   // SPLIT
                HAL_Display_ShowGame();
                printf("Cursor -> SPLIT\n");
                break;

            case 'f':
            case 'F':
                DisplayBridge_SetGameSelection(3);   // DOUBLE
                HAL_Display_ShowGame();
                printf("Cursor -> DOUBLE\n");
                break;

            case 'v':
            case 'V':
                HAL_Display_ShowDealerCards();
                printf("HAL_Display_ShowDealerCards()\n");
                break;

            case 'w':
            case 'W':
                HAL_Display_ShowWin(50);
                printf("Test win screen\n");
                break;

            case 'l':
            case 'L':
                HAL_Display_ShowLoss(25);
                printf("Test loss screen\n");
                break;

            case 'q':
            case 'Q':
                HAL_Display_ShowPush();
                printf("Test push screen\n");
                break;

            case '\r':
            case '\n':
                break;

            default:
                printf("Unknown command: %c\n", (char)ch);
                break;
        }

        sleep_ms(20);
    }
}