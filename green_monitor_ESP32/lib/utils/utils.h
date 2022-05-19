#include "OneWire.h"
#include "DallasTemperature.h"
#include <Arduino.h>

void trace_ds18b20_addr(OneWire ds);
float read_tempC(DallasTemperature td_sensor,DeviceAddress da);