//
//   NPCC Mobile Iris and Tally Slave
//          This program is intended to run on an Arduino microcontroller.  It receives Serial1 MasterMsgs
//          from the Master Arduino via XBee's.  The XBee's are configured in Beacon mode to allow the Master to 
//          control many slaves across the same XBee device.  This Slave code quietly performs what the Master requests.
//
//   DS1803 http://datasheets.maximintegrated.com/en/ds/DS1803.pdf
//          This chip houses 2 10K digital pots.  We wire them in series to get an operating range of 0-20K Ohms
//          We use the Arduino SCL and SDA pins to communicate with the DS1803.  The Arduino "Wire.h" interface makes it easy.
//   History:
//        03/06/14 - (PH) Original
//        03/20/14 - (PH) Took out MEGA #ifdefs
//        03/20/14 - (PH) Fixed msg for me logic
//        03/26/14 - (PH) limited combined pot values from 0-510 (511 values)
//        04/02/14 - (PH) changed pins for dips switches (from 5, 6, 7 to 11, 14, 15)
//        
//   ToDo:
//        Blink error code on LED if no inbound messages for set time.
//

#include <Wire.h>

#define WIPER_0 0xA9    // Directive to move pot 0
#define WIPER_1 0xAA    // Directive to move pot 1
#define WIPER_01 0xAF   // Directive to move both wipers to same value at once (we aren't using this)


int CameraAddress = 0;      // If we can't read our address don't respond to anything

void adjustPot (int pot, int step)
{
  Wire.beginTransmission(0x28);      // 0x28 is the address of "this" chip when all 3 address bits are grounded 
  Wire.write(pot);
  Wire.write(step);
  Wire.endTransmission();
}

  // Slave is written as simply as possible so most adjustments will be on the master side
  // Master simply sends the step at wich the slave needs to move to.
void setDigitalPot (int step)
{ 
  if (step > 255){
    
    if (step > 510)    // We can only represent 0-510 resistance steps w/ 2 pots. 0-255 plus 1-255 on the second. 
      step=510;       // 0th position on second pot is used for values up to 255, thus leaving only 1-255 steps above 255
    
    adjustPot (WIPER_0, 255);        // Since dualStep > 255 crank first pot to max
    adjustPot (WIPER_1, step -255);  // crank second pot to remainder above 255
  }
  else{
    adjustPot (WIPER_0, step);       // first pot can handle this value by itself !
    adjustPot (WIPER_1, 0);          // maybe next time wiper 1 :)
  }
}



  // This is just a test mode dummy routine.  It will be replaced by a true read/translate XBee message.
#define TALLY_PIN 10

String MasterMsg = "";    // MasterMsg may be assembled across 2 calls of readMasterMsg

int readMasterMsg (int *tally, int *potStep)
{
    char ch;
   
    while ((ch=Serial1.read()) != '<'){};    // discard until we get a <

    if (ch != '<')
      return 0;
      
    MasterMsg = "";
    while (1){

      ch = Serial1.read();

      if (ch == -1)
        continue;
      MasterMsg += ch;
      if (ch=='>')
        break;
    }

             
                  // Subtract ASCII '0' from the ASCII address to result in a binary address :)
    if (MasterMsg[0] -'0' == CameraAddress){ 
      

      Serial.print("<");
      Serial.println(MasterMsg);

        
      if (MasterMsg[1] == 'H')
        *tally = HIGH;
      else
        *tally = LOW;
                                      // convert 3 byte ascii decimal address to int :)
      *potStep = (MasterMsg[2] - '0') *100 + (MasterMsg[3] - '0') *10 + (MasterMsg[4] - '0');
      
      return 1;   // 1 means YES this message was for me !
      
    }   
    
    return 0;
}


#define DIP_ON LOW             // If voltage is LOW (grounded), the dip switch is in the ON position.  
#define NUM_ADDRESS_BITS 3     // 2**9 gives 0-511

unsigned int address_pins[NUM_ADDRESS_BITS] = {15,14,11};


  // Initialize ...
void setup() {
  
  int i;
  unsigned char val;
 
  Serial.begin(9600); 
  
  Serial1.begin(9600); 
  while (!Serial1){};    // Wait for Serial1 to connect to XBee


  
  Wire.begin();
  
  pinMode (TALLY_PIN, OUTPUT);
 
               // On power up or reset read camera address from dip switch 
  for (i=0; i<NUM_ADDRESS_BITS; i++)
  {
    pinMode(address_pins[i], INPUT);           // set pin to input
    digitalWrite(address_pins[i], HIGH);       // turn on pullup resistor
    
    val = digitalRead(address_pins[i]);
    if (val==DIP_ON)
      bitSet(CameraAddress, NUM_ADDRESS_BITS -i -1);   
  }  
  

  Serial.print("My Camera Address = ");
  Serial.println(CameraAddress);
  
}


  // Main processing loop ...
void loop() {
  
  static int tally, potStep;
  
  if (readMasterMsg(&tally, &potStep)){   // Read the next wessage that is destined for me (return !=0)
  
    Serial.print("tally, potStep = ");
    Serial.print(tally);
    Serial.print(", ");
    Serial.println(potStep);
  
    setDigitalPot(potStep);
  
    digitalWrite(TALLY_PIN, tally); 
  }
 
}
