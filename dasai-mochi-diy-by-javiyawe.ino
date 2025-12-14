#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <math.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h> 
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// ==========================================================================
// 1. CONFIGURACIÓN
// ==========================================================================
const char* BLE_NAME      = "MOCHI_YAWE_V43"; 
const char* BLE_PASSWORD  = "1234"; 

const char* DEFAULT_SSID     = "";      
const char* DEFAULT_PASSWORD = ""; 

long  gmtOffset_sec = 3600; 
int   daylightOffset_sec = 3600; 
const bool DEBUG_MODE = false; 

// ==========================================================================
// 2. SISTEMA
// ==========================================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
const char* ntpServer = "pool.ntp.org";
Preferences preferences; 

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

const int EYE_W = 34; const int EYE_H = 44; const int EYE_R = 8;      
const int EYE_Y = 40; const int BASE_X_L = 32; const int BASE_X_R = 96;

// ==========================================================================
// 3. DEFINICIONES
// ==========================================================================
enum PupilType { 
  P_NORMAL, P_CAT, P_HEART, P_X, P_STAR, P_SWIRL, P_LINE, P_DOT, P_FIRE, 
  P_DOLLAR, P_GLITCH, P_ARROW_UP, P_ARROW_DOWN, P_GEAR, P_LOCKED, P_SCAN, P_BAR, P_SICK 
};

enum ExtraIcon { 
  E_NONE, E_TEAR, E_STREAM, E_SWEAT, E_ZZZ, E_ANGER, E_QUESTION, 
  E_SUN, E_RAIN, E_STORM, E_SNOW, E_MAIL, E_CHAT, E_ALERT, E_WIND, E_LOCK, E_WIFI, E_SAVE, E_CLOUD, E_HEARTBEAT 
};

enum Mood {
  BLOQUEADO, GUARDANDO, CONECTANDO, CONECTADO_OK, SIN_WIFI, CARGANDO, TEXTO_BLE, LINTERNA, CONFIGURANDO, INFO_SCREEN, BUSCANDO_DATOS,
  MODO_MATRIX, MODO_DISCO, POMODORO_RUN, MODO_CRYPTO, MODO_GAMER, MODO_ESCANER, MODO_BARRA, MODO_LATIDO, MODO_PINGPONG,
  NEUTRAL, FELIZ, EUFORICO, TRISTE, LLORANDO,
  ENFADADO, FURIA, SORPRENDIDO, ASUSTADO, CANSADO, SLEEPY,
  DORMIDO, ENAMORADO, HIPNOTIZADO, CONFUNDIDO, SOSPECHOSO, ESCEPTICO,
  GUINO, MALVADO, MUERTO, GLITCH, DINERO,
  COQUETO, TIERNO, LOCO, ENFERMO, ASCO,
  SOLEADO, LLUVIA, TORMENTA, NIEVE, VENTOSO, NIEBLA, NOTIFICACION,
  MOOD_COUNT
};

struct EyeState {
  int8_t lidTop, lidBot; bool hasBrows; int8_t browAng, browY;    
  uint8_t pupilSize; PupilType pupilType; ExtraIcon extra;
  uint8_t shake; bool asymmetric;          
};

// ==========================================================================
// 4. VARIABLES GLOBALES
// ==========================================================================
EyeState target;     
float curTop=0, curBot=0, curBrowA=0, curBrowY=0;
float curPSize=10, curGazeX=0, curGazeY=0;
float breathVal = 0; 

uint32_t lastMoodTime = 0, moodDur = 3000;
uint32_t blinkNext = 1000, blinkStart = 0;
bool blinking = false;
uint8_t currentMood = NEUTRAL;

// Estado
bool wifiConnected = false;
bool bleConnected = false;
bool oldBleConnected = false;
bool isLocked = true; // Seguridad activada al inicio

int currentHour = -1; int currentWDay = -1; int currentMin = -1;
uint32_t lastTimeSync = 0;
uint32_t lastDataFetch = 0; 

String ssid = ""; String password = "";
String realCity = "---";
float realTemp = 0.0;
int realWeatherCode = -1; 

// Control
bool manualOverride = false; uint32_t overrideTimer = 0;
bool manualGaze = false; float manualGX = 0; float manualGY = 0;

// Secuencias
int sequenceStep = 0; uint32_t sequenceTimer = 0; int activeSequence = 0; 

// BLE
BLEServer *pServer = NULL;
String inputBuffer = ""; String bleMessage = ""; 

// Apps
uint32_t pomodoroStart = 0; bool pomodoroActive = false;
int matrixCols[128]; 

float mix(float a, float b, float f) { return a + (b - a) * f; }

