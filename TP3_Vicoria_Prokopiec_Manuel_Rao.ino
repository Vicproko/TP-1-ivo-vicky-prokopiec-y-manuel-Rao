#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "ioTB";
const char* password = "inventaronelVAR";

WebServer server(80);

const int DHTPIN = 2;  // Pin digital al que está conectado el DHT11

// Configura los pines para los LEDs
#define LED1 33 
#define LED2 32 
#define LED3 27 
#define LED4 26
#define LED5 25

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int humedad = 0; 

void setup() {

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);

    // Inicializa la comunicación serie
  Serial.begin(9600);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a la red WiFi");

  // Configura la página web
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor web iniciado");

}

void loop() {
    server.handleClient();

  humedad = dht.readHumidity();

  // Verifica si la lectura es válida
  if (isnan(humedad)) {
    Serial.println("Error para leer el sensor de humedad");
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(humedad);
  
  apagarTodos();
  
  if (humedad < 20) {
    digitalWrite(LED1, HIGH);
  } else if (humedad >= 20 && humedad < 40) {
    digitalWrite(LED2, HIGH);
  } else if (humedad >= 40 && humedad < 60) {
    digitalWrite(LED3, HIGH);
  } else if (humedad >= 60 && humedad < 80) {
    digitalWrite(LED4, HIGH);
  } else if (humedad >= 80) {
    digitalWrite(LED5, HIGH);
  }

  delay(750);
}
  
  void apagarTodos(){  
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  }

void handleRoot() {
  String page = "<html>\
  <head>\
    <meta http-equiv='refresh' content='2' />\
    <style>\
      .bar { width: 100%; background-color: #ddd; }\
      .progress { height: 30px; background-color: #4CAF50; width:" + String(humedad) + "%;}\
    </style>\
  </head>\
  <body>\
    <h1>Humedad actual: " + String(humedad) + "%</h1>\
    <div class='bar'><div class='progress'></div></div>\
  </body>\
  </html>";
  
  server.send(200, "text/html", page);
}