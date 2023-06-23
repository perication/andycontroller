#include "generated/andycontroller-1_menu.h"
#include <Preferences.h>
#include <ADS1X15.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "FRITZ!Powerline 1260E";
const char* password = "81831059739106230277";
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
const char* mqtt_username = "perication";
const char* mqtt_password = "public";
const char* mqttTopic = "voltaje";


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);



Preferences preferences;
//ADS1115 ADS(0x48);

const int RELAY_1_PIN = 26;
const int RELAY_2_PIN = 27;

const int BUZZER_PIN = 17;

// Floats for resistor values in divider (in ohms); to measure up to 15V
//const float R1 = 15000.0;
//const float R2 = 7500.0;

float lowbatlevel = 12.5;
float thresholdRange = 0.15;

bool relayActive = false;
bool RelaymanualOn = false;
bool RelaymanualOff = false;
unsigned long activationStartTime = 0;
int loopCount = 0;

float batteryV = 0.0; // Variable para almacenar el valor del voltaje

unsigned long lastMQTTMessageTime = 0;


/*float readVoltage(int pin) {
    int raw = ADS.readADC(pin);
    float voltage = ADS.toVoltage(raw);
    Serial.println("pin: " + String(pin) + " raw: " + String(raw) + " vol: " + String(voltage));
    voltage = voltage / (R2 / (R1 + R2));
    return voltage;
} */

String createKeyForMenuItem(AnalogMenuItem menuItem) {
    Serial.println("m" + String(menuItem.getId()));
    return ("m" + String(menuItem.getId()));
}

void initDefaultPreferenceFloat(AnalogMenuItem menuItem, float defaultValue) {
    const char *keyString = createKeyForMenuItem(menuItem).c_str();
    if (!preferences.isKey(keyString)) {
        preferences.putFloat(keyString, defaultValue);
    }
    menuItem.setFromFloatingPointValue(preferences.getFloat(keyString));
}

void callbackvoltage(char* mqttTopic, byte* payload, unsigned int length);

void initMenuByPreferences() {
    initDefaultPreferenceFloat(menuThreshold, 13.6);
    initDefaultPreferenceFloat(menuTrim, 0.0);
    initDefaultPreferenceFloat(menuLowbatlevel, 13.0);
    
 
}

void setupWiFi() {
    int retries = 0;
    const int maxRetries = 100;

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        if (retries < maxRetries) {
            delay(1000);
            Serial.println("Conectando a WiFi...");
            retries++;
            Serial.println("Reintentando: " + String(retries));
        } else {
            Serial.println("Error al conectar a WiFi después de " + String(maxRetries) + " intentos.");
            break;
        }
    }
        if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conexión WiFi establecida");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("No se pudo establecer la conexión WiFi. Reiniciando Arduino.");
        ESP.restart();
        }
}

void setupMQTT() {
    mqttClient.setServer(mqttServer, mqttPort);
    
    // Opcionalmente, establecer credenciales de autenticación si se requiere
    // mqttClient.setCredentials(mqtt_username, mqtt_password);
}
/*
void callbackvoltage(char* mqttTopic, byte* payload, unsigned int length) {
    // Convertir el payload recibido en un valor float
    String payloadStr = "";
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    float batteryV = payloadStr.toFloat();
    Serial.println(batteryV);*/



void reconnectMQTT() {
    int retriesmqtt = 0;
    const int maxRetriesmqtt = 100;

    while (!mqttClient.connected()) {
        if (retriesmqtt < maxRetriesmqtt) {
            Serial.println("Conectando al servidor MQTT...");

            String clientId = "denky32-" + String(millis()); // Generar un identificador único

            if (mqttClient.connect(clientId.c_str())) { // Utilizar el identificador único en la conexión
                Serial.println("Conexión MQTT establecida");
                mqttClient.subscribe(mqttTopic); // Suscribirse al topic 
                mqttClient.setCallback(callbackvoltage);
                Serial.println(mqttTopic);
                break;
            } else {
                Serial.print("Error al conectar al servidor MQTT. Estado: ");
                Serial.println(mqttClient.state());
                retriesmqtt++;
                Serial.println("Reintentando: " + String(retriesmqtt));
                delay(5000);
            }
        } else {
            Serial.println("Error al conectar al servidor MQTT después de " + String(maxRetriesmqtt) + " intentos.");
            break;
        }
    }

    if (!mqttClient.connected()) {
        Serial.println("No se pudo establecer la conexión MQTT. Reiniciando Arduino.");
        ESP.restart();
    }
}


void callbackvoltage(char* mqttTopic, byte* payload, unsigned int length) {
    Serial.println("Received MQTT message");
    // Convertir el payload recibido en un valor float
    String payloadStr = "";
    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }
    batteryV = payloadStr.toFloat();
    Serial.println(batteryV);
    // Actualizar el tiempo de la última recepción de mensaje de MQTT
    lastMQTTMessageTime = millis();
    loopCount++;
    Serial.print("Loop num: ");
    Serial.println(loopCount);
    Serial.print((millis() - lastMQTTMessageTime) / 1000 / 60);
    Serial.println(" min since last mqtt message");
    float lowbatlevel = menuLowbatlevel.getAsFloatingPointValue();
    Serial.print(lowbatlevel);
    Serial.println(" Low Battery Level");
    menuloopcount.setFromFloatingPointValue(loopCount);
    
}




    // Realizar la lógica del relé en base al valor del voltaje recibido

       // float batteryV = readVoltage(0);

