//This code was written by Jethro Jarrett in March 2022
//This was modified from original code and setup by Gadeken & Dorgan (2021) (doi.org/10.5670/oceanog.2021.202)
//During this modification, some redundant features may have remained. This code may not be optimsied, but it works.

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);


String inputstring = "";                             
String sensorstring = "";                            
boolean input_string_complete = false;                //have we received all the data from the PC
boolean sensor_string_complete = false;               //have we received all the data from the Atlas Scientific product


float DESIRED = 0;      //DESIRED DO for now, this will change later based on potentiometer
float DO;                                             //used to hold a floating point number that is the DO
float DObig;
float SUMfloat;
const int RELAY_PIN = 3;


#define WINDOW_SIZE 5  //size of window
int count = 0;

int INDEX = 0;
int VALUE = 0;
int SUM = 0;
int READINGS[WINDOW_SIZE];
float AVERAGED = 0;


int potPin = A0;    // Analog input pin that the potentiometer is attached to
float potValue = 0;  // value read from the pot
float DESIREDBIG = 20; 





void setup() {                                        //set up the hardware
  Serial.begin(9600);                                 //set baud rate for the hardware serial port_0 to 9600
  Serial3.begin(9600);                                //set baud rate for software serial port_3 to 9600
  inputstring.reserve(10);                            //set aside some bytes for receiving data from the PC
  sensorstring.reserve(30);                           //set aside some bytes for receiving data from Atlas Scientific product

  pinMode(RELAY_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();
}





void serialEvent() {                                  //if the hardware serial port_0 receives a char
  inputstring = Serial.readStringUntil(13);           //read the string until we see a <CR>
  input_string_complete = true;                       //set the flag used to tell if we have received a completed string from the PC
}





void serialEvent3() {                                 //if the hardware serial port_3 receives a char
  sensorstring = Serial3.readStringUntil(13);         //read the string until we see a <CR>
  sensor_string_complete = true;                      //set the flag used to tell if we have received a completed string from the PC
}






void loop() {                                         //here we go...

  if (input_string_complete == true) {                //if a string from the PC has been received in its entirety
    Serial3.print(inputstring);                       //send that string to the Atlas Scientific product
    Serial3.print('\r');                              //add a <CR> to the end of the string
    inputstring = "";                                 //clear the string
    input_string_complete = false;                    //reset the flag used to tell if we have received a completed string from the PC
  }




  if (sensor_string_complete == true) {               //if a string from the Atlas Scientific product has been received in its entirety                            


//calculating the rolling average
      DO = sensorstring.toFloat();                    //convert the string to a floating point number so it can be evaluated by the Arduino
      
    if(DO < 20 && DO != 2.20) {                                     //sometimes the input glitches and combines values, this stops most of that


//calculate desired DO from pot
      potValue = analogRead(potPin); // read the pot value  //read value from potentiometer
      DESIREDBIG = (map(potValue, 0, 1023, 0, 50));         //use that to calculate the desired DO
      DESIRED = DESIREDBIG/10;

//calculate rolling average
      DObig = sensorstring.toFloat();                    //convert the string to a floating point number so it can be evaluated by the Arduino
      DObig = DObig*100;

      SUM = SUM - READINGS[INDEX];       // Remove the oldest entry from the sum
      VALUE = DObig;        // Read the next sensor value
      READINGS[INDEX] = VALUE;           // Add the newest reading to the window
      SUM = SUM + VALUE;                 // Add the newest reading to the sum
      INDEX = (INDEX+1) % WINDOW_SIZE;   // Increment the index, and wrap to 0 if it exceeds the window size

      SUMfloat = float(SUM);                    //convert the string to a floating point number so it can be evaluated by the Arduino
      AVERAGED = SUMfloat / (WINDOW_SIZE * 100);      // Divide the sum of the window by the window size for the result

      count=count+1;


//print to the serial
    Serial.print(count); 
    Serial.print(" , ");
    Serial.print(potValue); 
    Serial.print(" , ");
    Serial.print(DESIRED); 
    Serial.print(" - ");
    Serial.print(sensorstring); 
    Serial.print(" , ");
    Serial.print(DO); 
    Serial.print(" , ");
    Serial.print(DObig);
    Serial.print(" , ");
    Serial.print(SUM);
    Serial.print(" , ");
    Serial.print(AVERAGED);
    Serial.print(" , ");
    delay(20);


//printing to LCD
     lcd.setCursor(2, 0); // Set the cursor on the third column and first row.
     lcd.print(DO);
     lcd.print("  ");
     lcd.print(AVERAGED);
     lcd.print("     ");


//turn relay on/off based on reading
    if (isdigit(sensorstring[0])) {                   //if the first character in the string is a digit
            
      if (AVERAGED >= DESIRED && count>WINDOW_SIZE) {                  //if the DO is greater than or equal to desired and isnt first couple of measures
        Serial.println("high");                       //print "high" this is demonstrating that the Arduino is evaluating the DO as a number and not as a string
        digitalWrite(RELAY_PIN, LOW); 
        
        lcd.setCursor(2, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
        lcd.print("Flow OFF ");
        lcd.print(DESIRED, 2); 
      }
      
      if (AVERAGED < DESIRED && count>WINDOW_SIZE) {                               //if the DO is less than or equal to 5.99
        Serial.println("low");                        //print "low" this is demonstrating that the Arduino is evaluating the DO as a number and not as a string
        digitalWrite(RELAY_PIN, HIGH); 
        
        lcd.setCursor(2, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
        lcd.print("Flow ON  "); 
        lcd.print(DESIRED,2);    
      }
    }

  }

  
delay(1000);

//flashy dot so you know LCD isnt frozen
    lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
    lcd.print("."); 

  sensorstring = "";                                  //clear the string:
  sensor_string_complete = false;                     //reset the flag used to tell if we have received a completed string from the Atlas Scientific product

delay(1000);
    
    lcd.setCursor(0, 1); //Set the cursor on the third column and the second row (counting starts at 0!).
    lcd.print(" "); 

  }


//if less than 5 readings...
  else {
    Serial.println("Calculating...");
    delay(500);
  }
  }
