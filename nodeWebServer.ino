/*
Automation System.
Author: mcaldas
Final project in electrical engineering at PUC-RIO
2018.1
 */


#include <ESP8266WiFi.h>
#include <Servo.h>


float currentTemperature;

Servo servo1;
const char* newtworkName = "xxxx";
const char* networkPass = "xxxx";

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
  digitalWrite(14, 0);
  delay(10);
  digitalWrite(12, 0);
  delay(10);
  digitalWrite(13, 0);
  delay(500);
  digitalWrite(14, 1);
  delay(10);
  digitalWrite(12, 1);
  delay(10);
  digitalWrite(13, 1);
  delay(1000);

  
  delay(500);
  digitalWrite(14, 0);
  delay(500);
  digitalWrite(12, 1);
  delay(500);
  digitalWrite(12, 0);
  delay(500); 
  digitalWrite(14, 1);
  delay(500);
  digitalWrite(14, 0);
  delay(500);
  digitalWrite(12, 1);
  delay(500);
  digitalWrite(12, 0);
  delay(500); 


  delay(500);
  digitalWrite(13, 0);
  delay(500);
  digitalWrite(13, 1);
  delay(500); 
  

   //SERVOS
   //servo1.attach(D6);
   
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
  
  if (req.indexOf("/gpio/0") != -1){
    val = "0";
    digitalWrite(D5, 0);
    digitalWrite(D6, 0);
  }
  else if (req.indexOf("/gpio/1") != -1){
    val = "1";
    digitalWrite(D5, 1);
    digitalWrite(D6, 1);
  }
  else if(req.indexOf("/whois") != -1){
    val = WiFi.localIP().toString();
    //servo1.write(180);
  }else if(req.indexOf("/temperature") != -1) {
    currentTemperature = ((analogRead(0)/1024.0)*(9.5))*10;
    Serial.printf("Temperature: ");
    Serial.println(String(analogRead(0)));
    Serial.println(String(analogRead(0)*0.322265625));
    Serial.println(currentTemperature);  
    Serial.println((9.5 * analogRead(A0) * 100.0) / 1024);
    val = (String(analogRead(0)*0.322265625));
  }
  // /write/digital/1/1
  else if(req.indexOf("/write/digital/")) {
       servo1.write(0);
        
      int firstSpace = req.indexOf(" ");
      int secondSpace = req.lastIndexOf(" ");
      String teste = req.substring(firstSpace + 1,secondSpace);
      Serial.println("TEEEEESTE\n");
      Serial.println(teste);
      ///write/digital/ has 14 caracteres
      String portNumberAndAction =  teste.substring(15);
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
  else {
    Serial.println("invalid request");
    val = "Invalido";
  }

  Serial.println(val);
    
  client.flush();


  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
  s += "<html>\r\n Valor: ";
  s += val;
  s += "\r\n</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

