#include "tm4c123gh6pm.h"
#include "stdint.h"


void SystemInit(){}
	
uint32_t timer=0;
uint32_t adc_result=0;
uint32_t temp=0;
char i;
char led_status;
	
void ADC_init(){
 
	SYSCTL_RCGCADC_R |= (1<<1); //enable clk for adc 1
  
	//SYSCTL_RCGCGPIO_R |= (1<<4);// ENABLE PORTE
	//GPIO_PORTE_DIR_R &= ~(1<<1);
	//GPIO_PORTE_DEN_R &= ~(1<<1); //PE1 IS ANALOG INPUT
	//GPIO_PORTE_AFSEL_R |= (1<<1); //REVIEW
	//GPIO_PORTE_AMSEL_R |= (1<<1); //DISABLE ANALOG ISOLATION CIRCUIT
	
	ADC1_ACTSS_R &= ~(1<<3); //DISABLE SEQUENCE 3 (1 CHANNEL)
	
	ADC1_EMUX_R |= (0XF <<12); //(CONTINUOUS SAMPLING)
		
	
	ADC1_SSCTL3_R |=(1<<1) | (1<<3); //USE TEMPERATURE SENSOR ON BOARD
	
	
	ADC1_ACTSS_R |= (1<<3); //enable sequencer

	

}	
uint32_t read_adc (void){
	
	
	return ADC1_SSFIFO3_R;
	
}

void writeChar(char data)
{
	while( (UART0_FR_R & 0x20) != 0);
	
	UART0_DR_R = data ;
}


void GPIO_PORTF_init()
{
//connect clk
	SYSCTL_RCGCGPIO_R |= (1<<5) ;
	
	//wait until port activated
	while( (SYSCTL_RCGCGPIO_R & (1<<5)) == 0) ;
	
	//unlock and commit portf
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
	GPIO_PORTF_CR_R = 0xFF;
	
	NVIC_EN0_R |= (1<<30);
	
	NVIC_PRI7_R |= (1<<21);
	NVIC_PRI7_R &= ~(0x3<<21);
	
	//pf1 is digital output
	GPIO_PORTF_DIR_R |= (1<<1);
	GPIO_PORTF_DEN_R |= (1<<1);
	
	//pf0 is digital input
	GPIO_PORTF_DIR_R &= ~1;
	GPIO_PORTF_DEN_R |= 1;
	
	//GPIO_PORTF_AFSEL_R |= 1;
	//enable pull up resistance 
	GPIO_PORTF_PUR_R |=1;
	
	//interrupt sense, 0 to be interrupted by edges not levels
	GPIO_PORTF_IS_R &= ~1;
	
	//interrupt one or both edges
	GPIO_PORTF_IBE_R &= ~1;
	
	//falling or rising edge
	GPIO_PORTF_IEV_R &= ~1;
	
	//mask the interrupt
	GPIO_PORTF_IM_R |=1;

}	
	
void GPIOF_Handler()
{
	//to acknowledge an interrupt, write 1 to it
	//GPIO_PORTF_RIS_R |= 1;
	GPIO_PORTF_ICR_R |=1;
	
	GPIO_PORTF_DATA_R ^= (1<<1);
	
	led_status = GPIO_PORTF_DATA_R & (1<<1);
	
	{
    char word[] = {'P', 'R', 'E', 'S', 'S', 'E', 'D'};
		
		for(i=0; i<7; i++)
		writeChar(word[i]);
		
		writeChar(' ');
	}
	
	
	if(led_status>0){
		
		char word[] = {'O', 'N'};
		
		for(i=0; i<2; i++)
		writeChar(word[i]);
	}
	
	else {
		char word[] = {'O', 'F', 'F'};
		
		for(i=0; i<3; i++)
		writeChar(word[i]);
		
		
	}
	
}

void SysTick_Handler()
{
	timer++;
	NVIC_ST_CURRENT_R = 0x000000;
	
	if(timer==3)
	{
		timer=0;
		//handle
		adc_result = read_adc();
		temp = 147.5 - ((75 * (3.3 - 0) * adc_result) / 4096);
		
		//send to cmd
		writeChar(temp);
	}
	
	
}

void Systick_init(void)
{
	NVIC_ST_CTRL_R = 0; 
	NVIC_ST_RELOAD_R = (16000000-1);  
	NVIC_ST_CURRENT_R = 0x000000;
	NVIC_ST_CTRL_R=0x07;
	
}





void uart_init()
{
	SYSCTL_RCGCGPIO_R |= 1;
	SYSCTL_RCGCUART_R |= 1;
	UART0_CTL_R &= ~1;
	UART0_IBRD_R = 104; //16mega/16*9600
	UART0_FBRD_R = 11;
	UART0_LCRH_R = 0X70;
	UART0_CTL_R = 0X301;
	
	GPIO_PORTA_AFSEL_R |= 0X3;
	GPIO_PORTA_PCTL_R |= (3<<17);
	GPIO_PORTA_DEN_R |= 0x3;
	
}

int main()
{
  __enable_irq();
	GPIO_PORTF_init();
	Systick_init();
	uart_init();
	ADC_init();
	
	
	while(1)
	{
		
		
	}
	
	
	
	return 0;
}
