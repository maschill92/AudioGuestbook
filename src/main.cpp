// #include <Arduino.h>
// #include <Bounce.h>
// #include <Audio.h>
// #include <Wire.h>
// #include <SPI.h>
// #include <SD.h>
// #include <TimeLib.h>

// // GUItool: begin automatically generated code
// AudioInputI2S micIn;             // xy=175.3333282470703,458.3333282470703
// AudioSynthWaveform beepWaveform; // xy=205.3333282470703,159.3333282470703
// AudioPlaySdRaw playSdRaw;        // xy=220.3333282470703,198.3333282470703
// AudioMixer4 audioMixer;          // xy=376.3333282470703,174.3333282470703
// AudioAnalyzePeak peakAnalyzer;   // xy=466.3333435058594,501.33334255218506
// AudioRecordQueue recordQueue;    // xy=470.3333282470703,461.3333282470703
// AudioOutputI2S audioOutput;      // xy=524.3333282470703,174.3333282470703
// AudioConnection patchCord1(micIn, 0, recordQueue, 0);
// AudioConnection patchCord2(micIn, 0, peakAnalyzer, 0);
// AudioConnection patchCord3(beepWaveform, 0, audioMixer, 0);
// AudioConnection patchCord4(playSdRaw, 0, audioMixer, 1);
// AudioConnection patchCord5(audioMixer, 0, audioOutput, 0);
// AudioConnection patchCord6(audioMixer, 0, audioOutput, 1);
// AudioControlSGTL5000 sgtl5000; // xy=214.3333282470703,597.3333282470703
// // GUItool: end automatically generated code

// // Use these with the Teensy Audio Shield
// #define SDCARD_CS_PIN 10
// #define SDCARD_MOSI_PIN 7
// #define SDCARD_SCK_PIN 14
// #define HOOK_PIN 0
// #define RECORD_PIN 4

// Bounce buttonHook = Bounce(HOOK_PIN, 350);
// Bounce buttonRecord = Bounce(RECORD_PIN, 150);

// // Filename to save audio recording on SD card
// char filename[15];
// // The file object itself
// File frec;

// enum Mode
// {
//   Initialising,
//   Ready,
//   Prompting,
//   Recording
// };
// Mode mode = Mode::Initialising;

// void printMode()
// { // only for debugging
//   Serial.print("Mode switched to: ");
//   switch (mode)
//   {
//   case Mode::Initialising:
//     Serial.println(" Initialising");
//     break;
//   case Mode::Ready:
//     Serial.println(" Ready");
//     break;
//   case Mode::Prompting:
//     Serial.println(" Prompting");
//     break;
//   case Mode::Recording:
//     Serial.println(" Recording");
//     break;
//   default:
//     Serial.println(" Undefined");
//     break;
//   }
// }

// void startRecording()
// {
//   for (uint16_t i = 0; i < 9999; i++)
//   {
//     // Format the counter as a five-digit number with leading zeroes, followed by file extension
//     snprintf(filename, 11, " %05d.RAW", i);
//     // Create if does not exist, do not open existing, write, sync after write
//     if (!SD.exists(filename))
//     {
//       break;
//     }
//   }
//   frec = SD.open(filename, FILE_WRITE);
//   if (frec)
//   {
//     Serial.print("Recording to ");
//     Serial.println(filename);
//     recordQueue.begin();
//     mode = Mode::Recording;
//     printMode();
//   }
//   else
//   {
//     Serial.print("Couldn't open file ");
//     Serial.print(filename);
//     Serial.println("to record!");
//   }
// }

// void continueRecording()
// {
//   if (recordQueue.available() >= 2)
//   {
//     Serial.print("Peak: ");
//     Serial.println(peakAnalyzer.read());
//     byte buffer[512];
//     // Fetch 2 blocks from the audio library and copy
//     // into a 512 byte buffer.  The Arduino SD library
//     // is most efficient when full 512 byte sector size
//     // writes are used.
//     memcpy(buffer, recordQueue.readBuffer(), 256);
//     recordQueue.freeBuffer();
//     memcpy(buffer + 256, recordQueue.readBuffer(), 256);
//     recordQueue.freeBuffer();
//     elapsedMicros usec = 0;
//     // write all 512 bytes to the SD card
//     frec.write(buffer, 512);
//     // Uncomment these lines to see how long SD writes
//     // are taking.  A pair of audio blocks arrives every
//     // 5802 microseconds, so hopefully most of the writes
//     // take well under 5802 us.  Some will take more, as
//     // the SD library also must write to the FAT tables
//     // and the SD card controller manages media erase and
//     // wear leveling.  The recordQueue object can buffer
//     // approximately 301700 us of audio, to allow time
//     // for occasional high SD card latency, as long as
//     // the average write time is under 5802 us.
//     // Serial.print("SD write, us=");
//     // Serial.println(usec);
//   }
// }

// void stopRecording()
// {
//   recordQueue.end();
//   // consume what's left in the record queue and write that to the file
//   while (recordQueue.available() > 0)
//   {
//     frec.write((byte *)recordQueue.readBuffer(), 256);
//     recordQueue.freeBuffer();
//   }
//   // close the file and make things ready again
//   frec.close();
//   mode = Mode::Ready;
//   printMode();
// }

