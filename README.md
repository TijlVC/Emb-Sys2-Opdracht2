# **Opdracht 2: GTK + MQTT + PJ_RPI_USER**

Deze opdracht bestaat uit een basisgedeelte en een mogelijke uitbreiding.

---

## **1. Basis**

### **Deel 1: GTK Programma**
Het programma moet:  
- Enkele GPIO's kunnen lezen en schrijven.  
- De mogelijkheid bieden om de gewenste IO’s te kiezen.  

### **Deel 2: MQTT en Node-RED**
#### **MQTT met Paho Library**
- De temperatuur van de TC74-sensor via MQTT ontvangen en naar GTK sturen voor visualisatie.  
- Gebruik maken van de Paho-library om gegevens te ontvangen en te verzenden.  

#### **Node-RED**
- De temperatuur via Node-RED publiceren.  

---

## **2. Uitbreiding (Optioneel)**

### **✅ Timer Functie**
- De output laten togglen via een instelbare timer in GTK.  

### **✅ Start Service**
- Een C-service maken die gestart kan worden met `systemctl`.  
- Deze service publiceert de temperatuur van de TC74 via MQTT met behulp van de Paho-library.  

---

## **3. Build & Run**

### **📌 Stap 1: Installeer vereisten**
#### **Installeer GTK3**
```bash
sudo apt-get update
sudo apt-get install libgtk-3-dev
```

#### **Installeer PJ_RPI_USER**
- Volg de instructies van de repository: [hans-naert/PJ_RPI_USER](https://github.com/hans-naert/PJ_RPI_USER).  
- Hiermee krijg je directe toegang tot `/dev/gpiomem` zonder `sudo`.  

#### **Installeer Paho MQTT**
```bash
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
cmake -Bbuild -H.
sudo cmake --build build/ --target install
sudo ldconfig
```

---

### **📌 Stap 2: Builden**
```bash
mkdir build
cd build
cmake ..
make
```

---

### **📌 Stap 3: Uitvoeren**
```bash
./opdracht2
```

---

### **📌 Stap 4: MQTT testen (indien nodig)**
Indien Node-RED nog niet operationeel is, kun je handmatig een MQTT-bericht verzenden:  
```bash
mosquitto_pub -t tc74/temperature -m "25"
```
*Hiermee push je handmatig een MQTT-bericht.*

---

## **4. GPIO Werking**

### **📌 Input Pins (BCM)**
- Voer het BCM-nummer in (bijv. 27) en klik op “Read” om de huidige status (0 of 1) te lezen.  
- Wil je een stabiel signaal? Configureer een pull-up of pull-down:  
  ```bash
  sudo raspi-gpio set 27 pu
  ```

### **📌 Output Pins (BCM)**
- Voer een BCM-nummer in (bijv. 17) en klik op “Toggle” om de pin hoog of laag te zetten.  
- Het label in de GUI toont of de pin “HIGH” of “LOW” is.  
- Gebruik indien nodig een LED + weerstand of een andere schakeling om de stand te verifiëren.  

---

## **5. Node-RED Flow (TC74-sensor)**

De TC74-temperatuur live publiceren naar MQTT (topic `tc74/temperature`) via Node-RED:

### **📌 Stap 1: Start Node-RED**
```bash
node-red
```
*(Of gebruik: `sudo systemctl start nodered`)*  

---

### **📌 Stap 2: Open de Node-RED flow-editor**
Open de browser en ga naar:  
```bash
http://<IP-van-je-Pi>:1880
```

---

### **📌 Stap 3: Flow Configureren**
Maak een nieuwe flow en voeg de volgende nodes toe:

1. **Inject Node**:  
   - Type: `msg.payload` als STRING  
   - Interval: Elke `5 seconden`  

2. **Exec Node**:  
   - Command:  
     ```bash
     i2cget -y 1 0x48
     ```
   - Output: "When the command is complete - exec mode"

3. **Functie Node** (Naam: "Omzetting hex naar integer"):  
   ```javascript
   // Hexadecimale waarde omzetten naar integer
   let tempHex = msg.payload.trim();
   let tempC = parseInt(tempHex, 16);
   
   // Formatteerde waarde doorsturen
   msg.payload = tempC;
   
   return msg;
   ```

4. **MQTT Out Node**:  
   - Instellen op `Server: localhost`  
   - Poort: `1883`  

5. **Deploy**:  
   - Klik rechtsboven op "Deploy" om de flow te starten.  

---

### **✅ Extra Tips**
- Controleer of `mosquitto` draait met:
  ```bash
  sudo systemctl status mosquitto
  ```
- Je kunt MQTT-berichten live monitoren met:
  ```bash
  mosquitto_sub -t "tc74/temperature"
  ```

---
