
#define inPin1 2 //pin connected to optocupler
#define inPin2 3 //pin connected to read switch
#define DEREControlPin 8
#define debugPin 7

#define sensorID 'Q'
#define separator ':'

#define BoundRate 115200

#define time2SetZero 1000

//Variables for freq meter
float interuptFreq = 8000.0; //Hz
bool enteredFirstHigh;
bool enteredSecondHigh;
long timer1 = 0;// time between one full rotation (in ms)
long timer2 = 0;
float freq1 = 0.00;
float freq2 = 0.00;

byte* toSend = new byte[4];

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
  //pinMode(13, OUTPUT);
  InitInterrupt();
}

void loop()
{
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
  
  while (Serial.available() > 0 && i < 1)
  {
    receivedBytes[i] = Serial.read();
    i++;
  }
  if (receivedBytes[0] == 0)
  {
    return false;
  }

  if (receivedBytes[0] != sensorID)
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
      PrintFreq(freq1);
      PrintFreq(freq2);
      break;
    case 'W':
      PrintFreq(freq1);
      break;
    case 'R':
      PrintFreq(freq2);
      break;
  }
  Serial.print(separator);
  Serial.flush();
  digitalWrite(DEREControlPin, LOW); //DE/RE=LOW Receive Enabled
}

void PrintFreq(float freq)
{
  toSend[0] = ~ ((byte *) &freq)[0];
  toSend[1] = ~ ((byte *) &freq)[1];
  toSend[2] = ~ ((byte *) &freq)[2];
  toSend[3] = ~ ((byte *) &freq)[3];
  Serial.write(toSend, 4);
  Serial.print(separator);
  //Twice to chack if there is error in received data
  Serial.write(toSend, 4);
  Serial.print(separator);
}

void InitInterrupt()
{
  //dissable interrupts
  cli();
  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  // Enable global interrupts: Ready to run!
  sei();
}

ISR(TIMER2_COMPA_vect)
{
  if (digitalRead(inPin1) == HIGH)
  {
    if (enteredFirstHigh)
    {
      enteredFirstHigh = false;
      freq1 = interuptFreq / timer1;
      //freq1 = timer1;
      timer1 = 0;
    }
    timer1 += 1;
  }
  else
  {
    if (!enteredFirstHigh)
    {
      enteredFirstHigh = true;
    }
    timer1 += 1;
  }

  if (timer1 > time2SetZero)
  {
    freq1 = 0;//if no new pulses from reed switch- tire is still, set Freq to 0
  }

  if (digitalRead(inPin2) == HIGH)
  {
    if (enteredSecondHigh)
    {
      enteredSecondHigh = false;
      freq2 = interuptFreq / timer2;
      timer2 = 0;
    }
    timer2 += 1;
  }
  else
  {
    if (!enteredSecondHigh)
    {
      enteredSecondHigh = true;
    }
    timer2 += 1;
  }
  if (timer2 > time2SetZero)
  {
    freq2 = 0;//if no new pulses from reed switch- tire is still, set Feeq to 0
  }
}

