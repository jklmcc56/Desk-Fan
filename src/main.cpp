#include <Arduino.h>
#include <LiquidCrystal.h>
#include <DHT.h>

//Initializes the LCD and DHT using the libraries above
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);
DHT dht(7, DHT11);

//Degree symbol
byte degree[8] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

//DHT11 needs at least two seconds between readings for accuracy
unsigned long dht_read_time = 0;
const unsigned long dht_read_interval = 2000;

bool on_off = true;
int speed = 100;

//Joystick button needs at least 50 ms for debouncing
unsigned long js_read_time = 0;
const unsigned long js_read_interval = 50;

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(0, degree);

  dht.begin();

  //Pins for the L293D chip
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(5, HIGH);
  digitalWrite(4, LOW);

  //Pin for the joystick module
  pinMode(3, INPUT_PULLUP);
}

void loop() {
  unsigned long current_time = millis();

  //Only reads DHT11 and updates LCD if enough time has passed
  if (current_time - dht_read_time >= js_read_interval) {
    float temp = dht.readTemperature(true);
    int humidity = dht.readHumidity();

    dht_read_time = current_time;

    //If the sensor reads incorrectly, I do not want the LCD to display weird symbols
    if (isnan(temp) || isnan(humidity)) {
      return;
    }

    char temp_buffer[16];
    char humidity_buffer[16];
    dtostrf(temp, 5, 2, temp_buffer);
    itoa(humidity, humidity_buffer, 10);

    lcd.setCursor(0, 0);
    lcd.print("Temp:    ");
    lcd.print(temp_buffer);
    //This writes the degree symbol
    lcd.write((uint8_t)0);
    lcd.write("F");

    lcd.setCursor(0, 1);
    lcd.print("Humidity:    ");
    lcd.print(humidity_buffer);
    lcd.print("%");

    //Temp variable is adjustable. I set it to 75 since I like warmer temperatures
    if (temp >= 75 && on_off) {
      digitalWrite(6, speed);
    }
    else {
      digitalWrite(6, 0);
    }
  }

  int js_y = analogRead(A0);
  int js_x = analogRead(A1);
  int js_switch = digitalRead(3);

  //Toggles fan given enough time has passed for a debouncing
  if (current_time - js_read_time >= js_read_interval && js_switch == LOW) {
    on_off = !on_off;

    js_read_time = current_time;
  }

  //The joystick variables read from 0-1023, so I took only the more extreme values for adjusting the speed and direction
  //This is done so when I adjust one axis, it does not accidentally trigger the other axis.
  if (js_x >= 895 && speed < 255) {
    speed++;
  }
  else if (js_x < 127 && speed > 0) {
    speed--;
  }
  if (js_y >= 895) {
    digitalWrite(5, HIGH);
    digitalWrite(4, LOW);
  }
  else if (js_y < 127) {
    digitalWrite(5, LOW);
    digitalWrite(4, HIGH);
  }
}