#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <EEPROM.h>
#include <avr/wdt.h>

#define radioID 1      //Informar "0" para Transmissor e "1" receptor
#define comentRadio 1  //Exibir comentário no monitor serial
#define CanalReceptor 100 //Define o canal do dispositivo receptor

#if radioID == 0
#include <PS2X_lib.h>
#include <MicroLCD.h>
#endif

#define PinCE 9    //Pino ao lado do Negativo
#define PinCSN 10  //Pino ao lado do positivo
//D11 MOSI
//D12 MISO
//D13 SCK
RF24 radio(PinCE, PinCSN);

unsigned long tempoOff;
int debounceBotao = 500;
unsigned long LastTimeVibra = millis();
int canal = EEPROM.read(150);

#if radioID == 0   //Se estiver no modo Transmissor executa esses dados
#define PS2_DAT 8  //Fio Marrom
#define PS2_CMD 7  //Fio Laranja
#define PS2_SEL 6  //Fio Amarelo
#define PS2_CLK 5  //Fio Azul
//Vermelho 3.3v
//Preto GND

PS2X ps2x;
LCD_SSD1306 lcd;
#define PinMotorVibro 2

int retornaZero(int mapa) {
  int PontoMortoControle = 15;
  if (mapa > -PontoMortoControle && mapa < PontoMortoControle) {
    return 0;
  } else {
    return mapa;
  }
}
#else  //Se estiver no modo Receptor executa esses dados
#define PinBuzzer A3
#define PinFarol 7
#define PinSetaE A1
#define PinSetaD A2
#define PinLed 2

#define PinMotorIN1 5
#define PinMotorIN2 6
#define PinMotorIN3 3
#define PinMotorIN4 4

int velocidadeMotor;
int marchaAtual = 1;
int SemSinal = 0;
int tempoDePisca = 8;  //Quando o bit 4 de millis ficar 1 o vaor e HIGH
int melody[] = { 300, 250, 600, 450, 250, 0, 300, 250 };
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };
unsigned long millisBuzina = 0;
unsigned long pausaEntreNotas;
int notaAtual;

void buzina() {
  unsigned long currentMillis = millis();
  if (notaAtual < sizeof(melody) && currentMillis - millisBuzina >= pausaEntreNotas) {
    millisBuzina = currentMillis;
    int noteDuration = 1000 / noteDurations[notaAtual];
    tone(PinBuzzer, melody[notaAtual], noteDuration);
    pausaEntreNotas = noteDuration * 1.30;
    notaAtual++;
  }
}

typedef struct SelectAtivado { //Estrutura de status dos botoes. Apenas no receptor
  bool R2;
  bool R3;
  bool L2;
  bool L3;
  bool Cima;
  bool Baixo;
  bool Esquerda;
  bool Direita;
  bool Quadrado;
  bool Triangulo;
  bool Bolinha; //Usado para o led interno
  bool Xis;
  bool Start;
} SelectAtivado;
SelectAtivado selectAtivado;

//Funções do PCF8574
#define qtdeCi  1
byte enderecosPCF8574[qtdeCi] = {32}; 
bool ciPinMode(byte pino, int modo) {
  static byte modoPinos[qtdeCi];
  if (modo == -1) {
     return bitRead(modoPinos[pino / 8], pino % 8); 
  } else {
     bitWrite(modoPinos[pino / 8], (pino % 8), modo);
     return modo;
  }
}
void ciWrite(byte pino, bool estado) {
  static bool inicio = true;
  static byte estadoPin[qtdeCi];
    if (inicio) {
       byte estadoCI;
       for (int nL = 0; nL < qtdeCi; nL++) {

           for (int nM = 0; nM < 8; nM++) {
               bitWrite(estadoCI, nM, !ciPinMode(nM + (nL * 8)) );  
           }
           estadoPin[nL] = estadoCI;
       }
       inicio = false;
    }
    bitWrite(estadoPin[pino / 8], pino % 8, estado);
    Wire.beginTransmission(enderecosPCF8574[pino / 8]);    
    Wire.write(estadoPin[pino / 8]);                            
    Wire.endTransmission();        
}
bool ciRead(byte pino) {
  byte lido;
  bool estado;
   Wire.requestFrom(enderecosPCF8574[pino / 8], 1);
   if(Wire.available()) {   
      lido = Wire.read();        
   }
   estado = bitRead(lido, pino % 8);
   return estado;  
}
#endif