// ==========================================================================
// 5. CONFIGURACIÓN DE EXPRESIONES (CATÁLOGO V43)
// ==========================================================================
EyeState getMoodConfig(uint8_t m) {
  EyeState s;
  s.lidTop=0; s.lidBot=0; s.hasBrows=true; s.browAng=0; s.browY=-32;
  s.pupilSize=10; s.pupilType=P_NORMAL; s.extra=E_NONE; s.shake=0; s.asymmetric=false;

  switch(m) {
    case INFO_SCREEN:  s.pupilSize=0; s.hasBrows=false; break;
    case BUSCANDO_DATOS: s.lidTop=10; s.lidBot=10; s.pupilType=P_SCAN; break;
    case BLOQUEADO:    s.lidTop=10; s.lidBot=10; s.pupilType=P_LOCKED; s.extra=E_LOCK; break;
    case GUARDANDO:    s.lidTop=5; s.lidBot=5; s.extra=E_SAVE; s.pupilType=P_DOT; break;
    case CONFIGURANDO: s.lidTop=10; s.lidBot=10; s.pupilType=P_GEAR; s.extra=E_WIFI; break;
    case CARGANDO:     s.lidTop=15; s.lidBot=15; s.hasBrows=false; s.pupilSize=0; break;
    case CONECTADO_OK: s.lidTop=0; s.lidBot=20; s.browY=-34; s.pupilSize=12; break;
    case SIN_WIFI:     s.lidTop=15; s.lidBot=0; s.browAng=-5; s.extra=E_TEAR; break;
    case LINTERNA:     s.pupilSize=0; s.hasBrows=false; break;
    case TEXTO_BLE:    s.pupilSize=0; s.hasBrows=false; break;
    
    // MODOS APP
    case MODO_MATRIX:  s.hasBrows=false; s.pupilSize=0; break;
    case MODO_DISCO:   s.lidTop=-5; s.lidBot=-5; s.pupilType=P_STAR; s.shake=4; break;
    case MODO_GAMER:   s.lidTop=10; s.lidBot=10; s.pupilType=P_GLITCH; s.shake=1; break;
    case MODO_CRYPTO:  s.lidTop=5; s.lidBot=5; s.pupilType=P_ARROW_UP; s.shake=1; break;
    case MODO_ESCANER: s.lidTop=15; s.lidBot=15; s.pupilType=P_SCAN; break;
    case MODO_BARRA:   s.lidTop=10; s.lidBot=10; s.pupilType=P_BAR; break;
    case MODO_LATIDO:  s.lidTop=5; s.lidBot=5; s.pupilType=P_HEART; s.extra=E_HEARTBEAT; break;
    case MODO_PINGPONG:s.lidTop=2; s.lidBot=2; break; 
    case POMODORO_RUN: s.lidTop=15; s.lidBot=0; s.hasBrows=true; s.browAng=5; s.pupilType=P_DOT; break;
    
    case NOTIFICACION: s.lidTop=-5; s.lidBot=-5; s.browY=-38; s.pupilType=P_DOT; break; 
    
    // CLIMA
    case SOLEADO:   s.lidTop=0; s.lidBot=15; s.browY=-36; s.extra=E_SUN; break;
    case LLUVIA:    s.lidTop=10; s.lidBot=5; s.browAng=-4; s.extra=E_RAIN; break;
    case TORMENTA:  s.lidTop=12; s.lidBot=8; s.browAng=8; s.extra=E_STORM; s.shake=1; break;
    case NIEVE:     s.lidTop=5; s.lidBot=5; s.browAng=0; s.extra=E_SNOW; break;
    case VENTOSO:   s.lidTop=10; s.lidBot=10; s.shake=1; s.extra=E_WIND; break;
    case NIEBLA:    s.lidTop=25; s.lidBot=25; s.pupilSize=6; s.extra=E_CLOUD; break;

    // EMOCIONES
    case NEUTRAL: s.lidTop=2; s.lidBot=2; s.hasBrows=false; break;
    case FELIZ: s.lidTop=0; s.lidBot=22; s.browY=-34; s.pupilSize=11; break;
    case EUFORICO: s.lidTop=-4; s.lidBot=-4; s.hasBrows=false; s.pupilType=P_STAR; s.pupilSize=13; s.shake=1; break;
    case TRISTE: s.lidTop=15; s.lidBot=0; s.browAng=-5; s.browY=-26; s.extra=E_TEAR; s.pupilSize=9; break;
    case LLORANDO: s.lidTop=22; s.lidBot=5; s.browAng=-7; s.browY=-24; s.extra=E_STREAM; s.shake=1; s.pupilSize=8; break;
    case ENFADADO: s.lidTop=12; s.lidBot=8; s.browAng=5; s.browY=-22; s.extra=E_ANGER; s.pupilSize=6; break;
    case FURIA: s.lidTop=18; s.lidBot=15; s.browAng=6; s.browY=-20; s.pupilType=P_FIRE; s.pupilSize=8; s.shake=3; break;
    case SORPRENDIDO: s.lidTop=-6; s.lidBot=-6; s.browY=-38; s.pupilType=P_DOT; s.pupilSize=4; break;
    case ASUSTADO: s.lidTop=-2; s.lidBot=-2; s.browAng=-5; s.browY=-36; s.shake=3; s.extra=E_SWEAT; s.pupilSize=5; break;
    case CANSADO: s.lidTop=28; s.lidBot=0; s.browAng=3; s.browY=-28; s.pupilSize=8; break;
    case SLEEPY:  s.asymmetric=true; s.lidTop=25; s.lidBot=0; s.pupilSize=7; break;
    case DORMIDO: s.lidTop=40; s.lidBot=10; s.hasBrows=false; s.extra=E_ZZZ; s.pupilSize=0; break;
    case ENAMORADO: s.lidTop=0; s.lidBot=10; s.browAng=-3; s.browY=-34; s.pupilType=P_HEART; s.pupilSize=13; break;
    case HIPNOTIZADO: s.lidTop=10; s.lidBot=5; s.browAng=0; s.pupilType=P_SWIRL; s.pupilSize=12; break;
    case CONFUNDIDO: s.asymmetric=true; s.browAng=-5; s.browY=-30; s.extra=E_QUESTION; break;
    case SOSPECHOSO: s.lidTop=30; s.lidBot=25; s.browAng=2; s.pupilType=P_LINE; s.pupilSize=7; break;
    case ESCEPTICO: s.asymmetric=true; s.browAng=5; s.browY=-35; break;
    case GUINO: s.asymmetric=true; s.lidBot=5; s.browY=-30; s.pupilSize=11; break;
    case MALVADO: s.lidTop=15; s.lidBot=8; s.browAng=5; s.browY=-22; s.pupilType=P_CAT; s.pupilSize=9; break;
    case MUERTO: s.lidTop=20; s.lidBot=15; s.hasBrows=false; s.pupilType=P_X; s.pupilSize=11; break;
    case GLITCH: s.lidTop=5; s.lidBot=5; s.hasBrows=false; s.pupilType=P_GLITCH; s.pupilSize=12; s.shake=2; break;
    case DINERO: s.lidTop=0; s.lidBot=15; s.browAng=-4; s.browY=-34; s.pupilType=P_DOLLAR; s.pupilSize=12; break;
    case COQUETO: s.lidTop=5; s.lidBot=5; s.browAng=-2; s.pupilSize=12; s.pupilType=P_NORMAL; break;
    case TIERNO: s.lidTop=0; s.lidBot=20; s.pupilSize=14; s.browY=-38; break;
    case LOCO: s.asymmetric=true; s.lidTop=0; s.lidBot=0; s.pupilType=P_SWIRL; s.shake=2; break;
    case ENFERMO: s.lidTop=20; s.lidBot=10; s.pupilType=P_SICK; s.shake=1; break;
    case ASCO: s.lidTop=15; s.lidBot=15; s.browAng=5; s.pupilSize=4; break;
  }
  return s;
}

