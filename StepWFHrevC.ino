/*
  EKT-1016 Digital Power Driver Shield 
 Stepper Motor Controller       Rev C
 
 Wave, Full or Half Stepper Motor Drivers
 
 ElectronicKit.com
 11/06/2023 1pm
 
 This test driver sketch wave-steps, full-steps or half-steps
 all 16 drivers on port A and B.
 The upper and lower 4 bits make up 2 stepper motor drives
 at 100ms / step (change time in main loop).
 
 Choose either wave, full or half step operation by
 enabling one of the following "#define" pseudo ops:
 
 #define WAVE
 #define FULL
 #define HALF (default code setting)
 -----------------------------------
 Chan A/B TERMINAL BLOCK (TB) pin assignments:
 
 TB  PHASE   MOTOR #
 PIN  LTR    A/B TB
 ----------------------
 1   +V STEPPER POWER SUPPLY
 2    A      1/3
 3    B      1/3
 4    C      1/3
 5    D      1/3
 6    A      2/4
 7    B      2/4
 8    C      2/4
 9    D      2/4
 10  GROUND FOR STEPPER POWER SUPPLY
 
 For 6 leaded unipolar stepper motors, connect the center tapped
 wires to +V STEPPER POWER SUPPLY.
 --------------------------
 There are 4 processes to handle 4 unipolar/bipolar stepper motors
 using the digital power drivers on the EKT-1016 card.  Each process
 execution steps the selected motor 1 step. Direction is set by
 specifing true/false for forward / backward steps respecively.  As
 examples:
 
 StepALwr(false); // Chan A Lower (bits 0-3) motor backward 1 step
 StepBUpr(true);  // Chan B Upper (bits 4-7) motor forward 1 step
 
 You can integrate the 4 "StepALwr/Upr(), StepBLwr/Upr()"
 processes into do, while, for loops to get repeated stepping per
 a desired count and time stepping by your code's timing:
 
 While (int i=0; i < 100; i++)
 {
 StepAUpr(true);  //motor forward 1 step
 delay(50);       //50ms step rate for 100 steps
 }
 
 User can change the card address and enable pin
 per the EKT-1016 User's Manual at
 http://www.electronickit.com/kitproduct/ekt-1016/man1016.pdf
 http://www.antona.com/EKT/kitproduct/ekt-1016/man1016.pdf
 */

#include "Wire.h"

char Rev[ ] = "Rev C";
/*
  USER SELECTED STEPPING MODE -
 WAVE = MAX ANGLE MOVE (LOW TORQUE)
 FULL = 1/2 OF WAVE STEP (MORE STEP RESOLUTION)
 HALF = 1/2 OF FULL STEP (EVEN MORE STEP RESOLUTION, HIGHEST TORQUE)
 ONLY UNCOMMENT 1 OF THESE
 */
// #define WAVE
// #define FULL
#define HALF
/*
  Enable Driver Output
 Enable only one line below
 Card shipped with enb = on always
 */
const int EnbAB = 0; 			// Always Enabled - as shipped
// const int EnbAB = 2; 		// use PD2
// const int EnbAB = 3; 		// use PD3
// const int EnbAB = 7; 		// use PD7
// const int EnbAB = 8; 		// use PB0

/* Choose I2C Shield Address
 // 0 = jumper on pins, 1 = no jumper
 // Card shipped with adr = 000 (all 3 jumpered)
 // Enable only 1 line below
 A2 A1 A0 */
const byte Adr = 0x00;   		//0  0  0 - as shipped
// const byte Adr = 0x01; 		//0  0  1
// const byte Adr = 0x02; 		//0  1  0
// const byte Adr = 0x03; 		//0  1  1
// const byte Adr = 0x04; 		//1  0  0
// const byte Adr = 0x05; 		//1  0  1
// const byte Adr = 0x06; 		//1  1  0
// const byte Adr = 0x07; 		//1  1  1

const byte Base = 0x20 + Adr; 	// shield base address
const byte GpioA = 0x12;      	// port A data reg
const byte GpioB = 0x13;      	// port B data reg

int i = 0;
byte Mot12 = B00000000;		      //Step motor 1 and 2
byte Mot34 = B00000000;		      //Step motor 3 and 4

/*
  WAVE STEP TABLE
 S  A  B  C  D
 1  1  0  0  0
 2  0  1  0  0
 3  0  0  1  0
 4  0  0  0  1
 */
