/*
  93Cx6.c - Library for the Three-wire Serial EEPROM chip
*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <wiringPi.h>
#include "93Cx6.h"

#define DELAY_CS	0
#define DELAY_READ	1
#define DELAY_WRITE	1
#define DELAY_WAIT	1

#define _DEBUG_ 0

enum OP { // Operations
	CONTROL		= 0x00, 
	WRITE 		= 0x01, 
	READ 		= 0x02, 
	ERASE 		= 0x03
};
enum CC { // Control Codes
	EW_DISABLE 	= 0x00, 
	WRITE_ALL 	= 0x01, 
	ERASE_ALL 	= 0x02, 
	EW_ENABLE 	= 0x03
};

// Open Memory Device
// model:EEPROM model(46/56/66/76/86)
// org:Organization Select(1=8Bit/2=16Bit)
int eeprom_open(int model, int org, int pCS, int pSK, int pDI, int pDO, struct eeprom * e)
{
	pinMode(pCS, OUTPUT);	// ChipSelect
	pinMode(pSK, OUTPUT);	// Clock
	pinMode(pDI, OUTPUT); 	// MOSI
	pinMode(pDO, INPUT);	// MISO
	digitalWrite(e->_pCS, LOW);
	e->_pCS = pCS;
	e->_pSK = pSK;
	e->_pDI = pDI;
	e->_pDO = pDO;
	e->_ew = false;
	e->_org = org;
	e->_model = model;
	e->_bytes = getBytesByModel(e->_org, e->_model);
	e->_addr = getAddrByModel(e->_org, e->_model);
	e->_mask = getMaskByModel(e->_org, e->_model);
	if(_DEBUG_) printf("e->_bytes=%d e->_addr=%d e->_mask=%x\n", e->_bytes, e->_addr, e->_mask);
	return e->_bytes;
};


// Erase/Write Enable
// Required Clock Cycle : 9-13
void eeprom_ew_enable(struct eeprom *e)
{
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);
	//uint16_t value = CONTROL<<e->_addr | EW_ENABLE<<(e->_addr-2);
	//printf("eeprom_ew_enable value=%04x\n", value); 
	send_bits(e, CONTROL<<e->_addr | EW_ENABLE<<(e->_addr-2), e->_addr + 2);
	digitalWrite(e->_pCS, LOW);
	e->_ew = true;
};

// Erase/Write Disable
// Required Clock Cycle : 9-13
void eeprom_ew_disable(struct eeprom *e)
{
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);
	send_bits(e, CONTROL<<e->_addr | EW_DISABLE<<(e->_addr-2), e->_addr + 2);
	digitalWrite(e->_pCS, LOW);
	e->_ew = false;
}

// Check Erase/Write Enable
bool eeprom_is_ew_enabled(struct eeprom *e)
{
	return e->_ew;
}

// Erase All Memory
// Required Clock Cycle : 9-13
void eeprom_erase_all(struct eeprom *e)
{
	if(!eeprom_is_ew_enabled(e)) return;
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);
	//uint16_t value = CONTROL<<e->_addr | ERASE_ALL<<(e->_addr-2);
	//printf("eeprom_erase_all value=%04x\n", value); 
	send_bits(e, CONTROL<<e->_addr | ERASE_ALL<<(e->_addr-2), e->_addr + 2);
	digitalWrite(e->_pCS, LOW);
	wait_ready(e);
}

// Erase Byte or Word
// Required Clock Cycle : 9-13
void eeprom_erase(struct eeprom *e, uint16_t addr)
{
	if(!eeprom_is_ew_enabled(e)) return;
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);
	if(e->_org == EEPROM_MODE_16BIT) {
		send_bits(e, ERASE<<e->_addr | (addr & e->_mask), e->_addr + 2);
	} else {
		send_bits(e, ERASE<<e->_addr | (addr & e->_mask), e->_addr + 2);
	}
	
	digitalWrite(e->_pCS, LOW);
	wait_ready(e);
}


// Write All Memory with same Data
// Required Clock Cycle : 25-29
void eeprom_write_all(struct eeprom *e, uint16_t value)
{
	if(!eeprom_is_ew_enabled(e)) return;
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);
	send_bits(e, CONTROL<<e->_addr | WRITE_ALL<<(e->_addr-2), e->_addr + 2);
	if(e->_org == EEPROM_MODE_16BIT) {
		send_bits(e, 0xFFFF & value, 16);
	} else {
		send_bits(e, 0xFF & value, 8);
	}
	digitalWrite(e->_pCS, LOW);
	wait_ready(e);
}

// Write Data to Memory
// Required Clock Cycle : 25-29
void eeprom_write(struct eeprom *e, uint16_t addr, uint16_t value)
{
	if(!eeprom_is_ew_enabled(e)) return;
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);
	if(e->_org == EEPROM_MODE_16BIT) {
		send_bits(e, WRITE<<e->_addr | (addr & e->_mask), e->_addr + 2);
		send_bits(e, 0xFFFF & value, 16);
	} else {
		send_bits(e, WRITE<<e->_addr | (addr & e->_mask), e->_addr + 2);
		send_bits(e, 0xFF & value, 8);
	}
	digitalWrite(e->_pCS, LOW);
	wait_ready(e);
}

// Read Data from Memory
uint16_t eeprom_read(struct eeprom *e, uint16_t addr)
{
	uint16_t val = 0;
	digitalWrite(e->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(e, HIGH, 1);	// Start bit
	
	int amtBits;
	if(e->_org == EEPROM_MODE_16BIT) {
		send_bits(e, READ<<e->_addr | (addr & e->_mask), e->_addr + 2);
		amtBits = 16;
	} else {
		send_bits(e, READ<<e->_addr | (addr & e->_mask), e->_addr + 2);
		amtBits = 8;
	}
	// Read bits
	for(int i = amtBits; i>0; i--) {
		digitalWrite(e->_pSK, HIGH);
		usleep(DELAY_READ);
		int in = digitalRead(e->_pDO) ? 1 : 0;
		digitalWrite(e->_pSK, LOW);
		usleep(DELAY_READ);
		val |= (in << (i-1));
	}
	digitalWrite(e->_pCS, LOW);
	return val;
}


static void send_bits(struct eeprom *e, uint16_t value, int len)
{
	if(_DEBUG_) printf("send_bits value=0x%04x len=%d\n",value, len);
	for(int i = len-1; i>=0; i--)
	{
		bool toSend = (value & 1<<i);
		// Send bit
		if (toSend) digitalWrite(e->_pDI, HIGH);
		if (!toSend) digitalWrite(e->_pDI, LOW);
		usleep(DELAY_WRITE);
		digitalWrite(e->_pSK, HIGH);
		usleep(DELAY_WRITE);
		digitalWrite(e->_pSK, LOW);
		usleep(DELAY_WRITE);
	}
}

static void wait_ready(struct eeprom *e)
{
	//Wait until action is done.
	digitalWrite(e->_pCS, HIGH);
	while(digitalRead(e->_pDO) != HIGH) {
		usleep(DELAY_WAIT);
	}
	digitalWrite(e->_pCS, LOW);
}


static int getBytesByModel(int org, int model)
{
	int byte = 0;
	if (org == EEPROM_MODE_8BIT) byte = 128;
	if (org == EEPROM_MODE_16BIT) byte = 64;
	if (model == 56) byte = byte * 2;	// 256/128
	if (model == 66) byte = byte * 4;	// 512/256
	if (model == 76) byte = byte * 8;	// 1024/256
	if (model == 86) byte = byte * 16;	// 2048/1024
	return byte;
}

static int getAddrByModel(int org, int model)
{
	int addr = 0;
	if (org == EEPROM_MODE_8BIT) addr = 7;
	if (org == EEPROM_MODE_16BIT) addr = 6;
	if (model == 56) addr = addr + 2;	// 9/8
	if (model == 66) addr = addr + 2;	// 9/8
	if (model == 76) addr = addr + 4;	// 11/10
	if (model == 86) addr = addr + 4;	// 11/10
	return addr;
}

static uint16_t getMaskByModel(int org, int model)
{
	uint16_t mask = 0;
	if (org == EEPROM_MODE_8BIT) mask = 0x7f;
	if (org == EEPROM_MODE_16BIT) mask = 0x3f;
	if (model == 56) mask = (mask<<2) + 0x03;	// 0x1ff/0xff
	if (model == 66) mask = (mask<<2) + 0x03;	// 0x1ff/0xff
	if (model == 76) mask = (mask<<4) + 0x0f;	// 0x7ff/0x3ff
	if (model == 86) mask = (mask<<4) + 0x0f;	// 0x7ff/0x3ff
	return mask;
}
