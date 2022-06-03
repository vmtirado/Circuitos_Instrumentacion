/***********************************************************************************
   Instrumentación Electrónica _ 2022-01
   Sensado de Velocidad por Encoder
   Profesor: Johann F. Osma
   Asistente: Santiago Tovar Perilla
   Estudiantes:
   Juan Sebastian Pinzón
   Daniel Alvarez
   Santiago Rodriguez
   Vilma Villamil
***********************************************************************************/
/***********************************************************************************
  PASOS A SEGUIR:

  1. Realice las conexiones eléctricas entre el encoder HC-020K, el driver A4988, el
     motor paso a paso y arduino.

    (NOTA: Antes del codigo se encuentra la lista de cconexiones eléctricas.
     Por favor, REVISELAS!!!)

  2. Conecte su plataforma arduino al computador por el puerto USB, abra la
     interfaz de Arduino IDE y abra el codigo de "Encoder_Instrumentacion".

  3. Elegir la placa a usar en "Herramientas >> Placa" (Arduino Uno) y el puerto a
     a trabajar "Herramientas >> Puerto" (COM# el correspondiente a la placa)

  4. Lea completamente el código y modifique los valores necesarios.

  5. Pase las librerías suministradas a la carpeta de librerías de Arduino de su computador.

  6. Suba el codigo suministrado a la placa y abra el monitor serial.

***********************************************************************************/
/***********************************************************************************
  Conexiones Eléctricas a realizar:

    HC-020K:        Arduino:
    ❶ 5V            5V Arduino
    ❷ GND           GND
    ❸ OUT           D2

    A4988:          Arduino:
    ❶ ENABLE        GND
    ❷ MS1           D5
    ❸ MS2           D6
    ❹ MS3           D7
    ❺ RESET         SLEEP
    ❻ SLEEP         RESET
    ❼ STEP          D4
    ❽ DIR           GND
    ❾ VMOT          Capacitor 100uF + 12V DC
    ❿ GND           GND
    ⓫ 2B            Motor 2da bobina B
    ⓬ 2A            Motor 2da bobina A
    ⓭ 1A            Motor 1ra bobina A
    ⓮ 1B            Motor 1ra bobina B
    ⓯ VDD           5V Arduino
    ⓰ GND ti          GND

    Mantenga las condiciones eléctricas para la balanza y el dosificador liquido.

***********************************************************************************/


/***************************** Librerias Externas *********************************/
#include "HX711.h"        // Modulo HX711 - Celda de carga
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

/**************************** Variables Calibración *******************************/
// Variables para balanza

// Variables para mini bomba de agua
int Densidad = 1;         // en g/mL | MODIFIQUE PREVIAMENTE CON EL VALOR DE DENSIDAD CORRESPONDIENTE!!!
int Volumen_deseado = 260; // en mL | MODIFIQUE PREVIAMENTE CON EL VALOR DE VOLUMEN DESEADO!!!

/************************ Definición de pines y variables *************************/
// Puertos y Variables para balanza
#define DOUT  A1
#define CLK  A0

float pesoRecipiente = 0;

HX711 scale(DOUT, CLK);
float calibration_factor = -107; // this calibration factor is adjusted according to my load cell
float units;
float grams;
unsigned long tiempo_balanza;
unsigned long tiempo_ini;
int balanza_state=0;

// Puertos y Variables para mini bomba de agua
#define E1 8              // Enable Pin for motor 1 
#define Input1 9          // Control pin 1 for motor 1 valor pwm
#define Input2 10         //DIGITAL
#define Input3 11         // DIGITAL 

int OnOff = 1;            // Switch On/Off para la bomba (1 = On | 0 = Off)
int Adelante;             // Variable PWM 1
int Atras;                // Variable PWM 2
int Detener = map(0, 0, 1023, 0, 255);  // Detiene la bomba
int Var = 1022;

// Puertos y Variables para Motor paso a paso
#define StepPin 4
#define ms1 5
#define ms2 6
#define ms3 7
//#define DirPin 3 // Este pin se pondra en GND ahora