#ifdef WAVE                      //Max move / step      
const int LstStep = 3;           //Last WAVE step entry
byte MskLim = B00000011;         //Mask bits 2 to 7
//Stepper Lower Nibble Masks
byte StepLwr[LstStep + 1] =      //WAVE STEP LOWER NIBBLE
{
  B00001000,
  B00000100,
  B00000010,
  B00000001
};
//Stepper Upper Nibble Masks
byte StepUpr[LstStep + 1] =       //WAVE STEP UPPER NIBBLE
{
  B10000000,
  B01000000,
  B00100000,
  B00010000
};
#endif
/*
  FULL STEP TABLE
 S  A  B  C  D
 1  1  0  1  0
 2  0  1  1  0
 3  0  1  0  1
 4  1  0  0  1
 */
#ifdef FULL                      //Full steps      
const int LstStep = 3;           //Last FULL step entry
byte MskLim = B00000011;         //Mask bits 2 to 7
//Stepper Lower Nibble Masks
byte StepLwr[LstStep + 1] =      //FULL STEPS LOWER NIBBLE
{
  B00001100,
  B00000110,
  B00000011,
  B00001001
};
//Stepper Upper Nibble Masks
byte StepUpr[LstStep + 1] =       //FULL STEPS UPPER NIBBLE
{
  B11000000,
  B01100000,
  B00110000,
  B10010000
};
#endif
/*
  8 STATES OF HALF STEP MOTION
 S  A  B  C  D
 1  1  0  0  1
 2  0  0  0  1
 3  0  0  1  1
 4  0  0  1  0
 5  0  1  1  0
 6  0  1  0  0
 7  1  1  0  0
 8  1  0  0  0
 */
#ifdef HALF
const int LstStep = 7;           //Last FULL step entry
byte MskLim = B00000111;         //Mask bits 3 to 7 off
//Stepper Lower Nibble Masks
byte StepLwr[LstStep + 1] =      //HALF STEP LOWER NIBBLE
{
  B00001001,
  B00000001,
  B00000011,
  B00000010,
  B00000110,
  B00000100,
  B00001100,
  B00001000
};
//Stepper Upper Nibble Masks
byte StepUpr[LstStep + 1] =       //HALF STEP UPPER NIBBLE
{
  B10010000,
  B00010000,
  B00110000,
  B00100000,
  B01100000,
  B01000000,
  B11000000,
  B10000000
};
#endif

byte MaskUpr = B00001111;		      //Mask out upr nibble
byte MaskLwr = B11110000;		      //Mask out Lwr nibble
boolean Dirc = true;			        //Step direction, forward = true, backward = false
int CtAUp = 0;					          //State counter Chan A upper
int CtALw = 0;					          //Chan A lower
int CtBUp = 0;					          //Chan B upper
int CtBLw = 0;					          //Chan B lower

void setup()
{
  Serial.begin(9600);             //For test and log output
  if (EnbAB > 0)
  {
    pinMode(EnbAB, OUTPUT);
    digitalWrite(EnbAB, HIGH); 	  // disable all drivers
  }
  Wire.begin(); 					        // start

  // Set Port A and B to Output

  Wire.beginTransmission(Base);   // Base address
  Wire.write(0x00); 		 		      // IODIRA set port A I/O
  Wire.write(0x00); 		 		      // Port A = output
  Wire.endTransmission();

  Wire.beginTransmission(Base); 	// Base address
  Wire.write(0x01); 		 		      // IODIRB set port B I/O
  Wire.write(0x00); 		 		      // Port B = output
  Wire.endTransmission();

  Serial.print("Stepper output ");
  Serial.println(Rev);
  /*
      ----------------------------
   M A I N  L O O P
   
   TEST ALL 4 STEPPER MOTOR DRIVES
   
   Each call to one of the Stepper drives sets
   the A/B port Up/Lw nibble 1 step backward/forward
   by "Dir" entry
   ----------------------------
   */
}
void loop()
{
  StepAUpr(true);		        //Chan A upr forward 1 step
  StepALwr(false);		      //Chan A Lwr backward 1 step
  StepBUpr(true);		        //Chan B upr forward 1 step
  StepBLwr(false);		      //Chan B Lwr backward 1 step
  DrvAB();		              //Drive TB A and B output pins

  Tp();                               //FOR DEBUG// Serial output port A/B
  delay(50); 	                      //FOR DEBUG// Milliseconds/step
}
/*
  ------------------------------
 Upper Stepper on A port
 ------------------------------
 */
