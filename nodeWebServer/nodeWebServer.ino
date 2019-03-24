
/*
  Automation System.
  Author: Matheus Caldas
  Final project in electrical engineering at PUC-RIO
  2018.1
*/



#include <ESP8266WiFi.h>
#include <Servo.h>
#include <ESP8266HTTPClient.h>

//TEMPERATURA E UMIDADE
#include "DHT.h"
#define DHTTYPE DHT11
#define dht_dpin 0
DHT dht(dht_dpin, DHTTYPE);




//variables
float currentTemperature;
float currentHumidity;
int currentVibrationValue;
int lastServoValue = 0;


//servos
Servo servo1;
Servo servo2;

//sound
#define soundDigitalPin 4

//rain
#define vibrationDigitalPin 15
#define rainAnalogPin A0
int currentRainValue;

//variable to prevent sending more than one notification
int isRainNotificationAvailable = 1;
int isVibrationNotificationAvailable = 1;


//WIFI CONNECTION
HTTPClient http;
const char* newtworkName = "networkName";
const char* networkPass = "networkPass";
String serverIpNotification = "http://192.168.0.199:9082/automation/notification";

//vibration



// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(8080);

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println("Welcome to debug mode for Automation System\n");

  pinMode(A0, INPUT);

  //just for tests
  pinMode(14, OUTPUT);
  delay(10);
  pinMode(12, OUTPUT);
  delay(10);
  pinMode(13, OUTPUT);
  delay(10);
  pinMode(15 , INPUT);
  delay(10);
  pinMode(soundDigitalPin, OUTPUT);

  delay(500);
  digitalWrite(14, 0); delay(10);
  digitalWrite(12, 0); delay(10);
  digitalWrite(13, 0); delay(10);
  digitalWrite(soundDigitalPin, 0); delay(10);
  delay(500);
  digitalWrite(14, 1); delay(10);
  digitalWrite(12, 1); delay(10);
  digitalWrite(13, 1); delay(10);
  digitalWrite(soundDigitalPin, 1); delay(10);
  delay(500);
  digitalWrite(14, 0); delay(10);
  digitalWrite(12, 0); delay(10);
  digitalWrite(13, 0); delay(10);
  digitalWrite(soundDigitalPin, 0); delay(10);
  delay(500);
  digitalWrite(14, 1); delay(10);
  digitalWrite(12, 1); delay(10);
  digitalWrite(13, 1); delay(10);

  //temperature e umidade
  dht.begin();

  // Connect to WiFi network
  Serial.print("Trying Connecting to newtork: ");
  Serial.print(newtworkName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(newtworkName, networkPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("\nStill not connected :/\n");
  }
  Serial.println("wifi has been successfully connected");

  // Start the server
  server.begin();
  Serial.println("Server started, is running on IP: ");
  Serial.println( WiFi.localIP());

}

void loop() {


  checkNotifications();

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  while (!client.available()) {
    //waiting while there is no client
    delay(1);
  }


  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  delay(50);
  // Match the request
  String val;
  String portNumber;

  if (req.indexOf("/temperature") != -1) {
    readTemperatureAndHumidity();
    val = (currentTemperature);
  } else if (req.indexOf("/humidity") != -1) {
    readTemperatureAndHumidity();
    val = (currentHumidity);
  } else if (req.indexOf("/rain") != -1) {
    readRain();
    val = (currentRainValue);
  } else if (req.indexOf("/stopSound") != -1) {
    Serial.println("Lets stop SOUND !!!");
    digitalWrite(soundDigitalPin, 0); delay(10);
  }
  else if (req.indexOf("/digital/write") != -1) {

    String cleanUrl = getCleanURL(req);
    ///write/digital/ has 14 caracteres
    String portNumberAndAction =  cleanUrl.substring(15);
    String action = portNumberAndAction.substring(3);
    String portNumber = portNumberAndAction.substring(0, 2);

    Serial.print("PortNumberAndAction: ");
    Serial.println(portNumberAndAction);

    Serial.print("portNumber: ");
    Serial.println(portNumber);

    Serial.print("action: ");
    Serial.println(action);

    if(portNumber.toInt() == 12){
      //relay light is inverted
      if(action.toInt() == 1) {
        digitalWrite(portNumber.toInt(), 0);
      } 
      else {
        digitalWrite(portNumber.toInt(), 1);
      }
    }
    else {
     digitalWrite(portNumber.toInt(), action.toInt());
    }
   
  }
  else if (req.indexOf("/changeServo") != -1) {

    ///write/digital/ has 14 caracteres
    String cleanURL = getCleanURL(req);
    String portNumber =  cleanURL.substring(13);


    if (lastServoValue == 0)
    {
      lastServoValue = 180;
    } else {
      lastServoValue = 0;
    }

    //SERVOS
    Serial.println("PORT NUMBER: " + portNumber);
    Serial.println("Servo value: ");
    Serial.println(lastServoValue);

    servo1.attach(portNumber.toInt());

    delay(500);
    servo1.write(lastServoValue);
    servo1.detach();
  }
  else {
    Serial.println("invalid request");
    val = "Invalido";
  }

  Serial.println(val);

  client.flush();


  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";
  s += "{\"value\": ";
  if (val == NULL) {
    val = "\"\"";
  }
  s += val;
  s += "}";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed
}

