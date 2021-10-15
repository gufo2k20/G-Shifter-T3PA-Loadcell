// partially based on

//Logitech Driving Force Shifter USB Adapter 
//By Armandoiglesias 2018
//Based on Jason Duncan functionreturnfunction Project
//.................
//Attribution-NonCommercial-NoDerivatives 4.0 International

#include <Joystick.h>
#include <HX711.h>

// loadcell PINs definition
const int LOADCELL_DOUT_PIN1 = 3;
const int LOADCELL_SCK_PIN1 = 5;

long Freno_cella;

// brake pedal loadcell range values
const long ZonainferioreFreno = -2850;
const long ZonasuperioreFreno = 12500;

// loadcell calibration factor
const long calibration_factor1 = 30;

// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
12, 0,                 // pulsanti, hat switch
false, false, false,   // X, Y, Z
false, false, false,   // Rx, Ry, Rz
false, true,           // rudder, throttle
true, true, false);    // accelerator, brake, steering

// create the loadcell scale
HX711 scale1;

// H-shifter mode analog axis thresholds
#define HS_XAXIS_12        400
#define HS_XAXIS_56        600
#define HS_YAXIS_135       800
#define HS_YAXIS_246       300

// Sequential shifter mode analog axis thresholds
//-----------------------------------------------------------------------
// MODIFY IN RELATION TO GEAR LEVER TRAVEL DISTANCE USED FOR SEQ SHIFTING
//-----------------------------------------------------------------------
#define SS_UPSHIFT_BEGIN   670
#define SS_UPSHIFT_END     600
#define SS_DOWNSHIFT_BEGIN 430
#define SS_DOWNSHIFT_END   500

// Digital inputs definitions
#define DI_REVERSE         1
#define DI_MODE            3
 
// Shifter state  (???, original code)
#define DOWN_SHIFT         -1
#define NO_SHIFT           0
#define UP_SHIFT           1

// Shifter mode
#define SHIFTER_H_MODE
int shiftSSmode = 0;

// Shifter state
int shift=NO_SHIFT;

int b[16];

int gear=0;               // Default value for H shifter is neutral
int gearSS=0;             // Default value for SEQ shifter is neutral


void setup() {

 // --- TO DO hardware switch ---
 // switch brake pedal "LOADCELL" or "POT" on PIN 7
  pinMode(7, INPUT_PULLUP);
  
  // loadcell HX711 brake pedal
  scale1.begin(LOADCELL_DOUT_PIN1, LOADCELL_SCK_PIN1);
  scale1.set_scale(calibration_factor1);
  
  // G29 shifter analog inputs configuration 
  pinMode(A1, INPUT_PULLUP);   // X axis
  pinMode(A0, INPUT_PULLUP);   // Y axis
  pinMode(2, INPUT);           // reverse switch

    for(int i=0; i<16; i++) b[i] = 0;
  b[DI_MODE] =0;
  
  // Initialize Joystick Library
  // GAS and CLUTCH potentiometers range
  Joystick.begin();
  Joystick.setThrottleRange (165,822);                              //CLUTCH
  Joystick.setAcceleratorRange (160,800);                           //GAS

  // Last state of the button
  int lastButtonState = 0;

//Serial.begin(9600);
}

void loop() {

// richiamo controllo pedaliera
int T3PA_SWITCH_OLD;
int T3PA_SWITCH = digitalRead(7);

if (T3PA_SWITCH != T3PA_SWITCH_OLD){                                  //Check BRAKE PEDAL switch(LC/POT)
  if (T3PA_SWITCH == HIGH){
    Joystick.setBrakeRange (ZonainferioreFreno, ZonasuperioreFreno);  //BRAKE LOAD CELL
      } else {
    Joystick.setBrakeRange (190,885);                                 //BRAKE POT
      }
    }
    T3PA_SWITCH_OLD = T3PA_SWITCH;

if (T3PA_SWITCH == HIGH){
    T3PA_LOADCELL();
    } else {
    T3PA_POT();
    }

// shifter mode selection
int shiftmode_x=analogRead(1);       // X axis for shifter mode check
int shiftmode_y=analogRead(0);       // Y axis for shifter mode check

int shiftmode_press = digitalRead(2);
if( shiftmode_press == 1 && (shiftmode_x>HS_XAXIS_12 && (shiftmode_x<HS_XAXIS_56))) {
      if
        (shiftmode_y > HS_YAXIS_135)
        {
          shiftSSmode = 1;
          }
        else
        {
      if
        (shiftmode_y < HS_YAXIS_246)
        {
          shiftSSmode = 0;
          }
        }
  }

// Go to the selected shifter mode section (H or SS)
if (shiftSSmode == 0) {
    SHIFTER_H();
    } else {
    SHIFTER_SS();
    }

  delay(15);
}

