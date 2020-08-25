// ###################################################################################
// # Project: 8WA Current Senser
// # Engineer:  Mic.Tsai
// # Date:  25 August 2020
// # Objective: Dev.board
// # Usage: ESP8266
// ###################################################################################
// # Mode select pin must serial 10k to GND (Floating will NOT trigger ADC work)
// ###################################################################################
// # Update Note:
// # Version 2.1: Save Data And Flash Dashboard By Minute
// ###################################################################################
// # Set Current Sensor Value
// # 100 (0.1ohm)
// ###################################################################################

float ADC0_A0_A1_Resistor = 0;   float ADC0_A0_A1_Resistor_Change = 115;
float ADC0_A2_A3_Resistor = 0;   float ADC0_A2_A3_Resistor_Change = 115;

float ADC1_A0_A1_Resistor = 0;   float ADC1_A0_A1_Resistor_Change = 30; 
float ADC1_A2_A3_Resistor = 0;   float ADC1_A2_A3_Resistor_Change = 20; 

float ADC2_A0_A1_Resistor = 0;   float ADC2_A0_A1_Resistor_Change = 30; 
float ADC2_A2_A3_Resistor = 0;   float ADC2_A2_A3_Resistor_Change = 20; 

float ADC3_A0_A1_Resistor = 0;   float ADC3_A0_A1_Resistor_Change = 30; 
float ADC3_A2_A3_Resistor = 0;   float ADC3_A2_A3_Resistor_Change = 20; 

// ###################################################################################
// ###################################################################################

#include "ADS1115.h"

//Declare 4x adc board as following I2C address.
ADS1115 adc0(0x4B);
ADS1115 adc1(0x4A);
ADS1115 adc2(0x48);
ADS1115 adc3(0x49); // NEED CONNECT 2x MORE ADS1115, SINGLE ADC3 will not recognize the addrss.


//Mode change declaration
const int  buttonPin = 10;    // Mode change # Mode select pin must serial 10k to GND (Floating will NOT trigger ADC work)
int sensorValue;

//Variables will change:
int buttonPushCounterDigi = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

//Analog used
int buttonPushCounter = 0;   // counter for the number of button presses

//Display ST7789 130x160
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_RST   2     // TFT RST pin is connected to NodeMCU pin D4 (GPIO2)
#define TFT_CS    0     // TFT CS  pin is connected to NodeMCU pin D4 (GPIO0)
#define TFT_DC    16 

// HARDWARE SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789( TFT_CS, TFT_DC, TFT_RST);

// Set fonts
#include <stdio.h>
#include <Fonts/FreeSansBoldOblique9pt7b.h>

// Set SD card
#include <SD.h>
const int chipSelect = 15;

// Set print_time
unsigned long print_time;

// Set for sd logging time please change the value here.
int sd_log_time=60000;
int display_update_time=50000;

// Set for sd logging time
int TimerLast=0;
int TimerNow=0;
int Timer=60000;

// Set for display update time
int TimerLast2=0;
int TimerNow2=0;
int Timer2=0;

// -----------------------------------------------------------------------------
// Arduino-Setup
// -----------------------------------------------------------------------------

void setup() {   
   
    //I2Cdev::begin();  // join I2C bus
    Wire.begin(5,4);
    Serial.begin(9600); // initialize serial communication 
    Serial.println("Testing device connections...");
    delay(200);

    Serial.println(adc0.testConnection() ? "ADS1115-adc0 connection successful" : "ADS1115-adc0 connection failed");
    Serial.println(adc1.testConnection() ? "ADS1115-adc1 connection successful" : "ADS1115-adc1 connection failed");
    Serial.println(adc2.testConnection() ? "ADS1115-adc2 connection successful" : "ADS1115-adc2 connection failed");
    Serial.println(adc3.testConnection() ? "ADS1115-adc3 connection successful" : "ADS1115-adc3 connection failed");

    // see if the card is present and can be initialized:
    Serial.print("Initializing SD card...");
    Serial.println(SD.begin(chipSelect) ? "card initialized." : "Card failed, or not present");

    // initialize ADS1115 16 bit A/D chip
    adc0.initialize(); 
    adc1.initialize();
    adc2.initialize();
    adc3.initialize();

    // Continuous sampling
    adc0.setMode(ADS1115_MODE_CONTINUOUS);
    adc1.setMode(ADS1115_MODE_CONTINUOUS);
    adc2.setMode(ADS1115_MODE_CONTINUOUS);
    adc3.setMode(ADS1115_MODE_CONTINUOUS);

    // Slow things down so that we can see that the "poll for conversion" code works
    adc0.setRate(ADS1115_RATE_250);
    adc1.setRate(ADS1115_RATE_250);
    adc2.setRate(ADS1115_RATE_250);
    adc3.setRate(ADS1115_RATE_250);

    //Mode change button input
    pinMode(buttonPin, INPUT);
    buttonPushCounter = 0;
    buttonPushCounterDigi = 0;

    //Display show...
    Serial.print(F("Hello! ST77xx TFT Test"));
    tft.init(130 , 162);  // 128x160 will show garbage image in corner.
    Serial.println(F("Initialized"));

    uint16_t time = millis();
    tft.fillScreen(ST77XX_BLACK);
    time = millis() - time;
    
    Serial.println(time, DEC);
    delay(500);
    tft.setTextWrap(false);
    tft.invertDisplay(false);      
    tft.fillScreen(ST77XX_BLACK);
    tft.setFont(&FreeSansBoldOblique9pt7b);
      
    // Display Project Infomation
    tft.setTextColor(ST77XX_CYAN); tft.setTextSize(1); tft.setCursor(1, 20);  tft.println("PowerCUMP");
    tft.setFont();
    tft.setTextColor(ST77XX_CYAN); tft.setTextSize(1); tft.setCursor(2, 31);  tft.println("Current Senser v2"); delay(1000);
    tft.setTextColor(ST77XX_CYAN); tft.setTextSize(1); tft.setCursor(62, 136);  tft.println("Engineer");
    tft.setTextColor(ST77XX_CYAN); tft.setTextSize(1); tft.setCursor(77, 150);  tft.println("Mic.Tsai"); delay(1000);
    tft.invertDisplay(false); delay(1000);
    tft.fillScreen(ST77XX_BLACK);
    tft.setFont();

    Serial.println("System Installation OK:");


    ADC0_A0_A1_Resistor = ADC0_A0_A1_Resistor_Change /1000 ; ADC0_A2_A3_Resistor = ADC0_A2_A3_Resistor_Change /1000 ;
    Serial.println(ADC0_A0_A1_Resistor_Change, 4); Serial.println(ADC0_A2_A3_Resistor_Change, 4);

    ADC1_A0_A1_Resistor = ADC1_A0_A1_Resistor_Change /1000 ; ADC1_A2_A3_Resistor = ADC1_A2_A3_Resistor_Change /1000 ;
    Serial.println(ADC1_A0_A1_Resistor_Change, 4); Serial.println(ADC1_A2_A3_Resistor_Change, 4);

    ADC2_A0_A1_Resistor = ADC2_A0_A1_Resistor_Change /1000 ; ADC2_A2_A3_Resistor = ADC2_A2_A3_Resistor_Change /1000 ;
    Serial.println(ADC2_A0_A1_Resistor_Change, 4); Serial.println(ADC2_A2_A3_Resistor_Change, 4);

    ADC3_A0_A1_Resistor = ADC3_A0_A1_Resistor_Change /1000 ; ADC3_A2_A3_Resistor = ADC3_A2_A3_Resistor_Change /1000 ;
    Serial.println(ADC3_A0_A1_Resistor_Change, 4); Serial.println(ADC3_A2_A3_Resistor_Change, 4);



    // SD card data log for Start Label
    SD.begin(chipSelect);
    File dataFile = SD.open("Log-ADC0-CH1-CH2.txt", FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) {
        
        dataFile.println("Power Consumption Start Recording:");
        dataFile.close();
        Serial.print("| Log SD done!");
        
        //Print_time
        print_time = millis();
        Serial.print(" "); Serial.print(print_time); Serial.print("ms ");

        tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
        tft.fillRect(100+2, 10+1, 25, 10, ST77XX_BLACK);
        tft.setCursor(100+2, 10+1 ); tft.print("OK");
     }
        // if the file isn't open, pop up an error:
        else {
          Serial.println("| error opening datalog.txt");
        
          tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
          tft.fillRect(100+2, 10+1, 25, 10, ST77XX_BLACK);
          tft.setCursor(100+2, 10+1 ); tft.print("FAIL");        
          }  
}

