#include <WiFi.h>
#include <WebServer.h>

// Configura los datos de la red WiFi a la que te quieres conectar
const char* ssid = "IoTB";
const char* password = "inventaronelVAR";

// Crea el servidor en el puerto 80
WebServer server(80);

// Define el pin del relé
const int relayPin = 2; // Cambia el pin si es necesario

// Variable para almacenar el estado del relé
bool relayState = LOW;

// Página HTML
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>Control del Relé</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body { text-align: center; font-family: Arial; }
      .button { padding: 20px; font-size: 20px; }
    </style>
  </head>
  <body>
    <h1>Control de Relé</h1>
    <p>Estado del Relé: <span id="relayState">%STATE%</span></p>
    <button class="button" onclick="toggleRelay()">Cambiar Estado</button>
    <script>
      function toggleRelay() {
        // Llamada a la función en el servidor para cambiar el estado del relé
        fetch('/toggleRelay')
          .then(response => response.text())
          .then(state => {
            document.getElementById('relayState').innerHTML = state; // Actualiza el estado en la página
          });
      }

      // Función para obtener el estado actual del relé
      setInterval(() => {
        fetch('/relayState')
          .then(response => response.text())
          .then(state => {
            document.getElementById('relayState').innerHTML = state;
          });
      }, 1000); // Consulta cada segundo
    </script>
  </body>
</html>
)rawliteral";

// Función para reemplazar el estado del relé en la página HTML
String processor(const String& var) {
  if (var == "STATE") {
    return relayState ? "ENCENDIDO" : "APAGADO";
  }
  return String();
}

// Función para manejar la petición de cambiar el estado del relé
void handleToggleRelay() {
  relayState = !relayState;               // Cambia el estado del relé
  digitalWrite(relayPin, relayState);      // Actualiza el pin
  server.send(200, "text/plain", relayState ? "ENCENDIDO" : "APAGADO"); // Responde con el nuevo estado
}

// Función para devolver el estado del relé
void handleRelayState() {
  server.send(200, "text/plain", relayState ? "ENCENDIDO" : "APAGADO");
}

void handleRoot() {
  // Remplaza %STATE% en el HTML con el estado real del relé
  String htmlContent = webpage;
  htmlContent.replace("%STATE%", relayState ? "ENCENDIDO" : "APAGADO");
  server.send(200, "text/html", htmlContent);
}

void setup() {
  // Inicializa el puerto serie
  Serial.begin(115200);

  // Inicializa el pin del relé
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, relayState); // Estado inicial del relé (apagado)

  // Conexión a la red WiFi
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

  // Ruta para cambiar el estado del relé
  server.on("/toggleRelay", handleToggleRelay);

  // Ruta para consultar el estado del relé
  server.on("/relayState", handleRelayState);

  // Inicia el servidor web
  server.begin();
  Serial.println("Servidor web iniciado.");
}

void loop() {
  // Maneja las peticiones de los clientes
  server.handleClient();
}