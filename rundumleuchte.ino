#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <FastLED.h>

#include "config.h"

FASTLED_USING_NAMESPACE

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;
#define DATA_PIN    D1
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    72
#define LEDS_PER_ROUND 12
CRGB leds[NUM_LEDS];
#define BRIGHTNESS         160
#define FRAMES_PER_SECOND  120

CRGB curr_color;

void setup_wifi() {
  delay(10);
  Serial.print("Connecting to wifi... ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("connected.");
  randomSeed(micros());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Output to console
  Serial.println("MQTT message received");
  Serial.print(topic);
  Serial.print(": ");
  payload[length] = '\0';
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  // colors
  if (strcmp((char *)payload, "blue") == 0) {
    curr_color = CRGB::Blue;
  } else if (strcmp((char *)payload, "red") == 0) {
    curr_color = CRGB::Red;
  } else if (strcmp((char *)payload, "green") == 0) {
    curr_color = CRGB::Green;
  } else if (strcmp((char *)payload, "yellow") == 0) {
    curr_color = CRGB::Yellow;
  } else if (strcmp((char *)payload, "off") == 0) {
    curr_color = CRGB::Black;
  } else {
    Serial.println("unknown");
  }
}

void set_all_leds(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
  }
  FastLED.show();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Create a random client ID
    String clientId = "rundumlicht";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      client.subscribe("mumalab/rundumlicht");
      Serial.println("MQTT connected");
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  for (int x = 0; x < LEDS_PER_ROUND; x++) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    FastLED.clear();
    for (int i = 0; i < NUM_LEDS; i++) {
      if ((i % LEDS_PER_ROUND == x) || ((i + 1) % LEDS_PER_ROUND == x)) {
        leds[i] = curr_color;
      }
    }
    FastLED.show();
    delay(15);
  }
}
