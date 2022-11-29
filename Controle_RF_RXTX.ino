#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>

#define radioID 1   //Informar "0" para Transmissor e "1" receptor
#define comentRadio 1 //Ativar e desativar comentários "0" ou "1"

#if radioID == 0
  #include "PushButton.h"
#endif

#define PinCE   9 //Amarelo
#define PinCSN  10 //Verde
RF24 radio(PinCE, PinCSN);

#if radioID == 0
  #define JOYSTICK_LX A3
  #define JOYSTICK_LY A2
  #define JOYSTICK_RX A1
  #define JOYSTICK_RY A0
  
  #define BTNA  4
  #define BTNB  3

  boolean btn_A, btn_B;

  PushButton botao1(BTNA);

#else
  #define PinBuzzer       A3
  #define PinFarol        A4
  
  //#define PinMotor1Pwm     6
  #define PinMotorIN1      7
  #define PinMotorIN2      8
  
  //#define PinMotor2Pwm     5
  #define PinMotorIN3      4
  #define PinMotorIN4      3

  int melody[] = {
    300, 250, 600, 450, 250, 0, 300, 250
  };

  int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4
  };
  
  unsigned long previousMillis = 0;
  unsigned long pauseBetweenNotes;
  int thisNote;

  void buzina(){
    unsigned long currentMillis = millis();
    if (thisNote < 8 && currentMillis - previousMillis >= pauseBetweenNotes) {
      previousMillis = currentMillis;
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(PinBuzzer, melody[thisNote], noteDuration);
      pauseBetweenNotes = noteDuration * 1.30;
      thisNote++;
    }
    
  }
#endif

const byte address[6] = "00001";

typedef struct Meujoystick  {
  int  ry;
  int  rx;
  int  ly;
  int  lx;
  byte BtnA;
  bool BtnB;
} MeuJoystick;

MeuJoystick joystick;

void setup() {
  Serial.begin(9600);
  wdt_enable(WDTO_4S);
  #if radioID == 0
    pinMode(BTNB, INPUT_PULLUP);
       
    radio.begin();
    radio.setAutoAck(false);
    radio.setChannel(100);
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
  #else
    pinMode(PinMotorIN1, OUTPUT);
    pinMode(PinMotorIN2, OUTPUT);
    //pinMode(PinMotor1Pwm, OUTPUT);
    pinMode(PinMotorIN3, OUTPUT);
    pinMode(PinMotorIN4, OUTPUT);
    //pinMode(PinMotor2Pwm, OUTPUT);
    
    pinMode(PinFarol, OUTPUT);
    pinMode(PinBuzzer, OUTPUT);
    
    radio.begin();
    radio.setAutoAck(false);
    radio.setChannel(100);
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
  #endif

  // Inicializando variaveis da struct Meujoystick
  joystick.rx = 0;
  joystick.ry = 0;
  joystick.lx = 0;
  joystick.ly = 0;
  joystick.BtnA = 0;
  joystick.BtnB = 0;
}

void loop() {
  wdt_reset();
  #if radioID == 0
    TX();
  #else
    RX();
  #endif

  #if comentRadio == 1
    Serial.print("LX: ");
    Serial.print(joystick.lx);
    Serial.print(", LY: ");
    Serial.print(joystick.ly);
    Serial.print(", RX: ");
    Serial.print(joystick.rx);
    Serial.print(", RY: ");
    Serial.print(joystick.ry);
    Serial.print(", BTNA: ");
    Serial.print(joystick.BtnA);
    Serial.print(", BTNB: ");
    Serial.print(joystick.BtnB);
    Serial.print(" Nota: ");
    Serial.println(thisNote);
  #endif  
}