const char* getMoodNameES(uint8_t m) { return "MOCHI V43"; }

// ==========================================================================
// 6. GESTIÓN RED Y DATOS REALES
// ==========================================================================
void showFeedback(Mood m, int duration, String msg) {
  currentMood = m; target = getMoodConfig(m);
  manualOverride = true; overrideTimer = millis() + duration; bleMessage = msg; 
}

void fetchRealData() {
  if (WiFi.status() != WL_CONNECTED) return;
  HTTPClient http;
  
  http.begin("http://ip-api.com/json/?fields=status,city,lat,lon"); 
  int httpCode = http.GET();
  float lat = 0, lon = 0;
  
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      const char* city = doc["city"];
      realCity = String(city);
      lat = doc["lat"];
      lon = doc["lon"];
      if (realCity.length() > 9) realCity = realCity.substring(0, 9);
    }
  }
  http.end();

  if (lat == 0 && lon == 0) return; 

  String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(lat) + "&longitude=" + String(lon) + "&current_weather=true";
  http.begin(url);
  httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      realTemp = doc["current_weather"]["temperature"];
      realWeatherCode = doc["current_weather"]["weathercode"];
    }
  }
  http.end();
}

void connectWifi() {
  wifiConnected = false;
  if (ssid == "" || ssid == "NULL") { showFeedback(SIN_WIFI, 3000, "SIN DATOS"); return; }

  currentMood = CONECTANDO; target = getMoodConfig(CONECTANDO);
  u8g2.clearBuffer(); u8g2.setFont(u8g2_font_5x7_tr); u8g2.setDrawColor(1); u8g2.drawStr(35, 62, "CONECTANDO..."); u8g2.sendBuffer();

  WiFi.begin(ssid.c_str(), password.c_str());
  int retry = 0; while (WiFi.status() != WL_CONNECTED && retry < 20) { delay(500); retry++; }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true; 
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org"); 
    
    int w = 0; while(time(nullptr) < 100000 && w < 10) { delay(500); w++; }
    struct tm timeinfo;
    if(getLocalTime(&timeinfo)){ currentHour=timeinfo.tm_hour; currentMin=timeinfo.tm_min; }

    currentMood = BUSCANDO_DATOS; target = getMoodConfig(BUSCANDO_DATOS);
    u8g2.clearBuffer(); drawEye(BASE_X_L, EYE_Y, true); drawEye(BASE_X_R, EYE_Y, false);
    u8g2.drawStr(35, 62, "DATOS..."); u8g2.sendBuffer();
    
    fetchRealData(); 
    showFeedback(INFO_SCREEN, 6000, ""); 
  } else {
    showFeedback(SIN_WIFI, 3000, "ERROR WIFI");
  }
}

// ==========================================================================
// 7. GESTIÓN BLUETOOTH
// ==========================================================================
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) { 
      bleConnected = true; 
      isLocked = true; 
      // AVISAR PERO NO BLOQUEAR LA ANIMACIÓN
      // (Se usa una variable temporal para no romper el loop visual)
      // En este caso, solo mostramos el icono si no estamos en manual
      if(!manualOverride) showFeedback(BLOQUEADO, 1500, ""); 
    };
    void onDisconnect(BLEServer* pServer) { 
      bleConnected = false; isLocked = true; 
      manualOverride = false; manualGaze = false;
      currentMood = NEUTRAL; 
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue(); 
      if (rxValue.length() > 0) { inputBuffer = rxValue; inputBuffer.trim(); }
    }
};

