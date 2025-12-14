# 🤖 MOCHI BOT V40  
### The Ultimate Desktop Companion

**Mochi Bot** es un compañero de escritorio inteligente basado en **ESP32-C6**.  
No es solo una cara bonita: es un dispositivo IoT *vivo* que reacciona a la **hora real**, al **clima de tu ciudad** y a tus **comandos por Bluetooth**.

Tiene personalidad propia, **biorritmos circadianos** y varias **utilidades de productividad**, todo en un formato compacto con ojos animados.

---

## ✨ Características Principales

### 🧠 Inteligencia “Biológica”
- **Ciclo circadiano real (NTP)**
  - ☀️ **Día (07:00 – 23:00)** → Activo, curioso, mira a los lados  
  - 🌙 **Noche (23:00 – 07:00)** → Se duerme automáticamente (Zzz)  
  - 🌅 **Mañana** → Se despierta con sueño (modo cansado)

- **Reacción climática**
  - Geolocaliza tu ciudad por IP
  - Consulta el clima real
  - Si llueve → se pone triste o muestra lluvia  
  - Si nieva → cambia a modo nieve  

---

### 📡 Conectividad Híbrida (WiFi + BLE)
- **WiFi One-Shot**
  - Se conecta solo cuando necesita datos
  - Si el WiFi cae, sigue funcionando **offline** sin bloquearse
- **Bluetooth seguro**
  - Control total desde el móvil
  - Sistema de contraseña para evitar intrusos

---

### 🎨 Motor Gráfico “Smooth Eyes”
- Ojos renderizados en **vectorial**
- Bordes suaves (radio 8)
- Parpadeo orgánico (curvas senoidales)
- Pupilas “vivas” que respiran y reaccionan

---

### 🛠️ Utilidades & Apps Integradas
- ⏱ **Pomodoro** (25 min)
- 🔔 **Notificaciones** (simulador WhatsApp / Email)
- 💬 **Mensajería** (muestra texto recibido por BLE)
- 🔦 **Linterna** (pantalla blanca a máximo brillo)

---

## 🔧 Requisitos

### Hardware
- **ESP32-C6**  
  *(recomendado por WiFi 6 y BLE 5.0)*
- **Pantalla OLED SSD1306** (128×64, I2C)
  - SDA → Pin 4  
  - SCL → Pin 5  

### Software
**Arduino IDE** con las siguientes librerías:
- `U8g2` (Oliver Kraus) → Gráficos
- `ArduinoJson` (Benoit Blanchon) → **CRÍTICA** para el clima

⚠️ **IMPORTANTE**  
En **Tools → Partition Scheme**, selecciona:

Huge APP (3MB No OTA / 1MB SPIFFS)

yaml
Copiar código

El proyecto es grande y **no compila** con esquemas pequeños.

---

## 🚀 Guía de Puesta en Marcha

### 1️⃣ Primer Encendido
Al conectar Mochi por primera vez:
- Muestra `CONECTANDO...`
- Como no tiene WiFi guardado → `SIN DATOS` (cara triste)
- Entra en modo **NEUTRAL**

---

### 2️⃣ Conexión Bluetooth
1. Instala una app de terminal BLE  
   - Android: *Serial Bluetooth Terminal*  
   - iOS: *Bluefruit Connect*
2. Busca y conecta a:
MOCHI_YAWE_V40

markdown
Copiar código
3. Mochi se **bloquea** (ojos con candado)
4. Envía la contraseña:
PASS:1234

yaml
Copiar código
5. Mochi sonríe → **control desbloqueado**

---

### 3️⃣ Configurar WiFi (solo una vez)
Desde Bluetooth desbloqueado, envía:

wifi:NombreDeTuRed,TuContraseña

makefile
Copiar código

Ejemplo:
wifi:Movistar_F3,patata123

yaml
Copiar código

Mochi:
- Guarda los datos en memoria
- Muestra iconos de **configuración** y **guardado**
- Reconecta automáticamente

Si todo va bien, aparece la **pantalla INFO** con ciudad y temperatura.

---

## 🎮 Comandos Bluetooth

> Requiere conexión BLE y contraseña correcta

### 🛠️ Utilidades
| Comando   | Acción |
|----------|--------|
| `info`   | WiFi, hora, ciudad, temperatura e IP |
| `pomodoro` | Inicia cuenta atrás de 25 min |
| `stop`  | Cancela cualquier modo activo |
| `luz`   | Linterna (pantalla blanca, 1 min) |
| `clima` | Fuerza actualización del clima |

---

### 💬 Texto
| Comando | Acción |
|-------|--------|
| `di [texto]` | Muestra texto en pantalla |

Ejemplo:
di hola guapo

yaml
Copiar código

---

### 🎉 Modos Visuales (Apps)
| Comando | Efecto |
|-------|--------|
| `matrix` | Lluvia de código verde |
| `disco` | Estroboscópico, modo fiesta |
| `gamer` | Ojos glitch concentrados |
| `crypto` | Flechas “stonks” |
| `escaner` | Ojo tipo Cylon / KITT |
| `carga` | Barra de carga progresiva |

---

### 🎭 Emociones (instantáneo)
feliz, triste, enfadado, amor, sorpresa,
dormir, dinero, esceptico, sospecha, confuso

yaml
Copiar código

---

### 🕹️ Control Manual (Marioneta)
Mueve los ojos manualmente:
arriba, abajo, izquierda, derecha, centro

css
Copiar código

Para salir del modo manual:
auto

yaml
Copiar código

---

## ☁️ Funcionamiento Automático (IA)
Cuando no recibe órdenes, Mochi decide solo:

- Cada **30 minutos**:
  - Consulta clima y hora
- Si llueve o nieva:
  - Cambia su estado base
- Si es de noche:
  - Fuerza modo **DORMIDO**
- Durante el día:
  - 70 % tranquilo
  - 30 % comportamientos aleatorios:
    - Miradas laterales
    - Curiosidad
    - Micro-glitch
    - Felicidad espontánea

---

## ❓ Solución de Problemas

**`Temp: 0.00C`**
- Fallo temporal de la API del clima  
- Solución:
clima

markdown
Copiar código

**No conecta al WiFi**
- Asegúrate de que la red es **2.4 GHz**
- Revisa mayúsculas y contraseña

**Error `Sketch too big`**
- Cambia el esquema de partición a:
Huge APP

yaml
Copiar código

---

## 🧠 Filosofía del Proyecto
Mochi no intenta ser un asistente aburrido.  
Es un **objeto con carácter**, diseñado para sentirse *vivo*, no útil solo por obligación.

Si no te mira raro alguna vez… algo va mal.

---

🧪 Proyecto experimental · ESP32 · IoT · Personalidad artificial  
