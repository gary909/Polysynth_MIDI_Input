/*
 * 
███████╗ ██████╗     ██╗                 ██████╗ 
██╔════╝██╔═══ ██╗   ██║                ██╔════╝ 
███████╗██║    ██║   ██║        █████╗  ███████╗ 
╚════██║██║    ██║   ██║        ╚════╝  ██╔══██╗
███████║╚██████╔╝    ███████╗           ╚██████╔
╚══════╝ ╚═════╝     ╚══════╝            ╚═════╝
                            
                            
                            POLYPHONIC SYNTHESIZER
                       
                    A BlogHoskins Monstrosity @ 2020
                    https://bloghoskins.blogspot.com/
                                        ANSI Shadow

This is a test to see if polyphony works.
Yet to run it on hardware, so can't confirm working
Code compiles
*/

#include <MIDI.h>
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <mozzi_midi.h>
#include <ADSR.h>


MIDI_CREATE_DEFAULT_INSTANCE();

// use #define for CONTROL_RATE, not a constant
#define CONTROL_RATE 64 // slow to save cpu -> increase when using teensy

// audio sinewave oscillator
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin3(SIN2048_DATA);

// envelope generator
ADSR <CONTROL_RATE, AUDIO_RATE> envelope1;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope2;
ADSR <CONTROL_RATE, AUDIO_RATE> envelope3;

// notes
byte note1=0;
byte note2=0;
byte note3=0;

//gains to carry control rate envelope levels from updateControl() to updateAudio()
byte gain1,gain2,gain3 = 0;

#define LED 13 // shows if MIDI is being recieved

void HandleNoteOn(byte channel, byte note, byte velocity) {
  if(note1==0){
    note1=note;
    aSin1.setFreq(mtof(float(note)));
    envelope1.noteOn();
  } else if(note2==0){
    note2=note;
    aSin2.setFreq(mtof(float(note)));
    envelope2.noteOn();
  } else if(note3==0){
    note3=note;
    aSin3.setFreq(mtof(float(note)));
    envelope3.noteOn();
  }
  // Turn on/off LED to show MIDI info is being sent
  if(note1+note2+note3==0){
    digitalWrite(LED,LOW);
  } else {
    digitalWrite(LED,HIGH);
  }
}

void HandleNoteOff(byte channel, byte note, byte velocity) {
  digitalWrite(LED,LOW);
  if(note==note1){
    envelope1.noteOff();
    note1=0;
  } else if(note==note2){
    envelope2.noteOff();
    note2=0;
  } else if(note==note3){
    envelope3.noteOff();
    note3=0;
  }
}

void setup() {
  pinMode(LED, OUTPUT);

  // Connect the HandleNoteOn function to the library, so it is called upon reception of a NoteOn.
  MIDI.setHandleNoteOn(HandleNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(HandleNoteOff);  // Put only the name of the function
  // Initiate MIDI communications, listen to all channels (not needed with Teensy usbMIDI)
  MIDI.begin(MIDI_CHANNEL_OMNI);

  envelope1.setADLevels(255,64);
  envelope1.setTimes(50,200,10000,200);
  envelope2.setADLevels(255,64);
  envelope2.setTimes(50,200,10000,200);
  envelope3.setADLevels(255,64);
  envelope3.setTimes(50,200,10000,200);

  aSin1.setFreq(440); // default frequency
  aSin2.setFreq(440); // default frequency
  aSin3.setFreq(440); // default frequency
  startMozzi(CONTROL_RATE);
}


void updateControl(){
  MIDI.read();
  envelope1.update();
  envelope2.update();
  envelope3.update();

  gain1 = envelope1.next();
  gain2 = envelope2.next();
  gain1 = envelope3.next();
}


int updateAudio(){
  //Not working? Change 1st int to long or try shifting down from 8 to 6
  return (int) (gain1 * aSin1.next() + 
    (int)gain2 * aSin2.next() +
    (int)gain3 * aSin3.next())>>8;
}


void loop() {
  audioHook(); // required here
}