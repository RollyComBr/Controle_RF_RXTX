#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>

#define radioID 0   //Informar "0" para Transmissor e "1" receptor
#define comentRadio 0 //Exibir comentário no monitor serial

#if radioID == 0
  #include <PS2X_lib.h>
#endif

#define PinCE   9 //Pino ao lado do Negativo
#define PinCSN  10 //Pino ao lado do positivo
//D11 MOSI
//D12 MISO
//D13 SCK
RF24 radio(PinCE, PinCSN);

unsigned long tempoOff;

#if radioID == 0 //Se estiver no modo Transmissor executa esses dados
  #define PS2_DAT        8  //Fio Marrom   
  #define PS2_CMD        7  //Fio Laranja
  #define PS2_SEL        6  //Fio Amarelo
  #define PS2_CLK        5  //Fio Azul
  //Vermelho 3.3v
  //Preto GND

  PS2X ps2x;
  #define PinMotorVibro 2

  int retornaZero(int mapa){
    if(mapa > -15 && mapa < 15){
      return 0;
    }else{
      return mapa;
    }
  }
#else //Se estiver no modo Receptor executa esses dados
  #define PinBuzzer       A3
  #define PinFarol        A4
  #define PinSetaE        A1
  #define PinSetaD        A2
  #define PinLed           2
  
  #define PinMotorIN1      5
  #define PinMotorIN2      6
  
  #define PinMotorIN3      3
  #define PinMotorIN4      4

  int marchaAtual = 1;
  int velocidadeMotor;

  int SemSinal = 0;
  int tempoDePisca = 2; //Quando o bit 4 de millis ficar 1 o vaor e HIGH

  int melody[] = {300, 250, 600, 450, 250, 0, 300, 250};

  int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};

  int debounceBotao = 500;
  
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

  typedef struct EstadoBotao  {
    bool  R1;
    bool  R2;
    bool  R3;
    bool  L1;
    bool  L2;
    bool  L3;
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
  } EstadoBotao;
  EstadoBotao estadoBotao;

  typedef struct LastTimeButtonTime  {
    unsigned long  R1 = millis();
    unsigned long  R2 = millis();
    unsigned long  R3 = millis();
    unsigned long  L1 = millis();
    unsigned long  L2 = millis();
    unsigned long  L3 = millis();
    unsigned long  Cima = millis();
    unsigned long  Baixo = millis();
    unsigned long  Esquerda = millis();
    unsigned long  Direita = millis();
    unsigned long  Quadrado = millis();
    unsigned long  Triangulo = millis();
    unsigned long  Bolinha = millis();
    unsigned long  Xis = millis();
    unsigned long  Start = millis();
    unsigned long  Select = millis();
  } LastTimeButtonTime;
  LastTimeButtonTime lastTimeButtonTime;

  typedef struct SelectAtivado  {
    bool  R1;
    bool  R2;
    bool  R3;
    bool  L1;
    bool  L2;
    bool  L3;
    bool  Cima;
    bool  Baixo;
    bool  Esquerda;
    bool  Direita;
    bool  Quadrado;
    bool  Triangulo;
    bool  Bolinha;
    bool  Xis;
    bool  Start;
  } SelectAtivado;
  SelectAtivado selectAtivado;
#endif

const byte address[6] = "00001";

typedef struct Meujoystick  {
  bool  R1 = false;
  bool  R2 = false;
  bool  R3 = false;
  int   RY = 0;
  int   RX = 0;
  bool  L1 = false;
  bool  L2 = false;
  bool  L3 = false;
  int   LY = 0;
  int   LX = 0;
  bool  Cima = false;
  bool  Baixo = false;
  bool  Esquerda = false;
  bool  Direita = false;
  bool  Quadrado = false;
  bool  Triangulo = false;
  bool  Bolinha = false;
  bool  Xis = false;
  bool  Start = false;
  bool  Select = false;
} MeuJoystick;
MeuJoystick joystick;

void setup() {
  #if comentRadio == 1
    Serial.begin(57600);
  #endif

  #if radioID == 0 //Se estiver no modo Transmissor executa esses dados
    pinMode(PinMotorVibro, OUTPUT);
    ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
       
    radio.begin();
    radio.setAutoAck(false);
    radio.setChannel(100);
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_HIGH);
    radio.stopListening();
  #else //Se estiver no modo Receptor executa esses dados
    pinMode(PinMotorIN1, OUTPUT);
    pinMode(PinMotorIN2, OUTPUT);
    pinMode(PinMotorIN3, OUTPUT);
    pinMode(PinMotorIN4, OUTPUT);
    
    pinMode(PinFarol, OUTPUT);
    pinMode(PinSetaE, OUTPUT);
    pinMode(PinSetaD, OUTPUT);
    pinMode(PinBuzzer, OUTPUT);
    pinMode(PinLed, OUTPUT);
    digitalWrite(PinLed,HIGH);
    
    radio.begin();
    radio.setAutoAck(false);
    radio.setChannel(100);
    radio.setDataRate(RF24_250KBPS);
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_LOW);
    radio.startListening();
  #endif
}

