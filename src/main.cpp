#include <Arduino.h>
#include <Bounce.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TimeLib.h>

// GUItool: begin automatically generated code
AudioInputI2S            micIn;         //xy=175.3333282470703,458.3333282470703
AudioSynthWaveform       beepWaveform;  //xy=205.3333282470703,159.3333282470703
AudioPlaySdRaw           playSdRaw;     //xy=220.3333282470703,198.3333282470703
AudioMixer4              audioMixer;    //xy=376.3333282470703,174.3333282470703
AudioAnalyzePeak         peekAnalyzer;  //xy=466.3333435058594,501.33334255218506
AudioRecordQueue         recordQueue;   //xy=470.3333282470703,461.3333282470703
AudioOutputI2S           audioOutput;   //xy=524.3333282470703,174.3333282470703
AudioConnection          patchCord1(micIn, 0, recordQueue, 0);
AudioConnection          patchCord2(micIn, 0, peekAnalyzer, 0);
AudioConnection          patchCord3(beepWaveform, 0, audioMixer, 0);
AudioConnection          patchCord4(playSdRaw, 0, audioMixer, 1);
AudioConnection          patchCord5(audioMixer, 0, audioOutput, 0);
AudioConnection          patchCord6(audioMixer, 0, audioOutput, 1);
AudioControlSGTL5000     sgtl5000;      //xy=214.3333282470703,597.3333282470703
// GUItool: end automatically generated code

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14
#define HOOK_PIN 0
#define RECORD_PIN 4

Bounce buttonHook = Bounce(HOOK_PIN, 350);
Bounce buttonRecord = Bounce(RECORD_PIN, 150);

// Filename to save audio recording on SD card
char filename[15];
// The file object itself
File frec;

// variables for writing to WAV file
unsigned long ChunkSize = 0L;
unsigned long Subchunk1Size = 16;
unsigned int AudioFormat = 1;
unsigned int numChannels = 1;
unsigned long sampleRate = 44100;
unsigned int bitsPerSample = 16;
unsigned long byteRate = sampleRate * numChannels * (bitsPerSample / 8); // samplerate x channels x (bitspersample / 8)
unsigned int blockAlign = numChannels * bitsPerSample / 8;
unsigned long Subchunk2Size = 0L;
unsigned long recByteSaved = 0L;
unsigned long NumSamples = 0L;
byte byte1, byte2, byte3, byte4;

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
  case Mode::Ready:
    Serial.println(" Ready");
    break;
  case Mode::Prompting:
    Serial.println(" Prompting");
    break;
  case Mode::Recording:
    Serial.println(" Recording");
    break;
  default:
    Serial.println(" Undefined");
    break;
  }
}