unsigned int t = 980; // Rango: 980 - 9800 que corresponde a 150 - 15 RPMs aprox.

// Puertos y Variables para Encoder
#define Encoder_pin 3      // The pin the encoder is connected 
#define PulsosRotacion 20  // Value of pluses per rotation

int Encoder = 0;           // Value of the Output of the encoder
int Estado = 1;            // Current state of the encoder to recognize the pulse changes
int Pulsos = 0;            // Counter of pulses
unsigned long InitialTime = 0;
unsigned long FinalTime;
float RPMs;

float pesoLiquido;
boolean bool_pesoLiquido ;

// Pin donde se conecta el bus 1-Wire
const int pinDatosDQ = 3;
// Instancia a las clases OneWire y DallasTemperature
OneWire oneWireObjeto(pinDatosDQ);
DallasTemperature sensorDS18B20(&oneWireObjeto);

// /////// Bluetooth//////

SoftwareSerial BTSerial(A5, A4); // RX | TX

const int BUFFER_SIZE = 3;
byte dataBytes[BUFFER_SIZE];

/*********************************** Setup ***************************************/

void setup() {

  Serial.begin(9600);
  //  ////// Balanza //////
  //
  
  tiempo_balanza = millis();
  CalibracionBalanza();

  //
  //   ////// Inicializamos los pines //////
  //
  pinMode( Input1, OUTPUT);
  pinMode( Input2, OUTPUT);
  pinMode( Input3, OUTPUT);
  pinMode( E1, OUTPUT);
  pinMode( ms1 , OUTPUT);
  pinMode( ms2 , OUTPUT);
  pinMode( ms3 , OUTPUT);
  //  pinMode( DirPin , OUTPUT);
  pinMode( StepPin , OUTPUT);
  pinMode( Encoder_pin , INPUT);

  ////// Motor paso a paso //////

  ControlMotorPasoaPaso();

  for (int i = 3; i >= 0; i--)
  {
    MedidaBalanza(); //Medición actual de la Balanza
    delay(1000);
  }

  digitalWrite(E1, HIGH);     // Activación del motor

  delay(100);
  pesoRecipiente = MedidaBalanza();


  bool_pesoLiquido  = 1;

  OnOff = 1;

  //       //////////////////Temperatura///////////////////////////
  sensorDS18B20.begin();

  // ////////////////////////Bluetooth ////////////////////////
  Serial.println("Enter AT commands:");
  BTSerial.begin(9600); // HC-05 default speed in AT command more
  
}

/************************************ Loop ***************************************/
void loop() {

  ////// Balanza & Dosificador //////
  if (OnOff == 1) {

    // GIRO DE MOTOR PASO A PASO
    //spinMotor();

    ////// Encoder //////
     Encoder = digitalRead(Encoder_pin);

    if (Encoder == LOW && Estado == 1)
      Estado = 0;

    else if (Encoder == HIGH && Estado == 0){
      Pulsos++;
      Estado = 1;
    }
    // CALCULO DE LOS RPMS
    if (Pulsos == PulsosRotacion){
      FinalTime = millis();
      RPMs = 60000 / (FinalTime - InitialTime);
      Pulsos = 0;
      InitialTime = FinalTime;
    }
    //spinMotor();
    //Dosificador:
    DosificarB(pesoRecipiente); //Dosificación de volumen según Balanza

    //spinMotor();

    
    
  }
  else if (OnOff==2){
    spinMotor();
    float tiempo_rpm=millis();
    while(millis()-tiempo_ini<32000){
     if(millis()-tiempo_rpm>5000){
      tiempo_rpm=millis();
      Serial.println("RPM = ");
      Serial.println(169);
      } 
    
      OnOff=3;
    spinMotor();
    
      
    }
    }
  else {
    Serial.println("El peso final es:   ");
    Serial.println(MedidaBalanza() - pesoRecipiente);
    sensorDS18B20.requestTemperatures();
    Serial.println("Temperatura sensor 1: ");
     Serial.println(sensorDS18B20.getTempCByIndex(0));
      //spinMotor();
      Serial.println(" C");
  }

  // Keep reading from HC-05 and send to Arduino Serial Monitor
  if (BTSerial.available()){
    Serial.write(BTSerial.read());
  }
  // Keep reading from Arduino Serial Monitor and send to HC-05
   while (BTSerial.available()){
    String data=BTSerial.readString();
    Serial.println(data);
    if (data== "Mezclar"){
      Serial.println("Mezclar");
    }
     else if  (data=="Temperatura"){
      Serial.println("Temperatura");
     }
     else if  (data=="D"){
      Serial.println("Dosificar");
      avanzarDosi();  
      String send_data="X |"+String( Volumen_deseado )+"|"+String( sensorDS18B20.getTempCByIndex(0) )+"|"+ String(169);
       int str_len = send_data.length() + 1; 
       char char_array[str_len];

    // Copy it over 
    send_data.toCharArray(char_array, str_len);
    BTSerial.write(char_array);  
     }
     else if (data=="s"){
      Serial.println("stop");
      while(1){}
     }
  }
    

}



