/* led-globe
 * Arnaud Dessein 
 * https://github.com/adessein/led-globe
 * 
 * Controls a strip of 4 Neo Pixel GRBW LEDs with and a ESP8266
 */

#include <IRremoteESP8266.h> // https://github.com/crankyoldgit/IRremoteESP8266
#include <IRrecv.h>
#include <IRutils.h>
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include <UniversalTelegramBot.h> // https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/tree/V1.2.0
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h> // SSL is required for Telegram

#include "credentials.h"
/* The file credentials.h contains:
#define BOT_TOKEN "XXXX:XXXX"  // your Bot Token (Get from Botfather)
#define CHAT_ID "XXXX" // Chat ID of where you want the message to go (You can use MyIdBot to get the chat ID)
#define WIFI_SSID "XXXX"
#define WIFI_PASS "XXXX"
*/

#define IR_PIN     13 // D7
#define LED_PIN     2  
#define NUM_LEDS    4
#define UPDATES_PER_SECOND 100
#define DELTA_BRIGHT 10
#define DELTA_WP 5

// SSL client needed for both libraries
WiFiClientSecure client;

// ------- Telegram config --------
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long lastTelegram, elapsedTime;
void sendTelegramMessage();

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

  lastTelegram = 0L;
    
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

    // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("[");
    Serial.print(WiFi.status());
    Serial.print("]");
    delay(1000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
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
        elapsedTime = millis() - lastTelegram;
        if (elapsedTime > 10000L)
        {
          sendTelegramMessage(); // Max one message a minute
        }
        else
        {
          Serial.println("Too eraly to telegram");
          Serial.println(elapsedTime);
        }
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

void sendTelegramMessage() {
  Serial.println("sendTelegramMessage");
  String message = "Message de Louis\n";
  if(bot.sendSimpleMessage(CHAT_ID, message, ""))
  {
    Serial.println("TELEGRAM Successfully sent");
    lastTelegram = millis();
  }
  else
  {
    Serial.println("ERROR while sending TELEGRAM");
  }
  
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