void T3PA_LOADCELL(){
  int g=analogRead(3);              //gas
  int z=analogRead(2);              //clutch
  Freno_cella = scale1.get_units(); //brake (cella di carico)

  Joystick.setThrottle(z);          //CLUTCH
  Joystick.setAccelerator(g);       //GAS
  Joystick.setBrake(Freno_cella);   //BRAKE CELLA DI CARICO
}

void T3PA_POT(){
  int g=analogRead(3);              //gas
  int z=analogRead(2);              //clutch
  int f=analogRead(10);             //brake (potenziometro)
  
  Joystick.setThrottle(z);         //CLUTCH
  Joystick.setAccelerator(g);      //GAS
  Joystick.setBrake(f);            //BRAKE POTENZIOMETRO
}

void SHIFTER_H(){
  int H_x=analogRead(1);       // X axis
  int H_y=analogRead(0);       // Y axis

  int _isreverse = digitalRead(2);
  int _gear_ = 0;

if( _isreverse == 1 && (H_x>HS_XAXIS_56 && (H_y<HS_YAXIS_246))) {
      _gear_ = 8;
      b[DI_REVERSE]= 1;
  }else{ 
  
  if(b[DI_MODE]==0)                      // H-shifter mode?
  {
    if(H_x<HS_XAXIS_12)                  // Shifter on the left?
    {
      if(H_y>HS_YAXIS_135) _gear_=1;       // 1st gear
      if(H_y<HS_YAXIS_246) _gear_=2;       // 2nd gear
    }
    else if(H_x>HS_XAXIS_56)             // Shifter on the right?
    {
      if(H_y>HS_YAXIS_135) _gear_=5;       // 5th gear
      if(H_y<HS_YAXIS_246) _gear_=6;       // 6th gear
    }
    else                               // Shifter is in the middle
    {
      if(H_y>HS_YAXIS_135) _gear_=3;       // 3rd gear
      if(H_y<HS_YAXIS_246) _gear_=4;       // 4th gear
      }
    }
  }
  
  if(gear!=6) b[DI_REVERSE]=0;         // Reverse gear is allowed only on 6th gear position
  
   if (_gear_ != gear ){
      gear = _gear_;
      deactivate();
      Joystick.setButton(gear-1, HIGH);
   }
}

void SHIFTER_SS(){

  int SS_x=analogRead(1);       // X axis
  int SS_y=analogRead(0);       // Y axis

  int _SSisreverse = digitalRead(2);
  int _gearSS_;
  unsigned long _timerSS_;
  unsigned long _delaySS_ = 250;
  
if(_SSisreverse != 1 && ((SS_x>HS_XAXIS_12) && (SS_x<HS_XAXIS_56))) {
      if (SS_y>HS_YAXIS_135) _gearSS_ = 10;       // SS downshift
      else if (SS_y<HS_YAXIS_246) _gearSS_ = 9;        // SS upshift
      else if (SS_y<HS_YAXIS_135 && (SS_y>HS_YAXIS_246)) _gearSS_ = 0;
  }
  else _gearSS_ = 0;
  
if (_gearSS_ != 0){  
   if (_gearSS_ != gearSS){
          gearSS = _gearSS_;
          _timerSS_ = millis();{
            if ((millis()-_timerSS_) > _delaySS_)
            deactivate();
            Joystick.setButton(gearSS-1, HIGH);
          }
        }
   } else
     gearSS = _gearSS_;
     deactivate();
     
}

void deactivate(){
  // Depress virtual button for current gear
  for(int i = 0; i <= 10 ; i++ )  Joystick.setButton(i, LOW);
}
