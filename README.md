# STM32 library for Allegro ACS37800

Simple library that contains the essential functions like measuring RMS and instantaneous voltage and current.

### Usage

#### Setup
The device can be set up by declaring the acs37800_t type
``` C
acs37800_t sensor;
```

The parameters for the sensor then needs to be set
``` C
sensor.divRes = 2000000;
sensor.i2c_address = 0x60;
sensor.i2c_device = &hi2c1;
sensor.senseRes = 3200;
sensor.maxCurrent = 90;
sensor.maxVolt = 160;
```

Set bypass n_enable if measuring DC and set amount of samples, (and write same to eeprom with third parameter (optional))
``` C
acs_setBybassNenable(&sensor, true, true);
acs_setNumberOfSamples(&sensor, 1023, true);
```

For DC, read instant current and voltage
``` C
acs_getInstCurrVolt(&sensor, &current1, &voltage1);
```

For AC, read RMS
``` C
acs_getRMS(&sensor, &current1, &voltage1);
```