void loop() {
  #if radioID == 0
    dispositivo_TX();
  #else
    dispositivo_RX();
  #endif

  Serial.print("LX: ");
  Serial.print(joystick.LX);
  Serial.print(", LY: ");
  Serial.print(joystick.LY);
  Serial.print(", RX: ");
  Serial.print(joystick.RX);
  Serial.print(", RY: ");
  Serial.print(joystick.RY);
  Serial.print(", Cima: ");
  Serial.print(joystick.Cima);
  Serial.print(", Baixo: ");
  Serial.print(joystick.Baixo);
  Serial.print(", Esquerda: ");
  Serial.print(joystick.Esquerda);
  Serial.print(", Direita: ");
  Serial.print(joystick.Direita);
  Serial.print(", Quadrado: ");
  Serial.print(joystick.Quadrado);
  Serial.print(", Bolinha: ");
  Serial.print(joystick.Bolinha);
  Serial.print(", Triangulo: ");
  Serial.print(joystick.Triangulo);
  Serial.print(", Xis: ");
  Serial.print(joystick.Xis);
  Serial.print(", R1: ");
  Serial.print(joystick.R1);
  Serial.print(", R2: ");
  Serial.print(joystick.R2);
  Serial.print(", R3: ");
  Serial.print(joystick.R3);
  Serial.print(", L1: ");
  Serial.print(joystick.L1);
  Serial.print(", L2: ");
  Serial.print(joystick.L2);
  Serial.print(", L3: ");
  Serial.print(joystick.L3);
  #if radioID == 1 //Se estiver no modo Receptor executa esses dados
    Serial.print(", Marcha: ");
    Serial.print(marchaAtual);
    Serial.print(", VelocidadeMotor: ");
    Serial.print(velocidadeMotor);
  #endif 
  Serial.print(", Select: ");
  Serial.print(joystick.Select);
  Serial.print(" Start: ");
  Serial.println(joystick.Start);
}

#if radioID == 0 //Se estiver no modo Transmissor executa esses dados
  void dispositivo_TX(){
    ps2x.read_gamepad(false, false);
    
    joystick.LY = retornaZero(map (ps2x.Analog(PSS_LY), 0, 255, 255, -255));
    joystick.LX = retornaZero(map (ps2x.Analog(PSS_LX), 0, 255, -255, 255));
    joystick.RY = retornaZero(map (ps2x.Analog(PSS_RY), 0, 255, 255, -255));
    joystick.RX = retornaZero(map (ps2x.Analog(PSS_RX), 0, 255, -255, 255));

    //NewButtonState = clicar e soltar
    //ButtonPressed = Muda o estado e mantem. Se tava true, fica false e fica enviando o estado mantido

    if(ps2x.NewButtonState(PSB_START)){
      joystick.Start = !joystick.Start;
    }
    if(ps2x.NewButtonState(PSB_SELECT)){
      joystick.Select = !joystick.Select;
    }
    if(ps2x.NewButtonState(PSB_PAD_UP)){
      joystick.Cima = !joystick.Cima;
    }
    if(ps2x.NewButtonState(PSB_PAD_DOWN)){
      joystick.Baixo = !joystick.Baixo;
    }
    if(ps2x.NewButtonState(PSB_PAD_LEFT)){
      joystick.Esquerda = !joystick.Esquerda;
    }
    if(ps2x.NewButtonState(PSB_PAD_RIGHT)){
      joystick.Direita = !joystick.Direita;
    }
    if(ps2x.NewButtonState(PSB_CROSS)){
      joystick.Xis = !joystick.Xis;
    }
    if(ps2x.NewButtonState(PSB_CIRCLE)){
      joystick.Bolinha = !joystick.Bolinha;
    }
    if(ps2x.NewButtonState(PSB_SQUARE)){
      joystick.Quadrado = !joystick.Quadrado;
    }
    if(ps2x.NewButtonState(PSB_TRIANGLE)){
      joystick.Triangulo = !joystick.Triangulo;
    }
    if(ps2x.NewButtonState(PSB_R1)){
      joystick.R1 = !joystick.R1;
    }
    if(ps2x.NewButtonState(PSB_R2)){
      joystick.R2 = !joystick.R2;
    }
    if(ps2x.NewButtonState(PSB_R3)){
      joystick.R3 = !joystick.R3;
    }
    if(ps2x.NewButtonState(PSB_L1)){
      joystick.L1 = !joystick.L1;
    }
    if(ps2x.NewButtonState(PSB_L2)){
      joystick.L2 = !joystick.L2;
    }
    if(ps2x.NewButtonState(PSB_L3)){
      joystick.L3 = !joystick.L3;
    }

    //Motorzinho de celular ligado ao Pino 2 para dar feedback ao usuário ao entrar no modo SELECT
    if(joystick.Select){
      digitalWrite(PinMotorVibro,HIGH);
    }else{
      digitalWrite(PinMotorVibro,LOW);
    }
    
    radio.write(&joystick, sizeof(joystick));
  }
