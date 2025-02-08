/* <for change this version 1.2>

  - DC motor  --> Servo motor 
  - Database
*/

/* <TFT pin>
  - vcc:    3V  
  - gnd:    GND
  - cs:     G5
  - reset:  EN
  - DC/RS:  G17
  - SDI(MOSI):  G23
  - SCK:    G18
  - LED:    3V
  - SD0(MISO):  -
  - T_CLK:  G18
  - T_CS:   G16
  - T_DIN:  G23
  - T_DO:   G19
  - T_IRQ:  -
*/

//ESP32 pinout: https://cdn.shopify.com/s/files/1/0609/6011/2892/files/doc-esp32-pinout-reference-wroom-devkit.png


//____________________________________________________< lib & define: WiFi >
#include <WiFi.h>
#include <HTTPClient.h>
#define ssid "Por-AIS4GHomeWiFi"
#define pass "por150444"

//____________________________________________________< lib & define: TFT >
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();


//____________________________________________________< lib & define: servo >
#include <ESP32Servo.h>
Servo servoMID; 
Servo servoL1; 
Servo servoL2; 
Servo servoL3; 
Servo servoR1; 
Servo servoR2; 
Servo servoR3; 

//____________________________________________________< define: IR sensor >
int CurVal_senL;    int LstVal_senL = 1;    int Count_senL = 0;
int CurVal_senR;    int LstVal_senR = 1;    int Count_senR = 0;

#define IR_L 36 //SN
#define IR_R 39 //SP

//____________________________________________________< define: Relay >
#define HotGun_L 4  //IN1
#define HotGun_R 0  //IN2

//____________________________________________________< define variables >
int QTY_AA = 0;
int QTY_AAA = 0;
int AllScore = 0;

int count_AAtoWarp = 0;
int count_AAAtoWarp = 0;

int state = 0;

unsigned long lst = 0;

String PhoneNumber = "";


//====================================================< setup >
void setup() {
  Serial.begin(9600);

  //_____________________< WiFi >
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED){Serial.println(".");}
  Serial.println("WiFi connected");

  //_____________________< servo >
  servoMID.attach(32);            
  servoL1.attach(33);
  servoL2.attach(25);
  servoL3.attach(26);
  servoR1.attach(27);
  servoR2.attach(14);
  servoR3.attach(12);

  servoMID.write(90);
  servoL1.write(90);
  servoL2.write(90);
  servoL3.write(90);
  servoR1.write(90);
  servoR2.write(90);
  servoR3.write(90);

  //_____________________< IR sensor >
  // pinMode(36,INPUT);
  // pinMode(39,INPUT);

  //_____________________< Relay >
  pinMode(4,OUTPUT);
  pinMode(0,OUTPUT);

  //Relay active high
  digitalWrite(4,1);
  digitalWrite(0,1);

  //_____________________< Interupt >
  lst = millis();

  //_____________________< TFT LCD >
  tft.init();
  tft.setRotation(3);
  uint16_t calData[5] = { 440, 3294, 425, 3109, 7 };  // Use this calibration code in setup():
  tft.setTouch(calData);

  Func_PreScreen();
}


//====================================================< loop >
void loop() {
  Func_StateMachine();
  delay(10);
}