typedef struct MeuReceptor {
  bool R1 = false;
  bool R2 = false; //Seta direita
  bool R3 = false;
  bool L1 = false;
  bool L2 = false; //Seta esquerda
  bool L3 = false;
  bool Cima = false;
  bool Baixo = false;
  bool Esquerda = false;
  bool Direita = false;
  bool Quadrado = false;
  bool Triangulo = false;
  bool Bolinha = false; //Ligar e desligar o farol
  bool Xis = false; //Buzina
  bool Start = false;
  bool Select = false;
  int MarchaAtual = 1;
} MeuReceptor;
MeuReceptor receptor;

typedef struct LastTimeButtonTime {
  unsigned long R1 = millis();
  unsigned long R2 = millis();
  unsigned long R3 = millis();
  unsigned long L1 = millis();
  unsigned long L2 = millis();
  unsigned long L3 = millis();
  unsigned long Cima = millis();
  unsigned long Baixo = millis();
  unsigned long Esquerda = millis();
  unsigned long Direita = millis();
  unsigned long Quadrado = millis();
  unsigned long Triangulo = millis();
  unsigned long Bolinha = millis();
  unsigned long Xis = millis();
  unsigned long Start = millis();
  unsigned long Select = millis();
} LastTimeButtonTime;
LastTimeButtonTime lastTimeButtonTime;

const byte address[][6] = { "00001", "00002" };

typedef struct Meujoystick {
  bool R1 = false;
  bool R2 = false;
  bool R3 = false;
  int RY = 0;
  int RX = 0;
  bool L1 = false;
  bool L2 = false;
  bool L3 = false;
  int LY = 0;
  int LX = 0;
  bool Cima = false;
  bool Baixo = false;
  bool Esquerda = false;
  bool Direita = false;
  bool Quadrado = false;
  bool Triangulo = false;
  bool Bolinha = false;
  bool Xis = false;
  bool Start = false;
  bool Select = false;
} MeuJoystick;
MeuJoystick joystick;

void setup() {
  wdt_enable(WDTO_2S);
#if comentRadio == 1
  Serial.begin(115200);
#endif

#if radioID == 0  //Se estiver no modo Transmissor executa esses dados
  lcd.begin();
  pinMode(PinMotorVibro, OUTPUT);
  ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);

  radio.begin();
  radio.setAutoAck(false);
  radio.setChannel(canal);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address[0]);
  radio.openReadingPipe(1, address[1]);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
#else  //Se estiver no modo Receptor executa esses dados
  pinMode(PinMotorIN1, OUTPUT);
  pinMode(PinMotorIN2, OUTPUT);
  pinMode(PinMotorIN3, OUTPUT);
  pinMode(PinMotorIN4, OUTPUT);

  pinMode(PinFarol, OUTPUT);
  pinMode(PinSetaE, OUTPUT);
  pinMode(PinSetaD, OUTPUT);
  pinMode(PinBuzzer, OUTPUT);
  pinMode(PinLed, OUTPUT);

  for (int nL=0; nL <= 7; nL++) { 
    ciPinMode(nL, OUTPUT);
  }

  digitalWrite(PinLed, HIGH);

  radio.begin();
  radio.setAutoAck(false);
  radio.setChannel(CanalReceptor);
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1, address[0]);
  radio.setPALevel(RF24_PA_HIGH);
  radio.startListening();
#endif
}

void loop() {
  wdt_reset();
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
  Serial.print(", Marcha: ");
  Serial.print(receptor.MarchaAtual);
  Serial.print(" Start: ");
  Serial.print(joystick.Start);
  Serial.print(" Select Ativado: ");
  Serial.println(joystick.Select);
}

