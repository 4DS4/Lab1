/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_uart.h"
#include "fsl_ftm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */

#define TARGET_UART UART4
#define FTM_MOTOR FTM0
#define FTM_CHANNEL_DC_MOTOR kFTM_Chnl_0
#define FTM_SERVO FTM0
#define FTM_CHANNEL_SERVO kFTM_Chnl_3


void setupPWM(ftm_chnl_t channel, FTM_Type* type)
{
	ftm_config_t ftmInfo;
	ftm_chnl_pwm_signal_param_t ftmParam;
	ftm_pwm_level_select_t pwmLevel = kFTM_HighTrue;
	ftmParam.chnlNumber = channel;
	ftmParam.level = pwmLevel;
	ftmParam.dutyCyclePercent = 7;
	ftmParam.firstEdgeDelayPercent = 0U;
	ftmParam.enableComplementary = false;
	ftmParam.enableDeadtime = false;
	FTM_GetDefaultConfig(&ftmInfo);
	ftmInfo.prescale = kFTM_Prescale_Divide_128;
	FTM_Init(type, &ftmInfo);
	FTM_SetupPwm(type, &ftmParam, 1U, kFTM_EdgeAlignedPwm, 50U, CLOCK_GetFreq(
			kCLOCK_BusClk));
	FTM_StartTimer(type, kFTM_SystemClock);
}

void updatePWM_dutyCycle(ftm_chnl_t channel, float dutyCycle)
{
	uint32_t cnv, cnvFirstEdge = 0, mod;
	/* The CHANNEL_COUNT macro returns -1 if it cannot match the FTM instance */
	assert(-1 != FSL_FEATURE_FTM_CHANNEL_COUNTn(FTM_MOTOR));
	mod = FTM_MOTOR->MOD;
	if (dutyCycle == 0U)
	{
		/* Signal stays low */
		cnv = 0;
	}
	else
	{
		cnv = mod * dutyCycle;
		/* For 100% duty cycle */
		if (cnv >= mod)
		{
			cnv = mod + 1U;
		}
	}
	FTM_MOTOR->CONTROLS[channel].CnV = cnv;
}

void setupUART()
{
uart_config_t config;
UART_GetDefaultConfig(&config);
config.baudRate_Bps = 57600;
config.enableTx = true;
config.enableRx = true;
config.enableRxRTS = true;
config.enableTxCTS = true;
UART_Init(TARGET_UART, &config, CLOCK_GetFreq(kCLOCK_BusClk));
}

int main(void)
{

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("hello world.\r\n");

    /*
     *
	ptc13 - uart4 cts
	ptc14 - rx
	ptc15 - tx
	pte27 - uart4 rts
     *
    */

    char ch[5];
    char txbuff[] = "Hello World\r\n";
    int speed;
    int angle;
	float dutyCycleSpeed;
	float dutyCycleAngle;

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    setupUART();
    setupPWM(FTM_CHANNEL_DC_MOTOR, FTM_MOTOR);
    setupPWM(FTM_CHANNEL_SERVO, FTM_SERVO);

    /******* Delay *******/
    for(volatile int i = 0U; i < 10000000; i++)
    __asm("NOP");
    PRINTF("%s", txbuff);
    UART_WriteBlocking(TARGET_UART, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
    UART_ReadBlocking(TARGET_UART, &ch, 9);
    PRINTF("%.9s\r\n", ch);
    sscanf(ch, "%d,%d", &speed, &angle);

    printf("%d\n", speed);
    printf("%d\n\n", angle);
	dutyCycleSpeed = speed * 0.0125f/100.0f + 0.07025;	//use these conversions
	dutyCycleAngle = angle * 0.0125f/100.0f + 0.079;

	updatePWM_dutyCycle(FTM_CHANNEL_DC_MOTOR, dutyCycleSpeed);
	FTM_SetSoftwareTrigger(FTM_MOTOR, true);

	updatePWM_dutyCycle(FTM_CHANNEL_SERVO, dutyCycleAngle);
	FTM_SetSoftwareTrigger(FTM_SERVO, true);
    }

}
