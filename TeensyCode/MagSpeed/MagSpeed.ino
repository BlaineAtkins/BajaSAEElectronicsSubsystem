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
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
}

void loop() {
  
  //Serial.println(analogRead(A0));
  if(analogRead(A0)>50){
    digitalWrite(LED_BUILTIN,HIGH);
    pole=true;
  }else{
    digitalWrite(LED_BUILTIN,LOW);
    pole=false;
  }

  if(lastPole!=pole){ //if we've spun to a new magnet
    long elapsedTime=millis()-timer;
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

    Serial.print("\t");
    Serial.print(vehicleSpeedMPH);
    Serial.println();
    timer=millis();
  }

  if(millis()-timer>1000){ //if we haven't seen a magnet in x seconds, we're not moving;
    rpm=0;
    hz=0;
    vehicleSpeedMetersPS=0;
    vehicleSpeedMPH=0;
  }

  //Serial.println(rpm);
/*  Serial.print("HZ: ");
  Serial.print(hz,4);
  Serial.print("\t");
  Serial.print("RPM: ");
  Serial.print(rpm,1);
  Serial.print("\t");
  Serial.print("Speed: ");
  Serial.print(vehicleSpeedMetersPS);
  Serial.print(" m/s");
  Serial.print("   ");
  Serial.print(vehicleSpeedMPH);
  Serial.print(" mph");
  Serial.print("\tTime between updates (s): ");
  Serial.println(timeBetweenUpdates);*/

  lastPole=pole;
}