//====================================================< Func_StateMachine >
void Func_StateMachine(){
  switch(state){
    //__________________________________________< PreScreenBegin >
    case 0:
      uint16_t x0, y0;
      if (tft.getTouch(&x0, &y0)){
        if ((x0 > 0 && x0 < 480) && (y0 > 0 && y0 < 320)) {
          Serial.println("Touch: PreScreenBegin");
          Func_MainScreen();
          state = 1;
        } 
      }
      break;

    //__________________________________________< MainScreen >
    case 1:
      //_____________________< Interupt >
      if(millis() - lst >= 100){
        lst = millis();
        //----------------< init >
        CountAll_IRsensor();

        //----------------< end >
      }
      uint16_t x1, y1;
      if (tft.getTouch(&x1, &y1)){
        //----------------< AA Button >
        if ((x1 > 375 && x1 < 450) && (y1 > 170 && y1 < 250)) {
          Serial.println("Touch: AA Button");
          servoMID.write(67);
          delay(1000);
        } 
        //----------------< AAA Button >
        else if ((x1 > 250 && x1 < 325) && (y1 > 170 && y1 < 250)) {
          Serial.println("Touch: AAA Button");
          servoMID.write(114);
          delay(1000);
        }
        //----------------< let's compute Button >
        else if ((x1 > 250 && x1 < 430) && (y1 > 0 && y1 < 55)) {
          Serial.println("Touch: let's compute Button");
          AllScore = (QTY_AA * 2)+(QTY_AAA * 3);
          Func_MainScreen_afterCompute();
          state = 9;
        }
      }
      if(count_AAtoWarp == 10 || count_AAAtoWarp == 10){
        Func_PopupWaitWarping();
        state = 10;
      }
      break;

    //__________________________________________< After pushed let's compute Button >
    case 9:
      uint16_t x9, y9;
      if (tft.getTouch(&x9, &y9)){
        //----------------< Claim Score Button >
        if ((x9 > 0 && x9 < 175) && (y9 > 0 && y9 < 55)) {
          Serial.println("Touch: Claim Score Button");
          Func_InputPhoneNumberScreen();
          state = 2;
        }
      }
      break;

    //__________________________________________< warpping batt state >
    case 10:
      Warpping_Batt();

      count_AAtoWarp = 0;
      count_AAAtoWarp = 0;

      state = 1;

      Func_MainScreen();
      break;

    //__________________________________________< InputPhoneNumberScreen 1 >
    case 2:
      uint16_t x2, y2;
      if (tft.getTouch(&x2, &y2)){
        //----------------< Button 1 >
        if ((x2 > 285 && x2 < 335) && (y2 > 185 && y2 < 235)) {
          Serial.println("Touch: Button 1");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "1";
          }
          state = 3;
        } 
        //----------------< Button 2 >
        else if ((x2 > 215 && x2 < 265) && (y2 > 185 && y2 < 235)) {
          Serial.println("Touch: Button 2");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "2";
          }
          state = 3;
        } 
        //----------------< Button 3 >
        else if ((x2 > 145 && x2 < 195) && (y2 > 185 && y2 < 235)) {
          Serial.println("Touch: Button 3");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "3";
          }
          state = 3;
        } 
        //----------------< Button 4 >
        else if ((x2 > 285 && x2 < 335) && (y2 > 130 && y2 < 180)) {
          Serial.println("Touch: Button 4");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "4";
          }
          state = 3;
        } 
        //----------------< Button 5 >
        else if ((x2 > 215 && x2 < 265) && (y2 > 130 && y2 < 180)) {
          Serial.println("Touch: Button 5");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "5";
          }
          state = 3;
        } 
        //----------------< Button 6 >
        else if ((x2 > 145 && x2 < 195) && (y2 > 130 && y2 < 180)) {
          Serial.println("Touch: Button 6");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "6";
          }
          state = 3;
        } 
        //----------------< Button 7 >
        else if ((x2 > 285 && x2 < 335) && (y2 > 75 && y2 < 125)) {
          Serial.println("Touch: Button 7");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "7";
          }
          state = 3;
        } 
        //----------------< Button 8 >
        else if ((x2 > 215 && x2 < 265) && (y2 > 75 && y2 < 125)) {
          Serial.println("Touch: Button 8");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "8";
          }
          state = 3;
        } 
        //----------------< Button 9 >
        else if ((x2 > 145 && x2 < 195) && (y2 > 75 && y2 < 125)) {
          Serial.println("Touch: Button 9");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "9";
          }
          state = 3;
        } 
        //----------------< Button 0 >
        else if ((x2 > 215 && x2 < 265) && (y2 > 20 && y2 < 70)) {
          Serial.println("Touch: Button 0");
          if(PhoneNumber.length() < 10){
            PhoneNumber += "0";
          }
          state = 3;
        }
        //----------------< Button "<" >
        else if ((x2 > 285 && x2 < 335) && (y2 > 20 && y2 < 70)) {
          Serial.println("Touch: Button <");
          PhoneNumber.remove(PhoneNumber.length()-1, 1);
          state = 3;
        }
        //----------------< Button "OK" >
        else if ((x2 > 145 && x2 < 195) && (y2 > 20 && y2 < 70)) {
          Serial.println("Touch: Button OK");
          if(PhoneNumber.length() == 10){
            state = 4;
          }
        }
      }
      break;

    //__________________________________________< InputPhoneNumberScreen 2>
    case 3:
      Func_InputPhoneNumberScreen();
      if(PhoneNumber.length() == 10){
        tft.fillCircle(310, 275, 25, tft.color24to16(0x538234)); //OK
        tft.setTextSize(2);
        tft.setTextColor(tft.color24to16(0xFFFFFF));
        tft.setTextDatum(MC_DATUM);
        tft.drawString("OK", 310, 275);
      }
      delay(500);
      state = 2;
      break;

    //__________________________________________< Popup Screen >
    case 4:
      Func_PopupThank();
      Func_RecordPoint();
      delay(5000);
      Func_PreScreen();
      //----------------< Reset values >
      PhoneNumber = "";
      AllScore = 0;       
      QTY_AA = 0;         
      QTY_AAA = 0;

      LstVal_senL = 1;    
      Count_senL = 0;
      LstVal_senR = 1;    
      Count_senR = 0;

      servoMID.write(90);

      state = 0;
      break;
  }
}

