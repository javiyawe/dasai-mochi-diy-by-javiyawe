# 🤖 Mochi Bot V43 by javiyawe (ESP32-C6)
### AI Desktop Companion · Non-Stop Living Device

**Mochi Bot** es un compañero de escritorio *vivo* basado en el microcontrolador **ESP32-C6**.  
No es una simple pantalla con ojos: es una **mascota digital autónoma** con biorritmos, conexión a internet para datos reales (hora y clima) y control total vía Bluetooth.

La versión **V43** introduce el concepto **Non-Stop AI**:  
👉 Mochi **nunca deja de “vivir”**.  
Parpadea, respira, mira y reacciona **incluso mientras estás conectado por Bluetooth o enviando comandos**.

---

## ✨ Características Principales

### 🧠 IA Biológica Autónoma
- **Ciclo Día / Noche**
  - 🌙 Se duerme automáticamente (23:00 – 07:00)
  - 🌅 Se despierta cansado por la mañana
- **Clima real**
  - Geolocalización por IP
  - Cambia de ánimo si llueve, hay viento o tormenta
- **Comportamiento aleatorio**
  - Durante el día alterna entre curiosidad, felicidad y aburrimiento

---

### 📡 Conectividad Híbrida
- **WiFi One-Shot**
  - Configuración con un solo comando
  - Si pierde WiFi → sigue funcionando offline
- **Bluetooth seguro**
  - Desbloqueo por contraseña (`PASS:1234`)
  - Las animaciones **no se detienen** al usar BLE

---

### 🎨 Motor Gráfico “Smooth Eyes”
- Renderizado vectorial
- Bordes redondeados (radio 8)
- Parpadeo orgánico
- Ojos suaves, vivos y expresivos

---

### 🛠️ Apps Integradas
- ⏱ **Pomodoro** (25 min)
- 🔔 **Notificaciones** (simuladas: WhatsApp / Email)
- 💬 **Mensajería** (texto desde el móvil)
- 🔦 **Linterna** (pantalla blanca al 100 %)

---

## 🔧 Requisitos de Hardware

Solo necesitas **dos componentes**:

- **ESP32-C6**
  - Recomendado por WiFi 6 y BLE 5.0
- **Pantalla OLED I2C SSD1306**
  - 0.96” o 1.3”
  - Resolución: 128×64

---

## 🔌 Conexión (Wiring)

| ESP32-C6 | OLED |
|--------|------|
| 3.3V / 5V | VCC |
| GND | GND |
| GPIO 4 | SDA |
| GPIO 5 | SCL |

---

## 💻 Instalación y Carga

### 1️⃣ Preparar Arduino IDE
Instala desde el Gestor de Librerías:

- `U8g2` (Oliver Kraus) → gráficos
- `ArduinoJson` (Benoit Blanchon) → **imprescindible para el clima**
- **ESP32 Board Definitions** (v3.0.0 o superior)

---

### 2️⃣ Configuración de la Placa ⚠️ IMPORTANTE
Para que el código quepa:

- **Board:** `ESP32C6 Dev Module`
- **Partition Scheme:**
Huge APP (3MB No OTA / 1MB SPIFFS)

Si no haces esto → **no compila**.

---

### 3️⃣ Subir el Código
Carga el archivo:

DasaiEyes_V43_TrulyAlive.ino

---

## 🚀 Manual de Usuario

### 1️⃣ Primer Arranque (Configurar WiFi)
Al encender por primera vez:
- Mochi intenta conectarse
- Como no tiene WiFi → cara triste (`SIN DATOS`)

Pasos:

1. Abre una app de terminal Bluetooth  
   (ej: *Serial Bluetooth Terminal*)
   
3. Conecta a:
MOCHI_YAWE_V43

4. Mochi muestra 🔒 (bloqueado)
5. 
6. Desbloquea:
PASS:1234

7. Configura WiFi:
wifi:NombreDeTuRed,TuContraseña

Mochi guarda los datos, se reinicia y, si todo va bien, muestra **ciudad y temperatura**.

---

### 2️⃣ Uso Diario
- Solo conéctalo por USB
- Mochi es **autónomo**
- Se duerme solo por la noche
- Usa `info` si quieres comprobar su estado

---

## 🎮 Lista de Comandos Bluetooth

> Requiere conexión BLE + contraseña correcta

---

### 🎭 Expresiones (5 s)
Fuerzan una emoción temporal y luego vuelven a automático.

| Comando | Expresión |
|------|----------|
| `feliz` | 😊 Sonrisa |
| `triste` | 😢 Lágrimas |
| `enfadado` | 😠 |
| `amor` | 😍 |
| `sorpresa` | 😮 |
| `neutro` | 😐 |
| `dormir` | 😴 (30 s) |
| `dinero` | 🤑 |
| `glitch` | 👾 |
| `loco` | 😵 |
| `coqueto` | 😉 |
| `tierno` | 🥺 |
| `sospecha` | 😑 |
| `esceptico` | 🤨 |

---

### 🎬 Secuencias (Animaciones)
- `susto`
- `broma`
- `detective`
- `llorar`
- `estornudo`
- `risa`

---

### 🛠️ Utilidades
- `info` → WiFi, IP, hora, ciudad, temperatura
- `clima` → fuerza actualización
- `luz` → linterna (1 min)
- `pomodoro` → 25 min
- `stop` → cancela cualquier modo
- `di [texto]` → muestra texto

Ejemplo:
di hola jefe

---

### 🔮 Modos Visuales (Screensavers)
- `matrix`
- `disco` / `fiesta`
- `escaner`
- `gamer`
- `crypto`
- `latido`
- `pingpong`

---

### 🕹️ Control Manual (Joystick)
Mueve los ojos manualmente:

arriba
abajo
izquierda
derecha
centro

⚠️ Para **devolver el control a la IA**:
auto

---

## ❓ Solución de Problemas

**🔒 Candado en los ojos**  
→ Falta contraseña  
PASS:1234

**“ERROR WIFI” / “SIN DATOS”**  
- Red **2.4 GHz**
- Contraseña correcta
- Reenvía `wifi:...`

**Temperatura incorrecta**  
- Geolocalización por IP imprecisa  
- Espera 30 min o usa `clima`

**Ojos bloqueados**  
- Estás en modo manual  
- Envía:
auto

---

## 🧠 Filosofía del Proyecto
Mochi no quiere ser útil.  
Quiere **sentirse vivo**.

Si algún día no te juzga con la mirada…  
algo ha fallado.

---

🧪 Proyecto experimental · ESP32 · IoT · IA con personalidad  