// -----------------------------------------------------------------------------
// Arduino-Loop
// -----------------------------------------------------------------------------

void loop() {
  
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Status change mode

  buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      buttonPushCounterDigi++;
      tft.fillScreen(ST77XX_BLACK);
    }
    delay(10);
  } 
  lastButtonState = buttonState;
//  Serial.println(buttonPushCounterDigi);
  
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  if (buttonPushCounterDigi > 1) {
    buttonPushCounterDigi = 0;
  }
  if (buttonPushCounterDigi < 0) {
    buttonPushCounterDigi = 1;
  }
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  // 4x button reading
  sensorValue = analogRead(A0);

  if (sensorValue <= 430){
      delay(30);        // Reads for stability

      if (sensorValue <= 10){   
      delay(30);        // Reads for stability

                  if ( buttonPushCounter == 0 && buttonPushCounterDigi == 0){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC0_A0_A1_Resistor = ADC0_A0_A1_Resistor + (0.001) ;
                    Serial.print("ADC0_A0_A1_Resistor:");Serial.println(ADC0_A0_A1_Resistor,3);
                    Serial.println("  ");
                  }
                  if ( buttonPushCounter == 0 && buttonPushCounterDigi == 1){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC0_A2_A3_Resistor = ADC0_A2_A3_Resistor + (0.001) ;
                    Serial.print("ADC0_A2_A3_Resistor:");Serial.println(ADC0_A2_A3_Resistor,3);
                    Serial.println("  ");
                  } 


                 if ( buttonPushCounter == 1 && buttonPushCounterDigi == 0){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC1_A0_A1_Resistor = ADC1_A0_A1_Resistor + (0.001) ;
                    Serial.print("ADC1_A0_A1_Resistor:");Serial.println(ADC1_A0_A1_Resistor,3);
                    Serial.println("  ");
                  }
                 if ( buttonPushCounter == 1 && buttonPushCounterDigi == 1){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC1_A2_A3_Resistor = ADC1_A2_A3_Resistor + (0.001) ;
                    Serial.print("ADC1_A2_A3_Resistor:");Serial.println(ADC1_A2_A3_Resistor,3);
                    Serial.println("  ");
                  } 
                  

                 if ( buttonPushCounter == 2 && buttonPushCounterDigi == 0){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC2_A0_A1_Resistor = ADC2_A0_A1_Resistor + (0.001) ;
                    Serial.print("ADC2_A0_A1_Resistor:");Serial.println(ADC2_A0_A1_Resistor,3);
                    Serial.println("  ");
                  }
                 if ( buttonPushCounter == 2 && buttonPushCounterDigi == 1){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC2_A2_A3_Resistor = ADC2_A2_A3_Resistor + (0.001) ;
                    Serial.print("ADC2_A2_A3_Resistor:");Serial.println(ADC2_A2_A3_Resistor,3);
                    Serial.println("  ");
                  } 


                 if ( buttonPushCounter == 3 && buttonPushCounterDigi == 0){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC3_A0_A1_Resistor = ADC3_A0_A1_Resistor + (0.001) ;
                    Serial.print("ADC3_A0_A1_Resistor:");Serial.println(ADC3_A0_A1_Resistor,3);
                    Serial.println("  ");
                  }
                 if ( buttonPushCounter == 3 && buttonPushCounterDigi == 1){
                  Serial.println("  ");
                  Serial.print("UP Button Pressed: value:"); Serial.println(sensorValue);
                    ADC3_A2_A3_Resistor = ADC3_A2_A3_Resistor + (0.001) ;
                    Serial.print("ADC3_A2_A3_Resistor:");Serial.println(ADC3_A2_A3_Resistor,3);
                    Serial.println("  ");
                  } 

      }
               
      else if (sensorValue <= 170){
      delay(30);        // Reads for stability
      Serial.println("  ");
      Serial.print("RIGHT Button Pressed: value:"); Serial.println(sensorValue);
      buttonPushCounter++;
      tft.fillScreen(ST77XX_BLACK);
      } 

      else  if (sensorValue <= 290){   
      delay(30);        // Reads for stability
        
                          if ( buttonPushCounter == 0 && buttonPushCounterDigi == 0){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC0_A0_A1_Resistor = ADC0_A0_A1_Resistor - (0.001) ;
                            Serial.print("ADC0_A0_A1_Resistor:");Serial.println(ADC0_A0_A1_Resistor,3);
                            Serial.println("  ");
                          }
                          if ( buttonPushCounter == 0 && buttonPushCounterDigi == 1){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC0_A2_A3_Resistor = ADC0_A2_A3_Resistor - (0.001) ;
                            Serial.print("ADC0_A2_A3_Resistor:");Serial.println(ADC0_A2_A3_Resistor,3);
                            Serial.println("  ");
                          } 
        
        
                         if ( buttonPushCounter == 1 && buttonPushCounterDigi == 0){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC1_A0_A1_Resistor = ADC1_A0_A1_Resistor - (0.001) ;
                            Serial.print("ADC1_A0_A1_Resistor:");Serial.println(ADC1_A0_A1_Resistor,3);
                            Serial.println("  ");
                          }
                         if ( buttonPushCounter == 1 && buttonPushCounterDigi == 1){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC1_A2_A3_Resistor = ADC1_A2_A3_Resistor - (0.001) ;
                            Serial.print("ADC1_A2_A3_Resistor:");Serial.println(ADC1_A2_A3_Resistor,3);
                            Serial.println("  ");
                          } 

                          
                         if ( buttonPushCounter == 2 && buttonPushCounterDigi == 0){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC2_A0_A1_Resistor = ADC2_A0_A1_Resistor - (0.001) ;
                            Serial.print("ADC2_A0_A1_Resistor:");Serial.println(ADC2_A0_A1_Resistor,3);
                            Serial.println("  ");
                          }
                         if ( buttonPushCounter == 2 && buttonPushCounterDigi == 1){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC2_A2_A3_Resistor = ADC2_A2_A3_Resistor - (0.001) ;
                            Serial.print("ADC2_A2_A3_Resistor:");Serial.println(ADC2_A2_A3_Resistor,3);
                            Serial.println("  ");
                          } 

                         if ( buttonPushCounter == 3 && buttonPushCounterDigi == 0){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC3_A0_A1_Resistor = ADC3_A0_A1_Resistor - (0.001) ;
                            Serial.print("ADC3_A0_A1_Resistor:");Serial.println(ADC3_A0_A1_Resistor,3);
                            Serial.println("  ");
                          }
                         if ( buttonPushCounter == 3 && buttonPushCounterDigi == 1){
                          Serial.println("  ");
                          Serial.print("DOWN Button Pressed: value:"); Serial.println(sensorValue);
                            ADC3_A2_A3_Resistor = ADC3_A2_A3_Resistor - (0.001) ;
                            Serial.print("ADC3_A2_A3_Resistor:");Serial.println(ADC3_A2_A3_Resistor,3);
                            Serial.println("  ");
                          } 
                          
      }

      else if (sensorValue <= 380 && sensorValue >= 360){
      delay(30);
      Serial.println("  ");
      Serial.print("LEFT  Button Pressed: value:"); Serial.println(sensorValue);
      buttonPushCounter--;
      tft.fillScreen(ST77XX_BLACK);
      }
      while (sensorValue <= 380){s
      sensorValue = analogRead(A0);
      delay(20);
    }
   }

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  //Mode 0, 1, 2 (Over 5 will be set to zero)

  if (buttonPushCounter > 5) {
    buttonPushCounter = 0;
  }
  if (buttonPushCounter < 0) {
    buttonPushCounter = 5;
  }
  
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Set timer as minutes to run follow script

    TimerNow = millis();
    Timer = TimerNow - TimerLast;
    
    TimerNow2 = millis();
    Timer2 = TimerNow2 - TimerLast2;
    
  if (Timer2 >1000 && buttonPushCounter == 0) {
    ADC0_CH1_CH2();
    TimerLast2 = TimerNow2;
  }
  if (Timer2 >1000 && buttonPushCounter == 1) {
    ADC1_CH1_CH2();
    TimerLast2 = TimerNow2;
  }
  if (Timer2 >1000 && buttonPushCounter == 2) {
    ADC2_CH1_CH2();
    TimerLast2 = TimerNow2;
  }
  if (Timer2 >1000 && buttonPushCounter == 3) {
    ADC3_CH1_CH2();
    TimerLast2 = TimerNow2;
  }
  if (Timer2 >1000 && buttonPushCounter == 4) {
    ADC_ALL();
    TimerLast2 = TimerNow2;
  }  
  if (Timer2 >1000 && buttonPushCounter == 5) {
    ADC_ALL2();
    TimerLast2 = TimerNow2;
  }

}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Display 
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// -----------------------------------------------------------------------------
// Different Group function
// -----------------------------------------------------------------------------

