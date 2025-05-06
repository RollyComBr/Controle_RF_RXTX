#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <EEPROM.h>

#define radioID 1   //Informar "0" para Transmissor e "1" receptor
#define comentRadio 0 //Exibir comentário no monitor serial
int canalReceptor= 100;
#define CanalTX 1 //Posição da eeprom em que está salvo o valor do canal
int canalTransmissor = EEPROM.read(CanalTX);


#if radioID == 0
  #include <PS2X_lib.h>
  #include <MicroLCD.h>
#endif

#define PinCE   9 //Pino ao lado do Negativo
#define PinCSN  10 //Pino ao lado do positivo
//D11 MOSI
//D12 MISO
//D13 SCK
RF24 radio(PinCE, PinCSN);

unsigned long tempoOff;
int debounceBotao = 500;
unsigned long LastTimeVibra = millis();

#if radioID == 0 //Se estiver no modo Transmissor executa esses dados
  byte PS2_DAT = 8;  //Fio Marrom   DI
  byte PS2_CMD = 7;  //Fio Laranja  DO
  byte PS2_SEL = 6;  //Fio Amarelo  CS
  byte PS2_CLK = 5;  //Fio Azul     CLK
  //Vermelho 3.3v
  //Preto GND

  PS2X ps2x;
  LCD_SSD1306 display;
  #define PinMotorVibro 2

  int retornaZero(int mapa){
    if(mapa > -15 && mapa < 15){
      return 0;
    }else{
      return mapa;
    }
  }
  void menuPadrao();
  void menu0();
  void menu1();
  String zerador(int entrada){
    String zerar = "";
    if (entrada < 100 && entrada >= 10) {
      zerar += "0";
    } else if (entrada < 10) {
      zerar += "00";
    } else {
      zerar = "";
    }
    zerar += entrada;
    return zerar;
  }
  String valorEixo(int entrada){
    String zerar = "";
    if(entrada < 10 && entrada >= 0) {
      zerar += "   "; //3 espaços
    }else if((entrada > -100 && entrada <= -10) || (entrada >= 100)){
      zerar += " "; //1 espaço
    } else if((entrada < 100 && entrada >= 10) || (entrada > -10 && entrada < 0)) {
      zerar += "  "; //2 espaços
    }else{
      zerar += ""; //0 espaço
    }
    zerar += entrada;
    return zerar;
  }