// // void stopRecording()
// // {
// //   // Stop adding any new data to the queue
// //   recordQueue.end();
// //   // Flush all existing remaining data from the queue
// //   while (recordQueue.available() > 0)
// //   {
// //     // Save to open file
// //     frec.write((byte *)recordQueue.readBuffer(), AUDIO_BLOCK_SAMPLES * sizeof(int16_t));
// //     recordQueue.freeBuffer();
// //     recByteSaved += AUDIO_BLOCK_SAMPLES * sizeof(int16_t);
// //   }
// //   writeOutHeader();
// //   // Close the file
// //   frec.close();
// //   Serial.println("Closed file");
// //   mode = Mode::Ready;
// //   printMode();
// //   // setMTPdeviceChecks(true); // enable MTP device checks, recording is finished
// // }

// void setup()
// {
//   Serial.begin(9600);
//   pinMode(HOOK_PIN, INPUT_PULLUP);
//   pinMode(RECORD_PIN, INPUT);
//   mode = Mode::Ready;
//   printMode();

//   // Audio connections require memory to work.  For more
//   // detailed information, see the MemoryAndCpuUsage example
//   AudioMemory(8);

//   // Comment these out if not using the audio adaptor board.
//   // This may wait forever if the SDA & SCL pins lack
//   // pullup resistors
//   sgtl5000.enable();
//   sgtl5000.volume(0.4);

//   SPI.setMOSI(SDCARD_MOSI_PIN);
//   SPI.setSCK(SDCARD_SCK_PIN);
//   if (!(SD.begin(SDCARD_CS_PIN)))
//   {
//     // stop here, but print a message repetitively
//     while (1)
//     {
//       Serial.println("Unable to access the SD card");
//       delay(500);
//     }
//   }
// }

// elapsedMillis fps;
// void loop()
// {
//   buttonHook.update();
//   buttonRecord.update();
//   switch (mode)
//   {
//   case Mode::Ready:
//     if (buttonRecord.risingEdge())
//     {
//       Serial.println("Begin recording!");
//       mode = Mode::Recording;
//       printMode();
//       // startRecording();
//     }
//     // if (buttonHook.fallingEdge())
//     // {
//     //   Serial.println("Handset lifted!");
//     //   mode = Mode::Prompting;
//     //   printMode();
//     // }
//     break;
//     // case Mode::Prompting:
//     //   delay(1000);

//     //   while (!playSdWav1.isStopped())
//     //   {
//     //     // Check whether the handset is replaced
//     //     buttonHook.update();
//     //     // Handset is replaced
//     //     if (buttonHook.risingEdge())
//     //     {
//     //       playSdWav1.stop();
//     //       mode = Mode::Ready;
//     //       printMode();
//     //       return;
//     //     }
//     //   }

//     //   // Check whether the record button is pressed
//     //   buttonRecord.update();
//     //   if (buttonRecord.fallingEdge())
//     //   {
//     //     mode = Mode::Recording;
//     //     printMode();
//     //     return;
//     //   }
//     // }

//     // Serial.println("Starting Recording");

//     // // Play the tone sound effect
//     // beepWaveform.begin(1, 440, WAVEFORM_SINE);
//     // delay(1250);
//     // beepWaveform.amplitude(0);
//     // // Start the recording function
//     // startRecording();
//     // printMode();
//     // break;
//   case Mode::Recording:
//     // Handset is replaced
//     if (buttonRecord.risingEdge())
//     {
//       // Debug log
//       Serial.println("Stop recording!");
//       mode = Mode::Ready;
//       printMode();
//       // Stop recording
//       // stopRecording();
//     }
//     else if (fps > 24 && peakAnalyzer.available())
//     {
//       fps = 0;
//       int monoPeak = peakAnalyzer.read() * 30.0;
//       Serial.print("|");
//       for (int cnt = 0; cnt < monoPeak; cnt++)
//       {
//         Serial.print(">");
//       }
//       Serial.println();

//       // continueRecording();
//     }
//     break;
//   case Mode::Initialising:
//   case Mode::Prompting:
//     break;
//   }
// }


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=422,772
AudioAnalyzePeak         peak1;          //xy=713,752
AudioAnalyzePeak         peak2;          //xy=716,797
AudioConnection          patchCord1(i2s1, 0, peak1, 0);
AudioConnection          patchCord2(i2s1, 1, peak2, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=505,881
// GUItool: end automatically generated code


void setup() {
  Serial.begin(9600);
  AudioMemory(8);
  // sgtl5000_1.enable();
  // sgtl5000_1.volume(0.5);
  // sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(36);
  delay(1000);
  Serial.println("Begin");
}

// for best effect make your terminal/monitor a minimum of 31 chars wide and as high as you can.

elapsedMillis fps;

void loop() {
  if (fps > 100) {
    if (peak1.available()) {
      fps = 0;
      int monoPeak = peak1.read() * 30.0;
      Serial.print("|");
      for (int cnt=0; cnt<monoPeak; cnt++) {
        Serial.print(">");
      }
      Serial.println();
    }
  }
}