void ADC_ALL()
{
// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc0.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC0_CH1 = 0;
    float IL_ADC0_CH1 = 0;
    int sensorOneCounts_ADC0_CH1 = adc0.getConversionP0N1(); 
    DiffV_ADC0_CH1 = sensorOneCounts_ADC0_CH1*adc0.getMvPerCount();
    IL_ADC0_CH1 = DiffV_ADC0_CH1 / ADC0_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC0_CH1;
    adc0.setGain(ADS1115_PGA_6P144); adc0.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC0_CH1 = adc0.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC0_CH1 = SingleEnd_ADC0_CH1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC0_CH1;
    PL_ADC0_CH1 = IL_ADC0_CH1 * SingleEnd_ADC0_CH1 ;  //  mA * V = mW

// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc0.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC0_CH2 = 0;
    float IL_ADC0_CH2 = 0;
    int sensorOneCounts_ADC0_CH2 = adc0.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC0_CH2 = sensorOneCounts_ADC0_CH2*adc0.getMvPerCount();
    IL_ADC0_CH2 = DiffV_ADC0_CH2 / ADC0_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC0_CH2;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC0_CH2 = adc0.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC0_CH2 = SingleEnd_ADC0_CH2*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC0_CH2;
    PL_ADC0_CH2 = IL_ADC0_CH2 * SingleEnd_ADC0_CH2 ;  //  mA * V = mW

// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc1.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC1_CH1 = 0;
    float IL_ADC1_CH1 = 0;
    int sensorOneCounts_ADC1_CH1 = adc1.getConversionP0N1(); 
    DiffV_ADC1_CH1 = sensorOneCounts_ADC1_CH1*adc1.getMvPerCount();
    IL_ADC1_CH1 = DiffV_ADC1_CH1 / ADC1_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC1_CH1;
    adc1.setGain(ADS1115_PGA_6P144); adc1.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC1_CH1 = adc1.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC1_CH1 = SingleEnd_ADC1_CH1*adc1.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC1_CH1;
    PL_ADC1_CH1 = IL_ADC1_CH1 * SingleEnd_ADC1_CH1 ;  //  mA * V = mW

// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc1.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC1_CH2 = 0;
    float IL_ADC1_CH2 = 0;
    int sensorOneCounts_ADC1_CH2 = adc1.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC1_CH2 = sensorOneCounts_ADC1_CH2*adc1.getMvPerCount();
    IL_ADC1_CH2 = DiffV_ADC1_CH2 / ADC1_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC1_CH2;
    adc1.setGain(ADS1115_PGA_6P144);
    adc1.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC1_CH2 = adc1.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC1_CH2 = SingleEnd_ADC1_CH2*adc1.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC1_CH2;
    PL_ADC1_CH2 = IL_ADC1_CH2 * SingleEnd_ADC1_CH2 ;  //  mA * V = mW

// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc2.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC2_CH1 = 0;
    float IL_ADC2_CH1 = 0;
    int sensorOneCounts_ADC2_CH1 = adc2.getConversionP0N1(); 
    DiffV_ADC2_CH1 = sensorOneCounts_ADC2_CH1*adc2.getMvPerCount();
    IL_ADC2_CH1 = DiffV_ADC2_CH1 / ADC2_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC2_CH1;
    adc2.setGain(ADS1115_PGA_6P144); adc2.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC2_CH1 = adc2.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC2_CH1 = SingleEnd_ADC2_CH1*adc2.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC2_CH1;
    PL_ADC2_CH1 = IL_ADC2_CH1 * SingleEnd_ADC2_CH1 ;  //  mA * V = mW

// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc2.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC2_CH2 = 0;
    float IL_ADC2_CH2 = 0;
    int sensorOneCounts_ADC2_CH2 = adc2.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC2_CH2 = sensorOneCounts_ADC2_CH2*adc2.getMvPerCount();
    IL_ADC2_CH2 = DiffV_ADC2_CH2 / ADC2_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC2_CH2;
    adc2.setGain(ADS1115_PGA_6P144);
    adc2.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC2_CH2 = adc2.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC2_CH2 = SingleEnd_ADC2_CH2*adc2.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC2_CH2;
    PL_ADC2_CH2 = IL_ADC2_CH2 * SingleEnd_ADC2_CH2 ;  //  mA * V = mW

// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc3.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC3_CH1 = 0;
    float IL_ADC3_CH1 = 0;
    int sensorOneCounts_ADC3_CH1 = adc3.getConversionP0N1(); 
    DiffV_ADC3_CH1 = sensorOneCounts_ADC3_CH1*adc3.getMvPerCount();
    IL_ADC3_CH1 = DiffV_ADC3_CH1 / ADC3_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC3_CH1;
    adc3.setGain(ADS1115_PGA_6P144); adc3.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC3_CH1 = adc3.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC3_CH1 = SingleEnd_ADC3_CH1*adc3.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC3_CH1;
    PL_ADC3_CH1 = IL_ADC3_CH1 * SingleEnd_ADC3_CH1 ;  //  mA * V = mW
        
// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc3.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC3_CH2 = 0;
    float IL_ADC3_CH2 = 0;
    int sensorOneCounts_ADC3_CH2 = adc3.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC3_CH2 = sensorOneCounts_ADC3_CH2*adc3.getMvPerCount();
    IL_ADC3_CH2 = DiffV_ADC3_CH2 / ADC3_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC3_CH2;
    adc3.setGain(ADS1115_PGA_6P144);
    adc3.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC3_CH2 = adc3.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC3_CH2 = SingleEnd_ADC3_CH2*adc3.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC3_CH2;
    PL_ADC3_CH2 = IL_ADC3_CH2 * SingleEnd_ADC3_CH2 ;  //  mA * V = mW


// Display CH1 data to 1.8" OLED
    tft.fillScreen(ST77XX_BLACK); tft.setTextWrap(false); tft.setFont();
    
    // ADC0-CH1
    tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0+2, 0+1 ); tft.print("ADC0 "); tft.print("IL:"); tft.print(IL_ADC0_CH1, 3); tft.print("mA");
    tft.setCursor(0+2, 10+1 ); tft.print("CH1  "); tft.print("PL:"); tft.print(PL_ADC0_CH1, 3); tft.print("mW");

    // ADC0-CH2
    tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0+2, 20+1 ); tft.print("ADC0 "); tft.print("IL:"); tft.print(IL_ADC0_CH2, 3); tft.print("mA");
    tft.setCursor(0+2, 30+1 ); tft.print("CH2  "); tft.print("PL:"); tft.print(PL_ADC0_CH2, 3); tft.print("mW");

    // ADC1-CH1
    tft.setTextSize(1); tft.setTextColor(ST77XX_MAGENTA);
    tft.setCursor(0+2, 40+1 ); tft.print("ADC1 "); tft.print("IL:"); tft.print(IL_ADC1_CH1, 3); tft.print("mA");
    tft.setCursor(0+2, 50+1 ); tft.print("CH1  "); tft.print("PL:"); tft.print(PL_ADC1_CH1, 3); tft.print("mW");

    // ADC1-CH2
    tft.setTextSize(1); tft.setTextColor(ST77XX_MAGENTA);
    tft.setCursor(0+2, 60+1 ); tft.print("ADC1 "); tft.print("IL:"); tft.print(IL_ADC1_CH2, 3); tft.print("mA");
    tft.setCursor(0+2, 70+1 ); tft.print("CH2  "); tft.print("PL:"); tft.print(PL_ADC1_CH2, 3); tft.print("mW");

    // ADC2-CH1
    tft.setTextSize(1); tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(0+2, 80+1 ); tft.print("ADC2 "); tft.print("IL:"); tft.print(IL_ADC2_CH1, 3); tft.print("mA");
    tft.setCursor(0+2, 90+1 ); tft.print("CH1  "); tft.print("PL:"); tft.print(PL_ADC2_CH1, 3); tft.print("mW");

    // ADC2-CH2
    tft.setTextSize(1); tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(0+2, 100+1 ); tft.print("ADC2 "); tft.print("IL:"); tft.print(IL_ADC2_CH2, 3); tft.print("mA");
    tft.setCursor(0+2, 110+1 ); tft.print("CH2  "); tft.print("PL:"); tft.print(PL_ADC2_CH2, 3); tft.print("mW");

    // ADC3-CH1
    tft.setTextSize(1); tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(0+2, 120+1 ); tft.print("ADC3 "); tft.print("IL:"); tft.print(IL_ADC3_CH1, 3); tft.print("mA");
    tft.setCursor(0+2, 130+1 ); tft.print("CH1  "); tft.print("PL:"); tft.print(PL_ADC3_CH1, 3); tft.print("mW");

    // ADC3-CH2
    tft.setTextSize(1); tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(0+2, 140+1 ); tft.print("ADC3 "); tft.print("IL:"); tft.print(IL_ADC3_CH2, 3); tft.print("mA");
    tft.setCursor(0+2, 150+1 ); tft.print("CH2  "); tft.print("PL:"); tft.print(PL_ADC3_CH2, 3); tft.print("mW");

    tft.setRotation(0);
}