#if radioID == 0  //Se estiver no modo Transmissor executa esses dados
void dispositivo_TX() {
  ps2x.read_gamepad(false, false);

  joystick.LY = retornaZero(map(ps2x.Analog(PSS_LY), 0, 255, 255, -255));
  joystick.LX = retornaZero(map(ps2x.Analog(PSS_LX), 0, 255, -255, 255));
  joystick.RY = retornaZero(map(ps2x.Analog(PSS_RY), 0, 255, 255, -255));
  joystick.RX = retornaZero(map(ps2x.Analog(PSS_RX), 0, 255, -255, 255));

  //NewButtonState = clicar e soltar
  //ButtonPressed = Muda o estado e mantem. Se tava true, fica false e fica enviando o estado mantido

  if (ps2x.NewButtonState(PSB_START)) {
    joystick.Start = !joystick.Start;
  }
  if (ps2x.NewButtonState(PSB_SELECT)) {
    if (millis() - lastTimeButtonTime.Select >= debounceBotao) {
        lastTimeButtonTime.Select = millis();
        joystick.Select = !joystick.Select;
    }
    if ((millis() - LastTimeVibra) >= 500) {
      LastTimeVibra = millis();
      digitalWrite(PinMotorVibro, HIGH);
    } else {
      digitalWrite(PinMotorVibro, LOW);
    }
  }
  if (ps2x.NewButtonState(PSB_PAD_UP)) {
    joystick.Cima = !joystick.Cima;
  }
  if (ps2x.NewButtonState(PSB_PAD_DOWN)) {
    joystick.Baixo = !joystick.Baixo;
  }
  if (ps2x.NewButtonState(PSB_PAD_LEFT)) {
    joystick.Esquerda = !joystick.Esquerda;
  }
  if (ps2x.NewButtonState(PSB_PAD_RIGHT)) {
    joystick.Direita = !joystick.Direita;
  }
  if (ps2x.NewButtonState(PSB_CROSS)) {
    joystick.Xis = !joystick.Xis;
  }
  if (ps2x.NewButtonState(PSB_CIRCLE)) {
    joystick.Bolinha = !joystick.Bolinha;
  }
  if (ps2x.NewButtonState(PSB_SQUARE)) {
    joystick.Quadrado = !joystick.Quadrado;
  }
  if (ps2x.NewButtonState(PSB_TRIANGLE)) {
    joystick.Triangulo = !joystick.Triangulo;
  }
  if (ps2x.NewButtonState(PSB_R1)) {
    joystick.R1 = !joystick.R1;
  }
  if (ps2x.NewButtonState(PSB_R2)) {
    joystick.R2 = !joystick.R2;
  }
  if (ps2x.NewButtonState(PSB_R3)) {
    joystick.R3 = !joystick.R3;
  }
  if (ps2x.NewButtonState(PSB_L1)) {
    joystick.L1 = !joystick.L1;
  }
  if (ps2x.NewButtonState(PSB_L2)) {
    joystick.L2 = !joystick.L2;
  }
  if (ps2x.NewButtonState(PSB_L3)) {
    joystick.L3 = !joystick.L3;
  }

  //Motorzinho de celular ligado ao Pino 2 para dar feedback ao usuário ao entrar no modo SELECT     
  if (joystick.Select) {
    if (ps2x.NewButtonState(PSB_R1)) {
      if (millis() - lastTimeButtonTime.R1 >= 300) {
        lastTimeButtonTime.R1 = millis();
        if (canal <= 127) {
          canal++;
        } else {
          canal = 1;
        }
        EEPROM.write(150, canal);
      }
      radio.setChannel(EEPROM.read(150));
    }
    if (ps2x.NewButtonState(PSB_L1)) {
      if (millis() - lastTimeButtonTime.L1 >= 300) {
        lastTimeButtonTime.L1 = millis();
        if (canal >= 1) {
          canal--;
        } else {
          canal = 127;
        }
        EEPROM.write(150, canal);
      }
      radio.setChannel(EEPROM.read(150));
    }
  }

  radio.stopListening();
  radio.write(&joystick, sizeof(joystick));
  radio.startListening();
  if (radio.available()) {
    radio.read(&receptor, sizeof(receptor));
  }

  lcd.setCursor(0, 0);
  lcd.setFontSize(FONT_SIZE_SMALL);
  String txtSelect = "Select: ";
  txtSelect += joystick.Select ? "ON " : "OFF";
  lcd.println(txtSelect);
  
  String zerar = "";  
  if(canal<100 && canal>=10){
    zerar += "0";
  }else if(canal<10){
    zerar += "00";
  }else{
    zerar = "";    
  }
  String txtCanal = "Canal: "+zerar;
  txtCanal += canal;
  lcd.println(txtCanal);

  String txtMarcha = "Marcha: ";
  txtMarcha += receptor.MarchaAtual;
  lcd.println(txtMarcha);
}
#else  //Se estiver no modo Receptor executa esses dados
void dispositivo_RX() {
  if (radio.available()) {
    radio.read(&joystick, sizeof(joystick));
    //Funções do MODO SELECT
    if (!joystick.Select) {  //Quando SELECT está desativado executa funções comum do carrinho
      if (joystick.Quadrado) {
        buzina();
      } else {
        noTone(PinBuzzer);
        notaAtual = 0;
      }

      if (joystick.Triangulo) {
        if (millis() - lastTimeButtonTime.Triangulo >= debounceBotao) {
          lastTimeButtonTime.Triangulo = millis();
          receptor.Triangulo = !receptor.Triangulo;
        }
      }
      if (receptor.Triangulo) {
        digitalWrite(PinSetaD, !bitRead(millis(), tempoDePisca));
        digitalWrite(PinSetaE, !bitRead(millis(), tempoDePisca));
      } else {
        digitalWrite(PinSetaE, LOW);
        digitalWrite(PinSetaD, LOW);
      }

      //Se apertar Bolinha uma vez muda o status do botao
      if (joystick.Bolinha) {
        if (millis() - lastTimeButtonTime.Bolinha >= debounceBotao) {
          lastTimeButtonTime.Bolinha = millis();
          receptor.Bolinha = !receptor.Bolinha;
        }
      }
      if (receptor.Bolinha) {
        digitalWrite(PinFarol, HIGH);
      } else {
        digitalWrite(PinFarol, LOW);
      }

      if (joystick.R2) {
        if (millis() - lastTimeButtonTime.R2 >= debounceBotao) {
          lastTimeButtonTime.R2 = millis();
          receptor.R2 = !receptor.R2;
        }
      }
      if (receptor.R2) {
        digitalWrite(PinSetaD, !bitRead(millis(), tempoDePisca));
      } else {
        digitalWrite(PinSetaD, LOW);
      }

      if (joystick.L2) {
        if (millis() - lastTimeButtonTime.L2 >= debounceBotao) {
          lastTimeButtonTime.L2 = millis();
          receptor.L2 = !receptor.L2;
        }
      }
      if (receptor.L2) {
        digitalWrite(PinSetaE, !bitRead(millis(), tempoDePisca));
      } else {
        digitalWrite(PinSetaE, LOW);
      }

      if (joystick.R1) {
        if (millis() - lastTimeButtonTime.R1 >= debounceBotao) {
          lastTimeButtonTime.R1 = millis();
          if (marchaAtual <= 4) {
            marchaAtual++;
          }
        }
        receptor.MarchaAtual = marchaAtual;
      }

      if (joystick.L1) {
        if (millis() - lastTimeButtonTime.L1 >= debounceBotao) {
          lastTimeButtonTime.L1 = millis();
          if (marchaAtual >= 2) {
            marchaAtual--;
          }
        }
        receptor.MarchaAtual = marchaAtual;
      }
      
      switch (marchaAtual) {
        case 1:
          velocidadeMotor = 135;
          break;
        case 2:
          velocidadeMotor = 165;
          break;
        case 3:
          velocidadeMotor = 195;
          break;
        case 4:
          velocidadeMotor = 225;
          break;
        case 5:
          velocidadeMotor = 255;
          break;
      }

    } else {  //Se SELECT estiver ativado
      if (joystick.Bolinha) {
        if (millis() - lastTimeButtonTime.Bolinha >= debounceBotao) {
          lastTimeButtonTime.Bolinha = millis();
          selectAtivado.Bolinha = !selectAtivado.Bolinha;
        }
      }
      if (selectAtivado.Bolinha) {
        digitalWrite(PinLed, LOW);
      } else {
        digitalWrite(PinLed, HIGH);
      }
    }

    //Mesmo com o SELECT ativo o Analógico continua funcionando LX, LY, RX e RY
    //DIREÇÃO
    int PontoMorto = 100; //Ponto morto do controle. Se o valor recebido for maior que PontoMorto, o motor aciona.
    if (joystick.LY > PontoMorto) {  //FRENTE
      analogWrite(PinMotorIN1, velocidadeMotor);
      digitalWrite(PinMotorIN2, LOW);
    } else if (joystick.LY < -PontoMorto) { //RÉ
      digitalWrite(PinMotorIN1, LOW);
      analogWrite(PinMotorIN2, velocidadeMotor);
    } else {
      digitalWrite(PinMotorIN1, LOW);
      digitalWrite(PinMotorIN2, LOW);
    }

    //SENTIDO
    if (joystick.RX > PontoMorto) {  //DIREITA
      digitalWrite(PinMotorIN3, LOW);
      analogWrite(PinMotorIN4, 150);
    } else if (joystick.RX < -PontoMorto) {  //ESQUERDA
      analogWrite(PinMotorIN3, 150);
      digitalWrite(PinMotorIN4, LOW);
    } else {
      digitalWrite(PinMotorIN3, LOW);
      digitalWrite(PinMotorIN4, LOW);
    }
    SemSinal = 0;

    radio.stopListening(); //Para de ler
    radio.write(&receptor, sizeof(receptor)); //Envia os dados
    radio.startListening(); //Volta a ler
  } else {
    if (millis() - tempoOff >= 500) {
      SemSinal++;
      if (SemSinal >= 3) {
        Serial.println("Sem Sinal");
        //Desliga o motor
        digitalWrite(PinMotorIN1, LOW);
        digitalWrite(PinMotorIN2, LOW);
        digitalWrite(PinMotorIN3, LOW);
        digitalWrite(PinMotorIN4, LOW);
      }
    }
  }
}
#endif
