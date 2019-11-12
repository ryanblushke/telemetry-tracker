/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*					WIN32 PORT & LINUX PORT
*                          (c) Copyright 2004, Werner.Zimmermann@fht-esslingen.de
*                 (Similar to Example 1 of the 80x86 Real Mode port by Jean J. Labrosse)
*                                           All Rights Reserved
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/
#define _CRT_SECURE_NO_WARNINGS
#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define	TASK_STK_SIZE		512       /* Size of each task's stacks (# of WORDs)            */

#define	TaskStartPrio		 1
#define	TaskScanKeyPrio		10
#define	TaskDispTimePrio	 9
#define TaskUpdateTimePrio      8
#define TaskTimerStatePrio      7

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK	TaskStartStk[TASK_STK_SIZE];
OS_STK	TaskScanKeyStk[TASK_STK_SIZE];
OS_STK	TaskDispTimeStk[TASK_STK_SIZE];
OS_STK	TaskUpdateTimeStk[TASK_STK_SIZE];
OS_STK	TaskTimerStateStk[TASK_STK_SIZE];

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

	void  TaskStart(void *data);		/* Function prototypes of Startup task           */
	void  TaskScanKey(void *data);
	void  TaskDispTime(void *data);
	void TaskUpdateTime(void *pdata);
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskDispSys(void);
void TaskTimerState(void *pdata);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

OS_EVENT *SemDisp;
OS_EVENT *SemKeyPress;
OS_EVENT *MboxDisp;
typedef enum {CLEAR, COUNT, STOP} TimerStates;
static TimerStates CurTimerState = CLEAR; 

int mm = 0;
int ss = 0;
int dd = 0;

int  main (void)
{
	PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */

	OSInit();                                              /* Initialize uC/OS-II                      */

	OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], TaskStartPrio);
	SemDisp = OSSemCreate(1);
	SemKeyPress = OSSemCreate(0);
	MboxDisp = OSMboxCreate((void *) 0);
	OSStart();                                             /* Start multitasking                       */
	return 0;
}


/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void  TaskStart (void *pdata)
{
	pdata = pdata;                                         /* Prevent compiler warning                 */

	TaskStartDispInit();                                   /* Initialize the display                   */

	TaskStartCreateTasks();                                /* Create all the application tasks         */

	OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
	
	for (;;) {
		// OSTaskSuspend(TaskStartPrio);
		OSTaskDel(TaskStartPrio);
	}
}


/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit (void)
{
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
    PC_DispStr( 0,  0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED);
#ifdef __WIN32__
    PC_DispStr( 0,  1, "  Original version by Jean J. Labrosse, 80x86-WIN32 port by Werner Zimmermann   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
#endif
#ifdef __LINUX__
    PC_DispStr( 0,  1, "  Original version by Jean J. Labrosse, 80x86-LINUX port by Werner Zimmermann   ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
#endif
    PC_DispStr( 0,  2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  3, "                                    STOPWATCH                                   ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  5, "                                      00:00                                     ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0,  9, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 10, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 12, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 13, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 14, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 15, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 16, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 17, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 18, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 21, "#Tasks          :        CPU Usage:     %                                       ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 22, "#Task switch/sec:                                                               ", DISP_FGND_BLACK + DISP_BGND_WHITE);
    PC_DispStr( 0, 23, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_WHITE);
/*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
/*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
}


/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskDispSys (void)
{
    char   s[80];

    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 21, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

#if OS_TASK_STAT_EN > 0
    sprintf(s, "%3d", OSCPUUsage /*OSIdleCtr/(OSIdleCtrMax/100)*/);/* Display CPU usage in %               */
    PC_DispStr(36, 21, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
#endif

    PC_GetDateTime(s);
    PC_DispStr(58, 21, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

#ifdef __WIN32__
    sprintf(s, "uCOS-II V%1d.%02d  WIN32 V%1d.%02d", OSVersion() / 100, OSVersion() % 100, OSPortVersion() / 100, OSPortVersion() % 100); /* Display uC/OS-II's version number    */
#endif
#ifdef __LINUX__
    sprintf(s, "uCOS-II V%1d.%02d  LINUX V%1d.%02d", OSVersion() / 100, OSVersion() % 100, OSPortVersion() / 100, OSPortVersion() % 100); /* Display uC/OS-II's version number    */
#endif

    PC_DispStr(52, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
}


/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks (void)
{
	OSTaskCreate(TaskScanKey, (void *) 0, &TaskScanKeyStk[TASK_STK_SIZE - 1], TaskScanKeyPrio);
	OSTaskCreate(TaskDispTime, (void *) 0, &TaskDispTimeStk[TASK_STK_SIZE - 1], TaskDispTimePrio);
	OSTaskCreate(TaskUpdateTime, (void *) 0, &TaskUpdateTimeStk[TASK_STK_SIZE - 1], TaskUpdateTimePrio);
	OSTaskCreate(TaskTimerState, (void *) 0, &TaskTimerStateStk[TASK_STK_SIZE - 1], TaskTimerStatePrio);
}

/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/


void  TaskScanKey (void *pdata) {
	INT16S     key;

	for (;;) {
        	if (PC_GetKey(&key) == TRUE) {			/* See if key has been pressed		*/
            		if (key == 0x1B) {				/* Yes, see if it's the ESCAPE key	*/
               			exit(0);				/* End program				*/
            		} else {
				OSSemPost(SemKeyPress);
			}
        	}
       		OSTimeDly(10);
	}
}

void  TaskDispTime (void *pdata) {
	INT8U err;
	int *sec;
	int *min;
	
    for (;;) {
 
		TaskDispSys();					/* Update the display			*/

		sec = OSMboxPend(MboxDisp, 0, &err);
		min = OSMboxPend(MboxDisp, 0, &err);
		char s[5];
		sprintf(s, "%2d", min); // convert minutes to a string
		PC_DispStr(38, 5, s, DISP_FGND_BLACK + DISP_BGND_WHITE);
		sprintf(s, "%2d", sec); // convert seconds to a string
		PC_DispStr(41, 5, s, DISP_FGND_BLACK + DISP_BGND_WHITE);
	
		OSMboxPost(MboxDisp, (void *)1);

		OSCtxSwCtr = 0;					/* Clear context switch counter		*/
		OSTimeDlyHMSM(0, 0, 1, 0); 
    }
}

void TaskUpdateTime(void *pdata) {
	INT8U err;
	int min, sec;

	for (;;) {
		OSTimeDly(1);
		OSMboxPend(MboxDisp, 0, &err);
		if (CurTimerState == COUNT) {
			if (dd == 99) {
				dd = 0;
				if (ss == 59) {
					ss = 0;
					if (mm == 59) {
						mm = 0;
					} else {
						mm++;
					}
				} else {
					ss++;
				}	
			} else {
				dd++;
			}
		} else if (CurTimerState == CLEAR) {
			mm = 0;
			ss = 0;
			dd = 0;
		}
		min = mm;
		sec = ss;
		OSMboxPost(MboxDisp, (void *) &sec);
		OSMboxPost(MboxDisp, (void *) &min);
	}
}

void TaskTimerState(void *pdata) {
	INT8U err;

	for (;;) {
		OSSemPend(SemKeyPress, 0, &err);

		if (CurTimerState == CLEAR) {
			CurTimerState = COUNT;
		} else if (CurTimerState == COUNT) {
			CurTimerState = STOP;
		} else {
			CurTimerState = CLEAR;
		}
	}
}