void ADC_ALL2()
{
// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc0.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC0_CH1 = 0;
    float IL_ADC0_CH1 = 0;
    int sensorOneCounts_ADC0_CH1 = adc0.getConversionP0N1(); 
    DiffV_ADC0_CH1 = sensorOneCounts_ADC0_CH1*adc0.getMvPerCount();
    IL_ADC0_CH1 = DiffV_ADC0_CH1 / ADC0_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC0_CH1;
    adc0.setGain(ADS1115_PGA_6P144); adc0.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC0_CH1 = adc0.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC0_CH1 = SingleEnd_ADC0_CH1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC0_CH1;
    PL_ADC0_CH1 = IL_ADC0_CH1 * SingleEnd_ADC0_CH1 ;  //  mA * V = mW

// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc0.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC0_CH2 = 0;
    float IL_ADC0_CH2 = 0;
    int sensorOneCounts_ADC0_CH2 = adc0.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC0_CH2 = sensorOneCounts_ADC0_CH2*adc0.getMvPerCount();
    IL_ADC0_CH2 = DiffV_ADC0_CH2 / ADC0_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC0_CH2;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC0_CH2 = adc0.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC0_CH2 = SingleEnd_ADC0_CH2*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC0_CH2;
    PL_ADC0_CH2 = IL_ADC0_CH2 * SingleEnd_ADC0_CH2 ;  //  mA * V = mW

// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc1.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC1_CH1 = 0;
    float IL_ADC1_CH1 = 0;
    int sensorOneCounts_ADC1_CH1 = adc1.getConversionP0N1(); 
    DiffV_ADC1_CH1 = sensorOneCounts_ADC1_CH1*adc1.getMvPerCount();
    IL_ADC1_CH1 = DiffV_ADC1_CH1 / ADC1_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC1_CH1;
    adc1.setGain(ADS1115_PGA_6P144); adc1.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC1_CH1 = adc1.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC1_CH1 = SingleEnd_ADC1_CH1*adc1.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC1_CH1;
    PL_ADC1_CH1 = IL_ADC1_CH1 * SingleEnd_ADC1_CH1 ;  //  mA * V = mW

// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc1.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC1_CH2 = 0;
    float IL_ADC1_CH2 = 0;
    int sensorOneCounts_ADC1_CH2 = adc1.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC1_CH2 = sensorOneCounts_ADC1_CH2*adc1.getMvPerCount();
    IL_ADC1_CH2 = DiffV_ADC1_CH2 / ADC1_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC1_CH2;
    adc1.setGain(ADS1115_PGA_6P144);
    adc1.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC1_CH2 = adc1.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC1_CH2 = SingleEnd_ADC1_CH2*adc1.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC1_CH2;
    PL_ADC1_CH2 = IL_ADC1_CH2 * SingleEnd_ADC1_CH2 ;  //  mA * V = mW

// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc2.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC2_CH1 = 0;
    float IL_ADC2_CH1 = 0;
    int sensorOneCounts_ADC2_CH1 = adc2.getConversionP0N1(); 
    DiffV_ADC2_CH1 = sensorOneCounts_ADC2_CH1*adc2.getMvPerCount();
    IL_ADC2_CH1 = DiffV_ADC2_CH1 / ADC2_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC2_CH1;
    adc2.setGain(ADS1115_PGA_6P144); adc2.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC2_CH1 = adc2.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC2_CH1 = SingleEnd_ADC2_CH1*adc2.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC2_CH1;
    PL_ADC2_CH1 = IL_ADC2_CH1 * SingleEnd_ADC2_CH1 ;  //  mA * V = mW

// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc2.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC2_CH2 = 0;
    float IL_ADC2_CH2 = 0;
    int sensorOneCounts_ADC2_CH2 = adc2.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC2_CH2 = sensorOneCounts_ADC2_CH2*adc2.getMvPerCount();
    IL_ADC2_CH2 = DiffV_ADC2_CH2 / ADC2_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC2_CH2;
    adc2.setGain(ADS1115_PGA_6P144);
    adc2.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC2_CH2 = adc2.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC2_CH2 = SingleEnd_ADC2_CH2*adc2.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC2_CH2;
    PL_ADC2_CH2 = IL_ADC2_CH2 * SingleEnd_ADC2_CH2 ;  //  mA * V = mW

// Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc3.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC3_CH1 = 0;
    float IL_ADC3_CH1 = 0;
    int sensorOneCounts_ADC3_CH1 = adc3.getConversionP0N1(); 
    DiffV_ADC3_CH1 = sensorOneCounts_ADC3_CH1*adc3.getMvPerCount();
    IL_ADC3_CH1 = DiffV_ADC3_CH1 / ADC3_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC3_CH1;
    adc3.setGain(ADS1115_PGA_6P144); adc3.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC3_CH1 = adc3.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC3_CH1 = SingleEnd_ADC3_CH1*adc3.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC3_CH1;
    PL_ADC3_CH1 = IL_ADC3_CH1 * SingleEnd_ADC3_CH1 ;  //  mA * V = mW
        
// Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc3.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC3_CH2 = 0;
    float IL_ADC3_CH2 = 0;
    int sensorOneCounts_ADC3_CH2 = adc3.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC3_CH2 = sensorOneCounts_ADC3_CH2*adc3.getMvPerCount();
    IL_ADC3_CH2 = DiffV_ADC3_CH2 / ADC3_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC3_CH2;
    adc3.setGain(ADS1115_PGA_6P144);
    adc3.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC3_CH2 = adc3.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC3_CH2 = SingleEnd_ADC3_CH2*adc3.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC3_CH2;
    PL_ADC3_CH2 = IL_ADC3_CH2 * SingleEnd_ADC3_CH2 ;  //  mA * V = mW


// Display CH1 data to 1.8" OLED
    tft.fillScreen(ST77XX_BLACK); tft.setTextWrap(false); tft.setFont(); tft.setRotation(3);
    
    // ADC0-CH1|1
    tft.setTextSize(1); tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0+2, 0+2 ); tft.print("1|"); tft.print("IL:"); tft.print(IL_ADC0_CH1, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC0_CH1, 3); tft.print("mW");
    // ADC0-CH2|2
    tft.setCursor(0+2, 10+2 ); tft.print("2|"); tft.print("IL:"); tft.print(IL_ADC0_CH2, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC0_CH2, 3); tft.print("mW");
    // ADC1-CH1|3
    tft.setTextSize(1); tft.setTextColor(ST77XX_MAGENTA);
    tft.setCursor(0+2, 20+2 ); tft.print("3|"); tft.print("IL:"); tft.print(IL_ADC1_CH1, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC1_CH1, 3); tft.print("mW");
    // ADC1-CH2|4
    tft.setCursor(0+2, 30+2 ); tft.print("4|"); tft.print("IL:"); tft.print(IL_ADC1_CH2, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC1_CH2, 3); tft.print("mW");
    // ADC2-CH1|5
    tft.setTextSize(1); tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(0+2, 40+2 ); tft.print("5|"); tft.print("IL:"); tft.print(IL_ADC2_CH1, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC2_CH1, 3); tft.print("mW");
    // ADC2-CH2|6
    tft.setCursor(0+2, 50+2 ); tft.print("6|"); tft.print("IL:"); tft.print(IL_ADC2_CH2, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC2_CH2, 3); tft.print("mW");
    // ADC2-CH2|7
    tft.setTextSize(1); tft.setTextColor(ST77XX_ORANGE);
    tft.setCursor(0+2, 60+2 ); tft.print("7|"); tft.print("IL:"); tft.print(IL_ADC3_CH1, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC3_CH1, 3); tft.print("mW");
    // ADC2-CH2|8
    tft.setCursor(0+2, 70+2 ); tft.print("8|"); tft.print("IL:"); tft.print(IL_ADC3_CH2, 3); tft.print("mA"); tft.print(" PL:"); tft.print(PL_ADC3_CH2, 3); tft.print("mW");

    tft.setRotation(0);
}


