#include <stdlib.h>
#include "usart.h"
#include "rcc.h"
#include "pcbuffer.h"

PC_Buffer *tx_buf[3], *rx_buf[3];

inline PC_Buffer *get_tx(USART_TypeDef* usart) {
	switch ((uint32_t) usart) {
	case USART1_BASE:  return tx_buf[0];
	case USART2_BASE:  return tx_buf[1];
	case LPUART1_BASE: return tx_buf[2];
	}
	return NULL;
}

inline PC_Buffer *get_rx(USART_TypeDef* usart) {
	switch ((uint32_t) usart) {
	case USART1_BASE:  return rx_buf[0];
	case USART2_BASE:  return rx_buf[1];
	case LPUART1_BASE: return rx_buf[2];
	}
	return NULL;
}

int _getc(USART_TypeDef* usart, bool block, char *c) {

	PC_Buffer *rx = get_rx(usart);

	/* check if a character can be retrieved */
	if (pc_buffer_empty(rx)) {
		if (!block) return 1;
		while (pc_buffer_empty(rx)) {;}
	}

	/* safely write the retrieved character */
	__disable_irq();
	pc_buffer_remove(rx, c);
	__enable_irq();

	return 0;
}

int _putc(USART_TypeDef* usart, bool block, char data) {

	PC_Buffer *tx = get_tx(usart);

	/* check if a character can be added */
	if (pc_buffer_full(tx)) {
		if (!block) return 1;
		while (pc_buffer_full(tx)) {;}
	}

	/* safely add the desired character */
	__disable_irq();
	pc_buffer_add(tx, data);
	__enable_irq();

	/* set TX-empty interrupt enable flag */
	usart->CR1 |= USART_CR1_TXEIE;

	return 0;
}

static IRQn_Type uart_get_irq_num(USART_TypeDef* usart) {
	switch((uint32_t) usart) {
		case USART1_BASE:	return USART1_IRQn;
		case USART2_BASE:	return USART2_IRQn;
		case LPUART1_BASE:	return LPUART1_IRQn;
		/* don't tamper with a random IRQ */
		default:			return USART1_IRQn;
	}
}

static int usart_bufferInit(USART_TypeDef* usart) {

	switch ((uint32_t) usart) {

	case USART1_BASE:
		tx_buf[0] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
		rx_buf[0] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
		if (!tx_buf[0] || !rx_buf[0])
			return -1;
		if (!pc_buffer_init(tx_buf[0], USART_BUF))
			return -1;
		if (!pc_buffer_init(rx_buf[0], USART_BUF))
			return -1;
		break;

	case USART2_BASE:
		tx_buf[1] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
		rx_buf[1] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
		if (!tx_buf[1] || !rx_buf[1])
			return -1;
		if (!pc_buffer_init(tx_buf[1], USART_BUF))
			return -1;
		if (!pc_buffer_init(rx_buf[1], USART_BUF))
			return -1;
		break;

	case LPUART1_BASE:
		tx_buf[2] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
		rx_buf[2] = (PC_Buffer *) malloc(sizeof(PC_Buffer));
		if (!tx_buf[2] || !rx_buf[2])
			return -1;
		if (!pc_buffer_init(tx_buf[2], USART_BUF))
			return -1;
		if (!pc_buffer_init(rx_buf[2], USART_BUF))
			return -1;
		break;

	default: return -1;	

	}

	return 0;
}

static int usart_setClock(USART_TypeDef* usart, bool state) {
	
	__IO uint32_t *reg;
	uint8_t bit;
	
	switch ((uint32_t) usart) {

	case USART1_BASE:
		reg = &RCC->APB2ENR;
		bit = RCC_APB2ENR_USART1EN_Pos;
		break;

	case USART2_BASE:
		reg = &RCC->APB1ENR1;
		bit = RCC_APB1ENR1_USART2EN_Pos;
		break;

	case LPUART1_BASE:
		reg = &RCC->APB1ENR2;
		bit = RCC_APB1ENR2_LPUART1EN_Pos;
		break;

	default: return -1;

	}
	
	if (state) *reg |= 0x1 << bit;
	else *reg &= ~(0x1 << bit);
	
	return 0;
}

static int usart_setClockSource(USART_TypeDef* usart, usart_clk_src_t src) {
	
	uint8_t pin_num = 0;
	
	switch ((uint32_t) usart) {
		case USART1_BASE:	pin_num =  0; break;
		case USART2_BASE:	pin_num =  2; break;
	 	case LPUART1_BASE:	pin_num = 10; break;
		default: return -1;
	}
	
	RCC->CCIPR = ~(0x3 << pin_num) | (src << pin_num);
	
	return 0;
}

