#include <MIDI.h>

/*
 note: FASTADC *sometimes* breaks things. Don't why yet.
 not sure it's necessary, as our resolution is actually
 fairly good right now.
 
 tests show:
 sample count   slow read time  fast read time
 100            14 (accurate)
 80             12 (accurate)
 60             9  (accurate)   3 (accurate)
 */
#define FASTADC 0
#define DEBUG 0
#define ACCEPTABLE_STDDEV 4
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

int octave=2;
const int SAMPLE_COUNT = 60;
const int SAMPLE_SET=SAMPLE_COUNT*0.6;
int dataPoints[SAMPLE_COUNT];
int ladder[]={ 
  285, 260, 245, // c, c#, d
  230, 210, 195, // d#, e, f
  175, 165, 145, // f#, g, g#
  125, 105, 70,  // a, a#, b
  50,            // c
  -1 
};

//----------------------------------------------------
void initSerial();

void swap(int * const a, int * const b);
void sort(int arr[], const int beg, const int end);
void noteOff(const int note);
void noteOn(const int note);
int readPin(const int pin);
int findIndex(const int frequency);
//----------------------------------------------------

void setup() {
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
#endif
  //  lcd.begin(16,2);

  // set up the A2 pin
  digitalWrite(A2, HIGH); 

  initSerial();
}

void loop() {
  int baseC1=24;
  int lastIndex=-1;
  int index=lastIndex;

  while(true) {
    octave=1+((analogRead(A0))>>7);
    index=readPin(A2);
    if(index!=lastIndex) {
      if(lastIndex!=-1) {
        noteOff(baseC1+octave*12+lastIndex);
      }
      if(index!=-1) {
        noteOn(baseC1+octave*12+index);
      }
    }
    lastIndex=index;
    delay(10);
#if DEBUG
    delay(250);
#endif
  }
}

int readPin(const int pin) {
  int idx;
#if DEBUG
  long start=millis();
#endif
  double stddev;
  int loops=0;
  int mean;
  do {
    double sum=0L;
    for(int counter=0; counter<SAMPLE_COUNT; counter++) {
      dataPoints[counter]=analogRead(pin);
    }
    sort(dataPoints,0, SAMPLE_COUNT);
    for(int counter=0; counter<SAMPLE_SET;counter++) {
      sum+=dataPoints[counter];
    }
    mean=(int)(sum/SAMPLE_SET);
    if(mean<900) {
      long stddev_sum=0L;  
      for(int counter=0;counter<SAMPLE_SET; counter++) {
        stddev_sum+=lsquare((dataPoints[counter]-mean));
      }
      stddev=sqrt(stddev_sum/SAMPLE_SET);
    }
#if DEBUG
    long endTime=millis();
    Serial.print("elapsed time: ");
    Serial.print(endTime-start);
    Serial.print("  loops: ");
    Serial.print(loops);
    Serial.print("  Average: ");
    Serial.print(mean);
    Serial.print("   Standard Deviation: ");
    Serial.println(stddev);
#endif
    idx=findIndex(mean);
  } 
  while(mean<900 && stddev>(ACCEPTABLE_STDDEV+(13-idx)/3) && (loops++)<5);

  return idx;
}

long lsquare(const long t) {
  return t*t;
}

void swap(int * const a, int * const b) { 
  int t=*a; 
  *a=*b; 
  *b=t; 
}

void sort(int arr[], const int beg, const int end) {
  if (end > beg + 1) {
    int piv = arr[beg], l = beg + 1, r = end;
    while (l < r) {
      if (arr[l] <= piv) 
        l++;
      else 
        swap(&arr[l], &arr[--r]);
    }
    swap(&arr[--l], &arr[beg]);
    sort(arr, beg, l);
    sort(arr, r, end);
  }
}

int findIndex(const int frequency) {
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

#if DEBUG
void initSerial() {
  Serial.begin(9600);
}

void noteOff(const int note) {
  Serial.print("-------------------------------------note off: ");
  Serial.println(note);
}

void noteOn(const int note) {
  Serial.print("-------------------------------------note on: ");
  Serial.println(note);
}

#else 
void initSerial() {
  MIDI.begin(4);
}

void noteOn(const int note) {
  MIDI.sendNoteOn(note, 127, 1);
}

void noteOff(const int note) {
  MIDI.sendNoteOff(note, 0, 1);
}
#endif


















