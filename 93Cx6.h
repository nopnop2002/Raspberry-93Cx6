/*
  93Cx6.h - Library for the Three-wire Serial EEPROM chip
*/
#ifndef _93CX6_H_
#define _93CX6_H_

#define EEPROM_MODE_UNKNOWN	0
#define EEPROM_MODE_8BIT	1
#define EEPROM_MODE_16BIT	2

#define	LOW			0
#define	HIGH			1

struct eeprom
{
	int _pCS;
	int _pSK;
	int _pDI;
	int _pDO;
	bool _ew;
	int _org;		// eeprom Organization(8bit/16bit)
	int _model;		// eeprom model(46/56/66/76/86)
	int _bytes;		// eeprom memory size(byte)
	int _addr;		// number of address bit
	uint16_t _mask;	// address mask
};


int eeprom_open(int model, int org, int pCS, int pSK, int pDI, int pDO, struct eeprom *dev);
void eeprom_ew_disable(struct eeprom *dev);
void eeprom_ew_enable(struct eeprom *dev);
bool eeprom_is_ew_enabled(struct eeprom *dev);
void eeprom_erase_all(struct eeprom *dev);
void eeprom_erase(struct eeprom *dev, uint16_t addr);
void eeprom_write_all(struct eeprom *dev, uint16_t value);
void eeprom_write(struct eeprom *dev, uint16_t addr, uint16_t value);
uint16_t eeprom_read(struct eeprom *dev, uint16_t addr);
static void send_bits(struct eeprom *dev, uint16_t value, int len);
static void wait_ready(struct eeprom *dev);
static int getBytesByModel(int bit, int model);
static int getAddrByModel(int bit, int model);
static uint16_t getMaskByModel(int bit, int model);

#endif
