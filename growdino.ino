#include <Wire.h> //l2c
#include "RTClib.h" //Real time clock
#include "DHT.h" //SENSOR DHT22
#include <LiquidCrystal_I2C.h> //LCD 0x27
//Iluminación
//Ventilador
//Extra/Intra
//Humificador

//Reles
const byte ReleLuz=8;
const byte ReleVentilador=9;
const byte ReleExtractor=10;
const byte ReleHumificador=11;
const byte ON=1;
const byte OFF=0;

//--Sensores
//DHT
const byte dhtPin=2;
DHT dht(dhtPin, DHT22);

//RTC
RTC_DS1307 rtc;
DateTime now;

//loops
int IntervaloSensores=3000;
long LastMillisSensores;
int IntervaloRTC=200;
long LastMillisRTC;
int IntervaloDisplay=200;
long LastMillisDisplay;

//LCD
LiquidCrystal_I2C lcd(0x27,20,4);  //Crear el objeto lcd  dirección  0x27 y 20 columnas x 4 filas
const byte char1[]{
  B01110,
  B01010,
  B01110,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

//Humedad y Temperatura
float HumedadIndoor;
float TempIndoor;
float HumedadMinima=40;
float HumedadMaxima=60;
float TempMinima=20; //Para apagar ventilador
float TempMaxima=30; //Para encender extractor
float ToleranciaTemp=(TempMaxima-TempMinima)/2; //Cuanto tiene que bajar/subir del maximo/minimo para que se apague
float ToleranciaHum=(HumedadMaxima-HumedadMinima)/2;
boolean ControlandoExtTemp=false; //Para extractor
boolean ControlandoExtHum=false;
boolean SubiendoTemp=false; //Para ventilador

//Variables de control de tiempo
byte TiempoExtractor=15;
byte TiempoVentilador=15;
byte nowMin;
byte nowHour;
byte HoraInicioLuz=20;
byte MinInicioLuz=0;
byte HoraFinLuz=8;
byte MinFinLuz=0;

void InicializarSensores(){
  //RTC
  rtc.begin();
  rtc.adjust(DateTime(2016,6,10,  7,55,55));
  now = rtc.now();

  //DHT
  dht.begin();

  //LCD
  lcd.init();
  lcd.backlight();
}
byte grado0=0;
void setup() {
  
  Serial.begin(9600); //Inicio comunicación serial

  InicializarSensores(); //Inicialización de sensores

  pinMode(ReleLuz, OUTPUT);
  pinMode(ReleVentilador, OUTPUT);
  pinMode(ReleExtractor, OUTPUT);
  pinMode(ReleHumificador, OUTPUT);

  //Serial.println("<<Iniciando arduino>>");
  lcd.createChar(0,char1);
}

void actualizarSensores(){
  if ((millis()-LastMillisSensores)>=IntervaloSensores){ //Sampling time sensores
    LastMillisSensores=millis();
    HumedadIndoor=dht.readHumidity();
    TempIndoor=dht.readTemperature();
  }
}

void actualizarRTC(){
  if ((millis()-LastMillisRTC)>=IntervaloRTC){
    LastMillisRTC=millis();
    nowMin=now.minute();
    nowHour=now.hour();
    now = rtc.now();
  } 
}
void actualizarDisplay(){
  if ((millis()-LastMillisDisplay)>=IntervaloDisplay){
    LastMillisDisplay=millis();
    lcd.clear();

    //Mostrar hora
    lcd.setCursor(0,0);
    if (now.hour()<10){
      lcd.print("0");
    }
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute()<10){
      lcd.print("0");
    }
    lcd.print(now.minute());
    //Fin mostrar hora

    //Inicio mostrar T y H
    lcd.setCursor(8,0);
    lcd.print(TempIndoor,0);
    lcd.write(0);
    lcd.print("C");

    lcd.setCursor(15,0);
    lcd.print(HumedadIndoor,0);
    lcd.print("%HR");
    //Fin mostrar T y H  

    lcd.setCursor(2,2);
    lcd.print("LUZ ");
    if (digitalRead(ReleLuz)){lcd.print("x");}
    lcd.setCursor(2,3);
    lcd.print("VEN ");
    if (digitalRead(ReleVentilador)){lcd.print("x");}
    lcd.setCursor(11,2);
    lcd.print("EXT ");
    if (digitalRead(ReleExtractor)){lcd.print("x");}
    lcd.setCursor(11,3);
    lcd.print("HUM ");
    if (digitalRead(ReleHumificador)){lcd.print("x");}
    
  }
}

