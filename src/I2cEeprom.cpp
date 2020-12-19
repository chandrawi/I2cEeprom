#include <I2cEeprom.h>

// Constructor

I2cEeprom::I2cEeprom(uint8_t deviceAddress)
{
    _deviceSize = EEPROM_DEFAULT_DEVICE_SIZE;
    _init(deviceAddress, _deviceSize);
    _autoSizing = true;
}

I2cEeprom::I2cEeprom(uint8_t deviceAddress, int16_t deviceSize)
{
    _deviceSize = deviceSize;
    _init(deviceAddress, deviceSize);
    _autoSizing = false;
}

// Public

bool I2cEeprom::begin()
{
    return begin(EEPROM_DEFAULT_I2C_CLOCK);
}

bool I2cEeprom::begin(uint32_t clock)
{
    Wire.begin();
    if ((clock != 10000L) || (clock != 100000L) || (clock != 400000L) || (clock != 1000000L)) clock = EEPROM_DEFAULT_I2C_CLOCK;
    Wire.setClock(clock);
    
    Wire.beginTransmission(_deviceAddress);
    if (Wire.endTransmission() != 0) return false;

#if EEPROM_AUTO_SIZING
    if (_autoSizing) {
        _deviceSize = getSize();
        _init(_deviceAddress, _deviceSize);
    }
#endif

    return true;
}

void I2cEeprom::end()
{
    Wire.end();
}

uint8_t I2cEeprom::read(uint16_t address)
{
    uint8_t data;
    _readBytes(address, &data, 1);
    return data;
}

uint16_t I2cEeprom::read(uint16_t address, uint8_t* data, uint16_t nBytes)
{
    return _readData(address, data, nBytes);
}

uint16_t I2cEeprom::read(uint16_t address, char* data, uint16_t nBytes)
{
    uint8_t* data_ = (uint8_t*) data;
    return _readData(address, data_, nBytes);
}

bool I2cEeprom::write(uint16_t address, uint8_t data)
{
    return _writeBytes(address, &data, 1);
}

bool I2cEeprom::write(uint16_t address, uint8_t* data, uint16_t nBytes)
{
    return _writeData(address, data, nBytes, true);
}

bool I2cEeprom::write(uint16_t address, char* data, uint16_t nBytes)
{
    uint8_t* data_ = (uint8_t*) data;
    return _writeData(address, data_, nBytes, true);
}

bool I2cEeprom::write(uint16_t address, uint8_t data, uint16_t nBytes)
{
    uint8_t dataArray[_pageSize];
    for (uint16_t i = 0; i < _pageSize; i++) dataArray[i] = data;
    return _writeData(address, dataArray, nBytes, false);
}

bool I2cEeprom::update(uint16_t address, uint8_t data)
{
    uint8_t readData;
    _readBytes(address, &readData, 1);
    if (readData == data) return false;
    return _writeBytes(address, &data, 1);
}

int16_t I2cEeprom::getSize()
{
    int16_t size = -1;
    uint8_t oldData0, oldData[11], matchData = 0x11, checkData = 0xEE;
    uint16_t addr, addrA, addrB;
    uint8_t i;

    // remember old values
    _readBytes(0, &oldData0, 1);
    for (i=0; i<11; i++)
    {
        addr = 64 << i;
        _readBytes(addr, &oldData[i], 1);
    }

    // scan address folding
    _writeBytes(0, &matchData, 1);
    for (i=0; i<11; i++)
    {
        addr = 64 << i;
        _writeBytes(addr, &checkData, 1);
        _readBytes(0, &matchData, 1);
        if (matchData == 0xEE) // folded!
        {
            size = i;
            break;
        }
    }

    // restore to old values
    _writeBytes(0, &oldData0, 1);
    for (i=0; i<11; i++)
    {
        addr = 64 << i;
        _writeBytes(addr, &oldData[i], 1);
    }
    
    if (size == 0) return 0;
    if (size == -1) return -1;
    return 0x01 << (size - 1);
}

