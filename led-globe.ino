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
#define DELTA_WP 5

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRBW + NEO_KHZ800);

uint8_t brightness, step;

IRrecv irrecv(IR_PIN);
decode_results results, old_results;

uint32_t Wheel(byte WheelPos);
byte wp, mode;

void setup() {
  Serial.begin(115200);

  step = 0;
  mode = 0;
  wp = 0;
    
  irrecv.enableIRIn();  // Start the receiver
  
  // Initialize all pixels to white
  brightness = 255;
  strip.begin();
  strip.setBrightness(brightness);
  for(uint16_t i=0; i<strip.numPixels(); i++) 
    strip.setPixelColor(i, 0, 0, 0, 255);
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
        mode = 0;
        wp = 0; // Red
        break;
      case 0xFF629D: // 2
        mode = 0;
        wp = 85; // Green
        break;
      case 0xFFE21D: // 3
        mode = 0;
        wp = 170; // Blue
        break;
      case 0xFF22DD: // 4
        break;
      case 0xFF02FD: // 5
        break;
      case 0xFFC23D: // 6
        break;
      case 0xFFE01F: // 7
        break;
      case 0xFFA857: // 8
        mode = 0;
        wp = 235; // Pink
        break;
      case 0xFF906F: // 9
        break;
      case 0xFF9867: // 0
        mode = 0;
        brightness = 0;
        break;
      case 0xFF6897: // *
        mode = 1;
        step = 0;
        break;
      case 0xFFB04F: // #
        mode = 2;
        step = 0;
        break;
      case 0xFF10EF: // LEFT
        // Decrease value
        if(mode != 1)
          wp = max(0, wp - DELTA_WP);
        break;
      case 0xFF5AA5: // RIGHT
        // Increase value
        if(mode != 1)
          wp = min(255, wp + DELTA_WP);
        break;
      case 0xFF18E7: // UP
        // Increase brightness
        if(mode != 2)
          brightness = min(255, brightness + DELTA_BRIGHT);
        break;
      case 0xFF4AB5: // DN
        // Decrease brightness
        if(mode != 2)
          brightness = max(0, brightness - DELTA_BRIGHT);
        break;
      case 0xFF38C7: // OK
        break;
    }

    serialPrintUint64(results.value, HEX);
    Serial.println("");
    Serial.println(wp);
    irrecv.resume();  // Receive the next value 
  }
  
  if(mode==1)
  {
    // Rainbow (color sine)
    wp = strip.sine8(step++);
  }
  
  if(mode==2)
  {
    // Brightness sine
    brightness = strip.sine8(step++);
  }
  
  for(uint16_t i=0; i<strip.numPixels(); i++) strip.setPixelColor(i, Wheel(wp));
  strip.setBrightness(brightness);
  strip.show();
  delay(50);
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
