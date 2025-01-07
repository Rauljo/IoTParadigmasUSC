#include <WiFi.h>
#include <PubSubClient.h>

// Configuración conexión WiFi
const char* ssid = "MIWIFI_mPNT 2.4";
const char* password = "hxQetGpu";

// Configuración conexión MQTT
const char* mqttServer = "192.168.1.131";
const int mqttPort = 1883;
const char* mqttTopicInput = "sistema/control";
const char* mqttTopicEstado = "sistema/estado";
const char* mqttTopicPIR = "sensor/pir";
const char* mqttTopicSW420 = "sensor/vibracion_sw420";
const char* mqttTopicKY002 = "sensor/vibracion_ky002";
const char* mqttTopicMagnetico = "sensor/magnetico";
const char* mqttTopicLED = "actuador/led";
const char* mqttTopicBuzzer = "actuador/buzzer";
const char* mqttTopicLEDControl = "control/led";
const char* mqttTopicBuzzerControl = "control/buzzer";

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
const int pinPulsadorLED = 38;
const int pinPulsadorBuzzer = 37;

// Variables para el estado del sistema
bool sistemaActivo = false;
bool presenciaDetectada = false;
bool vibracionSW420Detectada = false;
bool vibracionKY002Detectada = false;
bool magneticoDetectado = false;
bool alarmaActiva = false;
bool ledEncendido = false;
bool buzzerEncendido = false;

// Variables para debounce
unsigned long tiempoUltimoCambioLED = 0;
unsigned long tiempoUltimoCambioBuzzer = 0;
unsigned long tiempoUltimoCambioSistema = 0;
const unsigned long debounceDelay = 50;

// Variables de estado anteriores (para cada pulsador)
bool estadoAnteriorSistema = HIGH;
bool estadoAnteriorLED = HIGH;
bool estadoAnteriorBuzzer = HIGH;

// Variables para publicación periódica
unsigned long tiempoUltimaPublicacionSensores = 0;
const unsigned long intervaloPublicacionSensores = 60000; // 1 minuto

// Función para conectar a WiFi
void connectToWiFi() {
    Serial.print("Conectando a WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
}

// Función para conectar al broker MQTT
void connectToMQTT() {
    while (!client.connected()) {
        Serial.print("Conectando al broker MQTT...");
        String clientId = "ESP32-" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("Conectado al broker MQTT!");
            client.subscribe(mqttTopicInput);
            client.subscribe(mqttTopicLEDControl);
            client.subscribe(mqttTopicBuzzerControl);
        } else {
            Serial.print("Error al conectar al broker: ");
            Serial.println(client.state());
            delay(2000);
        }
    }
}

// Función para publicar estados en MQTT
void publicarEstadosMQTT() {
    client.publish(mqttTopicEstado, sistemaActivo ? "ACTIVO" : "DESACTIVADO");
    client.publish(mqttTopicPIR, presenciaDetectada ? "DETECTADO" : "SIN_MOVIMIENTO");
    client.publish(mqttTopicSW420, vibracionSW420Detectada ? "VIBRACION" : "SIN_VIBRACION");
    client.publish(mqttTopicKY002, vibracionKY002Detectada ? "VIBRACION" : "SIN_VIBRACION");
    client.publish(mqttTopicMagnetico, magneticoDetectado ? "ABIERTO" : "CERRADO");
    client.publish(mqttTopicLED, ledEncendido ? "ENCENDIDO" : "APAGADO");
    client.publish(mqttTopicBuzzer, buzzerEncendido ? "ENCENDIDO" : "APAGADO");
    Serial.println("Estados publicados en MQTT.");
}

// Leer pulsador con debounce optimizado
bool leerPulsador(int pin, unsigned long& tiempoUltimoCambio, bool& estadoAnterior) {
    bool estadoActual = digitalRead(pin);

    if (estadoActual != estadoAnterior) {
        unsigned long tiempoActual = millis();
        if (tiempoActual - tiempoUltimoCambio > debounceDelay) {
            tiempoUltimoCambio = tiempoActual;
            estadoAnterior = estadoActual;
            if (estadoActual == LOW) return true;
        }
    }
    return false;
}

// Función para activar alarma
void activarAlarma() {
    digitalWrite(pinLED, HIGH);
    digitalWrite(pinBuzzer, HIGH);
    alarmaActiva = true;
    publicarEstadosMQTT();
    Serial.println("ALARMA ACTIVADA.");
}

// Función para desactivar alarma
void desactivarAlarma() {
    digitalWrite(pinLED, LOW);
    digitalWrite(pinBuzzer, LOW);
    alarmaActiva = false;
    publicarEstadosMQTT();
    Serial.println("ALARMA DESACTIVADA.");
}

