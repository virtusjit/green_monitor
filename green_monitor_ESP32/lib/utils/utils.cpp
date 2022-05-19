#include "utils.h"

void trace_ds18b20_addr(OneWire ds)
{
byte i;
  byte addr[8];
 
  if (!ds.search(addr)) {
    Serial.println(" No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  Serial.print(" ROM =");
  for (i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
    
  }
  Serial.println();
}

float read_tempC(DallasTemperature dt_sensor,DeviceAddress da)
{
  dt_sensor.requestTemperatures();
  return dt_sensor.getTempC(da);
}