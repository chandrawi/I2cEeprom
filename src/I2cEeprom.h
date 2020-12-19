#ifndef _EXTERNAL_I2CEEPROM_H_
#define _EXTERNAL_I2CEEPROM_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <Wire.h>

#define EEPROM_DEFAULT_I2C_BUFFERSIZE       32
#define EEPROM_DEFAULT_I2C_CLOCK            100000L
#define EEPROM_DEFAULT_PAGESIZE             64
#define EEPROM_DEFAULT_WRITE_TIME           5000
#define EEPROM_DEFAULT_WRITE_TIME_MIN       1000
#define EEPROM_DEFAULT_DEVICE_SIZE          128
#define EEPROM_AUTO_SIZING                  true

class I2cEeprom
{
    public:

        I2cEeprom(uint8_t deviceAddress);
        I2cEeprom(uint8_t deviceAddress, int16_t deviceSize);

        bool begin();
        bool begin(uint32_t clock);
        void end();

        uint8_t read(uint16_t address);
        uint16_t read(uint16_t address, uint8_t* data, uint16_t nBytes);
        uint16_t read(uint16_t address, char* data, uint16_t nBytes);

        bool write(uint16_t address, uint8_t data);
        bool write(uint16_t address, uint8_t* data, uint16_t nBytes);
        bool write(uint16_t address, char* data, uint16_t nBytes);
        bool write(uint16_t address, uint8_t data, uint16_t nBytes);

        bool update(uint16_t address, uint8_t data);

        int16_t getSize();

        bool setPageSize(uint8_t pageSize);
        uint8_t getPageSize();

        bool setWriteTime(uint32_t writeTime);
        uint32_t getWriteTime();

        template <typename T> T get(uint16_t address, T &data)
        {
            const uint8_t size = sizeof(T);
            union conv{
                T Data;
                uint8_t Binary[size];
            };
            union conv u;
            _readData(address, u.Binary, size);
            data = u.Data;
            return data;
        }

        template <class T> bool put(uint16_t address, T data)
        {
            const uint8_t size = sizeof(T);
            union conv{
                T Data;
                uint8_t Binary[size];
            };
            union conv u;
            u.Data = data;
            return _writeData(address, u.Binary, size, true);
        }

    private:

        uint8_t _deviceAddress;
        int16_t _deviceSize;
        bool _autoSizing;
        uint8_t _pageSize;
        uint8_t _bufferSize;
        uint8_t _nAddressBytes;
        uint32_t _writeTime;

        void _init(uint8_t deviceAddress, uint16_t deviceSize);

        uint16_t _readBytes(uint16_t address, uint8_t* data, uint16_t nBytes);
        uint16_t _readData(uint16_t address, uint8_t* data, uint16_t nBytes);

        bool _writeBytes(uint16_t address, uint8_t* data, uint16_t nBytes);
        bool _writeData(uint16_t address, uint8_t* data, uint16_t nBytes, bool increment);
};

#endif