#if radioID == 0
  void TX(){
    botao1.button_loop();
  
    joystick.rx = analogRead(JOYSTICK_RX);
    joystick.ry = analogRead(JOYSTICK_RY);
    joystick.lx = analogRead(JOYSTICK_LX);
    joystick.ly = analogRead(JOYSTICK_LY);
    /*
     // Em caso de direção invertida
    joystick.rx = map (analogRead(JOYSTICK_RX), 1023, 0, 0, 1023);
    joystick.ry = map (analogRead(JOYSTICK_RY), 1023, 0, 0, 1023);
    joystick.lx = map (analogRead(JOYSTICK_LX), 1023, 0, 0, 1023);
    joystick.ly = map (analogRead(JOYSTICK_LY), 1023, 0, 0, 1023);
    */

    if(botao1.pressed()){
      btn_A = !btn_A;
      joystick.BtnA = btn_A;
    }

    if(!digitalRead(BTNB)){
      joystick.BtnB = 1;
    }else{
      joystick.BtnB = 0;
    }
    
    radio.write(&joystick, sizeof(joystick));
  }
#else
  void RX(){
    if (radio.available()) {
      radio.read(&joystick, sizeof(joystick));
      
      if (joystick.BtnA) {
        digitalWrite(PinFarol, HIGH);
      } else {
        digitalWrite(PinFarol, LOW);
      }

      if (joystick.BtnB) {
         buzina();
      } else {
        noTone(PinBuzzer);
        thisNote = 0;
      }
      
      //Sentido em que o carro irá andar - frente ou ré
     /* joystick.rx = map (joystick.rx, 0, 884, 255, -255);
      joystick.ry = map (joystick.ry, 0, 884, 255, -255);
      joystick.lx = map (joystick.lx, 0, 884, 255, -255);
      joystick.ly = map (joystick.ly, 0, 884, 255, -255);*/

      // Em caso de direção invertida
      joystick.rx = map (joystick.rx, 0, 1023, 255, -255);
      joystick.ry = map (joystick.ry, 1023, 0, 255, -255);
      joystick.lx = map (joystick.lx, 0, 1023, 255, -255);
      joystick.ly = map (joystick.ly, 1023, 0, 255, -255);

      char Direcao;
      if (joystick.ly > 40) {
        Direcao = 'U';
      } else if (joystick.ly < -40) {
        Direcao = 'D';
        joystick.ly = joystick.ly*-1;
      } else {
        Direcao = 'N';
        joystick.ly = 0;
      }
      if (joystick.lx > 40) {
        //Direcao = 'U';
      } else if (joystick.lx < -40) {
        joystick.lx = joystick.lx*-1;
      } else {
        joystick.lx = 0;
      }
      
      if (Direcao == 'U') {
        //analogWrite(PinMotor1Pwm, 255);
        digitalWrite(PinMotorIN1, LOW);
        digitalWrite(PinMotorIN2, HIGH);
      } else if(Direcao == 'D'){
        //analogWrite(PinMotor1Pwm, 255);
        digitalWrite(PinMotorIN1, HIGH);
        digitalWrite(PinMotorIN2, LOW);
      }else{
        digitalWrite(PinMotorIN1, LOW);
        digitalWrite(PinMotorIN2, LOW);
      }

      char Sentido;
      if (joystick.rx > 40) {
        Sentido = 'L';
      } else if (joystick.rx < -40) {
        Sentido = 'R';
        joystick.rx = joystick.rx*-1;
      } else {
        Sentido = 'N';
        joystick.rx = 0;
      }

      if (joystick.ry > 40) {
        //Sentido = 'L';
      } else if (joystick.ry < -40) {
        joystick.ry = joystick.ry*-1;
      } else {
        joystick.ry = 0;
      }
      
      if(Sentido == 'R'){
        //analogWrite(PinMotor2Pwm, joystick.rx);
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, HIGH); 
      } else if(Sentido == 'L'){
        //analogWrite(PinMotor2Pwm, joystick.rx);
        digitalWrite(PinMotorIN3, HIGH);
        digitalWrite(PinMotorIN4, LOW);
      }else{
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, LOW);
      }
      
    }
  }
#endif