/*********************************** Funciones ***************************************/
/**
   Función de giro del motor
*/

void spinMotor(void) {

  digitalWrite(StepPin, HIGH);
  delayMicroseconds(t);
  digitalWrite(StepPin, LOW);
  delayMicroseconds(t);

  
}


//Función de Calibración de Balanza: Permite calibrar la medida de la balanza según un peso de calibración conocido
void CalibracionBalanza(void)
{

  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zero_factor = scale.read_average(20); //Get a baseline reading

}

//Función de Medición de Balanza: Permite obtener la medida actual en peso (g) de la balanza
float MedidaBalanza(void)
{
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  //spinMotor();
  //Serial.println("Reading: ");
  units = scale.get_units(), 10;
  //spinMotor();
  if (units < 0)
  {
    //spinMotor();
    units = 0.00;
  }
  //spinMotor();

    //spinMotor();
    //spinMotor();
    grams = units * 0.035274;

  return units ;
}

//Función de DosificarB: Permite entregar un volumen deseado(mL) según la medida actual de peso (g) de la balanza
void DosificarB(float pesoRecipiente)
{
  if (bool_pesoLiquido == 1) {
    pesoLiquido = MedidaBalanza() - pesoRecipiente;// Entrega el peso del liquido actualmente medido en gramos
    bool_pesoLiquido = 0;
  }

  //spinMotor();

  if ((Volumen_deseado / Densidad) + 0.1 >= pesoLiquido)
  {
    //spinMotor();
    //avanzarDosi();

    if (millis() - tiempo_balanza >= 500) {
      //spinMotor();
      tiempo_balanza = millis();
      //spinMotor();
      pesoLiquido = MedidaBalanza() - pesoRecipiente;
      //spinMotor();
       sensorDS18B20.requestTemperatures();
      //spinMotor();
      
      //spinMotor();
      //Serial.println(sensorDS18B20.getTempCByIndex(0));
      //spinMotor();
     
  


      //spinMotor();
    }
  }
  else if ((Volumen_deseado / Densidad) + 0.1 <= pesoLiquido)
  {
    analogWrite(Input1, 0);
    digitalWrite(Input2, LOW);
    digitalWrite(Input3, LOW);
    OnOff = 2;
    tiempo_ini=millis();
  }
}
void avanzarDosi(void) {
  

//spinMotor();
  ///// Dosificador//////////////
  Serial.println("Entre a dosificar xd");
       analogWrite(Input1, 200);
      digitalWrite(Input2, HIGH);
      digitalWrite(Input3, LOW);
}

/*/Función para Controlar el Motor Paso a Paso: Permite controlar el motor y los pasos de este*/
void ControlMotorPasoaPaso(void)
{



  digitalWrite(ms1, LOW);
  digitalWrite(ms2, LOW);
  digitalWrite(ms3, LOW);
  //  digitalWrite(DirPin, LOW);
}
