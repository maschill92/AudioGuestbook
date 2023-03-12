#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>
#include <TimeLib.h>

// GUItool: begin automatically generated code
AudioPlaySdWav playSdWav1; // xy=331,330
AudioOutputI2S i2s1;       // xy=876,337
AudioConnection patchCord1(playSdWav1, 0, i2s1, 0);
AudioConnection patchCord2(playSdWav1, 1, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1; // xy=318,428
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14
#define HOOK_PIN 0
#define RECORD_PIN 4

Bounce buttonHook = Bounce(HOOK_PIN, 350);
Bounce buttonRecord = Bounce(RECORD_PIN, 150);

enum Mode
{
  Initialising,
  Ready,
  Playing,
  Recording
};

Mode mode = Mode::Initialising;

void printMode()
{ // only for debugging
  Serial.print("Mode switched to: ");
  switch (mode)
  {
  case Mode::Initialising:
    Serial.println(" Initialising");
    break;
  case Mode::Ready:
    Serial.println(" Ready");
    break;
  case Mode::Playing:
    Serial.println(" Playing");
    break;
  case Mode::Recording:
    Serial.println(" Recording");
    break;
  default:
    Serial.println(" Undefined");
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(HOOK_PIN, INPUT_PULLUP);
  pinMode(RECORD_PIN, INPUT);
  mode = Mode::Ready;
  printMode();

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.4);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN)))
  {
    // stop here, but print a message repetitively
    while (1)
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}

void loop()
{
  buttonHook.update();
  buttonRecord.update();
  if (buttonRecord.risingEdge())
  {
    Serial.println("start recording!");
  }

  if (buttonHook.risingEdge())
  {
    // rising edge is "lowering" the phone
    Serial.println("phone replaced!");
  }
  else if (buttonHook.fallingEdge())
  {
    // hook falling edge is "enabled"
    Serial.println("phone lifted!");
  }
  // switch (mode)
  // {
  // case Mode::Ready:
  //   if (buttonHook.fallingEdge())
  //   {
  //     mode = Mode::Playing;
  //     printMode();
  //   }
  //   else if (buttonRecord.risingEdge())
  //   {
  //     mode = Mode::Recording;
  //     printMode();
  //   }
  //   break;
  // case Mode::Playing:
  //   delay(1000);
  //   playSdWav1.play("absolute-power.wav");
  //   while (!playSdWav1.isStopped())
  //   {
  //     // Check whether the handset is replaced
  //     buttonHook.update();
  //     // Handset is replaced
  //     if (buttonHook.risingEdge())
  //     {
  //       playSdWav1.stop();
  //       mode = Mode::Ready;
  //       printMode();
  //       return;
  //     }

  //     // Check whether the record button is pressed
  //     buttonRecord.update();
  //     if (buttonRecord.fallingEdge())
  //     {
  //       mode = Mode::Recording;
  //       printMode();
  //       return;
  //     }
  //   }
  //   break;
  // case Mode::Recording:
  //   delay(1000);
  //   mode = Mode::Ready;
  //   printMode();
  //   break;
  // }
}