void processBluetoothCommand() {
  if(inputBuffer == "") return;
  String cmd = inputBuffer; String lowerCmd = cmd; lowerCmd.toLowerCase(); 
  uint32_t duration = 5000; bool isAction = true;

  if (isLocked) {
    if (cmd.equalsIgnoreCase(String("PASS:") + String(BLE_PASSWORD))) {
      isLocked = false; showFeedback(FELIZ, 2000, "HOLA!");
    } else {
       // SI NO ES LA PASS, SOLO IGNORAR O DAR FEEDBACK DE BLOQUEO BREVE
       showFeedback(BLOQUEADO, 1000, "BLOQUEADO");
    }
    inputBuffer = ""; return;
  }

  // ZONA Y WIFI
  if (lowerCmd.startsWith("zona:")) {
    int zona = lowerCmd.substring(5).toInt();
    gmtOffset_sec = zona * 3600;
    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org");
    showFeedback(CONFIGURANDO, 2000, "ZONA OK"); inputBuffer = ""; return;
  }

  if (lowerCmd.startsWith("wifi:")) {
    int commaIndex = cmd.indexOf(',');
    if (commaIndex > 0) {
      ssid = cmd.substring(5, commaIndex); password = cmd.substring(commaIndex + 1); ssid.trim(); password.trim();
      preferences.putString("ssid", ssid); preferences.putString("pass", password);
      showFeedback(GUARDANDO, 2000, ""); 
      inputBuffer = "internal_connect"; overrideTimer = millis() + 1500; return; 
    }
  }
  
  if (inputBuffer == "internal_connect" && millis() > (overrideTimer - 500)) { connectWifi(); inputBuffer = ""; return; }

  // COMANDOS
  if (lowerCmd == "auto") { manualGaze = false; manualOverride = false; pomodoroActive=false; activeSequence=0; currentMood = NEUTRAL; isAction=false;}
  else if (lowerCmd.startsWith("di ")) { bleMessage = cmd.substring(3); bleMessage.toUpperCase(); currentMood = TEXTO_BLE; duration = 6000; }
  
  else if (lowerCmd == "info") { showFeedback(INFO_SCREEN, 6000, ""); isAction=false; }
  else if (lowerCmd == "pomodoro") { currentMood = POMODORO_RUN; pomodoroActive = true; pomodoroStart = millis(); manualOverride=true; isAction=false; }
  else if (lowerCmd == "stop") { pomodoroActive = false; manualOverride=false; activeSequence=0; }
  else if (lowerCmd == "luz") { currentMood = LINTERNA; manualOverride = true; overrideTimer = millis() + 60000; isAction=false; }
  
  // SECUENCIAS
  else if (lowerCmd == "susto") { activeSequence=1; sequenceStep=0; sequenceTimer=0; isAction=false; manualOverride=true; overrideTimer=millis()+10000; }
  else if (lowerCmd == "broma") { activeSequence=2; sequenceStep=0; sequenceTimer=0; isAction=false; manualOverride=true; overrideTimer=millis()+10000; }
  else if (lowerCmd == "detective") { activeSequence=3; sequenceStep=0; sequenceTimer=0; isAction=false; manualOverride=true; overrideTimer=millis()+10000; }
  else if (lowerCmd == "llorar") { activeSequence=4; sequenceStep=0; sequenceTimer=0; isAction=false; manualOverride=true; overrideTimer=millis()+10000; }
  else if (lowerCmd == "estornudo") { activeSequence=5; sequenceStep=0; sequenceTimer=0; isAction=false; manualOverride=true; overrideTimer=millis()+10000; }
  else if (lowerCmd == "risa") { activeSequence=6; sequenceStep=0; sequenceTimer=0; isAction=false; manualOverride=true; overrideTimer=millis()+10000; }

  // EMOCIONES
  else if (lowerCmd == "feliz") currentMood = FELIZ;
  else if (lowerCmd == "triste") currentMood = TRISTE;
  else if (lowerCmd == "enfadado") currentMood = ENFADADO;
  else if (lowerCmd == "amor") currentMood = ENAMORADO;
  else if (lowerCmd == "sorpresa") currentMood = SORPRENDIDO;
  else if (lowerCmd == "neutro") currentMood = NEUTRAL;
  else if (lowerCmd == "coqueto") currentMood = COQUETO;
  else if (lowerCmd == "tierno") currentMood = TIERNO;
  else if (lowerCmd == "loco") currentMood = LOCO;
  else if (lowerCmd == "enfermo") currentMood = ENFERMO;
  else if (lowerCmd == "asco") currentMood = ASCO;
  else if (lowerCmd == "dinero") currentMood = DINERO;
  else if (lowerCmd == "dormir") { currentMood = DORMIDO; duration=30000; }
  else if (lowerCmd == "cansado") currentMood = CANSADO; 
  else if (lowerCmd == "muerto") currentMood = MUERTO;
  else if (lowerCmd == "esceptico") currentMood = ESCEPTICO;
  else if (lowerCmd == "sospecha") currentMood = SOSPECHOSO;
  else if (lowerCmd == "glitch") currentMood = GLITCH;
  
  // MODOS
  else if (lowerCmd == "fiesta" || lowerCmd == "disco") { currentMood = MODO_DISCO; duration=8000; }
  else if (lowerCmd == "matrix") { currentMood = MODO_MATRIX; duration=8000; }
  else if (lowerCmd == "crypto") { currentMood = MODO_CRYPTO; duration=8000; }
  else if (lowerCmd == "gamer") { currentMood = MODO_GAMER; duration=8000; }
  else if (lowerCmd == "hacker") { currentMood = MODO_MATRIX; duration=8000; }
  else if (lowerCmd == "escaner") { currentMood = MODO_ESCANER; duration=8000; }
  else if (lowerCmd == "carga") { currentMood = MODO_BARRA; duration=8000; }
  else if (lowerCmd == "latido") { currentMood = MODO_LATIDO; duration=8000; }
  else if (lowerCmd == "pingpong") { currentMood = MODO_PINGPONG; duration=10000; }
  else if (lowerCmd == "clima") { fetchRealData(); isAction=false; }

  // JOYSTICK
  else if (lowerCmd == "arriba") { manualGaze = true; manualGX=0; manualGY=-8; isAction=false; }
  else if (lowerCmd == "abajo") { manualGaze = true; manualGX=0; manualGY=8; isAction=false; }
  else if (lowerCmd == "izquierda") { manualGaze = true; manualGX=-10; manualGY=0; isAction=false; }
  else if (lowerCmd == "derecha") { manualGaze = true; manualGX=10; manualGY=0; isAction=false; }
  else if (lowerCmd == "centro") { manualGaze = true; manualGX=0; manualGY=0; isAction=false; }

  // NOTIFICACIONES
  else if (lowerCmd == "whatsapp") { currentMood = NOTIFICACION; target.extra = E_CHAT; duration=4000; }
  else if (lowerCmd == "email") { currentMood = NOTIFICACION; target.extra = E_MAIL; duration=4000; }
  else if (lowerCmd == "alerta") { currentMood = NOTIFICACION; target.extra = E_ALERT; duration=4000; }

  if (isAction && !pomodoroActive && !isLocked) {
    target = getMoodConfig(currentMood);
    manualOverride = true; overrideTimer = millis() + duration;
  } else if (!isAction && !pomodoroActive && activeSequence==0) {
    target = getMoodConfig(currentMood);
  }
  inputBuffer = "";
}

