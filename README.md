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

# Software requirement

WiringPi Library   

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
int eeprom_open(int model, int org, int pCS, int pSK, int pDI, int pDO, struct eeprom *dev);

// Erase/Write Enable
void eeprom_ew_enable(struct eeprom *dev);

// Erase/Write Disable
void eeprom_ew_disable(struct eeprom *dev);

// Check Erase/Write Enable
bool eeprom_is_ew_enabled(struct eeprom *dev);

// Erase All Memory
void eeprom_erase_all(struct eeprom *dev);

// Erase Byte or Word
void eeprom_erase(struct eeprom *dev, uint16_t addr)

// Write All Memory with same Data
void eeprom_write_all(struct eeprom *dev, uint16_t value)

// Write Data to Memory
void eeprom_write(struct eeprom *dev, uint16_t addr, uint16_t value)

// Read Data from Memory
uint16_t eeprom_read(struct eeprom *dev, uint16_t addr)
```

---

# Wireing for x8 Organization
![Raspberry-93cx6-byte](https://user-images.githubusercontent.com/6020549/79814750-06d50180-83ba-11ea-891c-c604fa2e69aa.jpg)

# Wireing for x16 Organization
![Raspberry-93cx6-word](https://user-images.githubusercontent.com/6020549/79814843-308e2880-83ba-11ea-81c3-a4874460ef1d.jpg)

**It's insufficient in 3.3V Power supply.**   
**You have to supply 5V.**   
**So you have to shift level about MISO line.**   

---

# 93C46
![93c46-byte](https://user-images.githubusercontent.com/6020549/79822923-507b1780-83cd-11ea-8c69-e5ed2dfcc444.jpg)

![93c46-word](https://user-images.githubusercontent.com/6020549/79822955-67216e80-83cd-11ea-8c95-5962a9612bc3.jpg)

# 93C56
![93c56-byte](https://user-images.githubusercontent.com/6020549/79822962-6a1c5f00-83cd-11ea-902b-dc8e95b56de0.jpg)

![93c56-word](https://user-images.githubusercontent.com/6020549/79822965-6c7eb900-83cd-11ea-9c7b-294f9a30f2cb.jpg)

# 93C66
![93c66-byte](https://user-images.githubusercontent.com/6020549/79822968-6f79a980-83cd-11ea-915b-83649c084351.jpg)

![93c66-word](https://user-images.githubusercontent.com/6020549/79822975-72749a00-83cd-11ea-8d93-b8e34b0acac7.jpg)

# 93C76
![93c76-byte](https://user-images.githubusercontent.com/6020549/83958191-f44f5280-a8a9-11ea-8b08-1fa633c7beaf.jpg)

![93c76-word](https://user-images.githubusercontent.com/6020549/83958192-f74a4300-a8a9-11ea-9a48-b98d4028e6eb.jpg)

# 93C86
![93c86-byte](https://user-images.githubusercontent.com/6020549/79822986-76a0b780-83cd-11ea-8963-88009dccdb8c.jpg)

![93c86-word](https://user-images.githubusercontent.com/6020549/79822989-799ba800-83cd-11ea-8e6d-02bad90bcdd4.jpg)
