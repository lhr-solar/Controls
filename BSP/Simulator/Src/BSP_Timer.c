#include "BSP_Timer.h"

#define FILE_NAME DATA_PATH(TIMER_CSV)

static void (*handlers[NUM_TIMERS]) (void);

/** 
 * @brief   Intializes the Timer by writing
 *          the proper reload value to the CSV file
 * @param   reload value of Timer
 * @param   handler interrupt handler
 * @param   timer specified by ticker_t
 * @return  None
 */
void BSP_Timer_Init(int reload, void (*handler) (void), ticker_t timer) {
    // Reads current values
    int current[NUM_TIMERS][2];
    FILE *fp = fopen(FILE_NAME, "r");
    int fno = fileno(fp);
    flock(fno, LOCK_EX);
    for (uint8_t i = 0; i < NUM_TIMERS; i++) {
        fscanf(fp, "%d,%d", current[i], current[i]+1);
    }
    flock(fno, LOCK_UN);
    fclose(fp);

    // Writes new values
    fp = fopen(FILE_NAME, "w");
    fno = fileno(fp);
    flock(fno, LOCK_EX);

    for (uint8_t i = 0; i < NUM_TIMERS; i++) {
        if (timer == i) {
            fprintf(fp, "%d,%d\n", reload, reload);
            handlers[i] = handler;
        } else {
            fprintf(fp, "%d,%d\n", current[i][0], current[i][1]);
        }
    }

    flock(fno, LOCK_UN);
    fclose(fp);
}

/**
* @brief   Updates the time by reading the CSV file
*          and calling relevant handler at the right time 
* @param   None
* @return  None
*/  
void BSP_Timer_Update() {
    int current[NUM_TIMERS][2];
    static bool called[NUM_TIMERS];
    FILE *file = fopen(FILE_NAME, "r");
    int fno = fileno(file);
    flock(fno, LOCK_EX);

    for (uint8_t i = 0; i < NUM_TIMERS; i++) {
        fscanf(file, "%d,%d", current[i], current[i]+1);
    }

    flock(fno, LOCK_UN);
    fclose(file);

    for (uint8_t i = 0; i < NUM_TIMERS; i++) {
        if (current[i][0] == 0) {
            if (!called[i]) {
                handlers[i]();
                called[i] = true;
            }
        } else {
            called[i] = false;
        }
    }
}
