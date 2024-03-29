#include "SPI.h"

static uint32_t timeout_ms = 1000;

void SPI1_Config(bool flagM){

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED5 | GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7;
	/*SET AF5 FOR PA5,6,7*/
	GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28);    
	
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; /*enable SPI clock*/
	SPI1->CR1 |= SPI_CR1_SPE; /*SPI PERIPHERAL ENABLE*/
	
	/*SELECT POLARITY AND PHASE*/
	SPI1->CR1 |= SPI_CR1_CPHA; 
	SPI1->CR1 &= ~SPI_CR1_CPOL; /*CLEAR SPI clock polarity */
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST; /*clears lsb n sends MSB FIRST*/
	SPI1->CR1 &= ~SPI_CR1_DFF; /*clears data frame format; 8 BIT DATA STREAM*/
	
	if(flagM){
		SPI1->CR1 |= 7 << SPI_CR1_BR_Pos; /*SET BAUD RATE*/
		
		/*enable software slave management & internal slave selection*/
		SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
		SPI1->CR1 &= ~SPI_CR1_RXONLY; /*disables receive-only mode*/
		
		SPI1->CR1 |= SPI_CR1_MSTR; /*SELECT MASTER MODE*/
		/*Baud Rate = Fpclk / 16 = 90mhz / 16 = 5.625*/
	}
	else{
		SPI1->CR1 &= ~SPI_CR1_RXONLY;
		SPI1->CR1 &= ~SPI_CR1_MSTR; /*SELECT SLAVE MODE*/
		SPI1->CR2 |= SPI_CR2_RXNEIE;
		NVIC_EnableIRQ(SPI1_IRQn);
		NVIC_DisableIRQ(USART2_IRQn);
	}	
}


void SPI1_Send(char *data){
	uint8_t temp;
	
	/*SEND ALL DATA BITS*/
	for(int i = 0;i < (int)strlen(data);i++){
		while(!(SPI1->SR & SPI_SR_TXE));
		while(SPI1->SR & SPI_SR_BSY);
		SPI1->DR = data[i];
	}
	
	/*WAIT FOR LAST BIT TO TRANSFER*/
	while(!(SPI1->SR & SPI_SR_TXE));
	/*WAIT UNITL SPI NOT BUSY*/
	while(SPI1->SR & SPI_SR_BSY);
	
	/*to clear pending flags after data reception*/
	temp = (uint8_t)SPI1->DR;
	temp = (uint8_t)SPI1->SR;
	sendString("Data Sent\n");
}

char* SPI1_Receive(void){
	char arr[100], ch = 0;
	int i = 0;

	while(ch != '!'){
		TIM3->CNT = 0;
		
		while(!(SPI1->SR & SPI_SR_RXNE)){
			if(TIM3->CNT > timeout_ms){
				return "";
			}
		}
		ch = (uint8_t)SPI1->DR;
		if(ch != '!'){
			arr[i++] = ch;
		}
		else break;
	}
	
	arr[i] = '\0';
	
	return arr;
}
