//====================================================< CountAll_IRsensor >
void CountAll_IRsensor(){
  //____________________________<sensor left>
  CurVal_senL = digitalRead(36);
  if(CurVal_senL != LstVal_senL){
    if(CurVal_senL == 0){
      Count_senL++;
      count_AAtoWarp++;
      QTY_AA = Count_senL;
      // ----------------< change screen & update values >
      Func_MainScreen();
    }
    else{
      // delay(100);
    }
  }
  LstVal_senL = CurVal_senL;

  //____________________________<sensor right>
  CurVal_senR = digitalRead(39);
  if(CurVal_senR != LstVal_senR){
    if(CurVal_senR == 0){
      Count_senR++;
      count_AAAtoWarp++;
      QTY_AAA = Count_senR;
      // ----------------< change screen & update values >
      Func_MainScreen();
    }
    else{
      // delay(100);
    }
  }
  LstVal_senR = CurVal_senR;

  Serial.println("  IR_L: " + String(Count_senL) + "  IR_R: " + String(Count_senR));
}

//====================================================< Warpping_Batt >
void Warpping_Batt(){
  // //_____________________< AAA side >
  // if(count_AAAtoWarp == 10){
  //   digitalWrite(25,1);
  //   digitalWrite(4,1);
  //   delay(20000);
  //   digitalWrite(25,0);
  //   digitalWrite(4,0);

  //   delay(2000);

  //   digitalWrite(33,1); 
  //   delay(950); //ok
  //   digitalWrite(33,0);

  //   // delay(1500);

  //   // digitalWrite(32,1); 
  //   // delay(740); //ok
  //   // digitalWrite(32,0);

    
  // }

  // //_____________________< AA side >
  // else if(count_AAtoWarp == 10){
  //   digitalWrite(26,1); 
  //   digitalWrite(0,1);
  //   // delay(10350);
  //   delay(20000);
  //   digitalWrite(26,0);
  //   digitalWrite(0,0);

  //   delay(2000);

  //   digitalWrite(27,1); 
  //   delay(780); //ok
  //   digitalWrite(27,0);
      
  //   // delay(1500);

  //   // digitalWrite(14,1); 
  //   // delay(735); //ok
  //   // digitalWrite(14,0);

  // }
}

//====================================================< Func_PreScreen >
void Func_PreScreen(){
  tft.fillScreen(tft.color24to16(0xFFFFFF));

  tft.drawRoundRect(85, 210, 310, 40, 10, tft.color24to16(0xb22222));
  tft.fillRect(0, 0, 480, 120, tft.color24to16(0xb22222));

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Intelligent Sorting Cabinet for Battery", 240, 40);
  tft.drawString("Disposal and Recycling", 240, 70);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Tap the screen to begin", 240, 230);
}

