
const byte RED[] = {255, 0, 0}; 
const byte BLUE[] = {0, 0, 255};
const byte MAGENTA[] = {255, 0, 255};
const byte OFF[] = {0,0,0};

int RGBLED[] = {A1, A2, A3};
int motor = 9; 
int thermistor = A0;
/*____________________________________________________________________________________*/
void setup(){
 Serial.begin(9600);
 pinMode(motor, OUTPUT);
for(int i = 0; i < 3; i++){
   pinMode(RGBLED[i], OUTPUT);   //Set the three LED pins as outputs
  }
  setColour(RGBLED, OFF);       //Turn off led 1
}
void loop(){
 analyseTemperatureAndAdjustFan();
}
/*____________________________________________________________________________________*/
void analyseTemperatureAndAdjustFan(){
  
  double temperature =(analogRead(thermistor) * .004882814);  
  temperature = ((temperature - .5) * 100); 
  
  // Fuzzify temperature into sets 
  double cold, warm, hot;
  cold = inverseGrade(temperature, 10, 15);
  warm = triangular  (temperature, 10, 20, 30);
  hot  = grade       (temperature, 20, 30);
  
  // Create rules for computing fan speeds and colour
  double stopFan, slowFan, maintainFan, fastFan, maxFan;
  double blueOff, blueLow, blueMid, blueHigh, blueOn, redOff, redLow, redMid, redHigh,redOn;
  
  stopFan     = AND(cold, NOR(warm,hot));        // If cold and neither warm nor hot
  slowFan     = AND(AND(cold, warm), NOT(hot));  // If between cold and warm but not hot
  maintainFan = OR(warm, AND(hot, cold));        // If warm or bizarrely hot and cold (abstract but makes sense in terms of logic table)
  fastFan     = AND(AND(hot,warm), NOT(cold));   // If between warm and hot but not cold
  maxFan      = AND(hot, NOR(cold,warm));        // If hot and neither cold nor warm
  
  // The same axioms as above are used to create rules for the values 
  // of Red and Blue (Green is 0 as we only want colours between Blue and Red) 
  blueOff     = AND(hot, NOR(cold,warm));
  blueLow     = AND(AND(cold, warm), NOT(hot)); 
  blueMid     = OR(warm, AND(hot, cold)); 
  blueHigh    = AND(AND(hot,warm), NOT(cold)); 
  blueOn      = AND(cold, NOR(warm,hot)); 
  redOff      = blueOn;
  redLow      = blueHigh; 
  redMid      = blueMid; 
  redHigh     = blueLow; 
  redOn       = blueOff;
 
  // And defuzzify out final fan speed and colour
  double rules[5] = {stopFan, slowFan, maintainFan, fastFan, maxFan}, crisps[5] = {0,64,128,192,255};
  double blueRules[5]= {blueOff, blueLow, blueMid, blueHigh, blueOn}, redRules[5] = {redOff, redLow, redMid, redHigh, redOn}, crispBlue[5] = {0,64,128,192,255}, crispRed[5] ={0,64,128,192,255};
  
  double fanSpeed = defuzzSingleton(rules, crisps);
  double cBlue    = defuzzSingleton(blueRules, crispBlue);
  double cRed    = defuzzSingleton(redRules, crispRed);
  const byte col[] = {cRed, 0, cBlue};
  
  // Send values to fan and LED
  analogWrite(motor,fanSpeed);  
  setColour(RGBLED, col);
}
void setColour(int* led, const byte* colour){
 for(int i = 0; i < 3; i++)
   analogWrite(led[i], 255 - colour[i]);
}
/*____________________________________________________________________________________*/
/**************************************************************************************/
/******************************** Fuzzy Logic Functions *******************************/
/**************************************************************************************/
// Grade Function	
double grade(double x, double x0, double x1){
		
  double fuzzied = 0;
		
  if(x <= x0)
    fuzzied = 0;
  else if(x > x0 && x < x1)
    fuzzied = (x / (x1 - x0)) - (x0 / (x1 - x0));  
  else if(x >= x1)
    fuzzied = 1;
		
  return fuzzied;
} 
// Negative Grade Function	
double inverseGrade(double x, double x0, double x1){
		
  double fuzzied = 0;
		
  if(x <= x0)
    fuzzied = 1;
  else if(x > x0 && x < x1)
    fuzzied = (-x / (x1 - x0)) + (x1 / (x1 - x0));
  else if(x >= x1)
    fuzzied = 0;
		
  return fuzzied;
}
// Triangular Membership Function
double triangular(double x, double x0, double x1, double x2){
		
  double fuzzied = 0;
		
  if(x <= x0 || x >= x2)
    fuzzied = 0;
  else if(x > x0 && x < x1)
    fuzzied = (x / (x1 - x0)) - (x0 / (x1 - x0));
  else if (x == x1)
    fuzzied = 1;
  else if (x > x1 && x < x2)
    fuzzied = (-x / (x2 - x1)) + (x2 / (x2 - x1));
		
  return fuzzied;
}
   
// Logical Axiom: Disjunction (2 values)					
double OR(double A, double B){
		
  return max(A, B);
		
}
// Logical Axiom: Conjunction (2 values)					
double AND(double A, double B){
		
  return min(A, B);
		
}
// Logical Axiom: Negation (2 values)					
double NOT(double A){
		
  return 1.0f - A;
}
// Logical Axiom: Negated Conjunction					
double NAND(double A, double B){
		
  return 1 - AND(A, B);
		
}
// Logical Axiom: Negated Disjunction					
double NOR(double A, double B){
		
  return 1 - OR(A, B);
		
}
// Weighted Average Defuzzification Technique or Singleton  
// technique:  µA(x*) = (∑µixi)/(∑xi)						
double defuzzSingleton(double  u[], double  x[]){
			
  double sumOfDegrees = 0;
  double sumOfDegreesMultipliedByCrisps = 0;
		
  for(int i = 0; i < 5; i++)	
    sumOfDegrees += u[i];
		
  for(int i = 0; i < 5; i++)
    sumOfDegreesMultipliedByCrisps += u[i]*x[i];
		
  return sumOfDegreesMultipliedByCrisps/sumOfDegrees;
			
}
