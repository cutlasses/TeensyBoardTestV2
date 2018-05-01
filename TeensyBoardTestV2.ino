#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <ADC.h>

#define AUDIO_THRU

const int NUM_LEDS(3);
const int LED_PINS[NUM_LEDS] = { 7, 11, 29 };

const int NUM_SWITCHES(2);
const int SWITCH_PINS[NUM_SWITCHES] = { 1, 2 };

const int NUM_POTS(6);
const int POT_PINS[NUM_POTS] = { A16, A17, A18, A19, A20, A13 };

Bounce SWITCH_BOUNCE[NUM_SWITCHES] = { Bounce( SWITCH_PINS[0], 10 ), Bounce( SWITCH_PINS[1], 10 ) };

// wrap in a struct to ensure initialisation order
struct IO
{
  ADC                         adc;
  AudioInputAnalog            audio_input;
  AudioOutputAnalog           audio_output;

  IO() :
    audio_input(A0),
    audio_output()
  {
  }
};

IO io;


#ifdef AUDIO_THRU
AudioConnection       patchCord1(io.audio_input, 0, io.audio_output, 0);
#else
AudioSynthWaveform    waveform;
//AudioOutputAnalog     analog_out;   // must be constructed after AudioInputAnalog
AudioConnection       patchCord1(waveform, 0, io.audio_output, 0);
#endif

void setup()
{
  Serial.println("Setup BEGIN");
  
  AudioMemory(12);
  
  Serial.begin(9600);

  pinMode( LED_BUILTIN, OUTPUT );

  for( int i = 0; i < NUM_LEDS; ++i )
  {
    pinMode( LED_PINS[i], OUTPUT );
  }

  for( int i = 0; i < NUM_SWITCHES; ++i )
  {
    pinMode( SWITCH_PINS[i], INPUT_PULLUP );
  }

  analogReference(INTERNAL);
  digitalWrite( LED_BUILTIN, HIGH );

  // Audio library has overriden this, so need to reset the reference voltages
  io.adc.setReference( ADC_REFERENCE::REF_1V2, ADC_1 ); // NOTE: ADC CODE CHECKS FOR SETTING SAME VALUE, SO SET IT TO SOMETHING ELSE FIRST
  io.adc.setReference( ADC_REFERENCE::REF_3V3, ADC_1 );

#ifndef AUDIO_THRU
  waveform.begin(WAVEFORM_SINE);
#endif
  
  delay(1000);

  Serial.println("Setup END");
}

void cycle_leds()
{
  static int led = 0;

  for( int i = 0; i < NUM_LEDS; ++i )
  {
    if( i == led )
    {
      digitalWrite( LED_PINS[i], HIGH );
    }
    else
    {
      digitalWrite( LED_PINS[i], LOW );
    }
  }

  led = ( led + 1 ) % NUM_LEDS;
}

void test_switches()
{
  for( int i = 0; i < NUM_SWITCHES; ++i )
  {
    SWITCH_BOUNCE[i].update();

    if( SWITCH_BOUNCE[i].fallingEdge() )
    {
      Serial.print("Switch ");
      Serial.print(i);
      Serial.println(" down");
    }

    if( SWITCH_BOUNCE[i].risingEdge() )
    {
      Serial.print("Switch ");
      Serial.print(i);
      Serial.println(" up");
    }
  }
}

void loop()
{ 
  // output pots
  for( int i=0; i<NUM_POTS; ++i )
  {
    unsigned int v = io.adc.analogRead( POT_PINS[i], ADC_1 );
    Serial.print(v);
    Serial.print(", ");
  }
  
  Serial.println("");

  static bool led_on = true;
  led_on = !led_on;
  if( led_on )
  {
 #ifndef AUDIO_THRU
    waveform.frequency(440);
    waveform.amplitude(0.9);
 #endif
    
    digitalWrite( LED_BUILTIN, HIGH );
  }
  else
  {
 #ifndef AUDIO_THRU
    waveform.amplitude(0);
#endif
    
    digitalWrite( LED_BUILTIN, LOW );
  }

  cycle_leds();

  test_switches();

  const uint16_t time = 200;
  delay(time);

  Serial.println("update");
}

