# Raspberry-93Cx6

Three-Wire Serial EEPROM Access Library for RaspberryPi.   

I used [this](https://github.com/0xJoey/Arduino_93C46) as a reference.

---

# Interface

It look like SPI.   
But CS is ACTIVE HIGH.   
And data bit is not always 8bit.   

---

# Memory size vs Organization

The 93Cx6 memory is organized either as bytes (x8) or as words (x16).   
If Organization Select (ORG) is left unconnected (or connected to VCC) the x16 organization is selected.   
When Organization Select (ORG) is connected to Ground (VSS) the x8 organization is selected.    

|Device|Number of Bits|Number of 8-bit Bytes|Number of 16-bit Words|
|:---|:---|:---|:---|
|93C46|1024|128|64|
|93C56|2048|256|128|
|93C66|4096|512|256|
|93C76|8192|1024|512|
|93C86|16384|2048|1024|

---

# Build
for 93C46   
cc -o main main.c 93Cx6.c -lwiringPi -DC46 [-DWORD/-DBYTE]

for 93C56   
cc -o main main.c 93Cx6.c -lwiringPi -DC56 [-DWORD/-DBYTE]

for 93C66   
cc -o main main.c 93Cx6.c -lwiringPi -DC66 [-DWORD/-DBYTE]

for 93C76   
cc -o main main.c 93Cx6.c -lwiringPi -DC76 [-DWORD/-DBYTE]

for 93C86   
cc -o main main.c 93Cx6.c -lwiringPi -DC86 [-DWORD/-DBYTE]

sudo ./main


---

# API

```
// Open Memory Device
// model:EEPROM model(46/56/66/76/86)
// org:Organization Select(1=8Bit/2=16Bit)
int eeprom_open(int model, int org, int pCS, int pSK, int pDI, int pDO, struct eeprom*);

// Erase/Write Enable
void eeprom_ew_enable(struct eeprom *e);

// Erase/Write Disable
void eeprom_ew_disable(struct eeprom *e);

// Check Erase/Write Enable
bool eeprom_is_ew_enabled(struct eeprom *e);

// Erase All Memory
void eeprom_erase_all(struct eeprom *e);

// Erase Byte or Word
void eeprom_erase(struct eeprom *e, uint16_t addr)

// Write All Memory with same Data
void eeprom_write_all(struct eeprom *e, uint16_t value)

// Write Data to Memory
void eeprom_write(struct eeprom *e, uint16_t addr, uint16_t value)

// Read Data from Memory
uint16_t eeprom_read(struct eeprom *e, uint16_t addr)
```

---

# Wireing for x8 Organization
![Wire-93cx6-byte](https://user-images.githubusercontent.com/6020549/61050459-00c0f600-a422-11e9-943b-50631bbef387.jpg)

# Wireing for x16 Organization
![Wire-93cx6-word](https://user-images.githubusercontent.com/6020549/61050458-00c0f600-a422-11e9-9720-1d12b4431962.jpg)

**It's insufficient in 3.3V Power supply.**   
**You have to supply 5V.**   
**So you have to shift level about MISO line.**   

---

# 93C46
![93C46-BYTE](https://user-images.githubusercontent.com/6020549/61050500-1a623d80-a422-11e9-82ad-d5a35fd970b2.jpg)   

![93C46-WORD](https://user-images.githubusercontent.com/6020549/61050499-1a623d80-a422-11e9-98d7-4397111388d4.jpg)   

# 93C56
![93C56-BYTE](https://user-images.githubusercontent.com/6020549/61050528-2cdc7700-a422-11e9-85e2-3d3f18f0adc0.jpg)   

![93C56-WORD](https://user-images.githubusercontent.com/6020549/61050527-2cdc7700-a422-11e9-861f-92315ff3e916.jpg)   

# 93C66
![93C66-BYTE](https://user-images.githubusercontent.com/6020549/61050568-441b6480-a422-11e9-9db8-b84fe4faebac.jpg)   

![93C66-WORD](https://user-images.githubusercontent.com/6020549/61050569-441b6480-a422-11e9-8897-1637ee88b8cd.jpg)   

# 93C86
![93C86-BYTE](https://user-images.githubusercontent.com/6020549/61335031-f611c100-a866-11e9-8e67-1a6788219154.jpg)

![93C86-WORD](https://user-images.githubusercontent.com/6020549/61335032-f9a54800-a866-11e9-9235-82821062c239.jpg)