//====================================================< Func_MainScreen >
void Func_MainScreen(){
  tft.fillScreen(tft.color24to16(0xFFFFFF));  //Background color

  //___________________________________________< Shape >
  // x, y, w, h, color ---<rect>
  // x, y, r, color ---<circle>

  // Biggest RectRed
  tft.fillRect(275, 5, 200, 275, tft.color24to16(0xb22222));

  // ResultScoresRect
  tft.fillRect(280, 190, 190, 60, tft.color24to16(0xFFFFFF));
  tft.fillRect(285, 195, 180, 50, tft.color24to16(0x76b5c5));

  // Claim Score Button Rect
  tft.fillRect(305, 265, 175, 55, tft.color24to16(0xFFFFFF));
  tft.fillRect(310, 270, 165, 45, tft.color24to16(0xd9d9d9));

  // CountBattRect
  tft.fillRect(20, 190, 230, 90, tft.color24to16(0xd9d9d9));

  // let's compute Button Rect
  tft.fillRect(45, 265, 180, 55, tft.color24to16(0xFFFFFF));
  tft.fillRect(50, 270, 170, 45, tft.color24to16(0xb22222));

  // ChooseBatt Button
  tft.fillCircle(75, 125, 50, tft.color24to16(0xa6a5a5));
  tft.fillCircle(75, 125, 45, tft.color24to16(0xb22222));
  tft.fillCircle(195, 125, 50, tft.color24to16(0xa6a5a5));
  tft.fillCircle(195, 125, 45, tft.color24to16(0xb22222));

  //___________________________________________< Text >
  //_______________< left screen >
  // tft.setTextSize(1);
  // tft.setTextColor(tft.color24to16(0xb22222));
  // tft.setTextDatum(TL_DATUM);
  // tft.drawString("Intelligent Sorting Cabinet for Battery Disposal and Recycling", 15, 10);

  tft.setTextSize(1);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Please choose the kind of battery", 15, 35);
  tft.drawString("you want to get rid of", 120, 50);

  tft.setTextSize(3);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("AA", 75, 125);
  tft.drawString("AAA", 195, 125);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("QTY of AA: " + String(QTY_AA), 35, 200);
  tft.drawString("QTY of AAA: " + String(QTY_AAA), 35, 240);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("let's compute", 59, 285);

  //_______________< right screen >
  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("processing...", 280, 10);

  // tft.setTextSize(2);
  // tft.setTextColor(tft.color24to16(0xFFFFFF));
  // tft.setTextDatum(TL_DATUM);
  // tft.drawString("processed", 280, 10);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Claim Score", 325, 285);
  
  tft.setTextSize(1);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("- AA Battery: 1 point / piece", 285, 60);
  // tft.drawString(String(QTY_AA) + "    pce", 390, 80);
  tft.drawString("0    pce", 390, 80);
  
  tft.setTextSize(1);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("- AAA Battery: 2 point / piece", 285, 120);
  // tft.drawString(String(QTY_AAA) + "    pce", 390, 140);
  tft.drawString("0    pce", 390, 140);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Result: " + String(AllScore) + " PTS", 293, 212);
}

//====================================================< Func_MainScreen_afterCompute >
void Func_MainScreen_afterCompute(){
  tft.fillScreen(tft.color24to16(0xFFFFFF));  //Background color

  //___________________________________________< Shape >
  // x, y, w, h, color ---<rect>
  // x, y, r, color ---<circle>

  // Biggest RectRed
  tft.fillRect(275, 5, 200, 275, tft.color24to16(0xb22222));

  // ResultScoresRect
  tft.fillRect(280, 190, 190, 60, tft.color24to16(0xFFFFFF));
  tft.fillRect(285, 195, 180, 50, tft.color24to16(0x538234));

  // Claim Score Button Rect
  tft.fillRect(305, 265, 175, 55, tft.color24to16(0xFFFFFF));
  tft.fillRect(310, 270, 165, 45, tft.color24to16(0xb22222));

  // CountBattRect
  tft.fillRect(20, 190, 230, 90, tft.color24to16(0xd9d9d9));

  // let's compute Button Rect
  tft.fillRect(45, 265, 180, 55, tft.color24to16(0xFFFFFF));
  tft.fillRect(50, 270, 170, 45, tft.color24to16(0xd9d9d9));

  // ChooseBatt Button
  tft.fillCircle(75, 125, 50, tft.color24to16(0xa6a5a5));
  tft.fillCircle(75, 125, 45, tft.color24to16(0xb22222));
  tft.fillCircle(195, 125, 50, tft.color24to16(0xa6a5a5));
  tft.fillCircle(195, 125, 45, tft.color24to16(0xb22222));

  //___________________________________________< Text >
  //_______________< left screen >
  // tft.setTextSize(1);
  // tft.setTextColor(tft.color24to16(0xb22222));
  // tft.setTextDatum(TL_DATUM);
  // tft.drawString("Intelligent Sorting Cabinet for Battery Disposal and Recycling", 15, 10);

  tft.setTextSize(1);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Please choose the kind of battery", 15, 35);
  tft.drawString("you want to get rid of", 120, 50);

  tft.setTextSize(3);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("AA", 75, 125);
  tft.drawString("AAA", 195, 125);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("QTY of AA: " + String(QTY_AA), 35, 200);
  tft.drawString("QTY of AAA: " + String(QTY_AAA), 35, 240);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("let's compute", 59, 285);

  //_______________< right screen >
  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("processing...", 280, 10);

  // tft.setTextSize(2);
  // tft.setTextColor(tft.color24to16(0xFFFFFF));
  // tft.setTextDatum(TL_DATUM);
  // tft.drawString("processed", 280, 10);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Claim Score", 325, 285);
  
  tft.setTextSize(1);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("- AA Battery: 1 point / piece", 285, 60);
  tft.drawString(String(QTY_AA) + "    pce", 390, 80);
  
  tft.setTextSize(1);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("- AAA Battery: 2 point / piece", 285, 120);
  tft.drawString(String(QTY_AAA) + "    pce", 390, 140);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Result: " + String(AllScore) + " PTS", 293, 212);
}

