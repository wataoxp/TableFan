/*
 * dma.h
 *
 *  Created on: Jun 8, 2025
 *      Author: wataoxp
 */

#ifndef DMA_H_
#define DMA_H_

#include "periph.h"

typedef struct{
	uint32_t RequestID;
	uint32_t Direction;
	uint32_t TransferMode;
	uint32_t PeriphInc;
	uint32_t MemoryInc;
	uint32_t PeriphSize;
	uint32_t MemorySize;
	uint32_t Priority;
}DMA_InitTypdef;

class DMA{
private:
	DMA_TypeDef *DMAx;
	void SetISR(uint32_t Channel);
public:
	DMA(DMA_TypeDef *DMAport);
	void Config(DMA_InitTypdef *pConfig,uint32_t Channel);
	uint32_t AddressSet(uint32_t Channel,uint32_t *MemoryAddress,uint32_t *PeriphAddress);
	uint32_t AddressSetM2M(uint32_t Channel,uint8_t *SrcAddress,uint8_t *DstAddress);
	void SPItoDMA(SPI_TypeDef *SPIx,uint8_t *MemoryAddress);
	void StartDMA(uint32_t Channel,uint32_t length);
	uint32_t StopDMA(uint32_t Channel);
	uint32_t StopDMAisChannel1(void);
	uint32_t StopDMAisChannel2(void);

};



#endif /* DMA_H_ */
