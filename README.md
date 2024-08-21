# USB2PS2
Convertidor de teclado USB a PS/2 con una placa RP2040 Zero.
Este proyecto es un fork de https://github.com/No0ne/ps2x2pico pensado como adaptacion para mi [ESPectrum](https://github.com/EremusOne/ESPectrum)

## Version 1.0

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


# Instalar
* Descargar  `usb2ps2.uf2` from https://github.com/destroyer-dcf/usb2ps2/releases/download/1.0
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
Gracias a NoOne por su ayuda y compartir este trabajo.