//====================================================< Func_InputPhoneNumberScreen >
void Func_InputPhoneNumberScreen(){
  //___________________________________________< Shape >
  // Biggest Rect
  tft.fillRect(90, 0, 300, 320, tft.color24to16(0xd9d9d9));
  tft.fillRect(100, 10, 280, 300, tft.color24to16(0xb22222));

  // Show num Rect
  tft.fillRoundRect(110, 35, 260, 40, 10, tft.color24to16(0xFFFFFF));

  // All Button 
  tft.fillCircle(170, 110, 25, tft.color24to16(0xFFFFFF)); //1
  tft.fillCircle(240, 110, 25, tft.color24to16(0xFFFFFF)); //2
  tft.fillCircle(310, 110, 25, tft.color24to16(0xFFFFFF)); //3

  tft.fillCircle(170, 165, 25, tft.color24to16(0xFFFFFF)); //4
  tft.fillCircle(240, 165, 25, tft.color24to16(0xFFFFFF)); //5
  tft.fillCircle(310, 165, 25, tft.color24to16(0xFFFFFF)); //6

  tft.fillCircle(170, 220, 25, tft.color24to16(0xFFFFFF)); //7
  tft.fillCircle(240, 220, 25, tft.color24to16(0xFFFFFF)); //8
  tft.fillCircle(310, 220, 25, tft.color24to16(0xFFFFFF)); //9

  tft.fillCircle(240, 275, 25, tft.color24to16(0xFFFFFF)); //0

  tft.fillCircle(170, 275, 25, tft.color24to16(0x76b5c5)); //<
  tft.fillCircle(310, 275, 25, tft.color24to16(0xc1c1c1)); //OK

  //___________________________________________< Text >
  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Input your Phonenumber", 110, 15);

  tft.setTextSize(3);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(TL_DATUM);
  tft.drawString(": " + PhoneNumber, 128, 46);

  tft.setTextSize(3);
  tft.setTextColor(tft.color24to16(0xb22222));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("0", 240, 275);
  tft.drawString("1", 170, 110);
  tft.drawString("2", 240, 110);
  tft.drawString("3", 310, 110);
  tft.drawString("4", 170, 165);
  tft.drawString("5", 240, 165);
  tft.drawString("6", 310, 165);
  tft.drawString("7", 170, 220);
  tft.drawString("8", 240, 220);
  tft.drawString("9", 310, 220);

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("<", 170, 275);
  tft.drawString("OK", 310, 275);

}

//====================================================< Func_PopupThank >
void Func_PopupThank(){
  //___________________________________________< Shape >
  tft.fillRect(45, 70, 390, 180, tft.color24to16(0x538234));
  tft.fillRect(55, 80, 370, 160, tft.color24to16(0xb22222));

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("The score has already been", 240, 110);
  tft.drawString("sent by the system,", 240, 130);
  tft.drawString("Thank you very much", 240, 180);
  tft.drawString("for using our service.", 240, 200);
}

//====================================================< Func_PopupWaitWarping >
void Func_PopupWaitWarping(){
  //___________________________________________< Shape >
  tft.fillRect(45, 80, 390, 160, tft.color24to16(0x76b5c5));
  tft.fillRect(55, 90, 370, 140, tft.color24to16(0xb22222));

  tft.setTextSize(2);
  tft.setTextColor(tft.color24to16(0xFFFFFF));
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Please wait a moment", 240, 130);
  tft.drawString("warpping batteries..", 240, 180);
}

//====================================================< Func_RecordPoint >
void Func_RecordPoint(){
  String str_QTY_AAA = String(QTY_AAA);
  String str_QTY_AA = String(QTY_AA);

  HTTPClient http;
  String url = "https://script.google.com/macros/s/AKfycbxmgbtWjHdyo9iiQZ7ytJHrfgWSAA7ODUdNztxbzTc_qvaj7XOAof-d-F2PUuFDL_7aMQ/exec?RecordPoint=1&AAA=" + str_QTY_AAA + "&AA=" + str_QTY_AA  +"&PhoneNumber=" + PhoneNumber;
  http.begin(url.c_str()); 
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

	int httpCode = http.GET();
  Serial.print("httpcode: " + String(httpCode));

  http.end();
}










