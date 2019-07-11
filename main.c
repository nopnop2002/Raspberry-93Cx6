/*
  I forked from here.
  https://github.com/0xJoey/Arduino_93C46

  93C46  cc -o main main.c 93Cx6.c -lwiringPi -DC46 [-DWORD/-DBYTE]
  93C56  cc -o main main.c 93Cx6.c -lwiringPi -DC56 [-DWORD/-DBYTE]
  93C66  cc -o main main.c 93Cx6.c -lwiringPi -DC66 [-DWORD/-DBYTE]
  93C76  cc -o main main.c 93Cx6.c -lwiringPi -DC76 [-DWORD/-DBYTE]
  93C86  cc -o main main.c 93Cx6.c -lwiringPi -DC86 [-DWORD/-DBYTE]
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wiringPi.h>

#include "93Cx6.h"

#define pCS	10
#define pSK	14
#define pDI	12	// MOSI
#define pDO	13	// MISO

void dump(char * title, int bits, uint16_t *dt, uint16_t n) {
  int clm = 0;
  uint16_t data;
  uint16_t saddr =0;
  uint16_t eaddr =n-1;

  printf("-------------------- [%s] --------------------\n", title);
  uint16_t addr;
  for (addr = saddr; addr <= eaddr; addr++) {
    data = dt[addr];
    if (clm == 0) {
      printf("%05x: ",addr);
    }

    if (bits == 8)  printf("%02x ",data);
    if (bits == 16) printf("%04x ",data);

    clm++;
    if (bits == 8 && clm == 16) {
      printf("\n");
      clm = 0;
    }
    if (bits == 16 && clm == 8) {
      printf("\n");
      clm = 0;
    }
  }
  printf("-------------------- [%s] --------------------\n", title);
}

void org8Mode(struct eeprom * e) {
  uint16_t mem_addr;
  uint16_t data;
  uint16_t rdata[128];
  int i;

  // erase/write enable
  eeprom_ew_enable(e);

  // erase all memory
  eeprom_erase_all(e);

  // write same data
  eeprom_write_all(e, 0x00);

  // read first blcok 8bit mode
  memset(rdata, 0, sizeof(rdata));
  for(i=0;i<128;i++) {
    mem_addr = i;
    rdata[i] = eeprom_read(e, mem_addr);
  }
  dump("8bit:address 0-128", 8, rdata, 128);

  // write first blcok 8bit mode
  for(i=0;i<128;i++) {
    mem_addr = i;
    data = i;
    eeprom_write(e, mem_addr, data);
  }

  // read first blcok 8bit mode
  memset(rdata, 0, sizeof(rdata));
  for(i=0;i<128;i++) {
    mem_addr = i;
    rdata[i] = eeprom_read(e, mem_addr);
  }
  dump("8bit:address 0-128", 8, rdata, 128);

  // write first blcok 8bit mode
  for(i=0;i<128;i++) {
    mem_addr = i;
    data = 128-i;
    eeprom_write(e, mem_addr, data);
  }

  // read first blcok 8bit mode
  memset(rdata, 0, sizeof(rdata));
  for(i=0;i<128;i++) {
    mem_addr = i;
    rdata[i] = eeprom_read(e, mem_addr);
  }
  dump("8bit:address 0-128", 8, rdata, 128);

}

void org16Mode(struct eeprom * e) {
  uint16_t mem_addr;
  uint16_t data;
  uint16_t rdata[128];
  int i;

  // erase/write enable
  eeprom_ew_enable(e);

  // erase all memory
  eeprom_erase_all(e);

  // write same data
  eeprom_write_all(e, 0x00);

  // read first blcok 16bit mode
  memset(rdata, 0, sizeof(rdata));
  for(i=0;i<64;i++) {
    mem_addr = i;
    rdata[i] = eeprom_read(e, mem_addr);
  }
  dump("16bit:address 0-64", 16, rdata, 64);

  // write first blcok 16bit mode
  for(i=0;i<64;i++) {
    mem_addr = i;
    data = i + 0xFF00;
    eeprom_write(e, mem_addr, data);
  }

  // read last blcok 16bit mode
  memset(rdata, 0, sizeof(rdata));
  for(i=0;i<64;i++) {
    mem_addr = i;
    rdata[i] = eeprom_read(e, mem_addr);
  }
  dump("16bit:address 0-64", 16, rdata, 64);

  // write first blcok 16bit mode
  for(i=0;i<64;i++) {
    mem_addr = i;
    data = 0xFFFF - i;
    eeprom_write(e, mem_addr, data);
  }

  // read last blcok 16bit mode
  memset(rdata, 0, sizeof(rdata));
  for(i=0;i<64;i++) {
    mem_addr = i;
    rdata[i] = eeprom_read(e, mem_addr);
  }
  dump("16bit:address 0-64", 16, rdata, 64);

}


int main(int argc, char *argv[])
{
  // set EEPROM memory size
  int eeprom_model = 0;
#ifdef C46
  eeprom_model = 46;
#endif
#ifdef C56
  eeprom_model = 56;
#endif
#ifdef C66
  eeprom_model = 66;
#endif
#ifdef C76
  eeprom_model = 76;
#endif
#ifdef C86
  eeprom_model = 86;
#endif
  //printf("eeprom_model=%d\n",eeprom_model);
  if (eeprom_model == 0) {
    printf("EEPROM model not found\n");
    return 1;
  }

  // set EEPROM organization
  int eeprom_org = EEPROM_MODE_8BIT;
  int bits = 8;
#ifdef BYTE
  eeprom_org = EEPROM_MODE_8BIT;
  bits = 8;
#endif
#ifdef WORD
  eeprom_org = EEPROM_MODE_16BIT;
  bits = 16;
#endif

  // start wiringPi
  if (wiringPiSetup() == -1) {
    printf("wiringPiSetup Error\n");
    return 1;
  }

  // open device
  struct eeprom e;
  int eeprom_bytes = eeprom_open(eeprom_model, eeprom_org, pCS, pSK, pDI, pDO, &e);
  printf("EEPROM chip=93C%02d, %dBit Organization, Total=%dByte\n",eeprom_model, bits, eeprom_bytes);

  if (eeprom_org == EEPROM_MODE_8BIT) org8Mode(&e);
  if (eeprom_org == EEPROM_MODE_16BIT) org16Mode(&e);
}
