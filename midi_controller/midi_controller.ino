#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#include <MIDI.h>

const int STAT7=7;

void noteOff(int note) {
  MIDI.sendNoteOff(note, 0, 1);
}

void noteOn(int note) {
  MIDI.sendNoteOn(note, 127, 1);
}

const int SAMPLE_COUNT = 100;
const int ACCEPTABLE_STDDEV=8;

int ladder[]={ 
  285, 265, 250,
  235, 220, 205,
  180, 165, 150,
  125, 105, 70,
  50,
  -1 
};

int dataPoints[SAMPLE_COUNT];

int readPin(int pin) {
  double stddev;
  int loops=0;
  int mean;
  do {
    double sum=0L;
    for(int counter=0; counter<SAMPLE_COUNT; counter++) {
      sum+=dataPoints[counter]=analogRead(pin);
    }
    mean=(int)(sum/SAMPLE_COUNT);
    long stddev_sum=0L;  
    for(int counter=0;counter<SAMPLE_COUNT; counter++) {
      stddev_sum+=(dataPoints[counter]-mean)*(dataPoints[counter]-mean);
    }
    stddev=sqrt(stddev_sum/SAMPLE_COUNT);
  } 
  while(mean<900 && stddev>ACCEPTABLE_STDDEV && (loops++)<5);

  return mean;
}

int findIndex(int frequency) {
  int index=-1;

  for(int curIndex=0; ladder[curIndex]!=-1; curIndex++) {
    if(frequency<ladder[curIndex]) {
      index=curIndex;
    }
    if(frequency>ladder[curIndex]) {
      break; 
    }
  }
  return index;
}

void setup() {
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
#endif

  // set up the A2 pin
  digitalWrite(A2, HIGH); 
  MIDI.begin(4);
  pinMode(STAT7, OUTPUT);
  digitalWrite(STAT7, HIGH);
}

int counter=0;

void loop() {
  int baseC1=24;
  int octave=2;
  int lastIndex=-1;
  int index=lastIndex;

  while(true) {
    index=findIndex(readPin(A2));
    if(index!=lastIndex) {
      if(lastIndex!=-1) {
        noteOff(baseC1+octave*12+lastIndex);
      }
      if(index!=-1) {
        noteOn(baseC1+octave*12+index);
      }
    }
    lastIndex=index;
  }
}


