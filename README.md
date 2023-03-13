Este código es un programa para Arduino que controla diferentes dispositivos como ventilador, humificador y extractor de aire, según las condiciones de temperatura y humedad dentro de una habitación.

El código comienza importando diferentes bibliotecas necesarias para el funcionamiento del programa, como Wire.h para la comunicación I2C, RTClib.h para el reloj en tiempo real, DHT.h para el sensor de temperatura y humedad DHT22 y LiquidCrystal_I2C.h para la pantalla LCD.

A continuación, se definen las siguientes constantes de pines que representan los pines en el microcontrolador conectados a los relés que controlan los dispositivos mencionados anteriormente: ReleLuz, ReleVentilador, ReleExtractor y ReleHumificador. También se definen las constantes ON y OFF que se utilizan para encender y apagar los dispositivos.

Luego, se definen diferentes intervalos de tiempo que controlan cuándo se deben actualizar los sensores de temperatura y humedad, cuándo se debe actualizar el reloj en tiempo real y cuándo se debe actualizar la pantalla LCD.

La función InicializarSensores() inicializa los sensores y la pantalla LCD.

En la función setup(), se inicializa la comunicación serial y se llaman a las funciones de inicialización de los sensores.

La función actualizarSensores() actualiza la temperatura y la humedad en intervalos regulares, mientras que la función actualizarRTC() actualiza el reloj en tiempo real.

La función actualizarDisplay() actualiza la pantalla LCD para mostrar la hora, la temperatura, la humedad y el estado de los dispositivos conectados.

La función extractor() controla el extractor de aire en función de la temperatura en la habitación. Si la temperatura es superior a la máxima permitida, el extractor se encenderá hasta que la temperatura baje por debajo de la máxima menos una tolerancia definida. Si la temperatura es inferior a la máxima menos la tolerancia, el extractor se apaga.

La función ventilador() controla el ventilador en función de la temperatura en la habitación. Si la temperatura es superior a la máxima permitida más una tolerancia definida, el ventilador se encenderá hasta que la temperatura baje por debajo de la máxima. Si la temperatura es inferior a la máxima menos la tolerancia, el ventilador se apaga.

La función luz() controla la luz en función de la hora del día. Si es de noche, la luz se encenderá hasta la hora definida para apagarla. Si es de día, la luz se apagará hasta la hora definida para encenderla.

La función loop() se ejecuta continuamente y llama a las funciones de actualización y control de dispositivos según sea necesario.
