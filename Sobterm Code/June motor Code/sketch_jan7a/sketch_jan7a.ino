char in = '0';

#define PutBoxStack_L 14
#define PutBoxSpin_L 27
#define SpinBox_L 26

#define PutBoxStack_R 32
#define PutBoxSpin_R 33
#define SpinBox_R 25

#define HotGun_L 0
#define HotGun_R 4

void setup() {
  Serial.begin(9600);

  //_____________________< Relay >
  pinMode(14,OUTPUT);
  pinMode(27,OUTPUT);
  pinMode(26,OUTPUT);
  pinMode(25,OUTPUT);
  pinMode(33,OUTPUT);
  pinMode(32,OUTPUT);
  pinMode(0,OUTPUT);
  pinMode(4,OUTPUT);

  // digitalWrite(14,1);
  // delay(716);  //ok
  // digitalWrite(14,0);
  // digitalWrite(27,1);
  // delay(736);  //ok
  // digitalWrite(27,0);
  // digitalWrite(26,1);
  // delay(10000);  
  // digitalWrite(26,0);
  digitalWrite(0,0); 
  digitalWrite(4,0); 

}

void loop() {
  if(Serial.available() > 0){
    in = Serial.read();
    //___________________L side
    if(in == '1'){
      digitalWrite(14,1); 
      delay(780); //ok
      digitalWrite(14,0);
      in = '0';
    }
    if(in == 'a'){
      digitalWrite(14,1); 
      delay(200);
      digitalWrite(14,0);
      in = '0';
    }

    if(in == '2'){
      digitalWrite(27,1); 
      delay(780); //ok
      digitalWrite(27,0);
      in = '0';
    }
    if(in == 'b'){
      digitalWrite(27,1); 
      delay(200);
      digitalWrite(27,0);
      in = '0';
    }

    if(in == 'l'){
      digitalWrite(26,1); 
      // digitalWrite(0,1);
      delay(10350);
      digitalWrite(26,0);
      // digitalWrite(0,0);
      in = '0';
    }
    if(in == 'L'){
      digitalWrite(26,1); 
      // digitalWrite(0,1);
      delay(250);
      digitalWrite(26,0);
      in = '0';
    }

    //___________________R side
    if(in == '3'){
      digitalWrite(32,1); 
      delay(780); //ok
      digitalWrite(32,0);
      in = '0';
    }
    if(in == 'c'){
      digitalWrite(32,1); 
      delay(200);
      digitalWrite(32,0);
      in = '0';
    }

    if(in == '4'){
      digitalWrite(33,1); 
      delay(950); //ok
      digitalWrite(33,0);
      in = '0';
    }
    if(in == 'd'){
      digitalWrite(33,1); 
      delay(200);
      digitalWrite(33,0);
      in = '0';
    }

    if(in == 'r'){
      digitalWrite(25,1); 
      // digitalWrite(4,1); 
      delay(10350);
      digitalWrite(25,0);
      // digitalWrite(4,0); 
      in = '0';
    }

    if(in == 'R'){
      digitalWrite(25,1); 
      // digitalWrite(4,1); 
      delay(200);
      digitalWrite(25,0);
    }
  }
}