// ==========================================================================
// 8. CEREBRO Y BIORRITMO (CORREGIDO V43: IA NON-STOP)
// ==========================================================================
void runSequences() {
  if (activeSequence == 0) return;
  uint32_t now = millis();
  
  if (activeSequence == 1) { // SUSTO
    if(sequenceStep==0) { currentMood=ASUSTADO; sequenceTimer=now+2000; sequenceStep++; }
    else if(sequenceStep==1 && now>sequenceTimer) { currentMood=ASUSTADO; target.shake=4; sequenceTimer=now+2000; sequenceStep++; }
    else if(sequenceStep==2 && now>sequenceTimer) { currentMood=ENFADADO; sequenceTimer=now+3000; sequenceStep++; }
    else if(sequenceStep==3 && now>sequenceTimer) { activeSequence=0; manualOverride=false; }
  }
  else if (activeSequence == 2) { // BROMA
    if(sequenceStep==0) { currentMood=EUFORICO; sequenceTimer=now+2000; sequenceStep++; }
    else if(sequenceStep==1 && now>sequenceTimer) { currentMood=GUINO; sequenceTimer=now+3000; sequenceStep++; }
    else if(sequenceStep==2 && now>sequenceTimer) { activeSequence=0; manualOverride=false; }
  }
  else if (activeSequence == 3) { // DETECTIVE
    if(sequenceStep==0) { currentMood=SOSPECHOSO; sequenceTimer=now+3000; sequenceStep++; }
    else if(sequenceStep==1 && now>sequenceTimer) { currentMood=LINTERNA; sequenceTimer=now+1000; sequenceStep++; }
    else if(sequenceStep==2 && now>sequenceTimer) { activeSequence=0; manualOverride=false; }
  }
  else if (activeSequence == 4) { // LLORAR
    if(sequenceStep==0) { currentMood=TRISTE; sequenceTimer=now+2000; sequenceStep++; }
    else if(sequenceStep==1 && now>sequenceTimer) { currentMood=LLORANDO; sequenceTimer=now+4000; sequenceStep++; }
    else if(sequenceStep==2 && now>sequenceTimer) { activeSequence=0; manualOverride=false; }
  }
  else if (activeSequence == 5) { // ESTORNUDO
    if(sequenceStep==0) { currentMood=NEUTRAL; target.lidTop=15; target.lidBot=15; sequenceTimer=now+1500; sequenceStep++; }
    else if(sequenceStep==1 && now>sequenceTimer) { currentMood=NEUTRAL; target.lidTop=25; target.lidBot=25; target.shake=1; sequenceTimer=now+1500; sequenceStep++; }
    else if(sequenceStep==2 && now>sequenceTimer) { currentMood=SORPRENDIDO; target.shake=6; sequenceTimer=now+1000; sequenceStep++; }
    else if(sequenceStep==3 && now>sequenceTimer) { currentMood=CONFUNDIDO; sequenceTimer=now+2000; sequenceStep++; }
    else if(sequenceStep==4 && now>sequenceTimer) { activeSequence=0; manualOverride=false; }
  }
  else if (activeSequence == 6) { // RISA
    if(sequenceStep==0) { currentMood=FELIZ; sequenceTimer=now+200; sequenceStep++; }
    else if(sequenceStep<10 && now>sequenceTimer) { 
      if(sequenceStep%2==0) target.lidBot=25; else target.lidBot=15;
      sequenceTimer=now+150; sequenceStep++;
    }
    else if(sequenceStep>=10 && now>sequenceTimer) { activeSequence=0; manualOverride=false; }
  }
  target = getMoodConfig(currentMood);
}

void syncTime() {
  if (!wifiConnected) return;
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){ currentHour=timeinfo.tm_hour; currentMin=timeinfo.tm_min; currentWDay=timeinfo.tm_wday; }
}

void decideNaturalMood() {
  // CLIMA REAL
  if (realWeatherCode != -1 && random(100) < 15) { 
    if (realWeatherCode == 0) currentMood = SOLEADO;
    else if (realWeatherCode <= 3) currentMood = NEUTRAL;
    else if (realWeatherCode <= 48) currentMood = NIEBLA;
    else if (realWeatherCode <= 67) currentMood = LLUVIA;
    else if (realWeatherCode <= 77) currentMood = NIEVE;
    else if (realWeatherCode >= 95) currentMood = TORMENTA;
    else currentMood = LLUVIA;
    moodDur = 5000; target = getMoodConfig(currentMood); return;
  }

  uint8_t nextMood = NEUTRAL;
  int roll = random(100);

  // LISTAS
  Mood calmMoods[] = { NEUTRAL, NEUTRAL, CANSADO, SLEEPY, CONFUNDIDO, NEUTRAL };
  Mood activeMoods[] = { FELIZ, EUFORICO, SORPRENDIDO, SOSPECHOSO, GUINO, DINERO, NEUTRAL, NEUTRAL, COQUETO, TIERNO };
  Mood crazyMoods[] = { GLITCH, MODO_MATRIX, MODO_CRYPTO, MALVADO, ENFADADO, LOCO, MODO_ESCANER };

  if (currentWDay == 6 && (currentHour >= 22 || currentHour <= 1)) {
    if (roll < 40) nextMood = MODO_DISCO; else if (roll < 70) nextMood = EUFORICO; else if (roll < 90) nextMood = ENAMORADO; else nextMood = GLITCH; 
    moodDur = random(3000, 6000); return; 
  }

  if (currentHour != -1 && (currentHour >= 23 || currentHour < 7)) { // Noche
    if(roll < 95) nextMood = DORMIDO; else nextMood = CANSADO; 
    moodDur = random(20000, 40000);
  }
  else if (currentHour >= 7 && currentHour < 10) { 
    nextMood = calmMoods[random(6)]; moodDur = random(5000, 10000);
  }
  else { 
    if (roll < 70) { 
       nextMood = NEUTRAL; moodDur = random(5000, 15000); 
    } else { 
       if (random(100) < 5) nextMood = crazyMoods[random(7)]; else nextMood = activeMoods[random(10)];
       moodDur = random(3000, 7000);
    }
  }
  if (nextMood <= BUSCANDO_DATOS) nextMood = NEUTRAL; 
  currentMood = nextMood;
  target = getMoodConfig(currentMood);
}

