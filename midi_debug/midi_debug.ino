#include <MIDI.h>
#include <LiquidCrystal.h>

#define FASTADC 0

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const int STAT7=7;
LiquidCrystal lcd(4,5, 8,9,10,11);
int octave=2;
const int SAMPLE_COUNT = 100;
const int SAMPLE_SET=(int)(SAMPLE_COUNT*0.8);
const int ACCEPTABLE_STDDEV=2;
int ladder[]={ 
  285, 265, 250,
  235, 220, 205,
  180, 165, 150,
  125, 105, 70,
  50,
  -1 
};

int dataPoints[SAMPLE_COUNT];
//----------------------------------------------------

void swap(int *a, int *b);
void sort(int arr[], int beg, int end);
void noteOff(int note);
void noteOn(int note);
int readPin(int pin);
int findIndex(int frequency);
long lsquare(long t);
//----------------------------------------------------

void setup() {
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
#endif

  // set up the A2 pin
  digitalWrite(A2, HIGH); 
  Serial.begin(9600);
  Serial.println("Hello!");
  pinMode(STAT7, OUTPUT);
  digitalWrite(STAT7, HIGH);
  lcd.begin(16,2);
  lcd.print("FrankenPedals v1");
  lcd.setCursor(0,1);
  lcd.print("O: ");
  lcd.setCursor(3,1);
  lcd.print(octave);
}

int counter=0;

void loop() {
  int baseC1=24;
  int lastIndex=-1;
  int index=lastIndex;

  while(true) {
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
    delay(150);
  }
}



void noteOff(int note) {
  Serial.print("-------------------------------------note off: ");
  Serial.println(note);
}

void noteOn(int note) {
  Serial.print("-------------------------------------note on: ");
  Serial.println(note);
}

int readPin(int pin) {
  int idx;
  long start=millis();
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
    long stddev_sum=0L;  
    for(int counter=0;counter<SAMPLE_SET; counter++) {
      stddev_sum+=lsquare((dataPoints[counter]-mean));
    }
    stddev=sqrt(stddev_sum/SAMPLE_SET);
    long endTime=millis();
    Serial.print("elapsed time: ");
    Serial.println(endTime-start);
    Serial.print("loops: ");
    Serial.println(loops);
    Serial.print("Average: ");
    Serial.println(mean);
    Serial.print("Standard Deviation: ");
    Serial.println(stddev);
    idx=findIndex(mean);
  } 
  while(mean<900 && stddev>(ACCEPTABLE_STDDEV+(13-idx)) && (loops++)<5);

  return idx;
}

void swap(int *a, int *b) { 
  int t=*a; 
  *a=*b; 
  *b=t; 
}

void sort(int arr[], int beg, int end) {
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

long lsquare(long t) {
  return t*t;
}