void StepAUpr(boolean Dir)
{
  if (Dir)			                //Forward ?
  {
    CtAUp++;			              //Move ahead 1 full step
    CtAUp = CtAUp & MskLim;     //Reset to 00 if >= limit
  }
  else				                  //Step Backward
  {
    CtAUp--;			              //Move back 1 full step
    if (CtAUp < 0)		          //Below beginning of table?
    {
      CtAUp = LstStep;		      //Reset to state end
    }
  }
  Mot12 = ((Mot12 & MaskUpr) | StepUpr[CtAUp]);
}
/*
  ------------------------------
 Lower Stepper on A port
 ------------------------------
 */
void StepALwr(boolean Dir)
{
  if (Dir)			               //Forward ?
  {
    CtALw++;			             //Move ahead 1 full step
    CtALw = CtALw & MskLim;    //Reset to 00 if >= limit
  }
  else				                //Step Backward
  {
    CtALw--;			            //Move back 1 full step
    if (CtALw < 0)		        //Below beginning of table?
    {
      CtALw = LstStep;		    //Reset to state end
    }
  }
  Mot12 = ((Mot12 & MaskLwr) | StepLwr[CtALw]);
}
/*
  ------------------------------
 Upr Stepper on B port
 ------------------------------
 */
void StepBUpr(boolean Dir)
{
  if (Dir)			              //Forward ?
  {
    CtBUp++;			            //Move ahead 1 full step
    CtBUp = CtBUp & MskLim;   //Reset to 00 if >= table end
  }
  else				                //Step Backward
  {
    CtBUp--;			            //Move back 1 full step
    if (CtBUp < 0)		        //Below beginning of table?
    {
      CtBUp = LstStep;		    //Reset to table end
    }
  }
  Mot34 = ((Mot34 & MaskUpr) | StepUpr[CtBUp]);
}
/*
  ------------------------------
 Lower Stepper on B port
 ------------------------------
 */
void StepBLwr(boolean Dir)
{
  if (Dir)			              //Forward ?
  {
    CtBLw++;			            //Move ahead 1 full step
    CtBLw = CtBLw & MskLim;   //Reset to 00 if >= table end
  }
  else				                //Step Backward
  {
    CtBLw--;			            //Move back 1 full step
    if (CtBLw < 0)		        //Below beginning of table?
    {
      CtBLw = LstStep;		    //Reset to table end
    }
  }
  Mot34 = ((Mot34 & MaskLwr) | StepLwr[CtBLw]);
}
/*
  ------------------------------
 Output Stepper Drive bytes
 
 This process, or a version of it,
 MUST be executed for the stepper 
 moves to be output
 ------------------------------
 */
void DrvAB()
{
  digitalWrite(EnbAB, HIGH); 	  //Disable all drivers

  Wire.beginTransmission(Base);
  Wire.write(GpioA); 			      // port A
  Wire.write(~Mot12); 			    // invert data
  Wire.endTransmission();

  Wire.beginTransmission(Base);
  Wire.write(GpioB);  			    // port B
  Wire.write(~Mot34); 			    // invert data
  Wire.endTransmission();

  digitalWrite(EnbAB, LOW); 		// enable all drivers
}
/*
  ------------------------------
 Output motor steps to serial port
 For testing only
 ------------------------------
 */
void Tp()
{
  Serial.print(CtAUp);          //Ref count for Upr A motor only
  Serial.print(" ");
  if (Mot12 < B01111111)
  {
    Serial.print("0");         //Restore leading zero
  }
  if (Mot12 < B00111111)
  {
    Serial.print("0");         //Restore 2nd leading zero
  }
  if (Mot12 < B00011111)
  {
    Serial.print("0");         //Restore 3rd leading zero
  }
  Serial.print(Mot12, BIN);
  Serial.print(" ");
  if (Mot34 < B01111111)
  {
    Serial.print("0");        //Restore leading zero
  }
  if (Mot34 < B00111111)
  {
    Serial.print("0");        //Restore 2nd leading zero
  }
  if (Mot34 < B00011111)
  {
    Serial.print("0");        //Restore 3rd leading zero
  }
  Serial.println(Mot34, BIN);
}