void updatePhysics() {
  processBluetoothCommand();
  runSequences();
  uint32_t now = millis();
  
  if (now - lastTimeSync > 3600000) { syncTime(); lastTimeSync = now; }
  if (wifiConnected && now - lastDataFetch > 1800000) { fetchRealData(); lastDataFetch = now; }

  // Auto-Reset
  if (manualOverride && now > overrideTimer && !manualGaze && !pomodoroActive && currentMood != LINTERNA && activeSequence==0) {
    manualOverride = false; currentMood = NEUTRAL; lastMoodTime = now; target = getMoodConfig(NEUTRAL);
  }

  // IA: SE EJECUTA SIEMPRE (Quitada restricción !isLocked)
  if (!manualOverride && !manualGaze) {
    if (now - lastMoodTime > moodDur) {
      lastMoodTime = now;
      decideNaturalMood();
    }
  }

  // Saccades: SE EJECUTAN SIEMPRE (Quitada restricción !isLocked)
  static uint32_t gazeT=0; static float tgtGX=0, tgtGY=0;
  if (manualGaze) { tgtGX = manualGX; tgtGY = manualGY; } 
  else {
    if (now-gazeT > random(500, 3000)) {
      gazeT = now;
      if (currentMood > TEXTO_BLE && currentMood != MUERTO && currentMood != DORMIDO && currentMood != HIPNOTIZADO) {
         tgtGX = random(-8, 9); tgtGY = random(-5, 6);
      } else { tgtGX=0; tgtGY=0; }
    }
  }

  float s = 0.2;
  curTop = mix(curTop, target.lidTop, s); curBot = mix(curBot, target.lidBot, s);
  curBrowA = mix(curBrowA, target.browAng, s); curBrowY = mix(curBrowY, target.browY, s);
  curGazeX = mix(curGazeX, tgtGX, 0.3); curGazeY = mix(curGazeY, tgtGY, 0.3);

  breathVal = sin(now / 1200.0); 
  float pSz = target.pupilSize;
  if (pSz > 0) pSz += breathVal * 0.8; 
  if(currentMood==ENAMORADO) pSz += (sin(now/150.0)>0.6)? 3.0:0; 
  if(currentMood==EUFORICO) pSz += sin(now/80.0)*2.0;    
  if(currentMood==MODO_CRYPTO) { if(random(100)<5) target.pupilType = (random(2)==0)?P_ARROW_UP:P_ARROW_DOWN; }
  if(currentMood==MODO_LATIDO) { pSz = 10 + sin(now/100.0)*4; }
  curPSize = mix(curPSize, pSz, s);

  if(target.pupilSize>0 && currentMood!=MUERTO && currentMood!=CARGANDO && currentMood!=GLITCH && currentMood!=DORMIDO && currentMood!=TEXTO_BLE && currentMood!=LINTERNA) {
    if(!blinking && now > blinkNext) { blinking = true; blinkStart = now; }
  } else blinking = false;
  
  if (!bleConnected && oldBleConnected) { delay(500); pServer->startAdvertising(); oldBleConnected = bleConnected; }
  if (bleConnected && !oldBleConnected) { oldBleConnected = bleConnected; }
}

// ==========================================================================
// 9. RENDERIZADO
// ==========================================================================
void drawExtras(int x, int y, bool isLeft) {
  u8g2.setDrawColor(1); int ani = (millis()/100); 
  if(target.extra==E_LOCK){ u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 74); }
  else if(target.extra==E_WIFI){ u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 80); }
  else if(target.extra==E_SAVE){ u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 71); }
  else if(target.extra==E_TEAR && isLeft){int dy=y+15+(ani%15); u8g2.drawDisc(x-10,dy,2); u8g2.drawLine(x-10,dy-3,x-10,dy);}
  else if(target.extra==E_STREAM){int lx=x+(isLeft?-12:12); u8g2.drawBox(lx,y+15,3,15); u8g2.drawDisc(lx+1,y+30+(ani%4),2);}
  else if(target.extra==E_SWEAT && !isLeft){u8g2.drawDisc(x+18,y-22,2); u8g2.drawDisc(x+20,y-18,1);}
  else if(target.extra==E_ZZZ && !isLeft){u8g2.setFont(u8g2_font_micro_tr); int z=(millis()/400)%3; if(z>=0)u8g2.drawStr(x+16,y-20,"z"); if(z>=1)u8g2.drawStr(x+22,y-26,"z");}
  else if(target.extra==E_QUESTION && !isLeft){u8g2.setFont(u8g2_font_7x14B_tr); u8g2.drawStr(x+16,y-20,"?");}
  else if(target.extra==E_ANGER && !isLeft){int ax=x+20,ay=y-22; u8g2.drawLine(ax,ay,ax+5,ay); u8g2.drawLine(ax,ay,ax,ay+5); u8g2.drawLine(ax+5,ay,ax+5,ay+5); u8g2.drawLine(ax,ay+5,ax+5,ay);}
  else if(target.extra==E_SUN && !isLeft) {int sx=x+20, sy=y-20; u8g2.drawDisc(sx, sy, 4); for(int i=0; i<8; i++) { float ang = (i*45 + (millis()/10)) * 0.0174; u8g2.drawLine(sx, sy, sx+cos(ang)*9, sy+sin(ang)*9); }}
  else if(target.extra==E_RAIN && !isLeft) {int cx=x+15, cy=y-25; u8g2.drawDisc(cx, cy, 4); u8g2.drawDisc(cx+6, cy-2, 5); u8g2.drawDisc(cx+12, cy, 4); int dropY = y-15 + (ani%10); u8g2.drawLine(cx+2, dropY, cx+2, dropY+3); u8g2.drawLine(cx+8, dropY-4, cx+8, dropY-1);}
  else if(target.extra==E_STORM && !isLeft) {if ((millis()/200)%2==0) { int lx=x+18, ly=y-25; u8g2.drawLine(lx, ly, lx-4, ly+6); u8g2.drawLine(lx-4, ly+6, lx, ly+6); u8g2.drawLine(lx, ly+6, lx-4, ly+12); }}
  else if(target.extra==E_SNOW && !isLeft) {int sx=x+15, sy=y-30; for(int i=0; i<3; i++) { int fall = (millis()/(100+i*50))%20; u8g2.drawPixel(sx+i*6, sy+fall); u8g2.drawPixel(sx+i*6+1, sy+fall); }}
  else if(target.extra==E_WIND && !isLeft) { u8g2.setFont(u8g2_font_open_iconic_weather_2x_t); u8g2.drawGlyph(x+10, y-25, 75); }
  else if(target.extra==E_CLOUD && !isLeft) { u8g2.setFont(u8g2_font_open_iconic_weather_2x_t); u8g2.drawGlyph(x+10, y-25, 64); }
  else if(target.extra==E_CHAT && !isLeft) { u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 65); }
  else if(target.extra==E_MAIL && !isLeft) { u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 64); }
  else if(target.extra==E_ALERT && !isLeft) { u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 79); }
  else if(target.extra==E_HEARTBEAT && !isLeft) { u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); u8g2.drawGlyph(x+10, y-25, 66); } 
}