void ADC0_CH1_CH2()
{
  
    // Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc0.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC0_CH1 = 0;
    float IL_ADC0_CH1 = 0;
    int sensorOneCounts_ADC0_CH1 = adc0.getConversionP0N1(); 
    DiffV_ADC0_CH1 = sensorOneCounts_ADC0_CH1*adc0.getMvPerCount();
    IL_ADC0_CH1 = DiffV_ADC0_CH1 / ADC0_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC0_CH1;
    adc0.setGain(ADS1115_PGA_6P144); adc0.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC0_CH1 = adc0.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC0_CH1 = SingleEnd_ADC0_CH1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC0_CH1;
    PL_ADC0_CH1 = IL_ADC0_CH1 * SingleEnd_ADC0_CH1 ;  //  mA * V = mW

    // Display A0-A1 data to 0.96" OLED
    tft.setTextWrap(false);

                    tft.drawRoundRect(0+2, 0+1 , 128, 23, 4, ST77XX_RED);
    tft.setFont(&FreeSansBoldOblique9pt7b);
    tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
                        tft.setCursor(5+2, 17+1 ); tft.print("      ADC-0 ");
    tft.setFont(); tft.setTextColor(ST77XX_GREEN);

    tft.setTextColor(ST77XX_CYAN);
                    tft.drawRoundRect(0+2, 0+1 +27, 128, 13, 2, ST77XX_CYAN);
    tft.setTextSize(1); tft.setCursor(1+2, 3+1 +27); tft.println("  POWER CONSUMPTION");
 
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_CYAN); 
    
                        tft.setCursor(0+2, 15+1 +27); tft.println(" A0-A1 ");
                        if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_CYAN);
                            }
                        tft.setCursor(0+2, 25+1 +27); tft.print(ADC0_A0_A1_Resistor, 3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_CYAN);

    tft.setTextSize(1); tft.setCursor(48+2, 15+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 15+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 15+1 +27); tft.println(SingleEnd_ADC0_CH1, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 15+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 25+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 25+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 25+1 +27); tft.println(DiffV_ADC0_CH1, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 25+1 +27); tft.println("mV");

    tft.setTextSize(1); tft.setCursor(0+2, 40+1 +27); tft.print("IL:");
