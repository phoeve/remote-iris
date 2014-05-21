//
//   NPCC Mobile Iris and Tally Slave
//          This program is intended to run on an Arduino microcontroller.  It receives serial commands
//          from the Master Arduino via XBee's.  The XBee's are configured in Beacon mode to allow the Master to 
//          control many slaves across the same XBee device.  This Slave code quietly performs what the Master requests.
//
//   DS1803 http://datasheets.maximintegrated.com/en/ds/DS1803.pdf
//          This chip houses 2 10K digital pots.  We wire them in series to get an operating range of 0-20K Ohms
//          We use the Arduino SCL and SDA pins to communicate with the DS1803.  The Arduino "Wire.h" interface makes it easy.
//   History:
//        03/06/14 - (CT) Original
//        03/20/14 - (PH) Working version.
//        03/26/14 - (PH) Added CAMERA_MIN_POT_STEP and MAX to allow for camera calibration
//      
//   ToDO:
//        Add buttons for presets/recall
//
//


#define NUM_CAMERAS 4

int camera_pot_pin[NUM_CAMERAS] = {0,1,2,3};      // Analog Arduino pins (A0, A1 ...)
int camera_tally_pin[NUM_CAMERAS] = {2,3,4,5};    // Digital Arduino pins

  // Initialize ...
void setup() {
  
  //delay(10000);    // Wait 10 secs to allow code upload b4 using serial !!!!
  
  Serial.begin(9600) ;
  while (!Serial){};    // Wait for serial to connect
  
  for (int i=0; i< NUM_CAMERAS; i++){
    
    pinMode (camera_tally_pin[i], INPUT);    // This belongs in the master sketch to read the tally system
    digitalWrite(camera_tally_pin[i], HIGH); // Set pull-up resistor
  }
  
}

#define CAMERA_MIN_POT_STEP 170  // below F11
#define CAMERA_MAX_POT_STEP 390  // above F1.7


  // Main processing loop ...
void loop() {
 char tally; 
 int pot;
 float camera_iris_range;
 
 
 for (int i=0; i< NUM_CAMERAS; i++){
   
   if (digitalRead(camera_tally_pin[i]) == HIGH)    // Port 2 (digital)
     tally = 'L';
   else
     tally = 'H';
        
   // the slave's digital pot only ranges from 0-511.  master's analog port ranges from 0-1023
   // So let's devide by 2   
   pot = analogRead(camera_pot_pin[i]) / 2;   // Port A0... (analog)
   
     // map CAMERA_MIN_POT_STEP -> CAMERA_MAX_POT_STEP   to   0-511
   camera_iris_range = CAMERA_MAX_POT_STEP - CAMERA_MIN_POT_STEP;
   pot = CAMERA_MIN_POT_STEP + (((float) pot * camera_iris_range) / 511.0);

   Serial.print("<");     // Start of new meessage   <0L255>
   Serial.print(i);       // address - 0,1,2,3
   Serial.print(tally);
   
   if (pot < 100)
     Serial.print("0");    // make sure we send 3 digits !
   if (pot < 10)
     Serial.print("0");
     
   Serial.print(pot);
   Serial.print(">");
   
 }
 
 delay(100); 
}