void updateRelayStatus() {
    float trim = menuTrim.getAsFloatingPointValue();
    float trimmedVoltage = batteryV + trim;
    menubatteryV.setFromFloatingPointValue(trimmedVoltage);

    float thresholdV = menuThreshold.getAsFloatingPointValue();
    float lowbatlevel = menuLowbatlevel.getAsFloatingPointValue();

    if (relayActive) {
        if (batteryV < lowbatlevel) {
            relayActive = false;
            digitalWrite(RELAY_1_PIN, HIGH);
            digitalWrite(RELAY_2_PIN, HIGH);
        }
    } else {
        if (trimmedVoltage > thresholdV - thresholdRange && trimmedVoltage < thresholdV + thresholdRange) {
            // Ejecutar el loop N veces antes de activar el relé
            if (loopCount >= 300) {
                relayActive = true;
                digitalWrite(RELAY_1_PIN, LOW);
                digitalWrite(RELAY_2_PIN, LOW);
            } else {
                // Si el relé ya está activo, incrementar el contador de loops
                if (relayActive) {
                    loopCount++;
                }
            }
        } else {
            loopCount = 0; // Reiniciar el contador de loops si el voltaje no está en el rango
        }
        
        // Verificar si el valor de lowbatlevel es mayor que batteryV y apagar el relé
        if (lowbatlevel > batteryV) {
            relayActive = false;
            digitalWrite(RELAY_1_PIN, HIGH);
            digitalWrite(RELAY_2_PIN, HIGH);
        }
    }
}



void setup() {
    
    
  Serial.begin(115200);

    setupWiFi();
    setupMQTT();

    
  
 /* WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  } 
    

    Serial.println("Connected to WiFi");
    mqttClient.setServer(mqttServer, mqttPort);

  */

  //  ADS.begin();
  //  Serial.println("ADS busy?: " + String(ADS.isBusy()) + " connected: " + String(ADS.isConnected()) + " ready: " + String(ADS.isReady()));

    setupMenu();
    renderer.turnOffResetLogic();

    preferences.begin("PereController", false);
    initMenuByPreferences();

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RELAY_1_PIN, OUTPUT);
    pinMode(RELAY_2_PIN, OUTPUT);

    digitalWrite(RELAY_1_PIN, HIGH);
    digitalWrite(RELAY_2_PIN, HIGH);

   taskManager.scheduleFixedRate(1000, [] {
      //  int raw = ADS.readADC(1);
      //  float internalV = ADS.toVoltage(raw);
      //  menuInternal.setFromFloatingPointValue(internalV);

    /*    float batteryV;
        float trim = menuTrim.getAsFloatingPointValue();
        float trimmedVoltage = batteryV + trim;
        menubatteryV.setFromFloatingPointValue(trimmedVoltage);

        float thresholdV = menuThreshold.getAsFloatingPointValue();

        if (relayActive) {
            if (batteryV < lowbatlevel) {
                relayActive = false;
                digitalWrite(RELAY_1_PIN, HIGH);
                digitalWrite(RELAY_2_PIN, HIGH);
                activationStartTime = 0;
            }
        } else {
            if (trimmedVoltage > thresholdV - thresholdRange && trimmedVoltage < thresholdV + thresholdRange) {
                if (activationStartTime == 0) {
                    activationStartTime = millis();
                } else if (millis() - activationStartTime >= 60000) {
                    relayActive = true;
                    digitalWrite(RELAY_1_PIN, LOW);
                    digitalWrite(RELAY_2_PIN, LOW);
                }
            } else {
                activationStartTime = 0;
            }
        } 
*/
    },  TIME_MILLIS); 
}

void loop() {
       if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
   
    taskManager.runLoop();

    bool RelaymanualOff = menuRelaymanualOff.getBoolean();

    bool RelaymanualOn = menuRelaymanualOn.getBoolean();

    if(RelaymanualOff == false && RelaymanualOn==false){
    updateRelayStatus();    
    } 
    if (RelaymanualOff == true){
        relayActive = false;
            digitalWrite(RELAY_1_PIN, HIGH);
            digitalWrite(RELAY_2_PIN, HIGH);
    }
     if (RelaymanualOn == true){
        relayActive = true;
            digitalWrite(RELAY_1_PIN, LOW);
            digitalWrite(RELAY_2_PIN, LOW);

    }
    
    // Verificar si ha pasado más de 5 minutos desde la última recepción de un mensaje de MQTT
    if (millis() - lastMQTTMessageTime >= 5 * 60 * 1000) {
        loopCount = 0;
    }
}



void updatePreferenceByMenuItemFloat(AnalogMenuItem menuItem) {
    preferences.putFloat(createKeyForMenuItem(menuItem).c_str(), menuItem.getAsFloatingPointValue());
}

void CALLBACK_FUNCTION onVoltageThresholdChange(int id) {
    updatePreferenceByMenuItemFloat(menuThreshold);
}

void CALLBACK_FUNCTION onTrimChanged(int id) {
    updatePreferenceByMenuItemFloat(menuTrim);
}


void CALLBACK_FUNCTION onlowbatlevelchanged(int id) {
    updatePreferenceByMenuItemFloat(menuLowbatlevel);
}

