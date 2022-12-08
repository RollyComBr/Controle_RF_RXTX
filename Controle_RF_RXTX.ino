#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <neotimer.h>
#include <avr/wdt.h>

#define radioID 1   //Informar "0" para Transmissor e "1" receptor
#define comentRadio 0 //Exibir comentário no monitor serial

#if radioID == 0
  #include <PS2X_lib.h>
#endif

#define PinCE   9 //Amarelo
#define PinCSN  10 //Verde
//D11 MOSI
//D12 MISO
//D13 SCK
RF24 radio(PinCE, PinCSN);

Neotimer timer = Neotimer(500);

#if radioID == 0
  #define PS2_DAT        8  //Fio Marrom   
  #define PS2_CMD        7  //Fio Laranja
  #define PS2_SEL        6  //Fio Amarelo
  #define PS2_CLK        5  //Fio Azul
  //Vermelho 3.3v
  //Preto GND

  PS2X ps2x;

  int retornaZero(int mapa){
    if(mapa > -15 && mapa < 15){
      return 0;
    }else{
      return mapa;
    }
  }
#else
  #define PinBuzzer       A3
  #define PinFarol        A4
  #define PinSetaE        A1
  #define PinSetaD        A2
  
  //#define PinMotor1Pwm   6
  #define PinMotorIN1      7
  #define PinMotorIN2      8
  
  //#define PinMotor2Pwm   5
  #define PinMotorIN3      4
  #define PinMotorIN4      3

  Neotimer setaDireita = Neotimer (500);
  Neotimer setaEsquerda = Neotimer (500);
  Neotimer setaAlerta = Neotimer (500);

  int SemSinal = 0;
  int tempoDePisca = 4; //Quando o bit 4 de millis ficar 1 o vaor e HIGH

  int melody[] = {300, 250, 600, 450, 250, 0, 300, 250};

  int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
  
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
  bool  R1;
  bool  R2;
  bool  R3;
  int   RY;
  int   RX;
  bool  L1;
  bool  L2;
  bool  L3;
  int   LY;
  int   LX;
  bool  Cima;
  bool  Baixo;
  bool  Esquerda;
  bool  Direita;
  bool  Quadrado;
  bool  Triangulo;
  bool  Bolinha;
  bool  Xis;
  bool  Start;
  bool  Select;
} MeuJoystick;
MeuJoystick joystick;

void setup() {
  Serial.begin(9600);
  wdt_enable(WDTO_4S);
  #if radioID == 0
    ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
       
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
    pinMode(PinSetaE, OUTPUT);
    pinMode(PinSetaD, OUTPUT);
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
  joystick.R1        = false;
  joystick.R2        = false;
  joystick.R3        = false;
  joystick.RY        = 0;
  joystick.RX        = 0;
  joystick.L1        = false;
  joystick.L2        = false;
  joystick.L3        = false;
  joystick.LY        = 0;
  joystick.LX        = 0;
  joystick.Cima      = false;
  joystick.Baixo     = false;
  joystick.Esquerda  = false;
  joystick.Direita   = false;
  joystick.Quadrado  = false;
  joystick.Triangulo = false;
  joystick.Bolinha   = false;
  joystick.Xis       = false;
  joystick.Start     = false;
  joystick.Select    = false;
}

void loop() {
  wdt_reset();
  #if radioID == 0
    dispositivo_TX();
  #else
    dispositivo_RX();
  #endif

  #if comentRadio == 1
    Serial.print("LX: ");
    Serial.print(joystick.LX);
    Serial.print(", LY: ");
    Serial.print(joystick.LY);
    Serial.print(", RX: ");
    Serial.print(joystick.RX);
    Serial.print(", RY: ");
    Serial.print(joystick.RY);
    Serial.print(", Quadrado: ");
    Serial.print(joystick.Quadrado);
    Serial.print(", Bolinha: ");
    Serial.print(joystick.Bolinha);
    Serial.print(", Trinagulo: ");
    Serial.print(joystick.Triangulo);
    Serial.print(", Xis: ");
    Serial.print(joystick.Xis);
    Serial.print(" Start: ");
    Serial.println(joystick.Start);
  #endif  
}

