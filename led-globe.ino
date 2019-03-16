/* led-globe
 * Arnaud Dessein 
 * https://github.com/adessein/led-globe
 * 
 * Controls a strip of 4 Neo Pixel GRBW LEDs with and a ESP8266
 */

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h>

#define IR_PIN     13
#define LED_PIN     2
#define NUM_LEDS    4
#define UPDATES_PER_SECOND 100
#define DELTA_BRIGHT 10

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

uint32_t color;
uint8_t brightness;

IRrecv irrecv(IR_PIN);
decode_results results, old_results;

void setup() {
  Serial.begin(115200);
    
  irrecv.enableIRIn();  // Start the receiver
  
  // Initialize all pixels to white
  color = strip.Color(0, 0, 0, 255);
  brightness = 255;
  strip.begin();
  strip.setBrightness(brightness);
  for(uint16_t i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, color);
  strip.show();

  while (!Serial) delay(50); // Wait for the serial connection to be establised.
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(IR_PIN);
}

void loop() {
  if (irrecv.decode(&results)) {
    
    /* If one keep a key pressed then the code 
     * of the key is issue and then a series
     * of 0xFFFFFFFFFFFFFFFF
     */
    if(results.value == 0xFFFFFFFFFFFFFFFF)
      results = old_results;
    else
      old_results = results;
    
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
      case 0xFF9867: // 0
        brightness = 0;
        break;
      case 0xFF18E7: // UP
        brightness = min(255, brightness + DELTA_BRIGHT);
        break;
      case 0xFF4AB5: // DN
        brightness = max(0, brightness - DELTA_BRIGHT);
        break;
    }

    for(uint16_t i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, color);
    strip.setBrightness(brightness);
    serialPrintUint64(results.value, HEX);
    Serial.println("");
    irrecv.resume();  // Receive the next value 
  }
  strip.show();
  delay(50);
}