//                         tft.fillRect(18+2, 34+1 +27, 85, 20, ST77XX_BLACK);
                         tft.fillRect(18+2, 34+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 34+1 +27); tft.print(IL_ADC0_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 34+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 56+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 50+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 50+1 +27); tft.print(PL_ADC0_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 50+1 +27); tft.print("mW");

    // Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc0.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC0_CH2 = 0;
    float IL_ADC0_CH2 = 0;
    int sensorOneCounts_ADC0_CH2 = adc0.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC0_CH2 = sensorOneCounts_ADC0_CH2*adc0.getMvPerCount();
    IL_ADC0_CH2 = DiffV_ADC0_CH2 / ADC0_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC0_CH2;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC0_CH2 = adc0.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC0_CH2 = SingleEnd_ADC0_CH2*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC0_CH2;
    PL_ADC0_CH2 = IL_ADC0_CH2 * SingleEnd_ADC0_CH2 ;  //  mA * V = mW

    //<<<<<<<<<<<<<<<<<<<<<<<< SD card logging
     // Set timer as minutes to run follow script
       if (Timer > display_update_time) {
          tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
          tft.fillRect(100+2, 10+1, 25, 10, ST77XX_BLACK);
          tft.setCursor(100+2, 10+1 ); tft.print("Log");
        }
       
       if (Timer > sd_log_time) {
          TimerLast = TimerNow;
     //>>>>>>>>>>>>>>>>>>>>>>>>
          
      // SD card data logger
      SD.begin(chipSelect);
      File dataFile = SD.open("Log-ADC0-CH1-CH2.txt", FILE_WRITE);
  
      // if the file is available, write to it:
      if (dataFile) {
        
        dataFile.print("ADC0 CH1 | "); 
        dataFile.print("V-load: "); dataFile.print(SingleEnd_ADC0_CH1, 3); dataFile.print("V | ");
        dataFile.print("*Voltage: "); dataFile.print(DiffV_ADC0_CH1, 3); dataFile.print("mV | ");
        dataFile.print("I-load: "); dataFile.print(IL_ADC0_CH1, 3); dataFile.print("mA | ");
        dataFile.print("P-load: "); dataFile.print(PL_ADC0_CH1, 3); dataFile.print("mW");
    
        dataFile.print("|| CH2 || "); 
        dataFile.print("V-load: "); dataFile.print(SingleEnd_ADC0_CH2, 3); dataFile.print("V | ");
        dataFile.print("*Voltage: "); dataFile.print(DiffV_ADC0_CH2, 3); dataFile.print("mV | ");
        dataFile.print("I-load: "); dataFile.print(IL_ADC0_CH2, 3); dataFile.print("mA | ");
        dataFile.print("P-load: "); dataFile.print(PL_ADC0_CH2, 3); dataFile.print("mW | ");
        dataFile.print("Time: "); dataFile.print(print_time); dataFile.println("ms");
  
        dataFile.close();
        Serial.print("| Log SD done!");
        
        //Print_time
        print_time = millis();
        Serial.print(" "); Serial.print(print_time); Serial.print("ms ");

        tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
        tft.fillRect(100+2, 10+1, 25, 10, ST77XX_BLACK);
        tft.setCursor(100+2, 10+1 ); tft.print("OK");
  
      }
        // if the file isn't open, pop up an error:
        else {
          Serial.println("| error opening datalog.txt");
          
          tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
          tft.fillRect(100+2, 10+1, 25, 10, ST77XX_BLACK);
          tft.setCursor(100+2, 10+1 ); tft.print("FAIL"); 
          
          }
      }

    // Display A2-A3 data to 0.96" OLED
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_GREEN);

                    tft.drawRoundRect(0+2, 67+1 +27, 128, 13, 2, ST77XX_GREEN);
    tft.setTextSize(1); tft.setCursor(1+2, 70+1 +27 ); tft.println("  POWER CONSUMPTION");
    
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 
                        tft.setCursor(0+2, 82+1 +27); tft.println(" A2-A3 ");
    
                        if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_GREEN);
                            }
                        tft.setCursor(0+2, 92+1 +27); tft.print(ADC0_A2_A3_Resistor, 3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_GREEN);

    tft.setTextSize(1); tft.setCursor(48+2, 82+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 82+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 82+1 +27); tft.println(SingleEnd_ADC0_CH2, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 82+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 92+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 92+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 92+1 +27); tft.println(DiffV_ADC0_CH2, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 92+1 +27); tft.println("mV"); 

    tft.setTextSize(1); tft.setCursor(0+2, 107+1 +27); tft.print("IL:");
                         tft.fillRect(18+2, 101+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 101+1 +27); tft.print(IL_ADC0_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 101+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 123+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 117+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 117+1 +27); tft.print(PL_ADC0_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 117+1 +27); tft.print("mW");


    //UART transfer data to PC, will tearing while this open.
    Serial.print("ADC0 CH1 | "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC0_CH1, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC0_CH1, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC0_CH1, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC0_CH1, 3); Serial.print("mW");

    Serial.print("|| CH2 || "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC0_CH2, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC0_CH2, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC0_CH2, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC0_CH2, 3); Serial.println("mW | ");
       
}   


void ADC1_CH1_CH2()
{
    // Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc1.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC1_CH1 = 0;
    float IL_ADC1_CH1 = 0;
    int sensorOneCounts_ADC1_CH1 = adc1.getConversionP0N1(); 
    DiffV_ADC1_CH1 = sensorOneCounts_ADC1_CH1*adc1.getMvPerCount();
    IL_ADC1_CH1 = DiffV_ADC1_CH1 / ADC1_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC1_CH1;
    adc1.setGain(ADS1115_PGA_6P144); adc1.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC1_CH1 = adc1.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC1_CH1 = SingleEnd_ADC1_CH1*adc1.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC1_CH1;
    PL_ADC1_CH1 = IL_ADC1_CH1 * SingleEnd_ADC1_CH1 ;  //  mA * V = mW

    // Display A0-A1 data to 0.96" OLED
    tft.setTextWrap(false);

                    tft.drawRoundRect(0+2, 0+1 , 128, 23, 4, ST77XX_RED);
    tft.setFont(&FreeSansBoldOblique9pt7b);
//                             tft.fillRect(79+2, 4+1, 15, 15, ST77XX_BLACK);
    tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
                        tft.setCursor(5+2, 17+1 ); tft.print("      ADC-1 ");
    tft.setFont(); tft.setTextColor(ST77XX_GREEN);

    tft.setTextColor(ST77XX_CYAN);
                    tft.drawRoundRect(0+2, 0+1 +27, 128, 13, 2, ST77XX_CYAN);
    tft.setTextSize(1); tft.setCursor(1+2, 3+1 +27); tft.println("  POWER CONSUMPTION");
 
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_CYAN); 
                        tft.setCursor(0+2, 15+1 +27); tft.println(" A0-A1 ");
                        
                        if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_CYAN);
                            }
                        
                        tft.setCursor(0+2, 25+1 +27); tft.print(ADC1_A0_A1_Resistor, 3); tft.println("/Ohm");      //Mame

 
                        
    tft.setTextColor(ST77XX_CYAN);

    tft.setTextSize(1); tft.setCursor(48+2, 15+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 15+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 15+1 +27); tft.println(SingleEnd_ADC1_CH1, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 15+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 25+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 25+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 25+1 +27); tft.println(DiffV_ADC1_CH1, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 25+1 +27); tft.println("mV");

    tft.setTextSize(1); tft.setCursor(0+2, 40+1 +27); tft.print("IL:");
                         tft.fillRect(18+2, 34+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 34+1 +27); tft.print(IL_ADC1_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 34+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 56+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 50+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 50+1 +27); tft.print(PL_ADC1_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 50+1 +27); tft.print("mW");

    // Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc1.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC1_CH2 = 0;
    float IL_ADC1_CH2 = 0;
    int sensorOneCounts_ADC1_CH2 = adc1.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC1_CH2 = sensorOneCounts_ADC1_CH2*adc1.getMvPerCount();
    IL_ADC1_CH2 = DiffV_ADC1_CH2 / ADC1_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC1_CH2;
    adc1.setGain(ADS1115_PGA_6P144);
    adc1.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC1_CH2 = adc1.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC1_CH2 = SingleEnd_ADC1_CH2*adc1.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC1_CH2;
    PL_ADC1_CH2 = IL_ADC1_CH2 * SingleEnd_ADC1_CH2 ;  //  mA * V = mW

    // Display A2-A3 data to 0.96" OLED
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_GREEN);

                    tft.drawRoundRect(0+2, 67+1 +27, 128, 13, 2, ST77XX_GREEN);
    tft.setTextSize(1); tft.setCursor(1+2, 70+1 +27 ); tft.println("  POWER CONSUMPTION");
    
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 
                        tft.setCursor(0+2, 82+1 +27); tft.println(" A2-A3 ");
