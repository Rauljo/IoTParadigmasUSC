# IoTParadigmasUSC

Nombre de la solución: HomeWatcher

La idea principal de este sistema es la detección de la entrada de personas externas en el interior de un lugar privado. Esto se conseguiría con la actuación de diferentes sensores que nos ayudan a saber el estado de la casa, oficina, ... Pueden ser sensores de detección de presencia, detección de vibraciones por si se intenta forzar una puera o una ventana, detector magnético, para detectar la apertura de puertas sin que sea el dueño, ... y también actuadores en caso de la detección de un agente externo, como buzzers o una alarma.

El grupo está formado por los siguientes miembros:
- Ivanildo 
- Lidia 
- Manuel José
- Raul José


## Node-Red

Hemos desarollado una arquitectura con soporte Node-Red, formada por los siguientes sensores y actuadores:sensor de rotura de cristales, sensor de presencia, sensor magnético, botón de pánico y buzzer.
Estos sensores forman parte de un sistema, IoT, de intrusión; que enviará datos a la nube de forma constante. La gestión de datos se hace con Node-Red, pudiendo acceder el cliente fácilmente a los mismos haciendo uso del dashboard asociado.

El sistema desarrollado se estructura en las siguientes capas, que conjuntamente forman y componen la arquitectura que se describe a continuación:
1. Capa de dispositivos físicos IoT (IoT Node Layer)
   Esta capa es la encargada de interactuar con el entorno; recogiendo datos en tiempo real del medio físico: movimiento/presencia, rotura de cristales, forcejeo de puertas y ventanas.
   Es el punto de entrada de datos al sistema; y de ejecución de las órdenes correspondientes.

2. Capa de Gateway
Este dispositivo actúa como un intermediario entre los dispositivos IoT locales y la nube.
Realiza la traducción de protocolos, el filtrado de datos y la agregación de información.
Facilita la conexión de dispositivos que usan tecnologías locales (como Zigbee, LoRa, Wi-Fi) al Internet global.

3. Capa de Comunicación (Internet)
Funciona como el canal de comunicación que conecta los dispositivos locales con la nube.
Asegura que los datos puedan ser transmitidos de manera bidireccional entre los dispositivos IoT y la nube/Internet.

4. Capa de Procesamiento (Node-RED y Computación en la Nube)
Node-RED:
Herramienta de programación visual para integrar flujos de datos entre diferentes dispositivos y servicios.
Actúa como un middleware, conectando sensores, actuadores y sistemas de análisis.
Cloud Computing:
Los datos son procesados, almacenados y analizados en la nube.
Ofrece capacidad para análisis avanzado, gestión de eventos y almacenamiento a largo plazo.

5. Capa de Usuarios
Usuarios:
Representa a los individuos o sistemas que interactúan con la plataforma IoT.
Los usuarios pueden ser operadores, administradores o clientes finales que acceden a los datos o controlan los dispositivos.
Interacción:
Los usuarios reciben notificaciones, toman decisiones o envían comandos a través de interfaces, como aplicaciones móviles o paneles web.


![image](https://github.com/user-attachments/assets/0bd0f7ef-1b74-4268-8265-edc4fac478cf)


La arquitectura descrita se ha traducido a un sistema que utiliza mydevices para simular dispositivos, MQTT para enviar los mensajes y Node-Red para recibirlos, procesarlos e incorporarlos al dashboard. 

Primeramente creamos los 4 dispositivos de interés en myDevices, definiendo un topic MQTT diferente para cada uno de ellos:
![image](https://github.com/user-attachments/assets/75ed8d18-f365-4e1b-bd53-147de5e8a31d)
A través de esta plataforma también podemos ejecutar el envío de datos cuando nosotros deseemos. 

Recibiremos los mensajes en los tópicos en Node-Red utilizando nodos "MQTT in", configurando 4 diferentes:
![image](https://github.com/user-attachments/assets/6b77c5e2-ebe4-442f-b9b5-ead17b5ba6cd)

Los mensajes recibidos en cada uno de ellos serán la entrada de nodos de funciones de python, con los que obtendremos, de todo el "payload" el dato que nos interesa del sensor:
![image](https://github.com/user-attachments/assets/afa37959-aea7-4abf-8b89-7284f29e9a87)

Finalmente, los datos procesados los utilizamos el diferentes nodos del dashboard (Texto, Gráfico de líneas y de Barras) para su visualización histórica y presente:
- Los datos de presencia (Motion) se muestra solo el estado actual
- Los datos de puerta abierta, ventana o puerta rota y botón de pánico pulsado se muestran históricamente desde el inicio de la ejecución
- Además, incorporamos un botón que al pulsarse envía un mensaje a través de un tópico MQTT para activar una alarma que sería instalada.

![image](https://github.com/user-attachments/assets/8c417883-2338-4864-9b3c-ff37e2db9ca1)
![image](https://github.com/user-attachments/assets/f6c4b98e-019a-47d2-991d-96ba2c46a7e3)




