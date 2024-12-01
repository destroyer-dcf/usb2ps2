# USB2PS2
Convertidor de teclado USB a PS/2 con una placa RP2040 Zero.
Este proyecto es un fork de https://github.com/No0ne/ps2x2pico pensado como adaptacion para mi [ESPectrum](https://github.com/EremusOne/ESPectrum)


## Version 1.1 (Keyboard and Mouse Support) (30/11/2024)


|![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/pcb1.1-A.jpg) |![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/pcb1.1-B.jpg) |
|-|-|
|![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/pcb1.1-D.jpg) |![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/pcb1.1-E.jpg) |


# Circuito
![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/circuito1.1.jpg) 
[Descarga Esquema v1.1](docs/Schematic_USB2PS-2_1.1_2024-12-01.pdf)

![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/pcb1.1-C.jpg) 
[Descarga Gerber v1.1](https://github.com/destroyer-dcf/usb2ps2/releases/download/1.0/Gerber_USB2PS-2_PCB_USB2PS-2_5_2024-08-10.zip)

# Componentes

| Cantidad | Componente| Link |
|----------|----------|----------|
| 1    | RP2040 Zero   | [ver](https://acortar.link/Bv6ozr)  |
| 1    | Convertidor de nivel logico   | [ver](https://acortar.link/reAbsi)   |
| 1    | Mini Dim 6p   | [ver](https://acortar.link/yrMd11)    |
| 1    | JST XH2.54 4 pin| [ver](https://acortar.link/VGCncE)    |
| 1    | JST XH2.54 2 pin (opcional)| [ver](https://acortar.link/VGCncE)    |
| 1    | LED (opcional)| [ver](https://acortar.link/4MLrlE)    |
| 1    | Resistencia 330Ω (opcional)| [ver](https://acortar.link/xnSlPZ)    |
| 1    | Adaptador USB-C a USB | [ver](https://es.aliexpress.com/item/1005004621683764.html#nav-specification) |


# Soldar JST XH2.54 4 pin a Dim 6

Se han de hacer 2 conectores, uno para raton y otro para el teclado.

![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/dim6.jpg) 

| Pin | Color| Funcion |
|----------|----------|----------|
| 3    | negro   | GND  |
| 4    | Rojo   | +5V  |
| 5    | Blanco   | CLOCK   |
| 1    | Amarillo| DATA    |
| 2    | Sin Uso| Sin Uso  |
| 6    | Sin Uso| Sin Uso |


# Instalar
* Descargar  `usb2ps2.uf2` from https://github.com/destroyer-dcf/usb2ps2/releases/tag/1.1
* Copie `usb2ps2.uf2` a su RP2040 presionando BOOT antes de enchufarlo.
* Conecta tu teclado/raton al usbc de la placa
* Si tu teclado/raton es reconocido el led (opcional) se encendera.

# Compilar

Para poder compilar es necesario tener descargado el [SDK de raspberry pico](https://github.com/raspberrypi/pico-sdk)

Una vez descargado es necesario actualizar la libreria TinyUSB al tag 0.16.0 para ello ..

```
export PICO_SDK_PATH=/path/to/pico-sdk
cd $PICO_SDK_PATH/lib/tinyusb
git checkout 0.16.0
```

Despues desde el directorio de nuestro proyecto

```
cd /path/to/usb2ps2
mkdir build
cd build
cmake ..
make
```

Si lo prefieres puedes ejecutar el script `compile.sh` cambiando la linea  `export PICO_SDK_PATH=~/pico-sdk` por tu path de pico-sdk


## Version 1.0 (Only Keyboard Support) (04/08/2024)

|![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/FOTO0.jpg) |![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/FOTO1.jpg) |
|-|-|
|![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/FOTO3.jpg) |![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/FOTO4.jpg) |


# Circuito
![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/circuito.jpg) 
[Descarga Esquema](docs/Schematic_USB2PS-2_2024-08-20.pdf)

![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/circuito2.jpg) 
[Descarga Gerber](https://github.com/destroyer-dcf/usb2ps2/releases/download/1.0/Gerber_USB2PS-2_PCB_USB2PS-2_5_2024-08-10.zip)

# Componentes

| Cantidad | Componente| Link |
|----------|----------|----------|
| 1    | RP2040 Zero   | [ver](https://acortar.link/Bv6ozr)  |
| 1    | Convertidor de nivel logico   | [ver](https://acortar.link/reAbsi)   |
| 1    | Mini Dim 6p   | [ver](https://acortar.link/yrMd11)    |
| 1    | JST XH2.54 4 pin| [ver](https://acortar.link/VGCncE)    |
| 1    | JST XH2.54 2 pin (opcional)| [ver](https://acortar.link/VGCncE)    |
| 1    | LED (opcional)| [ver](https://acortar.link/jYP3aA)    |
| 1    | Adaptador USB-C a USB | [ver](https://es.aliexpress.com/item/1005004621683764.html#nav-specification) |


# Soldar JST XH2.54 4 pin a Dim 6


![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/dim6.jpg) 

| Pin | Color| Funcion |
|----------|----------|----------|
| 3    | negro   | GND  |
| 4    | Rojo   | +5V  |
| 5    | Blanco   | CLOCK   |
| 1    | Amarillo| DATA    |
| 2    | Sin Uso| Sin Uso  |
| 6    | Sin Uso| Sin Uso |

```diff
- text in yellow
+ text in green
! text in orange
# text in gray
@@ text in purple (and bold)@@
```
$${\color{red}Welcome \space \color{lightblue}To \space \color{orange}Stackoverflow}$$
<font color="Yellow"><b>INFO: Esta placa no tiene soporte para raton. Si le queremos añadir seguiremos estos pasos, Si no la version 1.1 de la placa ya lo tiene.</b></font>

Para añadir soporte para raton, bastara con que soldemos dos cables uno al pin HV3 y otro al HV4

| Pin | Color| PS/2 |
|----------|----------|----------|
| HV3    | Amarillo   | CLK  |
| HV4    | VERDE   | DATA  |

![hw1](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/mouse_fix_1.jpg) |![hw2](https://raw.githubusercontent.com/destroyer-dcf/usb2ps2/main/images/mouse_fix_2.jpg) |

# Instalar
* Descargar  `usb2ps2.uf2` from https://github.com/destroyer-dcf/usb2ps2/releases/tag/1.0
* Copie `usb2ps2.uf2` a su RP2040 presionando BOOT antes de enchufarlo.
* Conecta tu teclado al usbc de la placa
* Si tu teclado es reconocido el led (opcional) se encendera.

# Compilar

Para poder compilar es necesario tener descargado el [SDK de raspberry pico](https://github.com/raspberrypi/pico-sdk)

Una vez descargado es necesario actualizar la libreria TinyUSB al tag 0.16.0 para ello ..

```
export PICO_SDK_PATH=/path/to/pico-sdk
cd $PICO_SDK_PATH/lib/tinyusb
git checkout 0.16.0
```

Despues desde el directorio de nuestro proyecto

```
cd /path/to/usb2ps2
mkdir build
cd build
cmake ..
make
```

Si lo prefieres puedes ejecutar el script `compile.sh` cambiando la linea  `export PICO_SDK_PATH=~/pico-sdk` por tu path de pico-sdk

# Recursos
* https://github.com/No0ne/ps2x2pico

# Agradecimientos
Gracias a NoOne por su ayuda y compartir su trabajo.