#else //Se estiver no modo Receptor executa esses dados
  void dispositivo_RX(){
    if (radio.available()) {
      radio.read(&joystick, sizeof(joystick));

      if(joystick.Select){
        if(millis() - lastTimeButtonTime.Triangulo >= debounceBotao){
          lastTimeButtonTime.Triangulo = millis();
          estadoBotao.Select= !estadoBotao.Select;
        }
      }
      //Funções do MODO SELECT
      if(!estadoBotao.Select){ //Quando SELECT está desativado executa funções comum do carrinho
        if (joystick.Quadrado) {
          buzina();
        } else {
          noTone(PinBuzzer);
          thisNote = 0;
        }

        if(joystick.Triangulo){
          if(millis() - lastTimeButtonTime.Triangulo >= debounceBotao){
            lastTimeButtonTime.Triangulo = millis();
            estadoBotao.Triangulo= !estadoBotao.Triangulo;
          }
        }
        if(estadoBotao.Triangulo){
          digitalWrite(PinSetaD,!bitRead(millis(), tempoDePisca));
          digitalWrite(PinSetaE,!bitRead(millis(), tempoDePisca));
        }else{
          digitalWrite(PinSetaE,LOW);
          digitalWrite(PinSetaD,LOW);
        }

        //Se apertar Bolinha uma vez muda o status do botao
        if(joystick.Bolinha){
          if(millis() - lastTimeButtonTime.Bolinha >= debounceBotao){
            lastTimeButtonTime.Bolinha = millis();
            estadoBotao.Bolinha= !estadoBotao.Bolinha;
          }
        }
        if(estadoBotao.Bolinha){
          digitalWrite(PinFarol,HIGH);
        }else{
          digitalWrite(PinFarol,LOW);
        }

        if(joystick.R2){
          if(millis() - lastTimeButtonTime.R2 >= debounceBotao){
            lastTimeButtonTime.R2 = millis();
            estadoBotao.R2= !estadoBotao.R2;
          }
        }
        if(estadoBotao.R2){
          digitalWrite(PinSetaD,!bitRead(millis(), tempoDePisca));
        }else{
          digitalWrite(PinSetaD,LOW);
        }

        if(joystick.L2){
          if(millis() - lastTimeButtonTime.L2 >= debounceBotao){
            lastTimeButtonTime.L2 = millis();
            estadoBotao.L2= !estadoBotao.L2;
          }
        }
        if(estadoBotao.L2){
          digitalWrite(PinSetaE,!bitRead(millis(), tempoDePisca));
        }else{
          digitalWrite(PinSetaE,LOW);
        }
        
        if(joystick.R1){
          if(millis() - lastTimeButtonTime.R1 >= debounceBotao){
            lastTimeButtonTime.R1 = millis();
            if(marchaAtual <= 4){
              marchaAtual++;
            } 
          }
        }

        if(joystick.L1){
          if(millis() - lastTimeButtonTime.L1 >= debounceBotao){
            lastTimeButtonTime.L1 = millis();
            if(marchaAtual >= 2){
              marchaAtual--;
            } 
          }
        }
        
        switch(marchaAtual){
          case 1:
          velocidadeMotor = 100;
          break;

          case 2:
          velocidadeMotor = 130;
          break;

          case 3:
          velocidadeMotor = 170;
          break;

          case 4:
          velocidadeMotor = 225;
          break;

          case 5:
          velocidadeMotor = 255;
          break;
        }

        
      }else{ //Se SELECT estiver ativado
        if(joystick.Bolinha){
          if(millis() - lastTimeButtonTime.Bolinha >= debounceBotao){
            lastTimeButtonTime.Bolinha = millis();
            selectAtivado.Bolinha= !selectAtivado.Bolinha;
          }
        }
        if(selectAtivado.Bolinha){
          digitalWrite(PinLed,LOW);
        }else{
          digitalWrite(PinLed,HIGH);
        }
      }

      //Mesmo com o SELECT ativo o Analógico continua funcionando LX, LY, RX e RY
      //DIREÇÃO
      if (joystick.LY > 20) { //FRENTE
        analogWrite(PinMotorIN1, velocidadeMotor);
        analogWrite(PinMotorIN2, LOW);
      } else if(joystick.LY < -20){ //RÉ
        analogWrite(PinMotorIN1, LOW);
        analogWrite(PinMotorIN2, velocidadeMotor);
      }else{
        digitalWrite(PinMotorIN1, LOW);
        digitalWrite(PinMotorIN2, LOW);
      }

      //SENTIDO
      if(joystick.RX > 20){ //DIREITA
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, 100); 
      } else if(joystick.RX < -20){ //ESQUERDA
        digitalWrite(PinMotorIN3, 100);
        digitalWrite(PinMotorIN4, LOW);
      }else{
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, LOW);
      }
      SemSinal=0;
    }else{
      if(millis() - tempoOff >= 500){
        SemSinal++;
        if(SemSinal >= 3){
          Serial.println("Sem Sinal");
          //Desliga o motor
          digitalWrite(PinMotorIN1, LOW);
          digitalWrite(PinMotorIN2, LOW);
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