#if radioID == 0
  void dispositivo_TX(){
    ps2x.read_gamepad(false, false);
    
    joystick.RX = retornaZero(map (ps2x.Analog(PSS_LY), 0, 255, 255, -255));
    joystick.RY = retornaZero(map (ps2x.Analog(PSS_LX), 0, 255, -255, 255));
    joystick.LX = retornaZero(map (ps2x.Analog(PSS_RY), 0, 255, 255, -255));
    joystick.LY = retornaZero(map (ps2x.Analog(PSS_RX), 0, 255, -255, 255));

    if(ps2x.ButtonPressed(PSB_START)){
      joystick.Start = !joystick.Start;
    }
    if(ps2x.ButtonPressed(PSB_SELECT)){
      joystick.Select = !joystick.Select;
    }
    if(ps2x.ButtonPressed(PSB_PAD_UP)){
      joystick.Cima = !joystick.Cima;
    }
    if(ps2x.ButtonPressed(PSB_PAD_DOWN)){
      joystick.Baixo = !joystick.Baixo;
    }
    if(ps2x.ButtonPressed(PSB_PAD_LEFT)){
      joystick.Esquerda = !joystick.Esquerda;
    }
    if(ps2x.ButtonPressed(PSB_PAD_RIGHT)){
      joystick.Direita = !joystick.Direita;
    }
    if(ps2x.ButtonPressed(PSB_CROSS)){
      joystick.Xis = !joystick.Xis;
    }
    if(ps2x.ButtonPressed(PSB_CIRCLE)){
      joystick.Bolinha = !joystick.Bolinha;
    }
    if(ps2x.NewButtonState(PSB_SQUARE)){
      joystick.Quadrado = !joystick.Quadrado;
    }
    if(ps2x.ButtonPressed(PSB_TRIANGLE)){
      joystick.Triangulo = !joystick.Triangulo;
    }
    if(ps2x.NewButtonState(PSB_R1)){
      joystick.R1 = !joystick.R1;
    }
    if(ps2x.ButtonPressed(PSB_R2)){
      joystick.R2 = !joystick.R2;
    }
    if(ps2x.ButtonPressed(PSB_R3)){
      joystick.R3 = !joystick.R3;
    }
    if(ps2x.NewButtonState(PSB_L1)){
      joystick.L1 = !joystick.L1;
    }
    if(ps2x.ButtonPressed(PSB_L2)){
      joystick.L2 = !joystick.L2;
    }
    if(ps2x.ButtonPressed(PSB_L3)){
      joystick.L3 = !joystick.L3;
    }
    
    radio.write(&joystick, sizeof(joystick));
  }
#else
  void dispositivo_RX(){
    if (radio.available()) {
      radio.read(&joystick, sizeof(joystick));
      
      if (joystick.Quadrado) {
         buzina();
         Serial.println("Biiiiiiiiiiiiii...");
      } else {
        noTone(PinBuzzer);
        thisNote = 0;
      }

      if(joystick.Triangulo){
        digitalWrite(PinSetaD,!bitRead(millis(), tempoDePisca));
        digitalWrite(PinSetaE,!bitRead(millis(), tempoDePisca));
      }else{
        digitalWrite(PinSetaE,LOW);
        digitalWrite(PinSetaD,LOW);
      }

      if(joystick.Bolinha){
        digitalWrite(PinFarol,HIGH);
      }else{
        digitalWrite(PinFarol,LOW);
      }

      if(joystick.R2){
        digitalWrite(PinSetaD,!bitRead(millis(), tempoDePisca));
      }else{
        digitalWrite(PinSetaD,LOW);
      }

      if(joystick.L2){
        digitalWrite(PinSetaE,!bitRead(millis(), tempoDePisca));
      }else{
        digitalWrite(PinSetaE,LOW);
      }

      char Direcao;
      if (joystick.LY > 20) {
        Direcao = 'U';
      } else if (joystick.LY < -20) {
        Direcao = 'D';
        joystick.LY = joystick.LY*-1;
      } else {
        Direcao = 'N';
        joystick.LY = 0;
      }
      
      if (Direcao == 'U') {
        //analogWrite(PinMotor1Pwm, joystick.LY);
        digitalWrite(PinMotorIN1, LOW);
        digitalWrite(PinMotorIN2, HIGH);
      } else if(Direcao == 'D'){
        //analogWrite(PinMotor1Pwm, joystick.LY);
        digitalWrite(PinMotorIN1, HIGH);
        digitalWrite(PinMotorIN2, LOW);
      }else{
        digitalWrite(PinMotorIN1, LOW);
        digitalWrite(PinMotorIN2, LOW);
      }

      char Sentido;
      if (joystick.RX > 20) {
        Sentido = 'L';
      } else if (joystick.RX < -20) {
        Sentido = 'R';
      } else {
        Sentido = 'N';
      }
      
      if(Sentido == 'R'){
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, HIGH); 
      } else if(Sentido == 'L'){
        digitalWrite(PinMotorIN3, HIGH);
        digitalWrite(PinMotorIN4, LOW);
      }else{
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, LOW);
      }
      timer.repeatReset();
      SemSinal=0;
    }else{
      if(timer.repeat()){
        SemSinal++;
        if(SemSinal >= 3){
          Serial.println("Sem Sinal");
          //Desliga o motor
          digitalWrite(PinMotorIN3, LOW);
          digitalWrite(PinMotorIN4, LOW);
          //Pisca alerta ativa
          digitalWrite(PinSetaE,!bitRead(millis(), tempoDePisca));
          digitalWrite(PinSetaD,!bitRead(millis(), tempoDePisca));
          digitalWrite(PinFarol, bitRead(millis(), tempoDePisca));
        }
      }
    }
  }
#endif
