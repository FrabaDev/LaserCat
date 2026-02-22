# LaserCat 🐱

Control de láser 2 ejes para gatos — NodeMCU ESP8266 con interfaz web, WebSocket y actualizaciones OTA.

Basado en el proyecto original [funny0facer/2AxisCatLaser](https://github.com/funny0facer/2AxisCatLaser) (ESP32 + Telnet), adaptado completamente para **ESP8266** con interfaz web moderna.

> El modelo 3D para imprimir el soporte se encuentra en [Printables](https://www.printables.com/model/415125).

---

## Características

- **Control en tiempo real** vía WebSocket desde cualquier navegador o celular
- **Joystick 2D táctil** — mueve el láser arrastrando el dedo
- **6 programas automáticos** de movimiento + modo aleatorio
- **Temporizador** — apaga el láser automáticamente al terminar la sesión
- **Modo Loop** — repite el programa seleccionado indefinidamente
- **Configuración WiFi** desde la propia interfaz web (sin recompilar)
- **OTA** — actualiza firmware y filesystem sin cable USB
- **Modo AP de emergencia** — si no hay credenciales WiFi, levanta un punto de acceso para configurar

---

## Hardware requerido

| Componente | Descripción |
|-----------|-------------|
| NodeMCU v1.0 | ESP8266 (ESP-12E), 4 MB Flash |
| Servo A | Pan (horizontal), rango 0°–90° |
| Servo B | Tilt (vertical), rango -85°–85° |
| Módulo láser | 5 mW, 650 nm (punto rojo) |
| Transistor NPN | BC547, 2N2222 o similar |
| Resistencia | 1 kΩ (base del transistor) |
| Fuente externa | 5 V / 2 A mínimo (para los servos) |

---

## Diagrama de conexión

```
NodeMCU                Componentes
─────────────────────────────────────────────
D5 (GPIO14) ──────────► Servo A  (señal)
D6 (GPIO12) ──────────► Servo B  (señal)
D8 (GPIO15) ──[ 1kΩ ]──► Base transistor NPN
                          Colector ──► Láser (+)
                          Emisor   ──► GND

VIN ──────────────────► Servo A/B (VCC)
                        Láser (+) vía transistor
GND ──────────────────► Servo A/B (GND)
                        Emisor transistor
                        Láser (GND)
```

> ⚠️ **No alimentes los servos desde el pin 3V3 del NodeMCU** — consumen demasiada corriente. Usa VIN (5 V) o una fuente externa.

---

## Software requerido

- [PlatformIO Core](https://platformio.org/) 6.x o superior
- Python 3.8+

```bash
# Instalar PlatformIO via pipx (recomendado en Linux)
pipx install platformio

# Verificar versión
pio --version   # debe mostrar 6.x
```

### Dependencias — se instalan automáticamente

| Librería | Versión | Uso |
|---------|---------|-----|
| ESPAsyncWebServer | ^3.7.3 | Servidor web asíncrono |
| ESPAsyncTCP | ^2.0.0 | TCP asíncrono para ESP8266 |
| ElegantOTA | ^3.1.6 | Panel de actualización OTA |
| ArduinoJson | ^7.0.0 | Protocolo WebSocket JSON |
| Servo | (core) | Control de servomotores |
| LittleFS | (core) | Filesystem para la interfaz web |

---

## Instalación desde cero (primera vez)

### 1. Clonar el repositorio

```bash
git clone https://github.com/FrabaDev/LaserCat.git
cd LaserCat
```

### 2. Permisos del puerto serial (solo Linux)

```bash
# Regla udev permanente para el chip CP210x del NodeMCU
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", MODE="0666"' \
  | sudo tee /etc/udev/rules.d/99-lasercat.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### 3. Flashear firmware

Con el NodeMCU conectado por USB:

```bash
pio run -e nodemcu -t upload
```

### 4. Flashear la interfaz web (LittleFS)

```bash
pio run -e nodemcu -t uploadfs
```

> ⚠️ Hazlo siempre en este orden: **firmware primero**, filesystem después.

### 5. Configurar WiFi (primera vez)

Al arrancar sin credenciales guardadas, el NodeMCU crea un punto de acceso:

1. Conéctate a la red **`CatLaser-Setup`** desde tu celular o PC
2. Abre **`http://192.168.4.1`** en el navegador
3. En la sección **WiFi**, introduce tu SSID y contraseña
4. Pulsa **Guardar y reiniciar**
5. El dispositivo se conecta a tu red — la IP aparece en el monitor serial:

```bash
pio device monitor -b 115200
```

A partir de aquí ya no necesitas el cable USB.

---

## Interfaz web

Abre `http://<IP-del-dispositivo>` en cualquier navegador.

### Joystick 2D

Arrastra el punto rojo para mover el láser en tiempo real:

- Eje **izquierda / derecha** → Servo B (pan)
- Eje **arriba / abajo** → Servo A (tilt)
- Botón **↔ Flip B** — invierte el eje horizontal si el movimiento está al revés respecto al montaje físico

### Laser

Botón de encendido y apagado manual del láser.

### Temporizador

| Control | Función |
|--------|---------|
| Botones 5 / 10 / 15 / 30 min | Presets rápidos |
| Campo numérico | Duración personalizada en minutos |
| Iniciar / Detener | Arranca o cancela la cuenta regresiva |

Al expirar el temporizador: **apaga el láser y detiene el loop automáticamente**.

### Programas automáticos

| # | Descripción |
|---|-------------|
| 1 | Movimiento en esquinas |
| 2 | Barrido horizontal y vertical |
| 3 | Círculos pequeños con pausas |
| 4 | Triángulos irregulares |
| 5 | Zigzag amplio |
| 6 | Círculo continuo |
| Aleatorio | Selecciona un programa al azar |

Activa **Loop** para repetir el programa seleccionado indefinidamente.

> Si el dispositivo lleva más de 2 minutos sin que ningún cliente se conecte, ejecuta un programa aleatorio de forma automática.

### Configuración WiFi

Puedes cambiar las credenciales WiFi en cualquier momento desde la sección **WiFi** de la interfaz, sin cable USB ni recompilación.

---

## Actualización OTA

Una vez configurado el WiFi, todas las actualizaciones se hacen desde el navegador en `http://<IP>/update`.

### Actualizar el firmware

```bash
# Compilar
pio run -e nodemcu
```
Luego en `http://<IP>/update` → **Firmware** → subir:
```
.pio/build/nodemcu/firmware.bin
```

### Actualizar la interfaz web

```bash
# Construir imagen del filesystem
pio run -e nodemcu -t buildfs
```
Luego en `http://<IP>/update` → **LittleFS/SPIFFS** → subir:
```
.pio/build/nodemcu/littlefs.bin
```

---

## Estructura del proyecto

```
LaserCat/
├── platformio.ini                   # Configuración de PlatformIO
├── data/
│   └── index.html                   # Interfaz web (SPA, LittleFS)
└── src/
    ├── main.cpp                     # Setup y loop principal
    ├── MyServo.cpp / .hh            # Control de servomotores
    ├── MyLaser.cpp / .hh            # Control del láser
    ├── MyWiFi.cpp  / .hh            # WiFi: STA desde LittleFS + AP fallback
    ├── MyProgramController.cpp / .hh  # Programas de movimiento automático
    └── WebInterface.cpp / .hh       # Servidor web, WebSocket, OTA
```

---

## Protocolo WebSocket

El WebSocket está disponible en `ws://<IP>/ws` para integración con otros clientes.

### Cliente → Dispositivo

```json
{"cmd": "servo",   "axis": "A", "angle": 45.0}
{"cmd": "servo",   "axis": "B", "angle": -30.0}
{"cmd": "laser",   "state": true}
{"cmd": "program", "number": 3}
{"cmd": "loop",    "active": true}
```

### Dispositivo → Cliente (broadcast de estado)

```json
{"servoA": 45.0, "servoB": -30.0, "laser": true, "program": 3, "loop": false}
```

---

## Solución de problemas

| Síntoma | Causa probable | Solución |
|--------|---------------|----------|
| No aparece `/dev/ttyUSB0` | Cable solo de carga | Usar cable USB con datos |
| `Permission denied` en el puerto | Usuario sin permisos | Aplicar regla udev (paso 2) |
| No levanta AP ni conecta WiFi | LittleFS no flasheado | Ejecutar `pio run -t uploadfs` |
| Servos no responden | Pines incorrectos | Verificar D5/D6 en el diagrama |
| OTA falla a mitad | Flash insuficiente | Usar NodeMCU con 4 MB |
| Interfaz no carga tras OTA | Caché del navegador | Forzar recarga con Ctrl+Shift+R |

---

## Licencia

MIT — ver [src/LICENSE](src/LICENSE)
