int numMagnets=4; //the number of magnets around the drive shaft (MUST be equally spaced)
float tireDiameter= 0.6604; //the diameter of the tire, in meters. (assuming 1:1 tire to driveshaft rotation)
//0.6604 m = 2'2"

bool pole=true;

unsigned long timer=0;

int runningAvg[4];

float rpm;
float hz;
float vehicleSpeedMetersPS=0;
float vehicleSpeedMPH=0;

float timeBetweenUpdates=0;

bool lastPole=true;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200); //change to 11500 to print faster when printing magnetic rotations
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(2,INPUT); //must be an interrupt capable pin. 
  /*LM339 comparator connections:
   * In- is a voltage divider w/ 4.7k to 3.3V and 390R to gnd (output around 250 mv)
   * In+ is output of hall effect sensor
   * OUTPUT has a pullup resistor on it (5k)
   */

  pinMode(2,INPUT); //must be an interrupt capable pin.
  attachInterrupt(digitalPinToInterrupt(2), interruptMagSpeedTransition, RISING);
}

void interruptMagSpeedTransition(){
  long elapsedTime=millis()-timer;
  timer=millis(); //reset timer
  timeBetweenUpdates=elapsedTime/1000.0;

  //Average last few values using HZ
  runningAvg[0]=runningAvg[1];
  runningAvg[1]=runningAvg[2];
  runningAvg[2]=runningAvg[3];
  runningAvg[3]=elapsedTime;

  elapsedTime=0;
  for(int i=0;i<4;i++){
    elapsedTime+=runningAvg[i];
  }
  elapsedTime=elapsedTime/4;
  
  hz = 1.0/(((elapsedTime/1000.0)*numMagnets));  //

  rpm = hz*60;

  float pointDistance = (PI*tireDiameter)/numMagnets; //distance between two adjacent magnets, scaled to distance on tire
  vehicleSpeedMetersPS = pointDistance/(elapsedTime/1000.0); // m/s
  vehicleSpeedMPH = vehicleSpeedMetersPS*(1/1609.34)*(3600/1);      //*(miles/meter)*(seconds/hour)
  
}

void loop() {
  //delay(5000);

  if(millis()-timer>2000){ //if we haven't seen a magnet in x seconds, we're not moving;
    rpm=0;
    hz=0;
    vehicleSpeedMetersPS=0;
    vehicleSpeedMPH=0;
  }
  //Serial.println(rpm);
  Serial.print("HZ:\t");
  Serial.print(hz,4);
  Serial.print("\t");
  Serial.print("RPM:\t");
  Serial.print(rpm,1);
  /*Serial.print("\t");
  Serial.print("Speed: ");
  Serial.print(vehicleSpeedMetersPS);
  Serial.print(" m/s");
  Serial.print("   ");
  Serial.print(vehicleSpeedMPH);
  Serial.print(" mph");
  Serial.print("\tTime between updates (s): ");
  Serial.print(timeBetweenUpdates);*/
  Serial.println();
}