String getCleanURL(String url) {
  //is something like 'POST /changeServo/5 HTTP/1.1'
  int firstSpace = url.indexOf(" ");
  int secondSpace = url.lastIndexOf(" ");
  //and returns something like '/changeServo/5'
  return url.substring(firstSpace + 1, secondSpace);
}


void checkNotifications() {
  checkRainNotification();
  checkVibrationNotification();

}

void checkRainNotification() {
  readRain();

  Serial.println("Lets check if notification is available");
  if (currentRainValue > 1000) {
    isRainNotificationAvailable = 1;
  }

  if (isRainNotificationAvailable == 0) {
    //nothing to be done because
    //notification has already been sent
    return ;
  }
  Serial.println("Rain analog pin 0");
  Serial.println(currentRainValue);

  if (currentRainValue < 1000) {
    isRainNotificationAvailable = 0;
    sendNotification("Alerta de Chuva");
  } else if (rainAnalogPin > 1000) {
    //has stopped raining
    isRainNotificationAvailable = 1;
  }

}

void readTemperatureAndHumidity() {
  currentHumidity = dht.readHumidity();
  currentTemperature = dht.readTemperature();
  Serial.print("Reading temperature\n and humidty...\nCurrent humidity = ");
  Serial.print(currentHumidity);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(currentTemperature);
  Serial.println("ºC");

}

void checkVibrationNotification() {
  readVibration();  
}
void readRain() {
  currentRainValue = analogRead(rainAnalogPin);
  Serial.print("Sensor de chuva\n");
  Serial.println("Valor analogico: ");
  Serial.println(currentRainValue);

}

void readVibration() {
  currentVibrationValue = digitalRead(vibrationDigitalPin);
  Serial.print("Sensor de vibracao\n");
  Serial.println("Valor digital: ");
  Serial.println(currentVibrationValue);
  
  Serial.println("Lets check if notification is available");
  if (currentVibrationValue == 1) {
    isVibrationNotificationAvailable = 1;
  }

  if (isVibrationNotificationAvailable == 0) {
    //nothing to be done because
    //notification has already been sent
    return ;
  }
  
  if (currentVibrationValue == 1) {
    isVibrationNotificationAvailable = 0;
    sendNotification("Alerta de vibracao");
    digitalWrite(soundDigitalPin, 1); delay(10);
  } else{
    //has stopped vibration
    isVibrationNotificationAvailable = 1;
  }

}

void sendNotification (String message) {
  int httpCode;
  String postRequest;
  http.begin(serverIpNotification);
  http.addHeader("Content-Type", "application/json");
  postRequest = "{\"message\": ";
  postRequest += "\"";
  postRequest += message;
  postRequest += "\"}";
  httpCode = http.POST(postRequest);
  http.end();
  Serial.println("end\n\n\n\n");
  Serial.println("httpCode:");
  Serial.println(httpCode);
  http.end();   //Close connection
}
