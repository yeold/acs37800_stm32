/*
 * acs37800.c
 *
 * Created: 08.08.2023
 *  Author: yeold
 */

#include <acs37800.h>
#include <stdbool.h>

static void acs_readRegister(acs37800_t* device, uint16_t address, uint32_t* data);
static void acs_writeRegister(acs37800_t* device, uint16_t address, uint32_t* data);

void acs_getRMS(acs37800_t* device, float * const pCurrent, float * const pVoltage)
{
	ACS37800_REGISTER_20_t store;
	acs_readRegister(device, ACS37800_R_IRMS_VRMS, &store.data.all);

	float volts = (float)store.data.bits.vrms;
	volts /= 55000.0;
	volts *= 250;
	volts /= 1000;
	float resMult = (device->divRes + device->senseRes) / device->senseRes;
	volts *= resMult;

	*pVoltage = volts;

	uint16_t unsignedData = store.data.bits.irms;
	float amps = (float)unsignedData;
	amps /= 55000.0;
	amps *= device->maxCurrent;

	*pCurrent = amps;
}

void acs_getInstCurrVolt(acs37800_t* device, float * const pCurrent, float * const pVoltage)
{
	ACS37800_REGISTER_2A_t store;

	acs_readRegister(device, ACS37800_R_ICODES_VCODES, &store.data.all);

	uint16_t unsignedData = store.data.bits.vcodes;
	float volts = (float)unsignedData;
	volts /= 27500.0;
	volts *= 250;
	volts /= 1000;
	float resMult = (device->divRes + device->senseRes) / device->senseRes;
	volts *= resMult;

	*pVoltage = volts;

	unsignedData = store.data.bits.icodes;
	float amps = (float) unsignedData;
	amps /= 27500.0;
	amps *= device->maxCurrent;

	*pCurrent = amps;
}

void acs_setBybassNenable(acs37800_t* device, _Bool bypass, _Bool eeprom)
{
	ACS37800_REGISTER_0F_t store;
	uint32_t key = ACS37800_R_ACCESS_CODE_KEY;

	acs_writeRegister(device, ACS37800_R_ACCESS_CODE, &key);
	acs_readRegister(device, ACS37800_R_I2C_CONFIG, &store.data.all);

	if (bypass == true)
	{
		store.data.bits.bypass_n_en = 1;
	}
	else
	{
		store.data.bits.bypass_n_en = 0;
	}

	acs_writeRegister(device, ACS37800_R_I2C_CONFIG, &store.data.all);

	if (eeprom == true)
	{
		acs_readRegister(device, (ACS37800_R_I2C_CONFIG - ACS37800_EEPROM_OFFSET), &store.data.all);

		if (bypass == true)
		{
			store.data.bits.bypass_n_en = 1;
		}
		else
		{
			store.data.bits.bypass_n_en = 0;
		}

		acs_writeRegister(device, (ACS37800_R_I2C_CONFIG - ACS37800_EEPROM_OFFSET), &store.data.all);
	}

	acs_writeRegister(device, ACS37800_R_ACCESS_CODE, 0);
	HAL_Delay(100);
}

void acs_setNumberOfSamples(acs37800_t* device, uint32_t numOfSamples, _Bool eeprom)
{
	ACS37800_REGISTER_0F_t store;
	uint32_t key = ACS37800_R_ACCESS_CODE_KEY;

	acs_writeRegister(device, ACS37800_R_ACCESS_CODE, &key);
	acs_readRegister(device, ACS37800_R_I2C_CONFIG, &store.data.all);

	store.data.bits.n = numOfSamples & 0x3FF;

	acs_writeRegister(device, ACS37800_R_I2C_CONFIG, &store.data.all);

	if (eeprom == true)
	{
		acs_readRegister(device, (ACS37800_R_I2C_CONFIG - ACS37800_EEPROM_OFFSET), &store.data.all);

		store.data.bits.n = numOfSamples & 0x3FF;

		acs_writeRegister(device, (ACS37800_R_I2C_CONFIG - ACS37800_EEPROM_OFFSET), &store.data.all);
	}

	acs_writeRegister(device, ACS37800_R_ACCESS_CODE, 0);

	HAL_Delay(100);
}

static void acs_readRegister(acs37800_t* device, uint16_t address, uint32_t* data)
{
	uint8_t recvData [4] = {0,0,0,0};

	HAL_I2C_Mem_Read(device->i2c_device, device->i2c_address<<1, address, 1, recvData, 4, 1000);

	*data = (uint32_t)recvData[0] | (uint32_t)recvData[1] << 8 | (uint32_t)recvData[2] << 16 | (uint32_t)recvData[3] << 24;
}

static void acs_writeRegister(acs37800_t* device, uint16_t address, uint32_t* data)
{
	uint8_t transmitData[4] = {0,0,0,0};

	transmitData[0] = *data & 0xFF;
	transmitData[1] = (*data >> 8) & 0xFF;
	transmitData[2] = (*data >> 16) & 0xFF;
	transmitData[3] = (*data >> 24) & 0xFF;

	HAL_I2C_Mem_Write(device->i2c_device, device->i2c_address<<1, address, 1, transmitData, 4, 1000);
}
