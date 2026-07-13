/*
 * spi.h
 *
 *  Created on: Feb 21, 2025
 *      Author: wataoxp
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#include "periph.h"

typedef struct{
	/* CR1 */
	uint32_t TransferDirection;		//3線or4線(BIDIMODE)3線の場合は送信or受信専用の設定(BIDIOE)
	uint32_t Mode;					//MSTR
	uint32_t ClockPolarity;			//CPOL アイドル時のSCK極性
	uint32_t ClockPhase;			//CPHA 0なら最初のエッジ、1なら2番目のエッジで転送
	uint32_t NSS;					//SSMおよびSSOE

	uint32_t BaudRate;				//BR SCK分周比
	uint32_t BitOrder;				//LSBFIRST 0ならMSB、1ならLSB

	uint32_t SSI;					//内部NSS値 Modeでまとめて設定。マルチマスタでないなら常にセットして良い

	/* CR2 */
	uint32_t DataWidth;				//DS 転送データビット幅
	uint32_t SSOE;					//SSOE NSSメンバでまとめて設定する

}SPI_InitTypedef;

/*** NSSのマクロと挙動 ***
 *
 *  LL_SPI_NSS_SOFT SSMをセットし、NSSはソフト管理とする。SSIはLL_SPI_MODE_MASTERでセット
 *  LL_SPI_NSS_HARD_INPUT NSS、SSOE共に0、マルチマスタ用。SSIもクリアすること
 *  LL_SPI_NSS_HARD_OUTPUT NSSはMODIFYでクリア。SSOEをセット
 *
 *  マスター、スレーブともにシングル→NSS=0、SSOE=1
 *  マルチスレーブ→NSS=1、SSOEは不問
 *  マルチマスタ、またはスレーブ→NSS=0、SSOE=0
 *********************/


class SPI{
private:
	SPI_TypeDef *SPIx;
	GPIO_TypeDef *NSSport;
	uint32_t NSSpin;
public:
	SPI(SPI_TypeDef *SPIPORT);

	void Config(SPI_InitTypedef *pConfig);
	void ConfigNSS(GPIO_TypeDef *GPIOx,uint32_t Pin,uint32_t mode);

	/* NSSがハード管理なら逐次管理 */
	void Enable(void);
	void Disable(void);

	uint32_t MasterTransmit(uint8_t *data,uint16_t length);
	void Receive(uint8_t *RXbuf,uint16_t length);

	/* NSS SoftControl */
	void ChipSelect(void);
	void ChipDeSelect(void);

	/* Arduino SPI */
	uint8_t Transfer(uint8_t data);
	void ClearFIFO(void);
	void begin(void);
	void end(void);

};

inline void SPI::ConfigNSS(GPIO_TypeDef *GPIOx,uint32_t Pin,uint32_t mode)
{
	NSSport = GPIOx;
	NSSpin = Pin;
	LL_SPI_SetNSSMode(SPIx, mode);
}

inline void SPI::Enable(void)
{
	LL_SPI_Enable(SPIx);
}

inline void SPI::Disable(void)
{
	LL_SPI_Disable(SPIx);
}

inline void SPI::ChipSelect(void)
{
	GPIO_CLEAR(NSSport,NSSpin);
}
inline void SPI::ChipDeSelect(void)
{
	GPIO_WRITE(NSSport,NSSpin);
}

inline void SPI::begin(void)
{
	LL_SPI_Enable(SPIx);
	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);
}
inline void SPI::end(void)
{
	while(LL_SPI_IsActiveFlag_BSY(SPIx) != 0);
	while(LL_SPI_GetTxFIFOLevel(SPIx) != 0);
	LL_SPI_Disable(SPIx);
}



#endif /* INC_SPI_H_ */
