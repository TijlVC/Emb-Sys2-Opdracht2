# Opdracht 2 GTK + MQTT + PJ_RPI_USER

Deze opdracht bestaat uit een basis gedeelte en een mogelijke uitbereiding.

---

## Basis

### Deel 1:
**GTK programma maken waarmee:**:  
    -  Enkele GPIO'skan lezen en schrijven.  
    -  Maak het mogelijk om de IO'ste kiezen.  

### Deel 2:

**MQTT met Paho lib**:
    - De temperatuur van de TC74 sensor via MQTT wordt ontvangen om naar de GTK te versturen voor visualisatie  
    - Ontvangen en versturen met de Paho lib om te kunnen visualiseren  

**Node-Red**
    - Publiceren van de temperatuur via Node-Red  

## Uitbereiding:

**Timer voorzien**
    - De output laten togglen via een timer met instelbare periode (GTK)  

**Start Service**
    - Een service maken in C die kan gestart worden met systemctl  
    -> de service dient om de temperatuur van de TC74 via MQTT met de paho lib te publiceren.  

# Build & Run

1. Installeer eerst GTK3:
   ```commandline :
   sudo apt-get update
   sudo apt-get install libgtk-3-dev
   ````  

2. Installeer PJ_RPI_USER (zie hans-naert/PJ_RPI_USER)
    - Hiermee heb je /dev/gpiomem voor directe GPIO-toegang zonder sudo.  

3. Installeer Paho MQTT
    ```bash
    git clone https://github.com/eclipse/paho.mqtt.c.git
    cd paho.mqtt.c
    cmake -Bbuild -H.
    sudo cmake --build build/ --target install
    sudo ldconfig
    ````  

4. Builden: 
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ````  

5. Uitvoeren :
    ```bash
    ./opdracht2
    ````  

6. Indien Node-red nog niet operationeel kun je MQTT testen : (in een andere terminal)
    ```bash
    mosquitto_pub -t tc74/temperature -m "25"
    ````
*hiermee push je manueel een mqtt bericht.*  

## GPIO Werking

**In de GUI kun je:**  
    - Input pin (BCM): Voer het BCM-nummer in (bijv. 27) en klik op “Read” om de huidige status (0 of 1) te lezen.  
    - Wil je een stabiel signaal, configureer dan een pull-up of pull-down, bv  
    ```bash    
    sudo raspi-gpio set 27 pu  
    ````  
    - Output pin (BCM): Voer een BCM-nummer in (bijv. 17) en klik op “Toggle” om de pin hoog/laag te zetten.  
    - Het label in de GUI toont of de pin “HIGH” of “LOW” is.  
    Gebruik indien nodig een LED + weerstand of een andere schakeling om de stand te zien.  

*Zo kun je vrij bepalen welke pinnen je als input of output gebruikt.*

## Node-RED Flow (TC74-sensor)

Om de TC74 temperatuur live te publiceren naar MQTT (topic `tc74/temperature`), kun je Node-RED als volgt configureren (op de Raspberry Pi):

1. **Start Node-RED**:
   ```bash
   node-red
   ´´´´  
(of gebruik ´´´commandline : sudo systemctl start nodered ´´´´)

2. **OPen de flow-editor in je browser:**
    ´´´bash
    http://<IP-van-je-Pi>:1880
    ´´´´  

3. Maak een nieuw flow aan met volgende nodes in deze volgorde:
    - Inject node "msg.payload" als STRING met een (repeat 5s). Verbind deze met een Exec node  
    - Exec node met als command: ´´´commandline : i2cget -y 1 0x48 ´´´´ en Output 'when the command is complete - exec mode'  
    - Deze exec is dan verbonden met een functie-node : noem deze bv: "Omzetting hex to integer"  
    ´´´bash
	// Extract hexadecimal value from output
	let tempHex = msg.payload.trim();  

	// Convert hex (e.g., "0x1A") to integer
	let tempC = parseInt(tempHex, 16);  

	// Set the formatted payload
	msg.payload = tempC;  
	
	return msg;  
	´´´´  
    - Deze functienode is dan verbonden met een mqtt-out-node met als instelling 'server: localhost  Port:1883'  
    - druk rechtsbovenaan op deploy om de flow te starten.  