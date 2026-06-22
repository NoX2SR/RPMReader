#include <Arduino.h>

#include <cassert>
#include <cmath>
#include <cstring>
#include <vector>

void setup();
void loop();
bool tryGetSensorCommand();
void executeCommand();
void PrintFreq(float freq);
void InitInterrupt();
void TIMER2_COMPA_vect();

#include "../RPMReader.ino"

namespace {
std::vector<byte> encodedFloat(float value) {
  byte bytes[sizeof(value)];
  std::memcpy(bytes, &value, sizeof(value));
  for (byte& item : bytes) {
    item = static_cast<byte>(~item);
  }
  return std::vector<byte>(bytes, bytes + sizeof(value));
}
}  // namespace

int main() {
  resetFakeArduino();
  setup();
  assert(Serial.baudRate == 115200UL);
  assert(fakePinModes[debugPin] == OUTPUT);
  assert(fakePinModes[DEREControlPin] == OUTPUT);
  assert(fakeDigitalValues[DEREControlPin] == LOW);
  assert(fakePinModes[inPin1] == INPUT);
  assert(fakePinModes[inPin2] == INPUT);
  assert(TCCR2A == (1 << WGM21));
  assert(TCCR2B == (1 << CS21));
  assert(OCR2A == 249);
  assert(TIMSK2 == (1 << OCIE2A));
  assert(fakeInterruptsEnabled);

  assert(!tryGetSensorCommand());
  Serial.pushInput({'X'});
  assert(!tryGetSensorCommand());
  Serial.pushInput({'Q'});
  assert(tryGetSensorCommand());
  assert(command == 'Q');

  freq1 = 10.5F;
  freq2 = 20.25F;
  Serial.output.clear();
  executeCommand();
  std::vector<byte> expected = {'Q', ':'};
  for (float value : {freq1, freq2}) {
    const std::vector<byte> encoded = encodedFloat(value);
    expected.insert(expected.end(), encoded.begin(), encoded.end());
    expected.push_back(':');
    expected.insert(expected.end(), encoded.begin(), encoded.end());
    expected.push_back(':');
  }
  expected.push_back(':');
  assert(Serial.output == expected);
  assert(fakeDigitalValues[DEREControlPin] == LOW);

  for (char requestedCommand : {'W', 'R'}) {
    command = requestedCommand;
    Serial.output.clear();
    executeCommand();
    assert(!Serial.output.empty());
    assert(Serial.output.front() == 'Q');
    assert(Serial.output.back() == ':');
  }

  enteredFirstHigh = false;
  enteredSecondHigh = false;
  timer1 = 0;
  timer2 = 0;
  freq1 = 0.0F;
  freq2 = 0.0F;
  fakeDigitalValues[inPin1] = HIGH;
  fakeDigitalValues[inPin2] = HIGH;
  TIMER2_COMPA_vect();
  fakeDigitalValues[inPin1] = LOW;
  fakeDigitalValues[inPin2] = LOW;
  TIMER2_COMPA_vect();
  fakeDigitalValues[inPin1] = HIGH;
  fakeDigitalValues[inPin2] = HIGH;
  TIMER2_COMPA_vect();
  assert(std::fabs(freq1 - 4000.0F) < 0.01F);
  assert(std::fabs(freq2 - 4000.0F) < 0.01F);

  for (int tick = 0; tick <= time2SetZero; ++tick) {
    TIMER2_COMPA_vect();
  }
  assert(freq1 == 0.0F);
  assert(freq2 == 0.0F);

  Serial.pushInput({'Q'});
  loop();
  assert(command == '1');
}
