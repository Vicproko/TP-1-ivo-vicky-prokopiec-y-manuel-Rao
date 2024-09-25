#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Para parsear la respuesta JSON de OpenWeatherMap


// Configuración del sensor DHT11
#define DHTPIN 4 // Pin donde está conectado el DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// Configura los datos de la red WiFi
const char* ssid = "IoTB";
const char* password = "inventaronelVAR";


// Configuración de la API de OpenWeatherMap
const char* apiKey = "67411e1f92d101f96cf4792856ab1846";
const char* city = "Buenos Aires";
const char* apiUrl = "http://api.openweathermap.org/data/2.5/weather?q=Buenos%20Aires&appid=TU_API_KEY&units=metric";


// Crea el servidor en el puerto 80
WebServer server(80);


// Definir el pin del relé
const int relayPin = 2;
bool relayState = LOW; // Estado inicial del relé (apagado)


// Página HTML
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>Control y Lectura de DHT11</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { text-align: center; font-family: Arial; }
      .button { padding: 20px; font-size: 20px; }
    </style>
  </head>
  <body>
    <h1>Control del Relé y Lectura de Datos</h1>
    <p>Estado del Relé: <span id="relayState">%STATE%</span></p>
    <button class="button" onclick="toggleRelay()">Cambiar Estado</button>
    <h2>Sensor DHT11</h2>
    <p>Temperatura: <span id="localTemp">Cargando...</span> °C</p>
    <p>Humedad: <span id="localHum">Cargando...</span> %</p>
    <h2>Clima en Buenos Aires</h2>
    <p>Temperatura: <span id="remoteTemp">Cargando...</span> °C</p>
    <p>Humedad: <span id="remoteHum">Cargando...</span> %</p>
    <script>
      function toggleRelay() {
        fetch('/toggleRelay')
          .then(response => response.text())
          .then(state => {
            document.getElementById('relayState').innerHTML = state;
          });
      }


      function updateData() {
        fetch('/sensorData')
          .then(response => response.json())
          .then(data => {
            document.getElementById('localTemp').innerHTML = data.localTemp;
            document.getElementById('localHum').innerHTML = data.localHum;
            document.getElementById('remoteTemp').innerHTML = data.remoteTemp;
            document.getElementById('remoteHum').innerHTML = data.remoteHum;
          });
      }


      setInterval(updateData, 5000); // Actualiza los datos cada 5 segundos
    </script>
  </body>
</html>
)rawliteral";


// Función para reemplazar variables en el HTML
String processor(const String& var) {
  if (var == "STATE") {
    return relayState ? "ENCENDIDO" : "APAGADO";
  }
  return String();
}


// Función para manejar el cambio del estado del relé
void handleToggleRelay() {
  relayState = !relayState;
  digitalWrite(relayPin, relayState);
  server.send(200, "text/plain", relayState ? "ENCENDIDO" : "APAGADO");
}


// Función para devolver el estado del relé
void handleRelayState() {
  server.send(200, "text/plain", relayState ? "ENCENDIDO" : "APAGADO");
}


// Función para obtener los datos del DHT11 y OpenWeatherMap
void handleSensorData() {
  // Lectura del sensor DHT11
  float localTemp = dht.readTemperature();
  float localHum = dht.readHumidity();


  // Chequeo de errores en la lectura del DHT11
  if (isnan(localTemp) || isnan(localHum)) {
    localTemp = 0.0;
    localHum = 0.0;
  }


  // Petición a la API de OpenWeatherMap
  HTTPClient http;
  http.begin(apiUrl);
  int httpCode = http.GET();


  float remoteTemp = 0.0;
  float remoteHum = 0.0;


  if (httpCode == 200) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
   
    remoteTemp = doc["main"]["temp"].as<float>();
    remoteHum = doc["main"]["humidity"].as<float>();
  }


  http.end();


  // Crear el JSON para enviar al cliente
  String jsonResponse;
  jsonResponse += "{\"localTemp\":" + String(localTemp);
  jsonResponse += ",\"localHum\":" + String(localHum);
  jsonResponse += ",\"remoteTemp\":" + String(remoteTemp);
  jsonResponse += ",\"remoteHum\":" + String(remoteHum) + "}";


  server.send(200, "application/json", jsonResponse);
}


// Función para servir la página principal
void handleRoot() {
  String htmlContent = webpage;
  htmlContent.replace("%STATE%", relayState ? "ENCENDIDO" : "APAGADO");
  server.send(200, "text/html", htmlContent);
}


void setup() {
  // Inicializa el puerto serie y el sensor DHT11
  Serial.begin(115200);
  dht.begin();


  // Inicializa el pin del relé
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState);


  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Conectado a la red WiFi.");
  Serial.println(WiFi.localIP());


  // Configura las rutas del servidor
  server.on("/", handleRoot);
  server.on("/toggleRelay", handleToggleRelay);
  server.on("/relayState", handleRelayState);
  server.on("/sensorData", handleSensorData); // Nueva ruta para los datos del sensor


  // Inicia el servidor web
  server.begin();
  Serial.println("Servidor web iniciado.");
}


void loop() {
  // Maneja las peticiones de los clientes
  server.handleClient();
}
