#include <MIDI.h>
#include <LiquidCrystal.h>

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
#define FASTADC 1
#define DEBUG 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

LiquidCrystal lcd(4,5,8,9,10,11);
int octave=2;
const int SAMPLE_COUNT = 100;
//const int SAMPLE_SET=(int)(SAMPLE_COUNT*0.6);
//const int ACCEPTABLE_STDDEV=2;
int ladder[]={ 
  285, 265, 250,
  235, 220, 205,
  180, 165, 150,
  125, 105, 70,
  50,
  -1 
};
String notes[]={
  "C", "C#", "D", "D#", "E", 
  "F", "F#", "G", "G#", "A",
  "A#", "B", "C",
};

//----------------------------------------------------
void initSerial();

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

  initSerial();

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
    octave=1+((analogRead(A0))>>7);
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
#if DEBUG
    delay(1500);
#endif
  }
}

int readPin(const int pin) {
  int idx;
  byte counter;
  int data[14];
  int pinSelect;
#if DEBUG
  long start=millis();
#endif
  // zero the reads
  for(counter=0;counter<14;counter++) {
    data[counter]=0;
  }
  for(counter=0;counter<SAMPLE_COUNT;counter++) {
    pinSelect=findIndex(analogRead(pin));
    pinSelect=(-1!=pinSelect?pinSelect:13);
    data[pinSelect]++;
  }
  Serial.print("data [");
  for(counter=0; counter<14; counter++) {
    Serial.print(data[counter]);
    Serial.print(",");
  }
  Serial.println("]");
  // now find the maximum reads in the set; that's our pin!
  pinSelect=-1;
  int maxPin=0;
  for(counter=0; counter<14; counter++) {
    if(data[counter]>maxPin) {
      maxPin=data[counter];
      pinSelect=counter;
    }
  }
  pinSelect=(pinSelect==13?-1:pinSelect);
#if DEBUG
  long endTime=millis();
  Serial.print("elapsed time: ");
  Serial.print(endTime-start);
  Serial.print("  note: ");
  Serial.println(pinSelect);
#endif
  return pinSelect;
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

#if DEBUG
void initSerial() {
  Serial.begin(9600);
  Serial.println("Hello!");
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
}

void noteOn(const int note) {
}

void noteOff(const int note) {
}
#endif