bool I2cEeprom::setPageSize(uint8_t pageSize)
{
    if ((pageSize == 8) || (pageSize == 16) || (pageSize == 32) || (pageSize == 64) || (pageSize == 128)) {
        _pageSize = pageSize;
        return true;
    }
    return false;
}

uint8_t I2cEeprom::getPageSize()
{
    return _pageSize;
}

bool I2cEeprom::setWriteTime(uint32_t writeTime)
{
    if (writeTime > EEPROM_DEFAULT_WRITE_TIME_MIN) {
        _writeTime = writeTime;
        return true;
    }
    return false;
}

uint32_t I2cEeprom::getWriteTime()
{
    return _writeTime;
}

// Private

void I2cEeprom::_init(uint8_t deviceAddress, uint16_t deviceSize)
{
    _deviceAddress = deviceAddress;
    
    if (deviceSize <= 2) {
        _nAddressBytes = 1;
        _pageSize = 8;
    }
    else if (deviceSize <= 8) {
        _nAddressBytes = 2;
        _pageSize = 16;
    }
    else if (deviceSize <= 32) {
        _nAddressBytes = 2;
        _pageSize = 32;
    }
    else if (deviceSize <= 256) {
        _nAddressBytes = 2;
        _pageSize = 64;
    }
    else{
        _deviceSize = getSize();
        _nAddressBytes = 2;
        _pageSize = 128;
    }
    
    _bufferSize = EEPROM_DEFAULT_I2C_BUFFERSIZE - _nAddressBytes;
    if (_bufferSize > _pageSize) _bufferSize = _pageSize;
    
    _writeTime = EEPROM_DEFAULT_WRITE_TIME;
}

uint16_t I2cEeprom::_readBytes(uint16_t address, uint8_t* data, uint16_t nBytes)
{
    uint16_t i = 0;
    
    Wire.beginTransmission(_deviceAddress);
    if (_nAddressBytes == 2) Wire.write(address >> 8);   // MSB
    Wire.write(address & 0xFF); // LSB
    Wire.endTransmission();
    
    Wire.requestFrom(_deviceAddress, nBytes);
    while(Wire.available()) data[i++] = Wire.read();
    
    return i;
}

uint16_t I2cEeprom::_readData(uint16_t address, uint8_t* data, uint16_t nBytes)
{
    uint16_t addr = address;
    uint16_t addrEnd = address + nBytes;
    uint16_t readLen;
    uint16_t nRead = 0;
    
    while(addr < addrEnd) {
        readLen = addrEnd - addr;
        if(readLen > _bufferSize) readLen = _bufferSize;
        
        nRead += _readBytes(addr, data, readLen);

        addr += readLen;   // Increment address for next read
        data += readLen;
    }
    
    return nRead;
}

bool I2cEeprom::_writeBytes(uint16_t address, uint8_t* data, uint16_t nBytes)
{
    Wire.beginTransmission(_deviceAddress);
    if (_nAddressBytes == 2) Wire.write(address >> 8);   // MSB
    Wire.write(address & 0xFF); // LSB
    Wire.write(data, nBytes);
    int8_t stat = Wire.endTransmission();
    
    uint32_t tWrite = micros();
    while ((micros() - tWrite) <= _writeTime)
    {
        Wire.beginTransmission(_deviceAddress);
        if (Wire.endTransmission() == 0) break;
        yield();
    }
    
    if (stat == 0) return true;
    return false;
}

bool I2cEeprom::_writeData(uint16_t address, uint8_t* data, uint16_t nBytes, bool increment)
{
    uint16_t addr = address;
    uint16_t addrEnd = address + nBytes;
    uint16_t writeLen;
    uint16_t nPageBytes;
    
    while(addr < addrEnd) {
        writeLen = addrEnd - addr;
        nPageBytes = _pageSize - (addr % _pageSize);
        if(writeLen > nPageBytes) writeLen = nPageBytes;
        if(writeLen > _bufferSize) writeLen = _bufferSize;
        
        if (!_writeBytes(addr, data, writeLen)) return false;

        addr += writeLen;   // Increment address for next write
        if (increment) data += writeLen;
    }
    
    return true;
}