void drawPupil(int x, int y, int sz, PupilType t) {
  u8g2.setDrawColor(0); 
  switch(t) {
    case P_GEAR: u8g2.drawDisc(x, y, 6); u8g2.setDrawColor(1); u8g2.drawDisc(x, y, 2); u8g2.setDrawColor(0); break;
    case P_LOCKED: u8g2.drawLine(x-5,y-5,x+5,y+5); u8g2.drawLine(x+5,y-5,x-5,y+5); break;
    case P_NORMAL: u8g2.drawDisc(x, y, sz); if(sz>5){u8g2.setDrawColor(1); u8g2.drawDisc(x+sz/3,y-sz/3,2); u8g2.drawDisc(x-sz/4,y+sz/3,1);} break;
    case P_DOT: u8g2.drawDisc(x, y, 2); break;
    case P_CAT: u8g2.drawBox(x-2, y-sz, 5, sz*2); break;
    case P_HEART: u8g2.drawDisc(x-3,y-3,4); u8g2.drawDisc(x+3,y-3,4); u8g2.drawTriangle(x-7,y-1,x+7,y-1,x,y+7); break;
    case P_STAR: u8g2.drawLine(x-sz,y,x+sz,y); u8g2.drawLine(x,y-sz,x,y+sz); u8g2.drawLine(x-sz/2,y-sz/2,x+sz/2,y+sz/2); u8g2.drawLine(x-sz/2,y+sz/2,x+sz/2,y-sz/2); break;
    case P_SWIRL: { int p=(millis()/60)%6; for(int r=sz;r>2;r-=3) u8g2.drawArc(x,y,r,(p*40)+(r*10),(p*40)+(r*10)+180); } break;
    case P_LINE: u8g2.drawBox(x-sz, y-1, sz*2, 3); break;
    case P_X: u8g2.drawLine(x-sz,y-sz,x+sz,y+sz); u8g2.drawLine(x-sz+1,y-sz,x+sz+1,y+sz); u8g2.drawLine(x+sz,y-sz,x-sz,y+sz); u8g2.drawLine(x+sz-1,y-sz,x-sz-1,y+sz); break;
    case P_FIRE: u8g2.drawTriangle(x,y-sz, x-sz/2,y+sz/2, x+sz/2,y+sz/2); break; 
    case P_DOLLAR: u8g2.setFont(u8g2_font_7x14B_tr); u8g2.drawStr(x-3, y+5, "$"); break;
    case P_GLITCH: for(int i=0; i<15; i++) u8g2.drawPixel(x+random(-sz,sz), y+random(-sz,sz)); break;
    case P_ARROW_UP: u8g2.drawTriangle(x, y-sz, x-sz+2, y+sz, x+sz-2, y+sz); break;
    case P_ARROW_DOWN: u8g2.drawTriangle(x, y+sz, x-sz+2, y-sz, x+sz-2, y-sz); break;
    case P_SCAN: { int pos = (millis()/10)%20 - 10; u8g2.drawBox(x+pos, y-2, 4, 4); } break;
    case P_BAR: u8g2.drawFrame(x-8, y-3, 16, 6); u8g2.drawBox(x-7, y-2, (millis()/100)%14, 4); break;
    case P_SICK: u8g2.drawLine(x-4, y, x+4, y); u8g2.drawLine(x, y-4, x, y+4); break;
  }
  u8g2.setDrawColor(1);
}

