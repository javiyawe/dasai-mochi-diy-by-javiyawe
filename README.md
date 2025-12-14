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

