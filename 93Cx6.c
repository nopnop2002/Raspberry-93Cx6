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
int eeprom_open(int model, int org, int pCS, int pSK, int pDI, int pDO, struct eeprom * dev)
{
	pinMode(pCS, OUTPUT);	// ChipSelect
	pinMode(pSK, OUTPUT);	// Clock
	pinMode(pDI, OUTPUT); 	// MOSI
	pinMode(pDO, INPUT);	// MISO
	digitalWrite(dev->_pCS, LOW);
	dev->_pCS = pCS;
	dev->_pSK = pSK;
	dev->_pDI = pDI;
	dev->_pDO = pDO;
	dev->_ew = false;
	dev->_org = org;
	dev->_model = model;
	dev->_bytes = getBytesByModel(dev->_org, dev->_model);
	dev->_addr = getAddrByModel(dev->_org, dev->_model);
	dev->_mask = getMaskByModel(dev->_org, dev->_model);
	if(_DEBUG_) printf("dev->_bytes=%d dev->_addr=%d dev->_mask=%x\n", dev->_bytes, dev->_addr, dev->_mask);
	return dev->_bytes;
};


// Erase/Write Enable
// Required Clock Cycle : 9-13
void eeprom_ew_enable(struct eeprom *dev)
{
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	//uint16_t value = CONTROL<<dev->_addr | EW_ENABLE<<(dev->_addr-2);
	//printf("eeprom_ew_enable value=%04x\n", value); 
	send_bits(dev, CONTROL<<dev->_addr | EW_ENABLE<<(dev->_addr-2), dev->_addr + 2);
	digitalWrite(dev->_pCS, LOW);
	dev->_ew = true;
};

// Erase/Write Disable
// Required Clock Cycle : 9-13
void eeprom_ew_disable(struct eeprom *dev)
{
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	send_bits(dev, CONTROL<<dev->_addr | EW_DISABLE<<(dev->_addr-2), dev->_addr + 2);
	digitalWrite(dev->_pCS, LOW);
	dev->_ew = false;
}

// Check Erase/Write Enable
bool eeprom_is_ew_enabled(struct eeprom *dev)
{
	return dev->_ew;
}

// Erase All Memory
// Required Clock Cycle : 9-13
void eeprom_erase_all(struct eeprom *dev)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	//uint16_t value = CONTROL<<dev->_addr | ERASE_ALL<<(dev->_addr-2);
	//printf("eeprom_erase_all value=%04x\n", value); 
	send_bits(dev, CONTROL<<dev->_addr | ERASE_ALL<<(dev->_addr-2), dev->_addr + 2);
	digitalWrite(dev->_pCS, LOW);
	wait_ready(dev);
}

// Erase Byte or Word
// Required Clock Cycle : 9-13
void eeprom_erase(struct eeprom *dev, uint16_t addr)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, ERASE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
	} else {
		send_bits(dev, ERASE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
	}
	
	digitalWrite(dev->_pCS, LOW);
	wait_ready(dev);
}


// Write All Memory with same Data
// Required Clock Cycle : 25-29
void eeprom_write_all(struct eeprom *dev, uint16_t value)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	send_bits(dev, CONTROL<<dev->_addr | WRITE_ALL<<(dev->_addr-2), dev->_addr + 2);
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, 0xFFFF & value, 16);
	} else {
		send_bits(dev, 0xFF & value, 8);
	}
	digitalWrite(dev->_pCS, LOW);
	wait_ready(dev);
}

// Write Data to Memory
// Required Clock Cycle : 25-29
void eeprom_write(struct eeprom *dev, uint16_t addr, uint16_t value)
{
	if(!eeprom_is_ew_enabled(dev)) return;
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, WRITE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		send_bits(dev, 0xFFFF & value, 16);
	} else {
		send_bits(dev, WRITE<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		send_bits(dev, 0xFF & value, 8);
	}
	digitalWrite(dev->_pCS, LOW);
	wait_ready(dev);
}

// Read Data from Memory
uint16_t eeprom_read(struct eeprom *dev, uint16_t addr)
{
	uint16_t val = 0;
	digitalWrite(dev->_pCS, HIGH);
	usleep(DELAY_CS);
	send_bits(dev, HIGH, 1);	// Start bit
	
	int amtBits;
	if(dev->_org == EEPROM_MODE_16BIT) {
		send_bits(dev, READ<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		amtBits = 16;
	} else {
		send_bits(dev, READ<<dev->_addr | (addr & dev->_mask), dev->_addr + 2);
		amtBits = 8;
	}
	// Read bits
	for(int i = amtBits; i>0; i--) {
		digitalWrite(dev->_pSK, HIGH);
		usleep(DELAY_READ);
		int in = digitalRead(dev->_pDO) ? 1 : 0;
		digitalWrite(dev->_pSK, LOW);
		usleep(DELAY_READ);
		val |= (in << (i-1));
	}
	digitalWrite(dev->_pCS, LOW);
	return val;
}


static void send_bits(struct eeprom *dev, uint16_t value, int len)
{
	if(_DEBUG_) printf("send_bits value=0x%04x len=%d\n",value, len);
	for(int i = len-1; i>=0; i--)
	{
		bool toSend = (value & 1<<i);
		// Send bit
		if (toSend) digitalWrite(dev->_pDI, HIGH);
		if (!toSend) digitalWrite(dev->_pDI, LOW);
		usleep(DELAY_WRITE);
		digitalWrite(dev->_pSK, HIGH);
		usleep(DELAY_WRITE);
		digitalWrite(dev->_pSK, LOW);
		usleep(DELAY_WRITE);
	}
}

static void wait_ready(struct eeprom *dev)
{
	//Wait until action is done.
	digitalWrite(dev->_pCS, HIGH);
	while(digitalRead(dev->_pDO) != HIGH) {
		usleep(DELAY_WAIT);
	}
	digitalWrite(dev->_pCS, LOW);
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
