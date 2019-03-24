/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "Led.h"
#include "Button.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


void delay() {
	volatile uint32_t i;
	for (i = 0; i != 0x50000; i++);
}


void usartInit() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	// Tx
	GPIO_InitTypeDef initStruct;
	initStruct.GPIO_Pin = GPIO_Pin_9;
	initStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	initStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &initStruct);

	// Rx
	initStruct.GPIO_Pin = GPIO_Pin_10;
	initStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	initStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &initStruct);

	USART_InitTypeDef usartInitStruct;
	usartInitStruct.USART_BaudRate = 115200;
	usartInitStruct.USART_WordLength = USART_WordLength_8b;
	usartInitStruct.USART_StopBits = USART_StopBits_1;
	usartInitStruct.USART_Parity = USART_Parity_No;
	usartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &usartInitStruct);

	USART_Cmd(USART1, ENABLE);
}


void send(char* charBuffer, unsigned int count) {
	while (count--) {
		USART_SendData(USART1, *charBuffer++);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}
	}
}


char buffer[30] = {'\0'};

int main(int argc, char* argv[]) {
	usartInit();

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	Led rows[] = {Led(GPIOA, GPIO_Pin_3),
				  Led(GPIOA, GPIO_Pin_2),
				  Led(GPIOA, GPIO_Pin_1),
				  Led(GPIOA, GPIO_Pin_0)};

	Button cols[] = {Button(GPIOB, GPIO_Pin_14),
					 Button(GPIOB, GPIO_Pin_13),
					 Button(GPIOB, GPIO_Pin_15)};

	int i;
	int currentCol = 0;
	int currentRow = 0;
	char currentButton = 0;

	while (1) {
		for (i = 0; i < 4; i++)
			rows[i].on();

		currentCol = -1;
		for (i = 0; i < 3; i++)
			if (cols[i].read())
				currentCol = i;

		if (currentCol >= 0) {
			for (i = 0; i < 4; i++)
				rows[i].off();

			for (i = 0; i < 4; i++) {
				rows[i].on();
				if (cols[currentCol].read()) {
					currentRow = i;
					break;
				}
			}

			currentButton = currentRow * 3 + (currentCol + 1);

			if (currentButton == 10)
				currentButton = '*';
			else if (currentButton == 11)
				currentButton = '0';
			else if (currentButton == 12)
				currentButton = '#';
			else
				currentButton += 48;

			sprintf(buffer, "%c\n", currentButton);
			send(buffer, sizeof(buffer));
		}

		delay();
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
