/***********************************************************************************
   Instrumentación Electrónica _ 2022-01
   Sensado de Velocidad por Encoder
   Profesor: Johann F. Osma
   Asistente: Santiago Tovar Perilla
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
    ⓰ GND           GND

    Mantenga las condiciones eléctricas para la balanza y el dosificador liquido.

***********************************************************************************/


/***************************** Librerias Externas *********************************/
#include "HX711.h"        // Modulo HX711 - Celda de carga

/**************************** Variables Calibración *******************************/
// Variables para balanza
int Peso_conocido = 20;   // en gramos | MODIFIQUE PREVIAMENTE CON EL VALOR DE PESO CONOCIDO!!!

// Variables para mini bomba de agua
int Densidad = 1;         // en g/mL | MODIFIQUE PREVIAMENTE CON EL VALOR DE DENSIDAD CORRESPONDIENTE!!!
int Volumen_deseado = 50; // en mL | MODIFIQUE PREVIAMENTE CON EL VALOR DE VOLUMEN DESEADO!!!

/************************ Definición de pines y variables *************************/
// Puertos y Variables para balanza
#define DOUT  A1
#define CLK  A0

float pesoRecipiente = 0;

HX711 balanza(DOUT, CLK);


// Puertos y Variables para mini bomba de agua
#define E1 8              // Enable Pin for motor 1
#define Input1 9          // Control pin 1 for motor 1
#define Input2 10         // Control pin 2 for motor 1

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

unsigned int t = 3700; // Rango: 980 - 9800 que corresponde a 150 - 15 RPMs aprox.

// Puertos y Variables para Encoder
#define Encoder_pin 2      // The pin the encoder is connected 
#define PulsosRotacion 20  // Value of pluses per rotation

int Encoder = 0;           // Value of the Output of the encoder 
int Estado = 1;            // Current state of the encoder to recognize the pulse changes
int Pulsos = 0;            // Counter of pulses
unsigned long InitialTime = 0;
unsigned long FinalTime; 
float RPMs;




/*********************************** Setup ***************************************/

void setup() {

Serial.begin(9600);
//  ////// Balanza //////
//
//  CalibracionBalanza();
//  Serial.println(" ");
//  Serial.println("¡¡¡LISTO PARA PESAR!!!");
//  Serial.println(" ");
//  Serial.println(" ");
//
//   ////// Inicializamos los pines //////
//
//  pinMode( Input1, OUTPUT);
//  pinMode( Input2, OUTPUT);
//  pinMode( E1, OUTPUT);
  pinMode( ms1 , OUTPUT);
  pinMode( ms2 , OUTPUT);
  pinMode( ms3 , OUTPUT);
//  pinMode( DirPin , OUTPUT);
  pinMode( StepPin , OUTPUT);
  pinMode( Encoder_pin , INPUT);

  ////// Motor paso a paso //////
  
  ControlMotorPasoaPaso();

}

/************************************ Loop ***************************************/
void loop() {

//  ////// Balanza & Dosificador //////
//
//  for (int i = 3; i >= 0; i--)
//  {
//    MedidaBalanza(); //Medición actual de la Balanza
//  }
//
//  if (OnOff == 1) {
//
//    digitalWrite(E1, HIGH);     // Activación del motor
//    Serial.println(" ");
//    Serial.println("¡¡¡LISTO PARA DOSIFICAR!!!");
//    Serial.println(" ");
//    Serial.println("Coloque el recipiente y no lo retire hasta finalizar");
//    Serial.println(" ");
//    Serial.println(" ");
//    delay(100);
//
//    pesoRecipiente = MedidaBalanza();
//
//    DosificarB(pesoRecipiente); //Dosificación de volumen según Balanza
//  }
//
//  MedidaBalanza(); //Medición actual de la Balanza

  ////// Motor paso a paso //////

  digitalWrite(StepPin, HIGH); 
  delayMicroseconds(t);           
  digitalWrite(StepPin, LOW);  
  delayMicroseconds(t);
  
  ////// Encoder //////
  Encoder = digitalRead(Encoder_pin);  

  if(Encoder == LOW && Estado == 1)
  {
     Estado = 0;             
  }

  if(Encoder == HIGH && Estado == 0)
  {
     Pulsos++; 
     Estado = 1;     
  }

  if(Pulsos == PulsosRotacion)
  {
    FinalTime = millis();
    RPMs = 60000/(FinalTime - InitialTime);
    Pulsos = 0; 
    InitialTime = FinalTime;
    Serial.print("RPM = ");
    Serial.println(RPMs); 
  }
}

