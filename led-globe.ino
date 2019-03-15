#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h>

const uint16_t kRecvPin = 13;
#define LED_PIN     2
#define NUM_LEDS    4
#define BRIGHTNESS  64
#define UPDATES_PER_SECOND 100

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_RGBW + NEO_KHZ800);

uint32_t color;

IRrecv irrecv(kRecvPin);
decode_results results;

void setup() {
  Serial.begin(115200);
  color = strip.Color(0, 0, 0, 0);
  
  irrecv.enableIRIn();  // Start the receiver
  
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show(); // Initialize all pixels to 'off'

  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(kRecvPin);
}

void loop() {

  if (irrecv.decode(&results)) {
    switch(results.value)
    {
      case 0xFFA25D: // 1
        color = strip.Color(255, 0, 0, 0);
        break;
      case 0xFF629D: // 2
        color = strip.Color(0, 255, 0, 0);
        break;
      case 0xFFE21D: // 3
        color = strip.Color(0, 0, 255, 0);
        break;
    }

    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // Receive the next value 
  }
  strip.show();
  delay(50);
  
  
}
