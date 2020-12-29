#include "stm32f10x.h"                  // Device header
#include "gp_drive.h"

volatile _Bool led_state=0;

int main(void)
{
		init_GP(PC,13,OUT50,O_GP_PP);	//reverse logic
		write_GP(PC,13,HIGH);	//turn off
		
	RCC->APB1ENR = RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;	//backup registers on
	PWR->CR |=PWR_CR_DBP;	//odblokuje zapis bo domyslnie zawsze zapis jest zablokowany do bkp regów!
	
	RCC->CSR |= RCC_CSR_LSION;
	while((RCC->CSR & RCC_CSR_LSIRDY)==0);		//wait for lsi to enable
	RCC->BDCR |= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSI;		//ebable rtc and select lsi for its clock src nie wiem czy lsi nie wylaczam przy standby moze hsE /128 bedzie lepsze bo i tak dziala 
	
	RTC->CRL &= ~RTC_CRL_RSF;	//zero the flag synchro
	while((RTC->CRL & RTC_CRL_RSF) ==0);	//wait for synchro between rtc clock and cpu main clock
	
	//takim blokiem sie zapisuje cokolwiek do rtc rejestrow ( procedura opisana w RM tak:
	while((RTC->CRL & RTC_CRL_RTOFF)==0);// wait for saving to rtc to finish
	RTC->CRL |= RTC_CRL_CNF;	//enter the config mode
	
	RTC->CRH = RTC_CRH_ALRIE;
	RTC->PRLL =40000-1;	// co ok 1 sekunde bedzie zliczal bo LSi ma fc=40kHz
	RTC->PRLH=0; //kontynuacja ustaieenia prescalera
	RTC->CNTL=0;	//ilosc zliczen zeruje
	RTC->CNTH=0;	//ilosc zliczen zeruje bo to reg 2^32 jest zliczen
	RTC->ALRL=10-1;	//10 zliczen wiec co 10 sek alarm
	RTC->ALRH=0;	//32 bity znowu
		
	RTC->CRL &= ~RTC_CRL_CNF;		//exit the config mode
	while((RTC->CRL & RTC_CRL_RTOFF)==0);// wait for saving to rtc to finish
	
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);
	///koniec operacji zapsiu do rtc
	while(1)
	{
		
	}
}


void RTC_IRQHandler(void){
	
	if((RTC->CRL & RTC_CRL_ALRF)){	//jak wywolane alarmem rtc przerwania
		while((RTC->CRL & RTC_CRL_RTOFF)==0);// wait for saving to rtc to finish
		RTC->CRL |= RTC_CRL_CNF;	//enter the config mode
		
		RTC->CRL &= ~RTC_CRL_ALRF;	//clar alarm flag
		RTC->CNTL=0;	//ilosc zliczen zeruje
		RTC->CNTH=0;	//ilosc zliczen zeruje bo to reg 2^32 jest zliczen
			
		RTC->CRL &= ~RTC_CRL_CNF;		//exit the config mode
		while((RTC->CRL & RTC_CRL_RTOFF)==0);// wait for saving to rtc to finish
		
		write_GP(PC,13,led_state);
		led_state=!led_state;
	}

}
