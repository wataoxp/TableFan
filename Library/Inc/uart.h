/*
 * uart.h
 *
 *  Created on: Feb 3, 2026
 *      Author: wataoxp
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "periph.h"

namespace Serial{
	typedef enum{
		Success,
		Failed,
	}UART_Status;


	enum class SerialParameters :uint8_t{
		Fifo_Enable,
		Fifo_Disable,

		AsyncMode,
		SyncMode,
		LinMode,
		IrDaMode,
		HerfDuplexMode,
	};

	typedef struct{
		// CR1
		uint32_t DataWidth;
		uint32_t Parity;
		uint32_t Direction;
		uint32_t OverSampling;
		SerialParameters FifoEN;

		// CR2
		uint32_t StopBits;

		// CR3
		uint32_t HardWareControl;
		uint32_t TxFifoThreshold;
		uint32_t RxFifoThreshold;

		// CR2&CR3
		SerialParameters SelectMode;

		// BRR
		uint32_t PreScalerDiv;
		uint32_t BaudRate;

	}UART_InitTypedef;
}

class UART{
private:
	USART_TypeDef *USARTx;
	void CR1Config(Serial::UART_InitTypedef *init);
	void WaitEnableUART(uint32_t Direction);
public:
	UART(USART_TypeDef *UARTPort);
	void Config(Serial::UART_InitTypedef *init,uint32_t SysClk);
	void TransmitData(uint8_t *buf,uint8_t size);
	uint16_t ReceiveData(void);

	void Enable();
	void Disable();
};

inline void UART::Enable()
{
	LL_USART_Enable(USARTx);
	__NOP();
	__NOP();
}

inline void UART::Disable()
{
	LL_USART_Disable(USARTx);
	__NOP();
	__NOP();
}



#endif /* INC_UART_H_ */
