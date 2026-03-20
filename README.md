# 🤖 Mochi Bot V43 — Ultra Version

### AI Desktop Companion · ESP32-C6 · by javiyawe

---

**Mochi Bot** es un compañero de escritorio *vivo* basado en **ESP32-C6**. Una mascota digital autónoma con biorritmos reales, conexión a internet (hora NTP + clima), control total vía Bluetooth BLE y más de **40 expresiones y modos visuales**.

La versión **V43 Ultra** introduce:

- 🧠 **Non-Stop AI** — Mochi nunca deja de vivir. Parpadea, respira y reacciona incluso mientras envías comandos BLE.
- 🕐 **Zona horaria POSIX** — Cambio automático de horario verano/invierno (configurable para cualquier país).
- 📺 **Pantalla INFO** de 2 páginas con estado completo del dispositivo.
- 💬 **Texto adaptativo** — El comando `di` auto-escala la fuente y hace word-wrap.

---

## 🔧 Hardware Necesario

| Componente | Detalle |
|---|---|
| **ESP32-C6** | WiFi 6 + BLE 5.0 |
| **Pantalla OLED SSD1306** | I2C, 128×64, 0.96" o 1.3" |

### Conexión (Wiring)

| ESP32-C6 | OLED |
|---|---|
| 3.3V / 5V | VCC |
| GND | GND |
| GPIO 2 | SDA |
| GPIO 3 | SCL |

---

## 💻 Instalación

### 1. Preparar Arduino IDE

Instala desde el **Gestor de Librerías**:

| Librería | Autor | Uso |
|---|---|---|
| `U8g2` | Oliver Kraus | Gráficos OLED |
| `ArduinoJson` | Benoit Blanchon | Parsing de datos clima/geolocalización |

Instala desde el **Gestor de Placas**:

- **ESP32** de Espressif (v3.0.0 o superior)

### 2. Configuración de la Placa ⚠️ IMPORTANTE

| Opción | Valor |
|---|---|
| **Board** | `ESP32C6 Dev Module` |
| **Partition Scheme** | `Huge APP (3MB No OTA / 1MB SPIFFS)` |

> Si no seleccionas este esquema de partición, **no compilará** (el código con BLE + WiFi + gráficos es grande).

### 3. Subir el código

Abre y carga el archivo `ultraversion.ino` desde Arduino IDE.

---

## 🚀 Primer Arranque

Al encender por primera vez, Mochi **no tiene WiFi configurado**, así que mostrará cara triste con `SIN DATOS`.

### Configurar WiFi vía Bluetooth

1. **Descarga una app BLE** en tu móvil:
   - [nRF Connect](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp) (recomendada)
   - Serial Bluetooth Terminal
   - BLE Terminal

2. **Busca y conecta** al dispositivo:
   ```
   MOCHI_YAWE_V43
   ```

3. Mochi muestra 🔒 candado. **Desbloquea** enviando:
   ```
   PASS:1234
   ```

4. **Configura tu WiFi** (separado por coma):
   ```
   wifi:NombreDeTuRed,TuContraseña
   ```
   > ✅ Los espacios en el nombre del WiFi funcionan bien. El separador es la coma.

5. Mochi guardará los datos, conectará y mostrará la pantalla de INFO con hora, ciudad y temperatura.

> Las credenciales se guardan en **memoria NVS** y sobreviven a reinicios. No necesitas reconfigurar cada vez.

---

## 🧠 Comportamiento Autónomo (IA)

Mochi tiene un "cerebro" que decide su estado emocional de forma autónoma basándose en:

| Factor | Comportamiento |
|---|---|
| **Hora del día** | 🌙 Se duerme (23:00–07:00), 🌅 se despierta cansado (07:00–10:00), activo el resto |
| **Día de la semana** | 🎉 Los sábados noche (22:00–01:00) entra en modo fiesta |
| **Clima real** | ☀️ Contento si sol, 🌧 triste si llueve, ⛈ asustado si tormenta |
| **Aleatoriedad** | Alterna entre curiosidad, felicidad, aburrimiento, guiños y más |

### Datos en tiempo real

- **Hora**: Sincronizada por NTP con cambio automático verano/invierno (zona POSIX).
- **Clima**: Geolocalización por IP + API de Open-Meteo (temperatura y código meteorológico).
- **Actualización**: Los datos se refrescan cada 30 minutos automáticamente.

---

## 📺 Pantalla INFO (2 páginas)

Al enviar el comando `info`, la pantalla alterna automáticamente entre dos páginas cada 4 segundos:

### Página 1 — Estado de conexión
```
--- MOCHI INFO ---
WIFI: ON (-65dBm)
RED: MiWiFiCasa
IP: 192.168.1.45
HORA: 16:45 (NTP OK)
[1/2] >>>
```

### Página 2 — Datos del mundo
```
--- DATOS SYNC ---
CITY: Valencia
TEMP: 18.5 C
METEO: cod 3
SYNC: TODO OK
BLE: CONECTADO       2/2
```

> Si algo falla, en `SYNC:` verás el código de error exacto (ej: `HTTP GEO:-1`, `NTP FAIL`, `JSON METEO`).

---

## 🎮 Comandos Bluetooth (BLE)

> ⚠️ Todos los comandos requieren estar conectado por BLE y haber enviado `PASS:1234` primero.

### ⚙️ Configuración