int usart_config(
	USART_TypeDef* usart, usart_clk_src_t src, uint32_t control[3],
	uint32_t baud, bool ie
) {
	
	uint32_t usartDiv, fck = 0, remainder;
	
	/* turn this USART off if it's on */
	if (usart->CR1 & USART_CR1_UE) {
		usart->CR1 &= ~USART_CR1_UE;
		while (usart->CR1 & USART_CR1_UE) {;}
	}
	
	if (usart_setClockSource(usart, src) || usart_bufferInit(usart))
		return -1;
	
	usart_setClock(usart, true);
	
	usart->ICR |= 0x121BDF;	/* clear any pending interrupts */
	
	if (control) {
		/* don't set TE, RE, UE yet */
		usart->CR1 = control[0] & ~(USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
		usart->CR2 = control[1];
		usart->CR3 = control[2];
	}
	
	if (ie) {
		NVIC_SetPriority(uart_get_irq_num(usart), USART_INT_PRIO);
		NVIC_EnableIRQ(uart_get_irq_num(usart));
	}
	
	/* setup baud, fck / USARTDIV */
	switch (src) {
		case APBX:		
			fck = rcc_get_APB1();
			if (usart == USART1)
				fck = rcc_get_APB2();
			break;
		case SYSCLK:	
			fck = SystemCoreClock;
			break;
		case HSI_SRC:	/* check if enabled */
			if (!(RCC->CR & RCC_CR_HSION)) return -1;
			fck = HSI_VALUE;
			break;
		case LSE_SRC:	/* check if enabled */
			if (!(RCC->BDCR & RCC_BDCR_LSEON)) return -1;
			fck = LSE_VALUE;
			break;
	}

	if (!fck)
		return -1;

	if (usart->CR1 & USART_CR1_OVER8)
		fck *= 2;
	
	remainder = fck % baud;
	usartDiv = fck / baud;
	
	if (remainder > baud / 2) usartDiv++;
	
	usart->BRR = usartDiv;

	/* set TE, RE, UE */
	usart->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE);
	
	return 0;
}

inline void USART_Handler(
	USART_TypeDef *usart, PC_Buffer *tx, PC_Buffer *rx,
	char *prev, char *prev2
) {

	char curr;

	/* character received */
	if (usart->ISR & USART_ISR_RXNE) {
		curr = usart->RDR;
		
		/* backspace */
		if (curr == 0x08 || curr == 0x7F) {
            if (!pc_buffer_empty(rx)) {
				rx->produce_count--;

				/* delete the character in console */
				if (!pc_buffer_full(tx)) pc_buffer_add(tx, 0x08);
				if (!pc_buffer_full(tx)) pc_buffer_add(tx, ' ');
				if (!pc_buffer_full(tx)) pc_buffer_add(tx, 0x08);
				usart->CR1 |= USART_CR1_TXEIE;
			}
		}
		
		/* otherwise add the character, don't allow arrow keys or other escaped characters */
        else if ((*prev != 0x5B && *prev2 != 0x1B) && curr != 0x1B && curr != 0x5B) {
            if (NEWLINE_GUARD(curr, *prev)) rx->message_available++;
            if (!pc_buffer_full(rx)) pc_buffer_add(rx, curr);
			if (!pc_buffer_full(tx)) {
				pc_buffer_add(tx, curr);
				if (curr == '\r') pc_buffer_add(tx, '\n');
				usart->CR1 |= USART_CR1_TXEIE;
			}
        }

		*prev2 = *prev;
		*prev = curr;
	}
	
	/* character ready to be sent */
	if (usart->ISR & USART_ISR_TXE) {
		if (!pc_buffer_empty(tx))
			pc_buffer_remove(tx, (char *) &usart->TDR);
		else
			usart->CR1 &= ~USART_CR1_TXEIE;
	}
}

void USART1_IRQHandler(void) {
	static char prev1 = '\0', prev2 = '\0';
	USART_Handler(USART1, tx_buf[0], rx_buf[0], &prev1, &prev2);
}

void USART2_IRQHandler(void) {
	static char prev1 = '\0', prev2 = '\0';
	USART_Handler(USART2, tx_buf[1], rx_buf[1], &prev1, &prev2);
}

void LPUART1_IRQHandler(void) {
	static char prev1 = '\0', prev2 = '\0';
	USART_Handler(LPUART1, tx_buf[2], rx_buf[2], &prev1, &prev2);
}
