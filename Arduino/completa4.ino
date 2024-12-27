#include <WiFi.h>
#include <PubSubClient.h>

// Configuración conexión WiFi
const char* ssid = "MIWIFI_mPNT 2.4";
const char* password = "hxQetGpu";

// Configuración conexión MQTT
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqttTopicInput = "sistema/control";
const char* mqttTopicEstado = "sistema/estado";
const char* mqttTopicPIR = "sensor/pir";
const char* mqttTopicSW420 = "sensor/vibracion_sw420";
const char* mqttTopicKY002 = "sensor/vibracion_ky002";
const char* mqttTopicMagnetico = "sensor/magnetico";
const char* mqttTopicLED = "actuador/led";
const char* mqttTopicBuzzer = "actuador/buzzer";

WiFiClient espClient;
PubSubClient client(espClient);

// Pines
const int pinPulsadorActivacion = 4;
const int pinPIR = 15;
const int pinSW420 = 18;
const int pinKY002 = 46;
const int pinMagnetico = 3;
const int pinLED = 16;
const int pinBuzzer = 17;

// Variables para el estado del sistema
bool sistemaActivo = false;
bool presenciaDetectada = false;
bool vibracionSW420Detectada = false;
bool vibracionKY002Detectada = false;
bool magneticoDetectado = false;
bool alarmaActiva = false;

// Variables para debounce
unsigned long tiempoUltimoCambio = 0;
const unsigned long debounceDelay = 50;

// Variables para publicación periódica
unsigned long tiempoUltimaPublicacionSensores = 0;
const unsigned long intervaloPublicacionSensores = 1 * 60 * 1000; // Cada minuto

// Función para conectar a WiFi
void connectToWiFi() {
    Serial.print("Conectando a WiFi...");
    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
        delay(1000);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi conectado!");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nError: No se pudo conectar a WiFi. Reiniciando...");
        ESP.restart();
    }
}

// Función para conectar al broker MQTT
void connectToMQTT() {
    while (!client.connected()) {
        Serial.print("Conectando al broker MQTT...");
        String clientId = "ESP32-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("Conectado al broker MQTT!");
            client.subscribe(mqttTopicInput);
        } else {
            Serial.print("Error al conectar al broker: ");
            Serial.println(client.state());
            delay(2000);
        }
    }
}

// Publicar estados en MQTT
void publicarEstadosMQTT() {
    client.publish(mqttTopicEstado, sistemaActivo ? "ACTIVO" : "DESACTIVADO");
    client.publish(mqttTopicPIR, presenciaDetectada ? "DETECTADO" : "SIN_MOVIMIENTO");
    client.publish(mqttTopicSW420, vibracionSW420Detectada ? "VIBRACION" : "SIN_VIBRACION");
    client.publish(mqttTopicKY002, vibracionKY002Detectada ? "VIBRACION" : "SIN_VIBRACION");
    client.publish(mqttTopicMagnetico, magneticoDetectado ? "ABIERTO" : "CERRADO");
    client.publish(mqttTopicLED, digitalRead(pinLED) ? "ENCENDIDO" : "APAGADO");
    client.publish(mqttTopicBuzzer, digitalRead(pinBuzzer) ? "ENCENDIDO" : "APAGADO");

    // Imprimir estados en el monitor serie
    Serial.println("\n=== ESTADOS ACTUALES ===");
    Serial.print("Estado del sistema: ");
    Serial.println(sistemaActivo ? "ACTIVO" : "DESACTIVADO");
    Serial.print("PIR: ");
    Serial.println(presenciaDetectada ? "DETECTADO" : "SIN_MOVIMIENTO");
    Serial.print("Vibración SW420: ");
    Serial.println(vibracionSW420Detectada ? "VIBRACION" : "SIN_VIBRACION");
    Serial.print("Vibración KY002: ");
    Serial.println(vibracionKY002Detectada ? "VIBRACION" : "SIN_VIBRACION");
    Serial.print("Magnético: ");
    Serial.println(magneticoDetectado ? "ABIERTO" : "CERRADO");
    Serial.print("LED: ");
    Serial.println(digitalRead(pinLED) ? "ENCENDIDO" : "APAGADO");
    Serial.print("Buzzer: ");
    Serial.println(digitalRead(pinBuzzer) ? "ENCENDIDO" : "APAGADO");
}

