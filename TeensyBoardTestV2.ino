#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <ADC.h>

#define AUDIO_THRU

const int NUM_LEDS(3);
const int LED_PINS[NUM_LEDS] = { 29, 11, 7 };

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
    adc(),
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

void set_adc1_to_3v3()
{

  ADC1_SC3 = 0; // cancel calibration
  ADC1_SC2 = ADC_SC2_REFSEL(0); // vcc/ext ref 3.3v

  ADC1_SC3 = ADC_SC3_CAL;  // begin calibration

  uint16_t sum;

  //serial_print("wait_for_cal\n");

  while( (ADC1_SC3 & ADC_SC3_CAL))
  {
    // wait
  }

  __disable_irq();

    sum = ADC1_CLPS + ADC1_CLP4 + ADC1_CLP3 + ADC1_CLP2 + ADC1_CLP1 + ADC1_CLP0;
    sum = (sum / 2) | 0x8000;
    ADC1_PG = sum;
    sum = ADC1_CLMS + ADC1_CLM4 + ADC1_CLM3 + ADC1_CLM2 + ADC1_CLM1 + ADC1_CLM0;
    sum = (sum / 2) | 0x8000;
    ADC1_MG = sum;

  __enable_irq();
  
}

void setup()
{
  Serial.println("Setup BEGIN");

  // Audio library has overriden this, so need to reset the reference voltages
  //io.adc.setReference( ADC_REFERENCE::REF_1V2, ADC_1 ); // NOTE: ADC CODE CHECKS FOR SETTING SAME VALUE, SO SET IT TO SOMETHING ELSE FIRST
  //io.adc.setReference( ADC_REFERENCE::REF_3V3, ADC_1 );
  set_adc1_to_3v3();
  
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
      Serial.print("led:");
      Serial.println(i);
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
    const float v = io.adc.analogRead( POT_PINS[i], ADC_1 ) / 65535.0f;
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

