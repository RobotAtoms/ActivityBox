#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <SdFat.h>
#include <SFEMP3Shield.h>

// Define pin numbers
const int musicButtonPin = A2;      // White Square Button - Plays music
const int ledButtonPin = A3;        // Blue Triangle Button - Turns on LED lights
const int micButtonPin = A4;        // Yellow Round Button - Turns on microphone that controls lights
const int micPin = A0;             // Microphone input pin
const int ledPin = 10;             // NeoPixel LED pin

// Variables to store the button states
int lastmusicButtonState = LOW;
int lastledButtonState = LOW;
int lastmicButtonState = LOW;
bool micActive = false;            // Microphone control activation flag
unsigned long lastMusicPlayTime = 20001;  // Time since last music play
int lastTrackNumber = 0;           // Last played track number

// MP3 Player setup
SdFat sd;
SFEMP3Shield MP3player;

// NeoPixel configuration
const int numPixels = 10;          // Number of NeoPixels in the strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, ledPin, NEO_GRB + NEO_KHZ800);

void setup() {
  // Initialize the button pins as inputs
  pinMode(musicButtonPin, INPUT_PULLUP);
  pinMode(ledButtonPin, INPUT_PULLUP);
  pinMode(micButtonPin, INPUT_PULLUP); 

  // Start the NeoPixel strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
 
  // Start serial communication
  Serial.begin(9600);
  
  // Initialize MP3 Player
  if (!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  uint8_t result = MP3player.begin();
  if (result != 0) {
    Serial.print(F("Error code: "));
    Serial.print(result);
    Serial.println(F(" when trying to start MP3 player"));
    if (result == 6) {
      Serial.println(F("Warning: patch file not found, skipping."));
    }
  }
  MP3player.setVolume(50, 50);
  randomSeed(analogRead(5));
}

void loop() {  
  // Read the state of the buttons
  bool currentmusicButtonState = digitalRead(musicButtonPin);
  bool currentledButtonState = digitalRead(ledButtonPin);
  bool currentmicButtonState = digitalRead(micButtonPin);

  // Handle button presses 
  if (lastmusicButtonState == HIGH && currentmusicButtonState == LOW) {
    handleMusicButtonPress();
  }
  if (lastledButtonState == HIGH && currentledButtonState == LOW) {
    handleLEDButtonPress();
  }
  if (lastmicButtonState == HIGH && currentmicButtonState == LOW) {
    Serial.println("Button was clicked");
    toggleMicControl();
  }

  lastmusicButtonState = currentmusicButtonState;
  lastledButtonState = currentledButtonState;
  lastmicButtonState = currentmicButtonState;

  // Handle continuous microphone listening
  if (micActive) {
    activateMicrophoneControlledLEDs();
  }

  // Add a small delay to prevent reading too fast
  delay(100);
}

void handleMusicButtonPress() {
  Serial.println("Music Button (White Square) pressed.");
  playRandomTrack();
  lastMusicPlayTime = millis();
}

void playRandomTrack() {
  // Generate a random track number
  int trackNumber = random(1, 125);  // Adjust 101 to the actual number of tracks if different

  // Construct the track name string
  char trackName[13];  // "trackXXX.mp3" + null terminator
  sprintf(trackName, "track%03d.mp3", trackNumber);
  Serial.println(trackName);

  // Play the track
  uint8_t result = MP3player.playMP3(trackName);
  if (result != 0) {
    Serial.print("Error playing ");
    Serial.println(trackName);
  } else {
    Serial.print("Playing ");
    Serial.println(trackName);
  }
}

void handleLEDButtonPress() {
  Serial.println("LED Button (Blue Triangle) pressed.");
  displayRainbow();
}

void toggleMicControl() {
  micActive = !micActive; // Toggle the state
  Serial.print("Mic Control is now ");
  Serial.println(micActive ? "ON" : "OFF");
}

void displayRainbow() {
  for(int i = 0; i < numPixels; i++) {
    int hue = map(i, 0, numPixels - 1, 0, 255);
    strip.setPixelColor(i, strip.ColorHSV(hue, 255, 64)); // Half brightness
  }
  strip.show();
}

void activateMicrophoneControlledLEDs() {
    int soundLevel = analogRead(micPin);
    int brightness = map(soundLevel, 0, 1023, 0, 64);  // Capped at 33% brightness
    int numLit = map(soundLevel, 0, 1023, 0, numPixels);  // Number of LEDs to light up

    // Determine color based on sound level
    uint32_t color;
    if (soundLevel < 341) {  // Lower third of input range
        color = strip.Color(0, 0, brightness);  // Blue
    } else if (soundLevel < 682) {  // Middle third
        color = strip.Color(0, brightness, 0);  // Green
    } else {  // Upper third
        color = strip.Color(brightness, 0, 0);  // Red
    }

    for (int i = 0; i < numLit; i++) {
        strip.setPixelColor(i, color);
    }
    for (int i = numLit; i < numPixels; i++) {
        strip.setPixelColor(i, 0);  // Turn off the rest
    }
    strip.show();
}