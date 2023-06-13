#include <Arduino.h>
#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TimeLib.h>

// GUItool: begin automatically generated code
AudioInputI2S micIn;             // xy=452,452
AudioPlaySdWav playSdWav;        // xy=605,632
AudioSynthWaveform beepWaveform; // xy=611,590
AudioMixer4 audioMixer;          // xy=816,611
AudioAnalyzePeak peakAnalyzer;   // xy=871,489
AudioRecordQueue recordQueue;    // xy=897,431
AudioOutputI2S audioOutput;      // xy=1129,606
AudioConnection patchCord1(micIn, 0, recordQueue, 0);
AudioConnection patchCord2(micIn, 0, peakAnalyzer, 0);
AudioConnection patchCord3(playSdWav, 0, audioMixer, 1);
AudioConnection patchCord4(playSdWav, 1, audioMixer, 2);
AudioConnection patchCord5(beepWaveform, 0, audioMixer, 0);
AudioConnection patchCord6(audioMixer, 0, audioOutput, 0);
AudioConnection patchCord7(audioMixer, 0, audioOutput, 1);
AudioControlSGTL5000 sgtl5000; // xy=188,400
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 11
#define SDCARD_SCK_PIN 13
#define HOOK_PIN 0
#define RECORD_PIN 4

Bounce buttonRecord = Bounce(RECORD_PIN, 150);

// Filename to save audio recording on SD card
char filename[15];
// The file object itself
File frec;

enum Mode
{
  Initialising,
  Ready,
  Prompting,
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
  case Mode::Prompting:
    Serial.println(" Prompting");
    break;
  case Mode::Ready:
    Serial.println(" Ready");
    break;
  case Mode::Recording:
    Serial.println(" Recording");
    break;
  }
}

void setMode(Mode newMode)
{
  mode = newMode;
  printMode();
}

void wait(unsigned int milliseconds)
{
  delayMicroseconds(100);
  elapsedMillis msec = 0;

  while (msec <= milliseconds)
  {
  }
}

/* #region Recording */

void startRecording()
{
  for (uint16_t i = 0; i < 9999; i++)
  {
    // Format the counter as a five-digit number with leading zeroes, followed by file extension
    snprintf(filename, 11, " %05d.pcm", i);
    // Create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename))
    {
      break;
    }
  }
  frec = SD.open(filename, FILE_WRITE);
  if (frec)
  {
    Serial.print("Recording to ");
    Serial.println(filename);
    recordQueue.begin();
    setMode(Mode::Recording);
  }
  else
  {
    Serial.print("Couldn't open file ");
    Serial.print(filename);
    Serial.println("to record!");
  }
}

elapsedMillis fps;
void continueRecording()
{

  if (fps > 100)
  {
    if (peakAnalyzer.available())
    {
      fps = 0;
      int monoPeak = peakAnalyzer.read() * 30.0;
      Serial.print("|");
      for (int cnt = 0; cnt < monoPeak; cnt++)
      {
        Serial.print(">");
      }
      Serial.println();
    }
  }

  if (recordQueue.available() >= 2)
  {
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    memcpy(buffer + 256, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    // elapsedMicros usec = 0;
    // write all 512 bytes to the SD card
    frec.write(buffer, 512);
    // Uncomment these lines to see how long SD writes
    // are taking.  A pair of audio blocks arrives every
    // 5802 microseconds, so hopefully most of the writes
    // take well under 5802 us.  Some will take more, as
    // the SD library also must write to the FAT tables
    // and the SD card controller manages media erase and
    // wear leveling.  The recordQueue object can buffer
    // approximately 301700 us of audio, to allow time
    // for occasional high SD card latency, as long as
    // the average write time is under 5802 us.
    // Serial.print("SD write, us=");
    // Serial.println(usec);
  }
}

/**
 * 1. End recording
 * 2. Consume remainder of data in record queue
 * 3. Close file
 */
void stopRecording()
{
  recordQueue.end();
  // consume what's left in the record queue and write that to the file
  while (recordQueue.available() > 0)
  {
    frec.write((byte *)recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
  }
  // close the file and make things ready again
  frec.close();
  Serial.printf("Saving file %s\n", filename);
  setMode(Mode::Ready);
}

/* #endregion Recording */

void setup()
{
  Serial.begin(9600);

  pinMode(RECORD_PIN, INPUT_PULLDOWN);

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  // Comment these out if not using the audio adaptor board.
  // This may wait forever if the SDA & SCL pins lack
  // pullup resistors
  sgtl5000.enable();
  sgtl5000.volume(0.4);
  sgtl5000.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000.micGain(0);

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

  setMode(Mode::Ready);
}

// elapsedMillis fps;
void loop()
{

  buttonRecord.update();
  switch (mode)
  {
  case Mode::Prompting:
    // wait for handset to be picked up
    wait(1500);
    // delay(100);
    playSdWav.play("greeting.wav");
    while (playSdWav.isPlaying())
    {
      buttonRecord.update();
      if (buttonRecord.fallingEdge())
      {
        playSdWav.stop();
        setMode(Mode::Ready);
        return;
      }
    }
    
    wait(500);

    beepWaveform.begin(0.1f, 825, WAVEFORM_SINE);
    wait(1250);
    beepWaveform.amplitude(0);

    Serial.println("Done prompting...");

    // setMode(Mode::Ready);
    startRecording();
  case Mode::Ready:
    if (buttonRecord.risingEdge())
    {
      // startRecording();
      setMode(Mode::Prompting);
    }
    break;
  case Mode::Recording:
    // Handset is replaced
    if (buttonRecord.fallingEdge())
    {
      stopRecording();
      return;
    }

    continueRecording();
    break;

    //
  default:
    break;
  }
}