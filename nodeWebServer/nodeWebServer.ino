/*
Automation System.
Author: Matheus Caldas
Final project in electrical engineering at PUC-RIO
2018.1
 */


#include <ESP8266WiFi.h>
#include <Servo.h>


float currentTemperature;
int lastServoValue = 0;

Servo servo1;
Servo servo2;
const char* newtworkName = "VirtuaLineTeu";
const char* networkPass = "paulo1968martha1969nanda1992teu1994";

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


  delay(500);
  digitalWrite(14, 0);delay(10);
  digitalWrite(12, 0);delay(10);
  digitalWrite(13, 0);delay(10);
  delay(500);
  digitalWrite(14, 1);delay(10);
  digitalWrite(12, 1);delay(10);
  digitalWrite(13, 1);delay(10);
  delay(500);
  digitalWrite(14, 0);delay(10);
  digitalWrite(12, 0);delay(10);
  digitalWrite(13, 0);delay(10);
  delay(500);
  digitalWrite(14, 1);delay(10);
  digitalWrite(12, 1);delay(10);
  digitalWrite(13, 1);delay(10);
  

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
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  while(!client.available()){
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
  
if(req.indexOf("/temperature") != -1) {
    currentTemperature = ((analogRead(0)/1024.0)*(9.5))*10;
    Serial.printf("Temperature: ");
    Serial.println(String(analogRead(0)));
    Serial.println(String(analogRead(0)*0.322265625));
    Serial.println(currentTemperature);  
    Serial.println((9.5 * analogRead(A0) * 100.0) / 1024);
    val = (String(analogRead(0)*0.322265625));
  } else if(req.indexOf("/digital/write") != -1) {
        
      String cleanUrl = getCleanURL(req);
      ///write/digital/ has 14 caracteres
      String portNumberAndAction =  cleanUrl.substring(15);
      String action = portNumberAndAction.substring(3);
      String portNumber = portNumberAndAction.substring(0,2);

      Serial.print("PortNumberAndAction: ");
      Serial.println(portNumberAndAction);
      
      Serial.print("portNumber: ");
      Serial.println(portNumber);
      
      Serial.print("action: ");
      Serial.println(action);

      digitalWrite(portNumber.toInt(), action.toInt());
    
  }
  else if(req.indexOf("/changeServo") != -1) {
    
      ///write/digital/ has 14 caracteres
      String cleanURL = getCleanURL(req);
      String portNumber =  cleanURL.substring(13);
      
      
      if(lastServoValue == 0)
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
  if(val == NULL){
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
    return url.substring(firstSpace + 1,secondSpace);
  }