//   tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 

                        if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_GREEN);
                            }
    
                        tft.setCursor(0+2, 92+1 +27); tft.print(ADC1_A2_A3_Resistor, 3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_GREEN);

    tft.setTextSize(1); tft.setCursor(48+2, 82+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 82+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 82+1 +27); tft.println(SingleEnd_ADC1_CH2, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 82+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 92+1 +27); tft.println("*V:");
//                         tft.fillRect(72+2, 92+1 +27, 43, 10, ST77XX_BLACK);
                         tft.fillRect(72+2, 92+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 92+1 +27); tft.println(DiffV_ADC1_CH2, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 92+1 +27); tft.println("mV"); 

    tft.setTextSize(1); tft.setCursor(0+2, 107+1 +27); tft.print("IL:");
                         tft.fillRect(18+2, 101+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 101+1 +27); tft.print(IL_ADC1_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 101+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 123+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 117+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 117+1 +27); tft.print(PL_ADC1_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 117+1 +27); tft.print("mW");


    //UART transfer data to PC, will tearing while this open.
    Serial.print("ADC1 CH1 | "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC1_CH1, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC1_CH1, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC1_CH1, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC1_CH1, 3); Serial.print("mW");

    Serial.print("|| CH2 || "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC1_CH2, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC1_CH2, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC1_CH2, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC1_CH2, 3); Serial.println("mW");
}   

 
void ADC2_CH1_CH2()
{
    // Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc2.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC2_CH1 = 0;
    float IL_ADC2_CH1 = 0;
    int sensorOneCounts_ADC2_CH1 = adc2.getConversionP0N1(); 
    DiffV_ADC2_CH1 = sensorOneCounts_ADC2_CH1*adc2.getMvPerCount();
    IL_ADC2_CH1 = DiffV_ADC2_CH1 / ADC2_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC2_CH1;
    adc2.setGain(ADS1115_PGA_6P144); adc2.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC2_CH1 = adc2.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC2_CH1 = SingleEnd_ADC2_CH1*adc2.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC2_CH1;
    PL_ADC2_CH1 = IL_ADC2_CH1 * SingleEnd_ADC2_CH1 ;  //  mA * V = mW

    // Display A0-A1 data to 0.96" OLED
    tft.setTextWrap(false);

                    tft.drawRoundRect(0+2, 0+1 , 128, 23, 4, ST77XX_RED);
    tft.setFont(&FreeSansBoldOblique9pt7b);
//                             tft.fillRect(79+2, 4+1, 15, 15, ST77XX_BLACK);
    tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
                        tft.setCursor(5+2, 17+1 ); tft.print("      ADC-2 ");
    tft.setFont(); tft.setTextColor(ST77XX_GREEN);

    tft.setTextColor(ST77XX_CYAN);
                    tft.drawRoundRect(0+2, 0+1 +27, 128, 13, 2, ST77XX_CYAN);
    tft.setTextSize(1); tft.setCursor(1+2, 3+1 +27); tft.println("  POWER CONSUMPTION");
 
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_CYAN); 
                        tft.setCursor(0+2, 15+1 +27); tft.println(" A0-A1 ");

                        if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_CYAN);
                            }
                        tft.setCursor(0+2, 25+1 +27); tft.print(ADC2_A0_A1_Resistor,3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_CYAN);

    tft.setTextSize(1); tft.setCursor(48+2, 15+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 15+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 15+1 +27); tft.println(SingleEnd_ADC2_CH1, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 15+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 25+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 25+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 25+1 +27); tft.println(DiffV_ADC2_CH1, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 25+1 +27); tft.println("mV");

    tft.setTextSize(1); tft.setCursor(0+2, 40+1 +27); tft.print("IL:");
                        // tft.fillRect(18+2, 34+1 +27, 85, 20, ST77XX_BLACK);
                         tft.fillRect(18+2, 34+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 34+1 +27); tft.print(IL_ADC2_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 34+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 56+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 50+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 50+1 +27); tft.print(PL_ADC2_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 50+1 +27); tft.print("mW");

    // Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc2.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC2_CH2 = 0;
    float IL_ADC2_CH2 = 0;
    int sensorOneCounts_ADC2_CH2 = adc2.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC2_CH2 = sensorOneCounts_ADC2_CH2*adc2.getMvPerCount();
    IL_ADC2_CH2 = DiffV_ADC2_CH2 / ADC2_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC2_CH2;
    adc2.setGain(ADS1115_PGA_6P144);
    adc2.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC2_CH2 = adc2.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC2_CH2 = SingleEnd_ADC2_CH2*adc2.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC2_CH2;
    PL_ADC2_CH2 = IL_ADC2_CH2 * SingleEnd_ADC2_CH2 ;  //  mA * V = mW

    // Display A2-A3 data to 0.96" OLED
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_GREEN);

                    tft.drawRoundRect(0+2, 67+1 +27, 128, 13, 2, ST77XX_GREEN);
    tft.setTextSize(1); tft.setCursor(1+2, 70+1 +27 ); tft.println("  POWER CONSUMPTION");
    
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 
                        tft.setCursor(0+2, 82+1 +27); tft.println(" A2-A3 ");
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 
    
                        if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_GREEN);
                            }
    
                        tft.setCursor(0+2, 92+1 +27); tft.print(ADC2_A2_A3_Resistor,3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_GREEN);

    tft.setTextSize(1); tft.setCursor(48+2, 82+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 82+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 82+1 +27); tft.println(SingleEnd_ADC2_CH2, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 82+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 92+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 92+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 92+1 +27); tft.println(DiffV_ADC2_CH2, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 92+1 +27); tft.println("mV"); 

    tft.setTextSize(1); tft.setCursor(0+2, 107+1 +27); tft.print("IL:");
                         tft.fillRect(18+2, 101+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 101+1 +27); tft.print(IL_ADC2_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 101+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 123+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 117+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 117+1 +27); tft.print(PL_ADC2_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 117+1 +27); tft.print("mW");


    //UART transfer data to PC, will tearing while this open.
    Serial.print("ADC2 CH1 | "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC2_CH1, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC2_CH1, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC2_CH1, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC2_CH1, 3); Serial.print("mW");

    Serial.print("|| CH2 || "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC2_CH2, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC2_CH2, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC2_CH2, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC2_CH2, 3); Serial.println("mW");
}   