void drawEye(int cx, int cy, bool isLeft) {
  // PINGPONG OFFSET
  int ppX = 0;
  if(currentMood == MODO_PINGPONG) { ppX = sin(millis()/200.0)*12; }

  int sx=0, sy=(int)(breathVal*1.5); 
  if(target.shake>0){sx+=random(-target.shake,target.shake+1); sy+=random(-target.shake,target.shake+1);}
  float bH=0;
  if(blinking){
    uint32_t t=millis()-blinkStart; 
    if(t>220){blinking=false; blinkNext=millis()+random(2000,6000);} else {float p=sin(M_PI*(float)t/220.0); bH=p*EYE_H;}
  }
  u8g2.setDrawColor(1); u8g2.drawRBox(cx+sx-EYE_W/2, cy+sy-EYE_H/2, EYE_W, EYE_H, EYE_R);
  float jX=(float)random(-10,10)/20.0, jY=(float)random(-10,10)/20.0;
  int px=cx+sx+(int)curGazeX+(int)jX+ppX, py=cy+sy+(int)curGazeY+(int)jY;
  px=constrain(px,cx-EYE_W/2+8,cx+EYE_W/2-8); py=constrain(py,cy-EYE_H/2+8,cy+EYE_H/2-8);
  drawPupil(px, py, (int)curPSize, target.pupilType);
  u8g2.setDrawColor(0); float lt=curTop+bH, lb=curBot+bH;
  if(target.asymmetric){if(currentMood==GUINO){if(isLeft){lt=44;lb=5;}else{lt=0;lb=12;}} else if(currentMood==CONFUNDIDO){if(isLeft){lt=0;lb=0;}else{lt=20;lb=15;}}}
  if(lt+lb>EYE_H-2){float d=(lt+lb)-(EYE_H-2); lt-=d/2; lb-=d/2;}
  if(lt>0)u8g2.drawBox(cx+sx-EYE_W/2, cy+sy-EYE_H/2, EYE_W, (int)lt);
  if(lb>0)u8g2.drawBox(cx+sx-EYE_W/2, cy+sy+EYE_H/2-(int)lb, EYE_W, (int)lb);
  if(target.hasBrows) {
    float ang = constrain((isLeft?curBrowA:-curBrowA), -6.0, 6.0); float rad = ang * 0.12; 
    float yOffset = curBrowY;
    if(target.asymmetric) { if(currentMood==CONFUNDIDO){if(isLeft)yOffset-=8; else yOffset+=5;} if(currentMood==GUINO&&isLeft)yOffset+=5; }
    int bx = cx+sx; int by = cy+sy + (int)yOffset; 
    float x0 = bx + (-9 * cos(rad)); float y0 = by + (-9 * sin(rad));
    float x1 = bx + ( 9 * cos(rad)); float y1 = by + ( 9 * sin(rad));
    u8g2.setDrawColor(1);
    u8g2.drawLine(x0, y0, x1, y1); u8g2.drawLine(x0, y0-1, x1, y1-1); u8g2.drawLine(x0, y0+1, x1, y1+1);
    u8g2.drawDisc(x0, y0, 1); u8g2.drawDisc(x1, y1, 1);
  }
  drawExtras(cx+sx, cy+sy, isLeft);
}

// ==========================================================================
// 9. SETUP & LOOP
// ==========================================================================
void setup() {
  Wire.begin(4, 5); u8g2.begin(); u8g2.setContrast(255); 
  randomSeed(analogRead(0));
  
  // INIT NVS & BLE
  preferences.begin("mochi_config", false);
  ssid = preferences.getString("ssid", DEFAULT_SSID);
  password = preferences.getString("pass", DEFAULT_PASSWORD);

  BLEDevice::init(BLE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();

  for(int i=0; i<128; i++) matrixCols[i] = random(-64, 0);

  // Intentar conectar con lo que haya guardado
  connectWifi();
  
  // AL TERMINAR SETUP, FORZAMOS NEUTRAL Y TIEMPO PARA QUE ARRANQUE LA VIDA
  currentMood = NEUTRAL;
  target = getMoodConfig(NEUTRAL);
  lastMoodTime = millis();
}

void loop() {
  updatePhysics(); 
  u8g2.clearBuffer();
  
  // RENDERIZADO POR MODOS
  if (currentMood == LINTERNA) {
    u8g2.setDrawColor(1); u8g2.drawBox(0,0,128,64); 
  }
  else if (currentMood == INFO_SCREEN) {
    u8g2.setFont(u8g2_font_5x7_tr); u8g2.setDrawColor(1);
    u8g2.setCursor(5, 10); u8g2.print("WIFI: "); u8g2.print(wifiConnected?"ON":"OFF");
    u8g2.setCursor(5, 25); u8g2.print("HORA: "); 
    if(currentHour<10) u8g2.print("0"); u8g2.print(currentHour); u8g2.print(":"); 
    if(currentMin<10) u8g2.print("0"); u8g2.print(currentMin);
    
    u8g2.setCursor(5, 40); u8g2.print("CITY: "); u8g2.print(realCity);
    u8g2.setCursor(5, 55); u8g2.print("TEMP: "); u8g2.print(realTemp); u8g2.print("C");
  }
  else if (currentMood == TEXTO_BLE) {
    u8g2.setFont(u8g2_font_9x15B_tr);
    int tw = u8g2.getStrWidth(bleMessage.c_str());
    u8g2.setDrawColor(1); u8g2.drawStr(64 - tw/2, 35, bleMessage.c_str());
    u8g2.drawArc(64, 45, 10, 0, 180);
  }
  else if (currentMood == MODO_MATRIX) {
    u8g2.setDrawColor(1); u8g2.setFont(u8g2_font_micro_tr);
    for(int i=0; i<128; i+=6) {
      if(random(10)>8) u8g2.drawStr(i, matrixCols[i], (random(2)==0)?"1":"0");
      matrixCols[i] += 2; if(matrixCols[i] > 64) matrixCols[i] = random(-20, 0);
    }
  }
  else if (currentMood == MODO_DISCO) {
    if(random(2)==0) u8g2.setDrawColor(0); else u8g2.setDrawColor(1);
    u8g2.drawBox(0,0,128,64); u8g2.setDrawColor(1);
    drawEye(BASE_X_L + random(-5,5), EYE_Y + random(-5,5), true); 
    drawEye(BASE_X_R + random(-5,5), EYE_Y + random(-5,5), false); 
  }
  else {
    drawEye(BASE_X_L, EYE_Y, true); drawEye(BASE_X_R, EYE_Y, false); 
    if(pomodoroActive) { 
      u8g2.setFont(u8g2_font_5x7_tr); u8g2.setDrawColor(1); 
      int mins = (25*60*1000 - (millis()-pomodoroStart))/60000;
      u8g2.setCursor(55, 62); u8g2.print(mins); u8g2.print(" min");
    }
    // Feedback de texto temporal
    if(bleMessage != "" && (currentMood == CONECTADO_OK || currentMood == SIN_WIFI || currentMood == BLOQUEADO || currentMood == FELIZ)) {
       u8g2.setFont(u8g2_font_5x7_tr); int tw=u8g2.getStrWidth(bleMessage.c_str());
       u8g2.setDrawColor(0); u8g2.drawBox(64-tw/2-2, 54, tw+4, 10);
       u8g2.setDrawColor(1); u8g2.drawStr(64-tw/2, 62, bleMessage.c_str());
    }
  }
  u8g2.sendBuffer();
}
