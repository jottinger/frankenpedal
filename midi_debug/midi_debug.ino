#include <MIDI.h>
#include <LiquidCrystal.h>
/*
   note: FASTADC breaks things. Don't why yet.
 not sure it's necessary, as our resolution is actually
 very very good right now.
 */

#define FASTADC 0

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

const int STAT7=7;
LiquidCrystal lcd(4,5,8,9,10,11);
int octave=2;
const int SAMPLE_COUNT = 100;
const int SAMPLE_SET=(int)(SAMPLE_COUNT*0.6);
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

void swap(int * const a, int * const b);
void sort(int arr[], const int beg, const int end);
void noteOff(const int note);
void noteOn(const int note);
int readPin(const int pin);
int findIndex(const int frequency);
long lsquare(const long t);
void scroll(const String s, const int row=0, const int col=0);
void updateOctave(const int octave, const int row=1, const int col=3);
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

  lcd.begin(16,2);
  scroll("FrankenPedal v1");
  lcd.setCursor(0,1);
  lcd.print("O: ");
  updateOctave(octave);  
  lcd.setCursor(5,1);
}

void loop() {
  int baseC1=24;
  int lastIndex=-1;
  int index=lastIndex;

  while(true) {
    octave=1+analogRead(A0)/128;
    updateOctave(octave);
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

void noteOff(const int note) {
  Serial.print("-------------------------------------note off: ");
  Serial.println(note);
}

void noteOn(const int note) {
  Serial.print("-------------------------------------note on: ");
  Serial.println(note);
}

int readPin(const int pin) {
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
    Serial.print(endTime-start);
    Serial.print("  loops: ");
    Serial.print(loops);
    Serial.print("  Average: ");
    Serial.print(mean);
    Serial.print("   Standard Deviation: ");
    Serial.println(stddev);
    idx=findIndex(mean);
  } 
  while(mean<900 && stddev>(ACCEPTABLE_STDDEV+(13-idx)) && (loops++)<5);

  return idx;
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

long lsquare(const long t) {
  return t*t;
}

void scroll(const String s, const int row, const int col) {
  String spaces("                ");
  lcd.setCursor(col, row);
  lcd.print(spaces);
  for(unsigned int pos=0;pos<s.length(); pos++) {
    String newString=s.substring(s.length()-pos-1)+spaces.substring(pos,spaces.length()-1);
    lcd.setCursor(col, row);
    lcd.print(newString);
    delay(75);
  }
  lcd.setCursor(0,0);
}

void updateOctave(const int octave, const int row, const int col) {
  lcd.setCursor(col,row);
  lcd.print(octave);
}

