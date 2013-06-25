//###############################\\
//###############################\\
//#     TASK 1 VERSION 4        #\\
//###############################\\
//#  Button selects arpeggiator #\\
//#  mode which determines the  #\\
//#  the order of LED flashes   #\\
//#  and potentiometer selects  #\\
//#  the speed at which they    #\\
//#  flash. This time 8 LEDs    #\\
//#  are used by shifting bits  #\\
//#  with a shift register      #\\
//###############################\\
//###############################\\

#define NOTEON  144
#define NOTEOFF 128

/****************************/
/***** Global Variables *****/
/****************************/

/***** PIN VARIABLES *****/
/* Shift Register Variables */
short data            = 2;  
short latch           = 3;  
short clock           = 4;
/* Button Variables */
short button          = 7;
int button_val        = 0;
/* Potentiometer Variable */
short potentiometer   = A0;

/***** BIT AND BIT MASK ARRAYS *****/
short bits[8]         = {1,2,4,8,16,32,64,128};
short masks[8]        = {254,253,251,247,239,223,191,127};
/* */
short ledState        = 0;

/* ARPEGGIATOR MODES */
const short DOWN      = 0;
const short UP        = 1;
const short DOWN_UP   = 2;
const short RANDOM    = 3;
short mode, mode_flag;

/* MIDI intS */
int notes[] = {60,62,64,65,67,69,71,72};
int velocity = 100;

/* Called when board first receives power */
void setup(){
  
  Serial.begin(31250);
  
  // Set pins to OUTPUT so we can send them data 
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  
  // Initialise  mode and mode_flag vars to 0
  mode = mode_flag = 0;
}

/* Called forever... or at least until turned off */
void loop(){
  
  // Aquire resistance level from potentiometer
  int potentiometer_val = analogRead(potentiometer);
  
  // Aquire Voltage Level from button
  button_val = digitalRead(button);
  
  // Button Pressed: set the mode flag to true
  if(button_val == LOW){
    mode_flag = 1;
  }
  // Button Not Pressed
  else if(button_val == UP){
    // If the button was previously pressed
    // set the flag to false and increment the
    // mode type if it is less than 3 otherwise
    // set it back to 0
    if(mode_flag){
      mode_flag = 0;
      mode < 3 ? mode++ : mode = 0;
    }
  }
  // Call the arpeggiator function to flash LEDs
  // at the speed specified by the value of the 
  // potentiometer
  arpeggiator(potentiometer_val,mode);
} 
/* Copied from the ardx example */
void updateLEDs(short value){
  // Pull the chips latch low 
  digitalWrite(latch, LOW);
  
  // Shifts out the 8 bits int the value
  // parameter to the chip 
  shiftOut(data,clock,MSBFIRST,value);
  
  // And pulls the latch high to update the bits
  digitalWrite(latch,HIGH);
}
/* Copied from ardx example */
void changeLED(short led, short state){
   // Masks bits in the current led state
   // turning on the requested led and the
   // rest off
   ledState = ledState & masks[led]; 
   if(state == HIGH) ledState = ledState | bits[led]; 
   updateLEDs(ledState);  
}
void midiout(int command, int note, int vel){

  Serial.write(command);
  Serial.write(note);
  Serial.write(vel);
}
void arpeggiator(short delayTime, short dir){
  short i;
  switch(dir){
    // If Down mode, flash LEDs from left
    // to right at the speed indicated by
    // the delayTime
    case DOWN:
      for(i = 0;i <= 7; i++){
        changeLED(i,HIGH);
        midiout(NOTEON, notes[i], velocity);
        delay(delayTime);
        changeLED(i,LOW);
        midiout(NOTEOFF, notes[i], 0);
      }
      break;
      
    // If UP mode, flash LEDs from right
    // to left at the speed indicated by
    // the delayTime
    case (UP):
      for(i = 7;i >= 0; i--){
        changeLED(i,HIGH);
        midiout(NOTEON, notes[i], velocity);
        delay(delayTime);
        changeLED(i,LOW);
        midiout(NOTEOFF, notes[i], 0);
      }
      break;
      
     // If Down_UP mode, flash LEDs from left
     // to right and back to left at the speed 
     // indicated by the delayTime
     case (DOWN_UP):
      for(i = 0;i <= 7; i++){
        changeLED(i,HIGH);
        midiout(NOTEON, notes[i], velocity);
        delay(delayTime);
        changeLED(i,LOW);
        midiout(NOTEOFF, notes[i], 0);
      }
      for(i = 7;i >= 0; i--){
        changeLED(i,HIGH);
        midiout(NOTEON, notes[i], velocity);
        delay(delayTime);
        changeLED(i,LOW);
        midiout(NOTEOFF, notes[i], 0);
      }
     break;
     
     // If RANDOM mode, flash whatever
     // LED is corresponding to the value
     // returned by arduino's random function
     // at the speed indicated by the delayTime
     case (RANDOM):
       for(i = 0;i <= 7; i++){
        byte r = random(0,8);
        changeLED(r,HIGH);
        midiout(NOTEON, notes[r], velocity);
        delay(delayTime);
        changeLED(r,LOW);
        midiout(NOTEOFF, notes[r], 0);
      }
     break;
  }
}