// Manejar mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == mqttTopicInput) {
        if (message == "ACTIVAR") {
            sistemaActivo = true;
            Serial.println("SISTEMA DE INTRUSIÓN ACTIVADO (por Node-RED)");
        } else if (message == "DESACTIVAR") {
            sistemaActivo = false;
            Serial.println("SISTEMA DE INTRUSIÓN DESACTIVADO (por Node-RED)");
            desactivarAlarma();
        }
        publicarEstadosMQTT();
    }
}

// Leer pulsador con debounce mejorado
bool leerPulsador(int pin) {
    static bool estadoAnterior = HIGH;
    static unsigned long tiempoUltimoCambio = 0;
    bool estadoActual = digitalRead(pin);

    if (estadoActual != estadoAnterior) {
        unsigned long tiempoActual = millis();
        if (tiempoActual - tiempoUltimoCambio > debounceDelay) {
            tiempoUltimoCambio = tiempoActual;
            estadoAnterior = estadoActual;

            if (estadoActual == LOW) {
                return true; // Pulsador presionado
            }
        }
    }
    return false;
}

// Activar alarma
void activarAlarma() {
    digitalWrite(pinLED, HIGH);
    digitalWrite(pinBuzzer, HIGH);
    alarmaActiva = true;
    Serial.println("ALARMA ACTIVADA: Detección de intrusión.");
    publicarEstadosMQTT();
}

// Desactivar alarma
void desactivarAlarma() {
    digitalWrite(pinLED, LOW);
    digitalWrite(pinBuzzer, LOW);
    alarmaActiva = false;
    publicarEstadosMQTT();
}

void setup() {
    Serial.begin(115200);
    pinMode(pinPulsadorActivacion, INPUT_PULLUP);
    pinMode(pinPIR, INPUT);
    pinMode(pinSW420, INPUT);
    pinMode(pinKY002, INPUT);
    pinMode(pinMagnetico, INPUT);
    pinMode(pinLED, OUTPUT);
    pinMode(pinBuzzer, OUTPUT);
    desactivarAlarma();
    connectToWiFi();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    connectToMQTT();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) connectToWiFi();
    if (!client.connected()) connectToMQTT();
    client.loop();

    // Leer pulsador físico
    if (leerPulsador(pinPulsadorActivacion)) {
        sistemaActivo = !sistemaActivo; // Alternar estado del sistema
        Serial.print("Sistema ");
        Serial.println(sistemaActivo ? "ACTIVO" : "DESACTIVADO");
        publicarEstadosMQTT(); // Publicar estado del sistema
        if (!sistemaActivo) desactivarAlarma();
    }

    // Leer estado de los sensores
    presenciaDetectada = digitalRead(pinPIR);
    vibracionSW420Detectada = digitalRead(pinSW420);
    vibracionKY002Detectada = digitalRead(pinKY002);
    magneticoDetectado = digitalRead(pinMagnetico);

    if (sistemaActivo && (presenciaDetectada || vibracionSW420Detectada || vibracionKY002Detectada || magneticoDetectado)) {
        if (!alarmaActiva) activarAlarma();
    } else if (!presenciaDetectada && !vibracionSW420Detectada && !vibracionKY002Detectada && !magneticoDetectado && alarmaActiva) {
        desactivarAlarma();
    }

    // Publicar estados de los sensores periódicamente
    unsigned long tiempoActual = millis();
    if (tiempoActual - tiempoUltimaPublicacionSensores >= intervaloPublicacionSensores) {
        publicarEstadosMQTT();
        tiempoUltimaPublicacionSensores = tiempoActual;
    }
}
