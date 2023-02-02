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

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    setupUART();

    /******* Delay *******/
    for(volatile int i = 0U; i < 10000000; i++)
    __asm("NOP");
    PRINTF("%s", txbuff);
    UART_WriteBlocking(TARGET_UART, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
    UART_ReadBlocking(TARGET_UART, &ch, 5);
    speed = (int)ch[0]*10 + (int)ch[1];
    angle = (int)ch[3]*10 + (int)ch[4];


    printf("%d\n", speed);
    printf("%d\n", angle);
    PRINTF("%.5s\r\n", ch);
    }

}
