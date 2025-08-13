#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <PCF8574.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


/*Buzzer Config*/
#define PIN_PWM_BUZZER 44
#define PWM_CHANNEL_BUZZER 1
#define PWM_RESOLUTION 8
int pwm_frequenz = 500;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/*Port Expander*/
PCF8574 pcf8574(0x38);
#define LED_ON false
#define LED_OFF true 

#define BUTTON_YELLOW 4
#define BUTTON_GREEN 6
#define BUTTON_RED 5
#define BUTTON_BLUE 7
const int Button[4] ={BUTTON_YELLOW, BUTTON_GREEN, BUTTON_RED, BUTTON_BLUE};

#define BUTTON_LED_YELLOW 0
#define BUTTON_LED_GREEN 2
#define BUTTON_LED_RED 1
#define BUTTON_LED_BLUE 3

const int ButtonLed[3][4] = {{BUTTON_YELLOW, BUTTON_GREEN, BUTTON_RED, BUTTON_BLUE},
  {BUTTON_LED_YELLOW, BUTTON_LED_GREEN, BUTTON_LED_RED, BUTTON_LED_BLUE},
  {400, 800, 1600, 3200}};

/*States*/
int state = 0;
#define STATE_INIT 0
#define STATE_START_GAME 1
#define STATE_PLAY_LEVEL 10
#define STATE_READ_LEVEL 11
#define STATE_GAME 50
#define STATE_GAME_OVER 51

int buttonPlayTime = 1000; //ms
int level = 0;
bool gameOver = false;
int gameValues[10] = {1,0,4,2,3,2,3,1,0,2};

void setButtonLeds(bool onOff);   //schaltet alle Lampen ein
void setButtonLed(int buttonLed, bool onOff);
void playButton(int button);
int readButton();


void setup() {

  ledcSetup(PWM_CHANNEL_BUZZER, pwm_frequenz, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM_BUZZER, PWM_CHANNEL_BUZZER);
  ledcWrite(PWM_CHANNEL_BUZZER, 0);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);

  if (pcf8574.begin()){
		//Serial.println("OK");
	}else{
		//Serial.println("failed");
	}
  /*Init Portexpander*/
  pcf8574.write8(0xFF); //Alle Pins auf High setzen.
  pcf8574.setButtonMask(0b11110000);
  delay(2000);
  Serial.begin();
}

void loop() {
  static int readLevel = 0;

  switch (state)
  {
  case STATE_INIT:
    level = 0;
    //setButtonLeds(LED_ON);
    //delay(500);
    setButtonLed(ButtonLed[1][0], LED_ON);
    delay(500);
    setButtonLed(ButtonLed[1][0], LED_OFF);
    delay(500);
    state = STATE_START_GAME;
    break;

  case STATE_START_GAME:
    setButtonLeds(LED_OFF);
    delay(500);
    state = STATE_PLAY_LEVEL;
    break;

  case STATE_PLAY_LEVEL:
      for (int i = 0; i <= level; i++)
      {
        playButton(gameValues[level]);
      }      
      state = STATE_READ_LEVEL;
    break;

  case STATE_READ_LEVEL:
      if(readLevel<= level){
        if(readButton() == gameValues[readLevel]){
          Serial.println("Korrekt");
          readLevel++;
        }else if(readButton() == -1){
          Serial.print(".");
        }else if(readButton() == 99){
          Serial.println("Falsch, Game Over");
          state = STATE_GAME_OVER;
        }
      }else{
        level++;
        state = STATE_PLAY_LEVEL;
      }     
    break;

  case STATE_GAME_OVER:
      state = STATE_INIT;
  default:
    break;
  }
  delay(200);
  // put your main code here, to run repeatedly:
}

void setButtonLeds(bool onOff){
    pcf8574.write(BUTTON_LED_YELLOW, onOff);
    delay(1000);
    pcf8574.write(BUTTON_LED_GREEN, onOff);
    delay(1000);
    pcf8574.write(BUTTON_LED_RED, onOff);
    delay(1000);
    pcf8574.write(BUTTON_LED_BLUE, onOff);
    delay(1000);
}

void setButtonLed(int buttonLed, bool onOff){
    pcf8574.write(buttonLed, onOff);
}

void playButton(int button){
    pcf8574.write(ButtonLed[1][button], LED_ON);
    ledcSetup(PWM_CHANNEL_BUZZER, ButtonLed[2][button], PWM_RESOLUTION);
    ledcAttachPin(PIN_PWM_BUZZER, PWM_CHANNEL_BUZZER);
    ledcWrite(PWM_CHANNEL_BUZZER, 125);
    delay(buttonPlayTime);
    pcf8574.write(ButtonLed[1][button], LED_OFF);
    ledcWrite(PWM_CHANNEL_BUZZER, 0);
}

int readButton(){
  int buttonvalue = 0;
  buttonvalue = pcf8574.read8() & 0b11110000;
  buttonvalue = buttonvalue >> 4;
  switch (buttonvalue)
  {
  case 14:
    return 0;
    break;
  case 11:
    return 1;
    break;
  case 13:
    return 2;
    break;
  case 7:
    return 3;
    break;
  case 15:
    return -1;
    break;
  default:
    return 99;
    break;
  }
}