#include <TM1637Display.h>

#define KNOB_PIN_CLK    2   // D2<->CLK - Encoder
#define KNOB_PIN_DT     3   // D3<->DT  - Encoder
#define KNOB_PIN_SW     4   // D4<->SW  - Encoder
#define DISPLAY_PIN_DIO 5
#define DISPLAY_PIN_CLK 6
#define BUZZ_PIN_IO     10

// Initial message
const byte SEGMENT_START_MESSAGE[] = 
{
    SEG_F | SEG_E | SEG_D,                  // L
    SEG_E | SEG_C | SEG_D,                  // u
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,  // d
    SEG_C | SEG_D | SEG_E | SEG_G           // o
};

// Counter of Knob clicks
volatile long knobCounter = 0;
// Status variable to check if Timer is in Setup mode (true) or Running mode (false)
bool timerSetup = true;
// Last value of millis in the Knob interrupt, a simple way to debaunce
unsigned long lastKnobInterruptMillis;
// Init the 4 digit display
TM1637Display display(6, 5);


void setup()
{
    //Serial.begin(115200);
    pinMode(KNOB_PIN_CLK, INPUT);
    pinMode(KNOB_PIN_DT, INPUT);    
    pinMode(KNOB_PIN_SW, INPUT_PULLUP);        
    pinMode(13, OUTPUT);             // Led builtin ?????????????????
    pinMode(BUZZ_PIN_IO, OUTPUT);
    
    digitalWrite(BUZZ_PIN_IO, HIGH);
    
    display.setBrightness(0x01);
    display.setSegments(SEGMENT_START_MESSAGE);
    delay(3000);
    display.showNumberDecEx(knobCounter, 0b01000000, true);

    attachInterrupt(digitalPinToInterrupt(KNOB_PIN_CLK), knobInterruptSR, LOW);
}
 
void loop()
{
    byte knobSwState = digitalRead(KNOB_PIN_SW);
    
    if(timerSetup)
    {
        display.showNumberDecEx(knobCounter2Time(knobCounter), 0b01000000, true);
    
        if (knobSwState == LOW)
        {
            // turn LED off:
            //digitalWrite(13, LOW);
            //digitalWrite(10, HIGH);
            timerSetup=false;
            
            detachInterrupt(digitalPinToInterrupt(KNOB_PIN_CLK));
        }
    }
    else
    {
        if(knobCounter >= 0)
        {
            delay(500);
            display.showNumberDecEx(knobCounter2Time(knobCounter), 0b01000000, true);
            delay(500);
            display.showNumberDec(knobCounter2Time(knobCounter), true);
            knobCounter--;
        }
        else
        {
            playBeep();
            timerSetup=true;
            knobCounter = 0;
            attachInterrupt(digitalPinToInterrupt(KNOB_PIN_CLK), knobInterruptSR, LOW);
        }
    }
}


void knobInterruptSR()
{
    unsigned long currentKnobInterruptMillis = millis();

    if((currentKnobInterruptMillis - lastKnobInterruptMillis) > 5)
    {
        if(digitalRead(KNOB_PIN_DT) == HIGH)
        {
            knobCounter += 10;
        }
        else
        {
            knobCounter = knobCounter >= 10 ? knobCounter-10 : 0;
        }
    }

    lastKnobInterruptMillis = currentKnobInterruptMillis;
}

void playBeep()
{
    for(byte i=0;i<5;i++)
    {
        analogWrite(BUZZ_PIN_IO, 128);
        delay(600);
        digitalWrite(BUZZ_PIN_IO, HIGH);
        delay(400);
    }

    digitalWrite(BUZZ_PIN_IO, HIGH);
}

long knobCounter2Time(long counterValue)
{
    int minutes = counterValue / 60;
    int seconds = (int)(counterValue % 60);
    long time = minutes * 100 + seconds;

    return time;
}