void writeOutHeader()
{ // update WAV header with final filesize/datasize

  //  NumSamples = (recByteSaved*8)/bitsPerSample/numChannels;
  //  Subchunk2Size = NumSamples*numChannels*bitsPerSample/8; // number of samples x number of channels x number of bytes per sample
  Subchunk2Size = recByteSaved - 42; // because we didn't make space for the header to start with! Lose 21 samples...
  ChunkSize = Subchunk2Size + 34;    // was 36;
  frec.seek(0);
  frec.write("RIFF");
  byte1 = ChunkSize & 0xff;
  byte2 = (ChunkSize >> 8) & 0xff;
  byte3 = (ChunkSize >> 16) & 0xff;
  byte4 = (ChunkSize >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  frec.write("WAVE");
  frec.write("fmt ");
  byte1 = Subchunk1Size & 0xff;
  byte2 = (Subchunk1Size >> 8) & 0xff;
  byte3 = (Subchunk1Size >> 16) & 0xff;
  byte4 = (Subchunk1Size >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = AudioFormat & 0xff;
  byte2 = (AudioFormat >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = byteRate & 0xff;
  byte2 = (byteRate >> 8) & 0xff;
  byte3 = (byteRate >> 16) & 0xff;
  byte4 = (byteRate >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write("data");
  byte1 = Subchunk2Size & 0xff;
  byte2 = (Subchunk2Size >> 8) & 0xff;
  byte3 = (Subchunk2Size >> 16) & 0xff;
  byte4 = (Subchunk2Size >> 24) & 0xff;
  frec.write(byte1);
  frec.write(byte2);
  frec.write(byte3);
  frec.write(byte4);
  frec.close();
  Serial.println("header written");
  Serial.print("Subchunk2: ");
  Serial.println(Subchunk2Size);
}

void startRecording()
{
  // setMTPdeviceChecks(false); // disable MTP device checks while recording
#if defined(INSTRUMENT_SD_WRITE)
  worstSDwrite = 0;
  printNext = 0;
#endif // defined(INSTRUMENT_SD_WRITE)
  // Find the first available file number
  //  for (uint8_t i=0; i<9999; i++) { // BUGFIX uint8_t overflows if it reaches 255
  for (uint16_t i = 0; i < 9999; i++)
  {
    // Format the counter as a five-digit number with leading zeroes, followed by file extension
    snprintf(filename, 11, " %05d.RAW", i);
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
    mode = Mode::Recording;
    printMode();
    recByteSaved = 0L;
  }
  else
  {
    Serial.print("Couldn't open file ");
    Serial.print(filename);
    Serial.println("to record!");
  }
}

void continueRecording() {
  if (recordQueue.available() >= 2) {
    byte buffer[512];
    // Fetch 2 blocks from the audio library and copy
    // into a 512 byte buffer.  The Arduino SD library
    // is most efficient when full 512 byte sector size
    // writes are used.
    memcpy(buffer, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    memcpy(buffer+256, recordQueue.readBuffer(), 256);
    recordQueue.freeBuffer();
    elapsedMicros usec = 0;
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
    Serial.print("SD write, us=");
    Serial.println(usec);
  }
}

// void continueRecording()
// {
// #if defined(INSTRUMENT_SD_WRITE)
//   uint32_t started = micros();
// #endif // defined(INSTRUMENT_SD_WRITE)
// #define NBLOX 16
//   // Check if there is data in the queue
//   if (recordQueue.available() >= NBLOX)
//   {
//     byte buffer[NBLOX * AUDIO_BLOCK_SAMPLES * sizeof(int16_t)];
//     // Fetch 2 blocks from the audio library and copy
//     // into a 512 byte buffer.  The Arduino SD library
//     // is most efficient when full 512 byte sector size
//     // writes are used.
//     for (int i = 0; i < NBLOX; i++)
//     {
//       memcpy(buffer + i * AUDIO_BLOCK_SAMPLES * sizeof(int16_t), recordQueue.readBuffer(), AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
//       recordQueue.freeBuffer();
//     }
//     // Write all 512 bytes to the SD card
//     frec.write(buffer, sizeof buffer);
//     recByteSaved += sizeof buffer;
//   }

// #if defined(INSTRUMENT_SD_WRITE)
//   started = micros() - started;
//   if (started > worstSDwrite)
//     worstSDwrite = started;

//   if (millis() >= printNext)
//   {
//     Serial.printf("Worst write took %luus\n", worstSDwrite);
//     worstSDwrite = 0;
//     printNext = millis() + 250;
//   }
// #endif // defined(INSTRUMENT_SD_WRITE)
// }

void stopRecording() {
  recordQueue.end();
  if (mode == 1) {
    while (recordQueue.available() > 0) {
      frec.write((byte*)recordQueue.readBuffer(), 256);
      recordQueue.freeBuffer();
    }
    frec.close();
  }
  mode = Mode::Ready;
  printMode();
}

// void stopRecording()
// {
//   // Stop adding any new data to the queue
//   recordQueue.end();
//   // Flush all existing remaining data from the queue
//   while (recordQueue.available() > 0)
//   {
//     // Save to open file
//     frec.write((byte *)recordQueue.readBuffer(), AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
//     recordQueue.freeBuffer();
//     recByteSaved += AUDIO_BLOCK_SAMPLES * sizeof(int16_t);
//   }
//   writeOutHeader();
//   // Close the file
//   frec.close();
//   Serial.println("Closed file");
//   mode = Mode::Ready;
//   printMode();
//   // setMTPdeviceChecks(true); // enable MTP device checks, recording is finished
// }

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

void loop() {
  buttonRecord.update();
  buttonRecord.risingEdge();
}
// void loop()
// {
//   buttonHook.update();
//   buttonRecord.update();
//   switch (mode)
//   {
//   case Mode::Ready:
//     if (buttonHook.fallingEdge())
//     {
//       Serial.println("Handset lifted!");
//       mode = Mode::Prompting;
//       printMode();
//     }
//     break;
//   case Mode::Prompting:
//     delay(1000);

//     while (!playSdWav1.isStopped())
//     {
//       // Check whether the handset is replaced
//       buttonHook.update();
//       // Handset is replaced
//       if (buttonHook.risingEdge())
//       {
//         playSdWav1.stop();
//         mode = Mode::Ready;
//         printMode();
//         return;
//       }
//     }

//     //   // Check whether the record button is pressed
//     //   buttonRecord.update();
//     //   if (buttonRecord.fallingEdge())
//     //   {
//     //     mode = Mode::Recording;
//     //     printMode();
//     //     return;
//     //   }
//     // }

//     Serial.println("Starting Recording");

//     // Play the tone sound effect
//     waveform1.begin(1, 440, WAVEFORM_SINE);
//     delay(1250);
//     waveform1.amplitude(0);
//     // Start the recording function
//     startRecording();
//     printMode();
//     break;
//   case Mode::Recording:
//     // Handset is replaced
//     if (buttonHook.risingEdge())
//     {
//       // Debug log
//       Serial.println("Stopping Recording");
//       // Stop recording
//       stopRecording();
//     }
//     else
//     {
//       continueRecording();
//     }
//     break;
//   }
// }

// if (buttonRecord.risingEdge())
// {
//   Serial.println("start recording!");
// }

// if (buttonHook.risingEdge())
// {
//   // rising edge is "lowering" the phone
//   Serial.println("phone replaced!");
// }
// else if (buttonHook.fallingEdge())
// {
//   // hook falling edge is "enabled"
//   Serial.println("phone lifted!");
// }