//Imagens
  const PROGMEM uint8_t Sinal[]= {
    0x7e,
    0x5a,
    0x18,
    0x3c,
    0x18,
    0x18,
    0x18,
    0x18
  };

  const PROGMEM uint8_t selectBolOff[]= {
    0x3c,
    0x7e,
    0xe7,
    0xc3,
    0xc3,
    0xe7,
    0x7e,
    0x3c
  };

  const PROGMEM uint8_t selectBolOn[]= {
    0x3c,
    0x7e,
    0xff,
    0xff,
    0xff,
    0xff,
    0x7e,
    0x3c
  };

  const PROGMEM uint8_t selectTriOff[]= {
    0xe0,
    0xb8,
    0x8e,
    0x83,
    0x83,
    0x8e,
    0xb8,
    0xe0
  };

  const PROGMEM uint8_t selectTriOn[]= {
    0xe0,
    0xf8,
    0xfe,
    0xff,
    0xff,
    0xfe,
    0xf8,
    0xe0
  };

  const PROGMEM uint8_t selectXisOff[]= {
    0xc3,
    0xa5,
    0x5a,
    0x24,
    0x24,
    0x5a,
    0xa5,
    0xc3
  };

  const PROGMEM uint8_t selectXisOn[]= {
    0xc3,
    0xe7,
    0x7e,
    0x3c,
    0x3c,
    0x7e,
    0xe7,
    0xc3
  };

  const PROGMEM uint8_t selectQuaOff[]= {
    0xff,
    0xff,
    0xc3,
    0xc3,
    0xc3,
    0xc3,
    0xff,
    0xff
  };

  const PROGMEM uint8_t selectQuaOn[]= {
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff,
    0xff
  };

  const PROGMEM uint8_t selectDownOff[]= {
    0x18,
    0x28,
    0x4f,
    0x81,
    0x81,
    0x4f,
    0x28,
    0x18
  };

  const PROGMEM uint8_t selectDownOn[]= {
    0x18,
    0x38,
    0x7f,
    0xff,
    0xff,
    0x7f,
    0x38,
    0x18
  };

  const PROGMEM uint8_t selectUpOn[]= {
    0x18,
    0x1c,
    0xfe,
    0xff,
    0xff,
    0xfe,
    0x1c,
    0x18
  };

  const PROGMEM uint8_t selectUpOff[]= {
    0x18,
    0x14,
    0xf2,
    0x81,
    0x81,
    0xf2,
    0x14,
    0x18
  };

  const PROGMEM uint8_t selectLeftOff[]= {
    0x18,
    0x24,
    0x42,
    0x81,
    0xe7,
    0x24,
    0x24,
    0x3c
  };

  const PROGMEM uint8_t selectLeftOn[]= {
    0x18,
    0x3c,
    0x7e,
    0xff,
    0xff,
    0x3c,
    0x3c,
    0x3c
  };

  const PROGMEM uint8_t selectRightOff[]= {
    0x3c,
    0x24,
    0x24,
    0xe7,
    0x81,
    0x42,
    0x24,
    0x18
  };

  const PROGMEM uint8_t selectRightOn[]= {
    0x3c,
    0x3c,
    0x3c,
    0xff,
    0xff,
    0x7e,
    0x3c,
    0x18
  };

  const PROGMEM uint8_t antena[]= {
    0x00,
    0x03,
    0x09,
    0xff,
    0xff,
    0x09,
    0xc3,
    0xc0
  };

  const PROGMEM uint8_t sinalOn[]= {
    0xf0,
    0xf0,
    0x00,
    0xfc,
    0xfc,
    0x00,
    0xff,
    0xff
  };

  const PROGMEM uint8_t sinalOff[]= {
    0x80,
    0x80,
    0x00,
    0x80,
    0x80,
    0x00,
    0x80,
    0x80
  };
  
  const PROGMEM uint8_t selectAtivo[]= {
    0x00,
    0x4c,
    0x92,
    0x92,
    0x64,
    0x00,
    0x03,
    0x03
  };

  const PROGMEM uint8_t iconNull[]= {
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
  };

  const PROGMEM uint8_t Alfas_logo_nome [128 * 64 / 8] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F,
    0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0x7F, 0x3F, 0xCF, 0xE7, 0xF7, 0xFB, 0xF9, 0x7D, 0x7C, 0x7E, 0x3E, 0x3E, 0x3F, 0x3F,
    0x3F, 0x3F, 0x7F, 0x7F, 0xFE, 0xFE, 0xFE, 0xFC, 0x7D, 0x79, 0x7B, 0x77, 0x6F, 0x9F, 0x3F, 0x7F,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F,
    0x03, 0x03, 0x03, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFC, 0xFC, 0xF8, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x3F, 0x0F, 0x07, 0xC3, 0xF9, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF0, 0xF0, 0xF9, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x07, 0xF1, 0x7E, 0x1F, 0x0F, 0x07, 0x03, 0x81, 0xE0, 0xF8, 0xFC, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
    0xFE, 0xF8, 0x80, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0xE0, 0xF0, 0xFC, 0xFF, 0xFF, 0xFF, 0xFC,
    0xE1, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x0F, 0x83, 0xF0, 0xFC,
    0xFF, 0xF8, 0xC0, 0x00, 0x00, 0x07, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xCF,
    0xCF, 0x00, 0x00, 0x00, 0xC7, 0xCF, 0xCF, 0xCF, 0xCF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x1F, 0x8F,
    0x8F, 0xCF, 0xCF, 0xCF, 0x8F, 0x0F, 0x0F, 0x1F, 0x3F, 0xFF, 0x7F, 0x1F, 0x0F, 0x0F, 0x0F, 0x0F,
    0xCF, 0xFF, 0xFF, 0x7F, 0x1F, 0x1F, 0x8F, 0xEF, 0xE7, 0xE7, 0xE7, 0xE7, 0xC7, 0x0F, 0x0F, 0xFF,
    0xFE, 0x83, 0x00, 0x00, 0x00, 0x00, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x1F, 0x0F,
    0x07, 0x01, 0x80, 0xC4, 0xE0, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,
    0x87, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x0F, 0x83, 0xE0, 0xE4, 0xE7, 0xE7, 0xE7,
    0xE7, 0xE7, 0xE7, 0xE7, 0xE0, 0xC0, 0x00, 0x00, 0x07, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x3F, 0x0F, 0x03, 0x01, 0x00, 0xE0, 0xF8, 0xFC, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x38, 0x80, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF8, 0xFE, 0xFF,
    0xFF, 0xFF, 0xFF, 0xF8, 0xF0, 0xE0, 0xC1, 0x83, 0x87, 0x07, 0x0F, 0x0F, 0x1F, 0x1F, 0x3E, 0xFF,
    0xFF, 0xFF, 0xFE, 0xF8, 0xF2, 0xE4, 0xCC, 0xDC, 0xB8, 0xB8, 0x78, 0x7C, 0xFC, 0xFC, 0xFC, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7E, 0x7C, 0xBC, 0x98, 0xD8, 0xEC, 0xE4, 0xF0, 0xFC, 0xFE,
    0xFF, 0xFF, 0x3F, 0x3F, 0x3F, 0x1F, 0x0F, 0x01, 0x00, 0x1C, 0x3F, 0x3F, 0xBF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x3E, 0x10, 0x00, 0x00, 0x00, 0x07, 0x1F, 0x3F, 0x3F, 0x3F,
    0xFF, 0xBF, 0x3F, 0x3F, 0x1F, 0x00, 0x00, 0x00, 0x1F, 0x3F, 0x3F, 0xFF, 0xFF, 0xFF, 0x3F, 0x3F,
    0x1F, 0x00, 0x00, 0x00, 0x0F, 0x3F, 0x38, 0x20, 0xC0, 0x80, 0x82, 0x8F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x8F, 0x87, 0x87, 0xC3, 0xC0, 0xE0, 0xF0, 0xF9, 0xF8, 0xE0, 0xC0, 0x80, 0x81, 0x0F, 0x1F, 0x1F,
    0xBF, 0x9F, 0xFB, 0x81, 0x87, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3E, 0x88, 0x80, 0xC0, 0xF0,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFE,
    0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };



#else //Se estiver no modo Receptor executa esses dados
  byte PinBuzzer  =   A3;
  byte PinFarol   =   2;
  byte PinSetaE   =   A1;
  byte PinSetaD   =   A2;
  byte PinLed     =   7;

  byte PinMotorIN1 = 5;
  byte PinMotorIN2 = 6;
  byte PinMotorIN3 = 3;
  byte PinMotorIN4 = 4;

  int marchaAtual = 1;
  int velocidadeMotor;

  int SemSinal = 0;
  int tempoDePisca = 8; //Quando o bit 4 de millis ficar 1 o vaor e HIGH

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

const byte address[][6] = { "00001", "00002" };

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

typedef struct MeuReceptor { //receptor que altera os valores de quando o select está aivado. - R1 e L1 é exclusivo do transmissor e não é programável
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
  bool Bolinha;
  bool Xis;
  bool Start;
  bool Select;
  bool Sinal;
  int retorno = 1;
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

void setup() {
  #if comentRadio == 1
    Serial.begin(115200);
  #endif

  #if radioID == 0 //Se estiver no modo Transmissor executa esses dados
    display.begin();
    display.clear(); 
    pinMode(PinMotorVibro, OUTPUT);
    ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
       
    radio.begin();
    radio.setAutoAck(false);
    radio.setChannel(canalTransmissor);
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(address[0]);
    radio.openReadingPipe(1, address[1]);
    radio.setPALevel(RF24_PA_HIGH);
    radio.startListening();

    //Contagem de carga
    String teste = "Carregando";
    for(int j=0; j<10; j++){
      display.setCursor(0, 0);
      teste += ".";
      display.print(teste);
      delay(200);
    }
    display.clear();
    display.draw(Alfas_logo_nome, 128, 64);
    delay(3000);
    display.clear();
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
    radio.setChannel(canalReceptor);
    radio.setDataRate(RF24_250KBPS);
    radio.openWritingPipe(address[1]);
    radio.openReadingPipe(1, address[0]);
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
    if (joystick.Select) {
      if (ps2x.NewButtonState(PSB_R1)) {
        if (millis() - lastTimeButtonTime.R1 >= 300) {
          lastTimeButtonTime.R1 = millis();
          if (canalTransmissor <= 127) {
            canalTransmissor++;
          } else {
            canalTransmissor = 1;
          }
          EEPROM.write(CanalTX, canalTransmissor);
        }
        radio.setChannel(EEPROM.read(CanalTX));
      }
      if (ps2x.NewButtonState(PSB_L1)) {
        if (millis() - lastTimeButtonTime.L1 >= 300) {
          lastTimeButtonTime.L1 = millis();
          if (canalTransmissor >= 1) {
            canalTransmissor--;
          } else {
            canalTransmissor = 127;
          }
          EEPROM.write(CanalTX, canalTransmissor);
        }
        radio.setChannel(EEPROM.read(CanalTX));
      }

      menu1();
    }else{
      menu0();
    }
    menuPadrao();

    radio.stopListening();
    radio.write(&joystick, sizeof(joystick));
    radio.startListening();
    if (radio.available()) {
      radio.read(&receptor, sizeof(receptor));
    }
  }

  void menuPadrao(){
    display.setFontSize(FONT_SIZE_SMALL);
    display.setCursor(0, 0);
    String txtCanal = "Canal: " + zerador(EEPROM.read(CanalTX));
    txtCanal += "  R:" + receptor.retorno;
    display.print(txtCanal);
    display.setCursor(98, 0);
    display.draw(joystick.Select ? selectAtivo : iconNull, 8, 8);
    display.setCursor(108, 0);
    display.draw(antena, 8, 8);
    display.setCursor(116, 0);
    display.draw(receptor.Sinal ? sinalOn : sinalOff, 8, 8);
  }

  void menu0(){
    int coluna = 0;
    int linha = 2;
    display.setCursor(coluna, linha);
    display.draw(joystick.Triangulo ? selectTriOn : selectTriOff, 8, 8);
    display.setCursor(coluna+10, linha);
    display.draw(joystick.Bolinha ? selectBolOn : selectBolOff, 8, 8);
    display.setCursor(coluna+20, linha);
    display.draw(joystick.Quadrado ? selectQuaOn : selectQuaOff, 8, 8);
    display.setCursor(coluna+30, linha);
    display.draw(joystick.Xis ? selectXisOn : selectXisOff, 8, 8);
    
    display.setCursor(coluna+40, linha);
    display.draw(joystick.Cima ? selectUpOn : selectUpOff, 8, 8);
    display.setCursor(coluna+50, linha);
    display.draw(joystick.Baixo ? selectDownOn : selectDownOff, 8, 8);
    display.setCursor(coluna+60, linha);
    display.draw(joystick.Esquerda ? selectLeftOn : selectLeftOff, 8, 8);
    display.setCursor(coluna+70, linha);
    display.draw(joystick.Direita ? selectRightOn : selectRightOff, 8, 8);

    String txtUm="";
    String txtDois="";
    txtUm += "LX:";
    txtUm += valorEixo(joystick.LX);
    txtUm += " LY:";
    txtUm += valorEixo(joystick.LY);

    txtDois += "RX:";
    txtDois += valorEixo(joystick.RX);
    txtDois += " RY:";
    txtDois += valorEixo(joystick.RY);

    display.setCursor(0, 4);
    display.print(txtUm);
    display.setCursor(0, 6);
    display.print(txtDois);
  }

  void menu1(){
    int coluna = 0;
    int linha = 2;
    display.setCursor(coluna, linha);
    display.draw(receptor.Triangulo ? selectTriOn : selectTriOff, 8, 8);
    display.setCursor(coluna+10, linha);
    display.draw(receptor.Bolinha ? selectBolOn : selectBolOff, 8, 8);
    display.setCursor(coluna+20, linha);
    display.draw(receptor.Quadrado ? selectQuaOn : selectQuaOff, 8, 8);
    display.setCursor(coluna+30, linha);
    display.draw(receptor.Xis ? selectXisOn : selectXisOff, 8, 8);
    
    display.setCursor(coluna+40, linha);
    display.draw(receptor.Cima ? selectUpOn : selectUpOff, 8, 8);
    display.setCursor(coluna+50, linha);
    display.draw(receptor.Baixo ? selectDownOn : selectDownOff, 8, 8);
    display.setCursor(coluna+60, linha);
    display.draw(receptor.Esquerda ? selectLeftOn : selectLeftOff, 8, 8);
    display.setCursor(coluna+70, linha);
    display.draw(receptor.Direita ? selectRightOn : selectRightOff, 8, 8);

    String txtEspR = "R2:";
    txtEspR += receptor.R2  ? "ON " : "OFF";
    txtEspR += " R3:";
    txtEspR += receptor.R3  ? "ON " : "OFF";
    display.setCursor(0, 4);
    display.print(txtEspR);

    String txtEspL = "L2:";
    txtEspL += receptor.L2  ? "ON " : "OFF";
    txtEspL += " L3:";
    txtEspL += receptor.L3  ? "ON " : "OFF";
    display.setCursor(0, 6);
    display.print(txtEspL);
  }
#else //Se estiver no modo Receptor executa esses dados
  void dispositivo_RX(){
    if (radio.available()) {
      radio.read(&joystick, sizeof(joystick));

      //Funções do MODO SELECT
      if(!joystick.Select){ //Quando SELECT está desativado executa funções comum do carrinho
        if (joystick.Quadrado) {
          buzina();
        } else {
          noTone(PinBuzzer);
          thisNote = 0;
        }

        if(joystick.Triangulo){
          if(millis() - lastTimeButtonTime.Triangulo >= debounceBotao){
            lastTimeButtonTime.Triangulo = millis();
            receptor.Triangulo= !receptor.Triangulo;
          }
        }
        if(receptor.Triangulo){
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
            receptor.Bolinha= !receptor.Bolinha;
          }
        }
        if(receptor.Bolinha){
          digitalWrite(PinFarol,HIGH);
        }else{
          digitalWrite(PinFarol,LOW);
        }

        //Se apertar Bolinha uma vez muda o status do botao
        if(joystick.Xis){
          if(millis() - lastTimeButtonTime.Bolinha >= debounceBotao){
            lastTimeButtonTime.Bolinha = millis();
            receptor.Xis= !receptor.Xis;
          }
        }

        if(joystick.R2){
          if(millis() - lastTimeButtonTime.R2 >= debounceBotao){
            lastTimeButtonTime.R2 = millis();
            receptor.R2= !receptor.R2;
          }
        }
        if(receptor.R2){
          digitalWrite(PinSetaD,!bitRead(millis(), tempoDePisca));
        }else{
          digitalWrite(PinSetaD,LOW);
        }

        if(joystick.L2){
          if(millis() - lastTimeButtonTime.L2 >= debounceBotao){
            lastTimeButtonTime.L2 = millis();
            receptor.L2= !receptor.L2;
          }
        }
        if(receptor.L2){
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
          receptor.retorno = marchaAtual;
        }

        if(joystick.L1){
          if(millis() - lastTimeButtonTime.L1 >= debounceBotao){
            lastTimeButtonTime.L1 = millis();
            if(marchaAtual >= 2){
              marchaAtual--;
            } 
          }
          receptor.retorno = marchaAtual;
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

      radio.stopListening(); //Para de ler
      radio.write(&receptor, sizeof(receptor)); //Envia os dados
      radio.startListening(); //Volta a ler
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
        }
      }
    }
  }
#endif