/*********************************** Funciones ***************************************/

//Función de Calibración de Balanza: Permite calibrar la medida de la balanza según un peso de calibración conocido
void CalibracionBalanza(void)
{
  float Escala;
  float PromMedicion;

  Serial.println("~ CALIBRACIÓN DE LA BALANZA ~");
  Serial.println(" ");
  delay(100);
  Serial.println("No ponga ningun objeto sobre la balanza");
  Serial.println(" ");
  balanza.set_scale(); //Ajusta la escala a su valor por defecto que es 1
  balanza.tare(20);  //El peso actual es considerado "Tara".
  delay(50);
  Serial.print("...Destarando...");
  delay(250);

  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Coloque un peso conocido ");
  delay(250);

  for (int i = 3; i >= 0; i--)
  {
    Serial.print(" ... ");
    Serial.print(i);
    PromMedicion = balanza.get_value(20);//20 mediciones  //Obtiene el promedio de las mediciones analogas según valor ingresado
  }

  Serial.println(" ");
  Serial.println(" ");
  Serial.println("Retire el peso ");
  delay(250);

  for (int i = 3; i >= 0; i--)
  {
    Serial.print(" ... ");
    Serial.print(i);
    delay(1000);
  }

  Escala = PromMedicion / Peso_conocido; // Relación entre el promedio de las mediciones analogas con el peso conocido en gramos
  balanza.set_scale(Escala); // Ajusta la escala correspondiente

  Serial.println(" ");
  Serial.println(" ");
}

//Función de Medición de Balanza: Permite obtener la medida actual en peso (g) de la balanza
float MedidaBalanza(void)
{
  float peso; // variable para el peso actualmente medido en gramos
  float PL1 = 0;
  float PL2 = 0;
  float PL3 = 0;
  float promPL = 0;

  for (int i = 1; i >= 0; i--)
  {
    peso = balanza.get_units(10); // Entrega el peso actualmente medido en gramos
    if (peso < 0) peso = peso * -1;

    PL1 = peso;
    PL2 = PL1;
    PL3 = PL2;

    promPL = (PL1 + PL2 + PL3) / 3;
  }

  Serial.print("Peso: ");
  Serial.print(promPL, 1);
  Serial.println(" g");
  //delay(25);

  return promPL;

}

//Función de DosificarB: Permite entregar un volumen deseado(mL) según la medida actual de peso (g) de la balanza
void DosificarB(float pesoRecipiente)
{
  float pesoLiquido = MedidaBalanza() - pesoRecipiente;// Entrega el peso del liquido actualmente medido en gramos


  while ((Volumen_deseado / Densidad) + 0.1 >= pesoLiquido)
  {
    Adelante = map(Var, 0, 1023, 0, 255); // Voltajea ingresar
    analogWrite(Input1, Adelante);

    pesoLiquido = MedidaBalanza() - pesoRecipiente;
  }
  if ((Volumen_deseado / Densidad) + 0.1 <= pesoLiquido)
  {
    OnOff = 0;
    analogWrite(Input1, Detener);
    digitalWrite(E1, LOW);      // Parar el motor
  }
}

//Función para Controlar el Motor Paso a Paso: Permite controlar el motor y los pasos de este
void ControlMotorPasoaPaso(void)
{
  
  Serial.println("Motor Control");

  digitalWrite(ms1, LOW);
  digitalWrite(ms2, LOW);
  digitalWrite(ms3, LOW);
//  digitalWrite(DirPin, LOW);
}