void extractor(){ //Se controla la temperatura por el extractor
  //Primero analizo si el extractor tiene que encenderse por el tiempo determinado
  //Que excede a la temperatura, o sea el tiempo que si o si el extractor
  //Tiene que estar encendido para renovar el aire
  if ((nowMin>=0)&&(nowMin<TiempoExtractor)){
    digitalWrite(ReleExtractor,ON);
  } else {

    //Si la temperatura es superior a la maxima
    if (TempIndoor>=TempMaxima) {
      digitalWrite(ReleExtractor,ON);
      //Asigno que estoy bajando la temperatura, para que cuando la temperatura
      //baje de la máxima, el extractor siga funcionando hasta que la temperatura
      //actual sea menor a (TempMaxima-ToleranciaTemp), de esta forma
      //me aseguro que siga funcionando y no se frene apenas llega a TempMaxima-0.1
      ControlandoExtTemp=true; 
    }
    //Si se activo el control de temperatura, y la actual aún no es menor que la maxima menos la tolerancia
    //sigue encendido
    if ((ControlandoExtTemp==true)&&(TempIndoor>=(TempMaxima-ToleranciaTemp))){
      digitalWrite(ReleExtractor,ON);
    }
    //Si está activo el control de temperatura máxima, pero ya la temperatura actual es menor
    //a la tempmax - tolerancia, dejo de controlar la temp max
    if ((ControlandoExtTemp==true)&&(TempIndoor<(TempMaxima-ToleranciaTemp))){
      ControlandoExtTemp=false;
    }
    //Si no estoy controlando la temperatura, y ésta es menor a la maxima, se apaga
    if ((ControlandoExtTemp==false) && (TempIndoor<=TempMaxima)&&(ControlandoExtHum==false)){
      digitalWrite(ReleExtractor,OFF);
    }
    //Asigno un trigger que me active el extractor en caso de que la humedad sea mayor
    //a la humedad máxima
    if (HumedadIndoor>HumedadMaxima){
      digitalWrite(ReleExtractor,ON);
      ControlandoExtHum=true;
    }
    if ((ControlandoExtHum==true)&&(HumedadIndoor>=(HumedadMaxima-ToleranciaHum))){
      digitalWrite(ReleExtractor,ON);
    }
    if ((ControlandoExtHum==true)&&(HumedadIndoor<(HumedadMaxima-ToleranciaHum))){
      ControlandoExtHum=false;
    }
    


  }
}

void ventilador(){
  //Primero analizo si el ventilador tiene que encenderse por el tiempo determinado
  //Que excede a la temperatura, o sea el tiempo que si o si el ventilador
  //Tiene que estar encendido para ventilación interior
  if ((nowMin>=0)&&(nowMin<TiempoVentilador)){
    digitalWrite(ReleVentilador,ON);
  } else {

    //Idealmente, el ventilador tiene que estar SIEMPRE encendido a excepción de cuando
    //la temperatura es menor a la mínima, en cuyo caso se enciende solamente el tiempo
    //que ha sido determinado en TiempoVentilador
    if (TempIndoor<TempMinima){
      digitalWrite(ReleVentilador,OFF);
      SubiendoTemp=true;
    }
    if ((SubiendoTemp==true) && (TempIndoor<=(TempMinima+ToleranciaTemp))){
      digitalWrite(ReleVentilador,OFF);
    }
    if ((SubiendoTemp==true)&&(TempIndoor>(TempMinima+ToleranciaTemp))){
      SubiendoTemp=false;
    }
    if ((SubiendoTemp==false)&&(TempIndoor>TempMinima)){
      digitalWrite(ReleVentilador,ON);
    }
  }
}

void humificador(){
  //Si la humedad es menor a la minima se enciende el humificador, y se activa
  //hasta que alcance la máxima, cuando la alcanza se apaga hasta que alcance la minima
  if (HumedadIndoor<HumedadMinima){
    digitalWrite(ReleHumificador,ON);
  } else {
    digitalWrite(ReleHumificador,OFF);
  }
}

void luces(){
  int startTime=HoraInicioLuz*60+MinInicioLuz;
  int stopTime=HoraFinLuz*60+MinInicioLuz;
  int nowTime=now.hour()*60+now.minute();

  //Si la hora de apagado es mayor a la hora de encendido, significa que las luces
  //se prenden y se apagan en el mismo día, caso contrario las luces se encienden un día
  //y se apagan al día siguiente
  if (stopTime>startTime){
    if((nowTime>=startTime)&&(nowTime<stopTime)){
      digitalWrite(ReleLuz,ON);
    } else {
      digitalWrite(ReleLuz,OFF);
    }
  //CASO INICIO Y FIN EN DIFERENTES DIAS
  //si la hora de apagado está mas adelante de la hora actual o la hora de encendido
  //es anterior a la hora actual, significa que el indoor tiene que estar encendido
  } else if ((stopTime>nowTime)||(startTime<=nowTime)){
    digitalWrite(ReleLuz,ON);
  } else {
    digitalWrite(ReleLuz,OFF);
  }
}

void loop() {
  actualizarSensores();
  actualizarRTC();
  actualizarDisplay();
  extractor();
  ventilador();
  humificador();
  luces();
}