// Función para manejar los mensajes MQTT
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    if (String(topic) == mqttTopicInput) {
        if (message == "ACTIVAR") {
            sistemaActivo = true;
        } else if (message == "DESACTIVAR") {
            sistemaActivo = false;
            desactivarAlarma();
        }
        publicarEstadosMQTT();
    } else if (String(topic) == mqttTopicLEDControl) {
        if (message == "ENCENDER") {
            ledEncendido = true;
            digitalWrite(pinLED, HIGH);
            client.publish(mqttTopicLED, "ENCENDIDO");
            Serial.println("LED: ENCENDIDO (control MQTT)");
        } else if (message == "APAGAR") {
            ledEncendido = false;
            digitalWrite(pinLED, LOW);
            client.publish(mqttTopicLED, "APAGADO");
            Serial.println("LED: APAGADO (control MQTT)");
        }
    } else if (String(topic) == mqttTopicBuzzerControl) {
        if (message == "ENCENDER") {
            buzzerEncendido = true;
            digitalWrite(pinBuzzer, HIGH);
            client.publish(mqttTopicBuzzer, "ENCENDIDO");
            Serial.println("Buzzer: ENCENDIDO (control MQTT)");
        } else if (message == "APAGAR") {
            buzzerEncendido = false;
            digitalWrite(pinBuzzer, LOW);
            client.publish(mqttTopicBuzzer, "APAGADO");
            Serial.println("Buzzer: APAGADO (control MQTT)");
        }
    }
}

// Función para controlar actuadores manualmente
void controlarActuadoresManual() {
    if (leerPulsador(pinPulsadorLED, tiempoUltimoCambioLED, estadoAnteriorLED)) {
        ledEncendido = !ledEncendido;
        digitalWrite(pinLED, ledEncendido ? HIGH : LOW);
        client.publish(mqttTopicLED, ledEncendido ? "ENCENDIDO" : "APAGADO");
        Serial.print("LED: ");
        Serial.println(ledEncendido ? "ENCENDIDO (control manual)" : "APAGADO (control manual)");
    }
    if (leerPulsador(pinPulsadorBuzzer, tiempoUltimoCambioBuzzer, estadoAnteriorBuzzer)) {
        buzzerEncendido = !buzzerEncendido;
        digitalWrite(pinBuzzer, buzzerEncendido ? HIGH : LOW);
        client.publish(mqttTopicBuzzer, buzzerEncendido ? "ENCENDIDO" : "APAGADO");
        Serial.print("Buzzer: ");
        Serial.println(buzzerEncendido ? "ENCENDIDO (control manual)" : "APAGADO (control manual)");
    }
}

// Configuración inicial
void setup() {
    Serial.begin(115200);
    pinMode(pinPulsadorActivacion, INPUT_PULLUP);
    pinMode(pinPIR, INPUT);
    pinMode(pinSW420, INPUT);
    pinMode(pinKY002, INPUT);
    pinMode(pinMagnetico, INPUT);
    pinMode(pinLED, OUTPUT);
    pinMode(pinBuzzer, OUTPUT);
    pinMode(pinPulsadorLED, INPUT_PULLUP);
    pinMode(pinPulsadorBuzzer, INPUT_PULLUP);
    desactivarAlarma();
    connectToWiFi();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    connectToMQTT();
}

// Bucle principal
void loop() {
    if (WiFi.status() != WL_CONNECTED) connectToWiFi();
    if (!client.connected()) connectToMQTT();
    client.loop();
    controlarActuadoresManual();

    if (leerPulsador(pinPulsadorActivacion, tiempoUltimoCambioSistema, estadoAnteriorSistema)) {
        sistemaActivo = !sistemaActivo;
        publicarEstadosMQTT();
        Serial.println(sistemaActivo ? "SISTEMA ACTIVADO" : "SISTEMA DESACTIVADO");
        if (!sistemaActivo) desactivarAlarma();
    }

    presenciaDetectada = digitalRead(pinPIR);
    vibracionSW420Detectada = digitalRead(pinSW420);
    vibracionKY002Detectada = digitalRead(pinKY002);
    magneticoDetectado = digitalRead(pinMagnetico);

    if (sistemaActivo && (presenciaDetectada || vibracionSW420Detectada || vibracionKY002Detectada || magneticoDetectado)) {
        if (!alarmaActiva) activarAlarma();
    } else if (!presenciaDetectada && !vibracionSW420Detectada && !vibracionKY002Detectada && !magneticoDetectado && alarmaActiva) {
        desactivarAlarma();
    }

    if (millis() - tiempoUltimaPublicacionSensores >= intervaloPublicacionSensores) {
        publicarEstadosMQTT();
        tiempoUltimaPublicacionSensores = millis();
    }
}