| Comando | Descripción |
|---|---|
| `PASS:1234` | Desbloquear Mochi (obligatorio al conectar) |
| `wifi:SSID,PASSWORD` | Configurar WiFi (se guarda en memoria) |
| `zona:1` | Zona horaria UTC+1 (sin DST automático) |
| `zona:CET-1CEST,M3.5.0/2,M10.5.0/3` | Zona España con DST automático (por defecto) |
| `ciudad:Valencia` | Establecer ciudad manualmente |

### 🛠️ Utilidades

| Comando | Descripción |
|---|---|
| `info` | Pantalla de estado completa (2 páginas) |
| `clima` | Forzar actualización de datos meteorológicos |
| `di [texto]` | Mostrar texto en pantalla (auto-escala la fuente) |
| `luz` | Linterna (pantalla blanca, 1 minuto) |
| `pomodoro` | Temporizador de 25 minutos |
| `stop` | Cancelar cualquier modo activo |
| `auto` | Devolver el control a la IA |

#### Texto adaptativo (`di`)

El texto se ajusta automáticamente al tamaño de la pantalla:

| Longitud | Comportamiento |
|---|---|
| ≤ 8 caracteres | Fuente grande, centrada |
| 9–14 caracteres | Fuente media, centrada |
| 15+ caracteres | Fuente pequeña con word-wrap multilínea (hasta 7 líneas) |

Ejemplo:
```
di hola jefe
di este es un mensaje largo que se adapta solo
```

### 🎭 Expresiones (duración: 5 segundos)

| Comando | Expresión | Comando | Expresión |
|---|---|---|---|
| `feliz` | 😊 Sonrisa | `triste` | 😢 Tristeza |
| `enfadado` | 😠 Enfado | `amor` | 😍 Enamorado |
| `sorpresa` | 😮 Sorprendido | `neutro` | 😐 Normal |
| `coqueto` | 😏 Coqueto | `tierno` | 🥺 Ojos grandes |
| `loco` | 😵 Locura | `enfermo` | 🤢 Enfermo |
| `dinero` | 🤑 Signos de dólar | `asco` | 🤮 Disgustado |
| `sospecha` | 🕵️ Sospechoso | `esceptico` | 🤨 Escéptico |
| `cansado` | 😩 Agotado | `muerto` | 💀 Muerto |
| `glitch` | 👾 Efecto glitch | `dormir` | 😴 Dormido (30 seg) |

### 🎬 Secuencias Animadas (~10 segundos)

| Comando | Animación |
|---|---|
| `susto` | Asustado → tiembla → enfadado |
| `broma` | Eufórico → guiño |
| `detective` | Sospechoso → linterna |
| `llorar` | Triste → llanto |
| `estornudo` | Ojos cerrados → estornudo → confundido |
| `risa` | Párpados temblando rápido |

### 🔮 Modos Visuales (8 segundos)

| Comando | Efecto |
|---|---|
| `disco` / `fiesta` | Parpadeo estroboscópico con ojos |
| `matrix` / `hacker` | Lluvia de código estilo Matrix |
| `crypto` | Flechas arriba/abajo |
| `gamer` | Glitch visual |
| `escaner` | Escaneo horizontal |
| `carga` | Barra de progreso en las pupilas |
| `latido` | Pupilas pulsando como corazón |
| `pingpong` | Ojos rebotando lateralmente |

### 🔔 Notificaciones (4 segundos)

| Comando | Icono |
|---|---|
| `whatsapp` | 💬 Chat |
| `email` | ✉️ Correo |
| `alerta` | ⚠️ Alerta |

### 🕹️ Joystick (control de mirada)

| Comando | Dirección |
|---|---|
| `arriba` | ⬆️ |
| `abajo` | ⬇️ |
| `izquierda` | ⬅️ |
| `derecha` | ➡️ |
| `centro` | 🎯 |
| `auto` | 🤖 Devolver control a la IA |

---

## ❓ Solución de Problemas

### 🔒 Los ojos muestran un candado
→ Falta enviar la contraseña:
```
PASS:1234
```

### "ERROR WIFI" o "SIN DATOS"
- Asegúrate de que tu red es **2.4 GHz** (el ESP32-C6 no soporta 5 GHz para WiFi 4)
- Verifica que la contraseña es correcta
- Envía de nuevo: `wifi:NombreDeTuRed,TuContraseña`
- Usa `info` para ver el error exacto en la línea `SYNC:`

### La hora sale mal
- Por defecto usa zona horaria de **España** (CET/CEST con cambio automático)
- Si estás en otro país, cambia la zona: `zona:0` (UTC), `zona:-5` (Colombia), etc.
- Usa `info` y comprueba que dice `NTP OK`

### La ciudad no es correcta
- La geolocalización por IP no es precisa (depende de tu proveedor de internet)
- Ponla manualmente: `ciudad:Valencia`

### Los ojos no se mueven
- Estás en modo manual. Envía: `auto`

### No compila
- Verifica que tienes seleccionado **Huge APP (3MB No OTA / 1MB SPIFFS)** como Partition Scheme
- Verifica que tienes las librerías `U8g2` y `ArduinoJson` instaladas

---

## 🔐 Seguridad

- Al conectar por BLE, Mochi se **bloquea automáticamente**
- Requiere contraseña (`PASS:1234`) para aceptar cualquier comando
- Al desconectar BLE, vuelve a bloquearse
- La contraseña se puede cambiar en el código (variable `BLE_PASSWORD`)

---

## 🧠 Filosofía del Proyecto

Mochi no quiere ser útil.
Quiere **sentirse vivo**.

Si algún día no te juzga con la mirada…
algo ha fallado.

---

`🧪 ESP32-C6 · IoT · BLE · NTP · Open-Meteo · IA con personalidad · by javiyawe`
