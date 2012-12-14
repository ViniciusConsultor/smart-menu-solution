/*
 * Main.c
 *
 * Created: 11/18/2012 2:11:27 PM
 *  Author: Minh Thanh
 */ 
#define F_CPU 8000000UL

#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "SMR.h"
#include "lcd.h"
#include "TimerEvent.h"
#include "rf.h"

//send button
#define BUTTONSEND_DDR DDRC
#define BUTTONSEND_PORT PORTC
#define BUTTONSEND_PIN PINC
#define BUTTONSEND PORTC0
//output led
#define LEDOUT_DDR DDRC
#define LEDOUT_PORT PORTC
#define LEDOUT PORTC1
#define LEDOUTPAUSE 300

struct SMR* l;
char bufferin[NRF24L01_PAYLOAD];
char bufferout[NRF24L01_PAYLOAD];

void sMRIfaceLCD_writeString(const sc_string chr) {
	LCDWriteString(chr);
}
void sMRIfaceLCD_writeStringXY(const sc_string chr, const sc_integer x, const sc_integer y) {
	LCDWriteStringXY(x,y,chr);
}
void sMRIfaceLCD_writeNumberXY(const sc_integer num, const sc_integer x, const sc_integer y, const sc_integer l) {
	LCDWriteIntXY(x,y,num,l);
}
void sMRIfaceLCD_clear() {
	LCDClear();
}
void sMRIfaceLCD_init() {
	LCDInit(0);
}

sc_string sMRIfaceRF_getData() {
	uint8_t pipe = 0;
	if (nrf24l01_readready(&pipe)) {
		//clear buffer
		for(uint8_t i=0; i<sizeof(bufferin); i++) bufferin[i] = 0;
		
		//read buffer
		nrf24l01_read(pipe,bufferin);
		return bufferin;
	} else {
		return "";
	}
}

sc_boolean sMRIfaceRF_sendMsg(const sc_string msg) {
	uint8_t writeret = nrf24l01_write(msg);
	if(writeret == 1) {
		return true;
	} else {
		return false;
	}
}

sc_boolean sMRIfaceRF_sendData(const sc_integer cmd, const sc_integer id, const sc_integer dish_id, const sc_integer amount) {
	
	unsigned char num;
	unsigned char mod;
	//clear buffer
	for(uint8_t i=0; i<sizeof(bufferout); i++) bufferout[i] = 0;
	
	//Lenh cho data: 1-goi mon, 2-huy mon, 3-thanh toan, 4-goi boi ban
	bufferout[0]=cmd+'0'; //Doi cmd tu so sang chu so
	
	//Chuyen doi ma ban an hoac nha bep
	num=id;
	for (int i=1;i>=0;i--) {
		mod=num % 10;
		num/=10;
		bufferout[i+1]=mod+'0';
	}
	
	//Chuyen doi ma mon an
	num=dish_id;
	for (int i=2;i>=0;i--) {
		mod=num % 10;
		num/=10;
		bufferout[3+i]=mod+'0';
	}
	
	//Chuyen doi so luong mon an
	num=amount;
	for (int i=1;i>=0;i--) {
		mod=num % 10;
		num/=10;
		bufferout[6+i]=mod+'0';
	}
	
	uint8_t writeret = nrf24l01_write(bufferout);
	if(writeret == 1) {
		return true;
	} else {
		return false;
	}
}

void sMR_setTimer(const sc_eventid evid, const sc_integer time_ms, const sc_boolean periodic){
	TimerSet(evid,time_ms);
}
void sMR_unsetTimer(const sc_eventid evid) {
	TimerUnSet(evid);
}

void sMRIfaceRF_init() {
	nrf24l01_init();
}

int main(void)
{
	uint8_t i;
	DDRC=0x0F;
	PORTC=0x0F;
	l=malloc(sizeof(SMR*)) ;
	TimerInit();
	sMR_init(l);
	sMR_enter(l);
	
	//setup buffer
	for(i=0; i<sizeof(bufferout); i++)
	bufferout[i] = i+'a';
	for(i=0; i<sizeof(bufferin); i++)
	bufferin[i] = 0;
	
	uint8_t down=0;
	
	while(1)
	{
		sMR_runCycle(l);
	}
}

ISR (TIMER0_OVF_vect) {
	TCNT0=131;
	TimerCheck(l);
}