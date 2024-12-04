# IoTParadigmasUSC

Nombre de la solución: HomeWatcher

La idea principal de este dispostivo es la detección de la entrada de personas externas en el interior de un lugar privado. Esto se conseguiría con la actuación de diferentes sensores que nos ayudan a saber el estado de la casa, oficina, ... Pueden ser sensores de detección de presencia, detección de vibraciones por si se intenta forzar una puera o una ventana, detector magnético, para detectar la apertura de puertas sin que sea el dueño, ... y también actuadores en caso de la detección de un agente externo, como buzzers o una alarma.

El grupo está formado por los siguientes miembros:
- Ivanildo 
- Lidia 
- Manuel José
- Raul José


## Node-Red

Hemos desarollado una arquitectura basada en Node-Red para recoger datos de sensores de presencia, apertura y rotura de puertas y ventanas, botones de pánico... y mostrarlos en un dashboard para ver el estado del sistema. 

Comenzamos desarrollando una arquitectura:

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




