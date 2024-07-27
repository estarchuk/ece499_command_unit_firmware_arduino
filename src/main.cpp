#include <Arduino.h>
#include <RotaryEncoder.h>
#include <Wire.h>
#include <string>
#include <cstring>
#include <array>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>

#define encoder_b D7
#define encoder_a D6
#define nozzle_0 D2
#define nozzle_1 D3
#define nozzle_2 D4
#define nozzle_3 D5
#define button D8
#define uart_tx D10
#define uart_rx D9
#define lcd_sda D1
#define lcd_scl D0

int nozzle_state_0 = 0;
int nozzle_state_1 = 0;
int nozzle_state_2 = 0;
int nozzle_state_3 = 0;
int button_flag = 0;
int update_flag = 0;
int last_cursor_pos = 0;

RotaryEncoder *encoder = nullptr;
HardwareSerial uart_comm(1);
LiquidCrystal_I2C lcd(0x27,20,4);

int last_debounce_time = 0;
int debounce_time = 0;

void _button_isr(){
  debounce_time = millis();
  if((debounce_time - last_debounce_time) > 20){
    button_flag = 1;
  }
  last_debounce_time = debounce_time;
}
 
void checkPosition()
{
  encoder->tick(); // just call tick() to check the state.
}

void cursor_update(int line){

    for(int i = 0; i < 4; i++){
        lcd.setCursor(17, i);
        lcd.print(' ');
    }

    lcd.setCursor(17, line);
    lcd.print('*');    

}

void lcd_update(){
  std::string nozzle_0_status, nozzle_1_status, nozzle_2_status, nozzle_3_status;
  nozzle_0_status = "Nozzle 0: ";
  if(nozzle_state_0){
    nozzle_0_status.append("Open  ");
  } else {
    nozzle_0_status.append("Closed");
  }
  nozzle_1_status = "Nozzle 1: ";
  if(nozzle_state_1){
    nozzle_1_status.append("Open  ");
  } else {
    nozzle_1_status.append("Closed");
  }
  nozzle_2_status = "Nozzle 2: ";
  if(nozzle_state_2){
    nozzle_2_status.append("Open  ");
  } else {
    nozzle_2_status.append("Closed");
  }
  nozzle_3_status = "Nozzle 3: ";
  if(nozzle_state_3){
    nozzle_3_status.append("Open  ");
  } else {
    nozzle_3_status.append("Closed");
  }

  char * nozzle_0_out = new char [nozzle_0_status.length() + 1];
  char * nozzle_1_out = new char [nozzle_0_status.length() + 1];
  char * nozzle_2_out = new char [nozzle_0_status.length() + 1];
  char * nozzle_3_out = new char [nozzle_0_status.length() + 1];

  std::strcpy(nozzle_0_out, nozzle_0_status.c_str());
  std::strcpy(nozzle_1_out, nozzle_1_status.c_str());
  std::strcpy(nozzle_2_out, nozzle_2_status.c_str());
  std::strcpy(nozzle_3_out, nozzle_3_status.c_str());

  lcd.setCursor(0,0);
  lcd.printstr(nozzle_0_out);
  lcd.setCursor(0,1);
  lcd.printstr(nozzle_1_out);
  lcd.setCursor(0,2);
  lcd.printstr(nozzle_2_out);
  lcd.setCursor(0,3);
  lcd.printstr(nozzle_3_out);
}

void gpio_setup(){
  pinMode(nozzle_0, OUTPUT);
  pinMode(nozzle_1, OUTPUT);
  pinMode(nozzle_2, OUTPUT);
  pinMode(nozzle_3, OUTPUT);
  pinMode(button, INPUT);
  Wire.begin(lcd_sda, lcd_scl);
  encoder = new RotaryEncoder(encoder_b, encoder_a, RotaryEncoder::LatchMode::FOUR0); 
}

void uart_setup(){
  uart_comm.begin(115200, SERIAL_8N1 ,uart_rx, uart_tx);
}

void interrupt_setup(){
  attachInterrupt(digitalPinToInterrupt(button), _button_isr, LOW);
  attachInterrupt(digitalPinToInterrupt(encoder_b), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder_a), checkPosition, CHANGE);
}

void lcd_setup(){
  lcd.init();
  lcd.backlight();
  lcd_update();
  cursor_update(0);
}

void setup() {
  // Don't turn this on unless you have to
  //Serial.begin(115200); 
  gpio_setup();
  uart_setup();
  interrupt_setup();
  lcd_setup();
  
}


void loop() {
  static int pos = 0;

  if(last_cursor_pos != pos){
      cursor_update(pos);
      last_cursor_pos = pos;
  }

  int newPos = encoder->getPosition();
  if (pos != newPos) {
    if(newPos > 3){
      newPos = 3;
      encoder->setPosition(3);
    } else if(newPos < 0){
      newPos = 0;
      encoder->setPosition(0);
    }
    pos = newPos;
  } // if

  if(button_flag){
    button_flag = 0;
    update_flag = 1;
    int nozzle_select[1];
    nozzle_select[0] = 0;
    switch (pos)
    {
      case 0:
        digitalWrite(nozzle_0, !nozzle_state_0);
        nozzle_state_0 = !nozzle_state_0;
        nozzle_select[0] = 0;
        uart_comm.write(nozzle_select[0]);
        break;
      case 1:
        digitalWrite(nozzle_1, !nozzle_state_1);
        nozzle_state_1 = !nozzle_state_1;
        nozzle_select[0] = 1;
        uart_comm.write(nozzle_select[0]);
        break;
      case 2:
        digitalWrite(nozzle_2, !nozzle_state_2);
        nozzle_state_2 = !nozzle_state_2;
        nozzle_select[0] = 2;
        uart_comm.write(nozzle_select[0]);
        break;  
      case 3:
        digitalWrite(nozzle_3, !nozzle_state_3);
        nozzle_state_3 = !nozzle_state_3;
        nozzle_select[0] = 3;
        uart_comm.write(nozzle_select[0]);
        break;

      default:
        break;
    }
  }

  if(update_flag){
    update_flag = 0;
    lcd_update();
  }

}
