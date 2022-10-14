// POTENTIOMETERS//

const int pot1 = A0;  // potentiometer pins
const int pot2 = A1;
//const int pot3 = A2;
const int slidePot = A2;

int potVal1 = 0;
int potVal2 = 0;
int potVal3 = 0;
int slidePotVal = 0;

int lastPotVal1 = 0;
int lastPotVal2 = 0;
int lastPotVal3 = 0;
int lastSlidePotVal = 0;

// JOYSTICKS //

const int joyX = A3;  // joystick pins
const int joyY = A4;
const int joyX2 = A5;


int joyXval = 0;
int joyYval = 0;
int joyXval2 = 0;

int lastJoyXval = 0;
int lastJoyYval = 0;
int lastJoyXval2 = 0;
// KEYPAD //

#include <Keypad.h>

const byte ROWS = 4; // four rows
const byte COLS = 4; // three columns
char keys[ROWS][COLS] = {  // keypad keys, 1-9, 0, S for star (asterisk) and P for pound (square)
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'E', '0', 'F', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; // keypad row pinouts
byte colPins[COLS] = {5, 4, 3, 2}; // keypad column pinouts

Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int midC = 60; // MIDI note value for middle C on a standard keyboard

const int transposePin1 = 22; // pins for the switch controlling transposing
const int transposePin2 = 23;
int transpose = 0;  // if = 0 no transposing
int transposeLeft = 0;
int transposeRight = 0;
int oldTransposeLeft = 0;
int oldTransposeRight = 0;
unsigned long transposeTimer = 0;  // for debouncing


void MIDImessage(byte status, byte data1, byte data2)
{
  Serial.write(status);
  Serial.write(data1);
  Serial.write(data2);
}

void readPots() {

  int diff = 4; // difference amount

  // READ POTENTIOMETERS //
  
  potVal1 = analogRead(pot1);
  potVal2 = analogRead(pot2);
  //potVal3 = analogRead(pot3);
  slidePotVal = analogRead(slidePot);

  // CALCULATE DIFFERENCE BETWEEN NEW VALUE AND LAST RECORDED VALUE //
  
  int potVal1diff = potVal1 - lastPotVal1;
  int potVal2diff = potVal2 - lastPotVal2;
  //int potVal3diff = potVal3 - lastPotVal3;
  int slidePotValdiff = slidePotVal - lastSlidePotVal;

  // SEND MIDI MESSAGE //
  
  if (abs(potVal1diff) > diff) // execute only if new and old values differ enough
  {
    MIDImessage(177, 0, map(potVal1, 0, 1023, 0, 127)); // map sensor range to MIDI range
    lastPotVal1 = potVal1; // reset old value with new reading
  }

  if (abs(potVal2diff) > diff)
  {
    MIDImessage(177, 1, map(potVal2, 0, 1023, 0, 127));
    lastPotVal2 = potVal2;
  }

  /*if (abs(potVal3diff) > diff)
  {
    MIDImessage(177, 2, map(potVal3, 0, 1023, 0, 127));
    lastPotVal3 = potVal3;
  }*/

  if (abs(slidePotValdiff) > diff)
  {
    MIDImessage(177, 3, map(slidePotVal, 1023, 0, 0, 127));
    lastSlidePotVal = slidePotVal;
  }

  delay(2); // small delay further stabilizes sensor readings

}

/*
  The joystick is essentially two potentiometers, one
  in each direction. Due to their limited movement
  range and low range of resistance we are only
  reading a part of their range (~380-640) and
  therefore we have to map our output accordingly.
  Unfortunately that also means that the joystick
  will not have as high of a fidelity as a full range
  10kOhm resistor, like we use elsewhere in The
  MIDInator, would have.
  It is also possible to turn off one or both axis of
  the joystick using the two toggle switches under the
  joystick.
  Because we are using the Y axis of the joystick to
  send a modulation MIDI message, like the modulation
  wheel you would find on many MIDI keyboards, some
  additional value splitting has to be done to be
  able to send a 14 bit value in two chunks.
  Because of inherent jitteryness of some potentiometers
  a 'difference' value is set up and checked. Events
  will only trigger if the value of the last recorded
  reading is higher than this value. Set to '1' if your
  potentiometer is very steady. A value of '2' will
  stop it from jittering between two adjacent numbers.
  If your potentiometer is very noisy a higher value
  may be required. Keep in mind that higher difference
  values will result in further less fidelity of the
  potentiometer reading.
*/

void readJoysticks() {

  int diff = 2;

  // READ LEFT-RIGHT //
  joyXval = analogRead(joyX);
  int joyXvalDiff = joyXval - lastJoyXval;

  if (abs(joyXvalDiff) > diff) // send MIDI message only if new and old values differ enough
  {
    MIDImessage(178, 8,  map(joyXval, 0, 1023, 127, 0)); // map sensor range to MIDI range
    
    lastJoyXval = joyXval; // reset old value with new reading
  }
  delay(2); // for stability


  joyYval = analogRead(joyY);
  int joyYvalDiff = joyYval - lastJoyYval;
  if (abs(joyYvalDiff) > diff)
  {
    MIDImessage(179, 8, map(joyYval, 0, 1023, 127, 0));

    lastJoyYval = joyYval;
    delay(2);
  }

  // READ LEFT-RIGHT 2 //
  joyXval2 = analogRead(joyX2);
  int joyXvalDiff2 = joyXval2 - lastJoyXval2;

  if (abs(joyXvalDiff2) > diff) // send MIDI message only if new and old values differ enough
  {
    MIDImessage(180, 8,  map(joyXval2, 0, 1023, 127, 0)); // map sensor range to MIDI range
    
    lastJoyXval2 = joyXval2; // reset old value with new reading
  }
  delay(2); // for stability

}

/*
  The keypad runs on the keypad.h library, which you also can use
  to run other similar keypads with similar pinouts, or even use
  regular micro switches in an array to build your own keypad.
  With the use of a bidirectional toggle switch the keypad
  mapping can also be transposed, or moved, up and down the
  scale. The keypad starts with middle C (note number 60) on the
  Nr 1 key. Using the transpose switch moves the keypad one entire
  octave up or down, within the full range (0 - 127) of available
  notes.
*/

int kpc = 144; // the function of the keypad, 144 = Channel 1 Note on

void readKeyPad() {
  
  // KEYPAD //

  if (kpd.getKeys())  // supports up to ten simultaneous key presses
  {
    for (int i = 0; i < LIST_MAX; i++) // scan the whole key list
    {
      if ( kpd.key[i].stateChanged)   // find keys that have changed state
      {
        if (kpd.key[i].kstate == PRESSED) // sends MIDI on message when keys are pressed
        {
          switch (kpd.key[i].kchar) {
            case '1':
              MIDImessage(kpc, midC + 0, 127);
              break;
            case '2':
              MIDImessage(kpc, midC + 1, 127);
              break;
            case '3':
              MIDImessage(kpc, midC + 2, 127);
              break;
            case '4':
              MIDImessage(kpc, midC + 3, 127);
              break;
            case '5':
              MIDImessage(kpc, midC + 4, 127);
              break;
            case '6':
              MIDImessage(kpc, midC + 5, 127);
              break;
            case '7':
              MIDImessage(kpc, midC + 6, 127);
              break;
            case '8':
              MIDImessage(kpc, midC + 7, 127);
              break;
            case '9':
              MIDImessage(kpc, midC + 8, 127);
              break;
            case '0':
              MIDImessage(kpc, midC + 9, 127);
              break;
            case 'A':
              MIDImessage(kpc, midC + 10, 127);
              break;
            case 'B':
              MIDImessage(kpc, midC + 11, 127);
              break;
            case 'C':
              MIDImessage(kpc, midC + 12, 127);
              break;
            case 'D':
              MIDImessage(kpc, midC + 13, 127);
              break;
            case 'E':
              MIDImessage(kpc, midC + 14, 127);
              break;
            case 'F':
              MIDImessage(kpc, midC + 15, 127);
              break;
          }
        }

        if (kpd.key[i].kstate == RELEASED)  // sends MIDI off message when keys are released
        {
          switch (kpd.key[i].kchar) {
            case '1':
              MIDImessage(kpc - 16, midC + 0, 0);
              break;
            case '2':
              MIDImessage(kpc - 16, midC + 1, 0);
              break;
            case '3':
              MIDImessage(kpc - 16, midC + 2, 0);
              break;
            case '4':
              MIDImessage(kpc - 16, midC + 3, 0);
              break;
            case '5':
              MIDImessage(kpc - 16, midC + 4, 0);
              break;
            case '6':
              MIDImessage(kpc - 16, midC + 5, 0);
              break;
            case '7':
              MIDImessage(kpc - 16, midC + 6, 0);
              break;
            case '8':
              MIDImessage(kpc - 16, midC + 7, 0);
              break;
            case '9':
              MIDImessage(kpc - 16, midC + 8, 0);
              break;
            case '0':
              MIDImessage(kpc - 16, midC + 9, 0);
              break;
            case 'A':
              MIDImessage(kpc - 16, midC + 10, 0);
              break;
            case 'B':
              MIDImessage(kpc - 16, midC + 11, 0);
              break;
            case 'C':
              MIDImessage(kpc - 16, midC + 12, 127);
              break;
            case 'D':
              MIDImessage(kpc - 16, midC + 13, 127);
              break;
            case 'E':
              MIDImessage(kpc - 16, midC + 14, 127);
              break;
            case 'F':
              MIDImessage(kpc - 16, midC + 15, 127);
              break;
          }
        }
      }
    }
  }
}

void setup() { 
  Serial.begin(115200); // enable serial communication
 }

void loop() { 
  readPots();
  readKeyPad();
  readJoysticks();
  }