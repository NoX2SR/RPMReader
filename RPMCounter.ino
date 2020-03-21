
#define inPin1 2 //pin connected to optocupler
#define inPin2 3 //pin connected to read switch
#define DEREControlPin 8 
#define debugPin 7

#define sensorID 'Q'
#define separator ':'

#define BoundRate 115200

#define interuptFreq  8000 //Hz

//Variables for freq meter
bool enteredFirstHigh;
bool enteredSecondHigh;
long time1 = 0;// time between one full rotation (in ms)
long time2 = 0;
float freq1 = 0.00;
float freq2 = 0.00;

char command = '1';

bool interuptEntered = false;

void setup() 
{
   pinMode(debugPin, OUTPUT);
  Serial.begin(BoundRate);
  pinMode(DEREControlPin, OUTPUT);//DE/RE Controling pin of RS-485
  digitalWrite(DEREControlPin, LOW); //DE/RE=LOW Receive Enabled M1
  pinMode(inPin1, INPUT);;//redd switch
  pinMode(inPin2, INPUT);;//redd switch
  pinMode(13, OUTPUT);
}

void loop()
{
  if(interuptEntered)
  {
    return;
  }
  
  if (tryGetSensorCommand())
  {
    executeCommand();
    command = '1';
  }
}

bool tryGetSensorCommand()
{
  byte receivedBytes[1] {0};
  int i = 0;
  ;
  while (Serial.available() > 0 && i < 1)
  {
    receivedBytes[i] = Serial.read();
    i++;
  }
  if (receivedBytes[0] == 0)
  {
    return false;
  }

  if(receivedBytes[0] != sensorID)
  {
    return false;
  }
  
  if (receivedBytes[0] == 'Q')
  {
    command = receivedBytes[0];
    return true;
  }
  return false;
}

void executeCommand()
{
  digitalWrite(DEREControlPin, HIGH); //DE/RE=HIGH Transmit enable
  delay(1);
  Serial.print(sensorID);
  Serial.print(separator);
  switch (command)
  {
    case 'Q': //Q:ǱB:ǱB:⸮⸮C:⸮⸮C::
     Serial.print("InputPinStatus: \n");
      Serial.print(digitalRead(inPin1));
      Serial.print("\n");
       Serial.print("InputPinStatus2: \n");
      Serial.print(digitalRead(inPin2));
      Serial.print("\n");
      break;
    case 'W':
       PrintFreq1();
      break;
    case 'R':
       PrintFreq2();
      break;
  }
  Serial.print(separator);
  Serial.flush();
  digitalWrite(DEREControlPin, LOW); //DE/RE=LOW Receive Enabled
}

void PrintFreq1()
{
  //digitalWrite(debugPin, HIGH);
  byte* b = (byte *) &freq1;
  
  Serial.write(b, 4);
  Serial.print(separator);
  //Twice to chack if there is error in received data
  Serial.write(b, 4);
  Serial.print(separator);
  //digitalWrite(debugPin, LOW);
}

void PrintFreq2()
{
  //digitalWrite(debugPin, HIGH);
 
  byte* b = (byte *) &freq2;
  
  Serial.write(b, 4);
  Serial.print(separator);
  //Twice to chack if there is error in received data
  Serial.write(b, 4);
  Serial.print(separator);
  //digitalWrite(debugPin, LOW);
}




