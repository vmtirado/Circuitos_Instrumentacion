#include <OneWire.h>
#include <DallasTemperature.h>
// Pin donde se conecta el bus 1-Wire
const int pinDatosDQ = 3;
// Instancia a las clases OneWire y DallasTemperature
OneWire oneWireObjeto(pinDatosDQ);
DallasTemperature sensorDS18B20(&oneWireObjeto);

void setup() {
 Serial.begin(9600);
 sensorDS18B20.begin();
}

void loop() {
 // Mandamos comandos para toma de temperatura a los sensores
 //Serial.println("Mandando comandos a los sensores");
 sensorDS18B20.requestTemperatures();
 // Leemos y mostramos los datos de los sensores DS18B20
 Serial.print("Temperatura sensor 1: ");
 Serial.print(sensorDS18B20.getTempCByIndex(0));
 Serial.println(" C");
 //Serial.print("Temperatura sensor 2: ");
 //Serial.print(sensorDS18B20.getTempCByIndex(1));
 //Serial.println(" C");
delay(1000);
}
