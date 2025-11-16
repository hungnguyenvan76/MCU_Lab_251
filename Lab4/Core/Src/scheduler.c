/*
 * app_scheduler.c
 *
 *  Created on: Nov 16, 2025
 *      Author: VAIO
 */
//#include "Arduino.h"
/*
 * scheduler.c – STM32 HAL version
 */

#include "scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    void (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint8_t RunMe;
    uint32_t TaskID;
} sTask;

static sTask SCH_tasks_G[SCH_MAX_TASKS];
static uint32_t newTaskID = 0;


static uint32_t Get_New_Task_ID(void) {
    newTaskID++;
    if (newTaskID == NO_TASK_ID) newTaskID++; //tranh overflow
    return newTaskID;
}


void SCH_Init(void) {
    for (int i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask = 0;
        SCH_tasks_G[i].Delay = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe = 0;
        SCH_tasks_G[i].TaskID = 0;
    }
}

void SCH_Update(void) {
    if (SCH_tasks_G[0].pTask && SCH_tasks_G[0].RunMe == 0) {

        if (SCH_tasks_G[0].Delay > 0) {
            SCH_tasks_G[0].Delay--;
        }

        if (SCH_tasks_G[0].Delay == 0) {
            SCH_tasks_G[0].RunMe = 1;
        }
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    uint32_t sumDelay = 0;
    uint32_t newDelay = 0;
    uint8_t index = 0;

    for (index = 0; index < SCH_MAX_TASKS; index++) {

        sumDelay += SCH_tasks_G[index].Delay;

        if (sumDelay > DELAY) {

            newDelay = DELAY - (sumDelay - SCH_tasks_G[index].Delay);
            SCH_tasks_G[index].Delay = sumDelay - DELAY;

            for (uint8_t j = SCH_MAX_TASKS - 1; j > index; j--) {
                SCH_tasks_G[j] = SCH_tasks_G[j - 1];
            }

            SCH_tasks_G[index].pTask  = pFunction;
            SCH_tasks_G[index].Delay  = newDelay;
            SCH_tasks_G[index].Period = PERIOD;
            SCH_tasks_G[index].RunMe  = (newDelay == 0) ? 1 : 0;
            SCH_tasks_G[index].TaskID = Get_New_Task_ID();

            return SCH_tasks_G[index].TaskID;
        }

        else {
            if (SCH_tasks_G[index].pTask == 0) {
                SCH_tasks_G[index].pTask  = pFunction;
                SCH_tasks_G[index].Delay  = DELAY - sumDelay;
                SCH_tasks_G[index].Period = PERIOD;
                SCH_tasks_G[index].RunMe  = (SCH_tasks_G[index].Delay == 0) ? 1 : 0;
                SCH_tasks_G[index].TaskID = Get_New_Task_ID();

                return SCH_tasks_G[index].TaskID;
            }
        }
    }

    return 0; // FULL
}


uint8_t SCH_Delete_Task(uint32_t taskID) {
    uint8_t index, j;

    if (taskID == NO_TASK_ID) return 0;

    for (index = 0; index < SCH_MAX_TASKS; index++) {

        if (SCH_tasks_G[index].TaskID == taskID) {

            if (index < SCH_MAX_TASKS - 1 && SCH_tasks_G[index + 1].pTask != 0) {
                SCH_tasks_G[index + 1].Delay += SCH_tasks_G[index].Delay;
            }

            for (j = index; j < SCH_MAX_TASKS - 1; j++) {
                SCH_tasks_G[j] = SCH_tasks_G[j + 1];
            }

            SCH_tasks_G[j].pTask  = 0;
            SCH_tasks_G[j].Delay  = 0;
            SCH_tasks_G[j].Period = 0;
            SCH_tasks_G[j].RunMe  = 0;
            SCH_tasks_G[j].TaskID = 0;

            return 1;
        }
    }

    return 0; 
}

void SCH_Dispatch_Tasks(void) {

    if (SCH_tasks_G[0].RunMe > 0) {

        (*SCH_tasks_G[0].pTask)();
        SCH_tasks_G[0].RunMe = 0;

        sTask temp = SCH_tasks_G[0];
        SCH_Delete_Task(temp.TaskID);
        if (temp.Period > 0) {
            SCH_Add_Task(temp.pTask, temp.Period, temp.Period);
        }
    }
}

#ifdef __cplusplus
}
#endif