void ADC3_CH1_CH2()
{
    // Group1 //
    // Get Differential of A0-A1, and get single-end reading A1.
    adc3.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC3_CH1 = 0;
    float IL_ADC3_CH1 = 0;
    int sensorOneCounts_ADC3_CH1 = adc3.getConversionP0N1(); 
    DiffV_ADC3_CH1 = sensorOneCounts_ADC3_CH1*adc3.getMvPerCount();
    IL_ADC3_CH1 = DiffV_ADC3_CH1 / ADC3_A0_A1_Resistor;

    //Get single-end V-load
    float SingleEnd_ADC3_CH1;
    adc3.setGain(ADS1115_PGA_6P144); adc3.setMultiplexer(ADS1115_MUX_P1_NG);
    SingleEnd_ADC3_CH1 = adc3.getConversionP1GND();  // counts up to 16-bits  
    SingleEnd_ADC3_CH1 = SingleEnd_ADC3_CH1*adc3.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC3_CH1;
    PL_ADC3_CH1 = IL_ADC3_CH1 * SingleEnd_ADC3_CH1 ;  //  mA * V = mW

    // Display A0-A1 data to 0.96" OLED
    tft.setTextWrap(false);

                    tft.drawRoundRect(0+2, 0+1 , 128, 23, 4, ST77XX_RED);
    tft.setFont(&FreeSansBoldOblique9pt7b);
//                             tft.fillRect(79+2, 4+1, 15, 15, ST77XX_BLACK);
    tft.setTextSize(1);tft.setTextColor(ST77XX_RED); 
                        tft.setCursor(5+2, 17+1 ); tft.print("      ADC-3 ");
    tft.setFont(); tft.setTextColor(ST77XX_GREEN);

    tft.setTextColor(ST77XX_CYAN);
                    tft.drawRoundRect(0+2, 0+1 +27, 128, 13, 2, ST77XX_CYAN);
    tft.setTextSize(1); tft.setCursor(1+2, 3+1 +27); tft.println("  POWER CONSUMPTION");
 
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_CYAN); 
                        tft.setCursor(0+2, 15+1 +27); tft.println(" A0-A1 ");

                        if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 0)
                            {
                              tft.setTextColor(ST77XX_CYAN);
                            }
                        tft.setCursor(0+2, 25+1 +27); tft.print(ADC3_A0_A1_Resistor,3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_CYAN);

    tft.setTextSize(1); tft.setCursor(48+2, 15+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 15+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 15+1 +27); tft.println(SingleEnd_ADC3_CH1, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 15+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 25+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 25+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 25+1 +27); tft.println(DiffV_ADC3_CH1, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 25+1 +27); tft.println("mV");

    tft.setTextSize(1); tft.setCursor(0+2, 40+1 +27); tft.print("IL:");
                         //tft.fillRect(18+2, 34+1 +27, 85, 20, ST77XX_BLACK);
                         tft.fillRect(18+2, 34+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 34+1 +27); tft.print(IL_ADC3_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 34+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 56+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 50+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 50+1 +27); tft.print(PL_ADC3_CH1, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 50+1 +27); tft.print("mW");

    // Group2 //
    // Get Differential of A2-A3, and get single-end reading A1.
    adc3.setGain(ADS1115_PGA_0P256);
    float DiffV_ADC3_CH2 = 0;
    float IL_ADC3_CH2 = 0;
    int sensorOneCounts_ADC3_CH2 = adc3.getConversionP2N3();  // counts up to 16-bits  
    DiffV_ADC3_CH2 = sensorOneCounts_ADC3_CH2*adc3.getMvPerCount();
    IL_ADC3_CH2 = DiffV_ADC3_CH2 / ADC3_A2_A3_Resistor; // (0.2 ohm)

    //Get single-end V-load
    float SingleEnd_ADC3_CH2;
    adc3.setGain(ADS1115_PGA_6P144);
    adc3.setMultiplexer(ADS1115_MUX_P3_NG);
    SingleEnd_ADC3_CH2 = adc3.getConversionP3GND();  // counts up to 16-bits  
    SingleEnd_ADC3_CH2 = SingleEnd_ADC3_CH2*adc3.getMvPerCount() /1000;
    
    //Calculate PWR
    float PL_ADC3_CH2;
    PL_ADC3_CH2 = IL_ADC3_CH2 * SingleEnd_ADC3_CH2 ;  //  mA * V = mW

    // Display A2-A3 data to 0.96" OLED
    tft.setTextWrap(false);
    tft.setTextColor(ST77XX_GREEN);

                    tft.drawRoundRect(0+2, 67+1 +27, 128, 13, 2, ST77XX_GREEN);
    tft.setTextSize(1); tft.setCursor(1+2, 70+1 +27 ); tft.println("  POWER CONSUMPTION");
    
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 
                        tft.setCursor(0+2, 82+1 +27); tft.println(" A2-A3 ");
    tft.setTextSize(1); tft.setTextColor(ST77XX_BLACK, ST77XX_GREEN); 

                        if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_BLACK, ST77XX_WHITE); 
                            }
                            else if (buttonPushCounterDigi == 1)
                            {
                              tft.setTextColor(ST77XX_GREEN);
                            }
                        tft.setCursor(0+2, 92+1 +27); tft.print(ADC3_A2_A3_Resistor,3); tft.println("/Ohm");      //Mame
    tft.setTextColor(ST77XX_GREEN);

    tft.setTextSize(1); tft.setCursor(48+2, 82+1 +27); tft.println("V-Load:");
                         tft.fillRect(90+2, 82+1 +27, 40, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(90+2, 82+1 +27); tft.println(SingleEnd_ADC3_CH2, 3);
    tft.setTextSize(1); tft.setCursor(122+2, 82+1 +27); tft.println("V");

    tft.setTextSize(1); tft.setCursor(55+2, 92+1 +27); tft.println("*V:");
                         tft.fillRect(72+2, 92+1 +27, 60, 10, ST77XX_BLACK);
    tft.setTextSize(1); tft.setCursor(72+2, 92+1 +27); tft.println(DiffV_ADC3_CH2, 4);
    tft.setTextSize(1); tft.setCursor(116+2, 92+1 +27); tft.println("mV"); 

    tft.setTextSize(1); tft.setCursor(0+2, 107+1 +27); tft.print("IL:");
                         tft.fillRect(18+2, 101+1 +27, 150, 20, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 101+1 +27); tft.print(IL_ADC3_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 101+1 +27); tft.print("mA");

    tft.setTextSize(1); tft.setCursor(0+2, 123+1 +27); tft.print("PL:");
                         tft.fillRect(18+2, 117+1 +27, 150, 15, ST77XX_BLACK);
    tft.setTextSize(2); tft.setCursor(18+2, 117+1 +27); tft.print(PL_ADC3_CH2, 3);
    tft.setTextSize(2); tft.setCursor(104+2, 117+1 +27); tft.print("mW");


    //UART transfer data to PC, will tearing while this open.
    Serial.print("ADC3 CH1 | "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC3_CH1, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC3_CH1, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC3_CH1, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC3_CH1, 3); Serial.print("mW");

    Serial.print("|| CH2 || "); 
    Serial.print("V-load: "); Serial.print(SingleEnd_ADC3_CH2, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(DiffV_ADC3_CH2, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(IL_ADC3_CH2, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PL_ADC3_CH2, 3); Serial.println("mW");
}   


// -----------------------------------------------------------------------------
// Different Group function
// -----------------------------------------------------------------------------
