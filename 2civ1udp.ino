#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncUDP.h>

/*********************** Custom Config ***********/
#define SSID "Livebox-****"  // SSID de la box
#define PASSWD "********" // wiFi password
#define HOSTNAME "F1RTD-2IC1UDP" //device hostname
#define UDP_PORT 27000 // N° de Port UDP 
#define ICOM1_ADDRESS 0x94 //Adresse CI-V Tx1 (ic7300)
#define ICOM2_ADDRESS 0xA2 //Adresse CI-V Tx2 (ic9700)
/**************************************************/

#define debug Serial
#define UDP_LED 22
#define WIFI_LED 23
#define BUFFERSIZE 256

#define UART_BAUD1 19200         
#define SERIAL_PARAM1 SERIAL_8N1  
#define SERIAL1_TXPIN 17      
#define SERIAL1_RXPIN 16
#define SERIAL1_LEDTX 5

#define UART_BAUD2 19200        
#define SERIAL_PARAM2 SERIAL_8N1   
#define SERIAL2_TXPIN 4    
#define SERIAL2_RXPIN 15 
#define SERIAL2_LEDTX 0

AsyncUDP udp;
HardwareSerial Serial_one(1);
HardwareSerial Serial_two(2);
HardwareSerial* COM[2] = { &Serial_one, &Serial_two};
uint8_t buffer[2][128];
uint16_t ix[2] = {0, 0};
uint8_t serial_led_tx [2] =  {SERIAL1_LEDTX,SERIAL2_LEDTX};
uint8_t icom_adress [2] =  {ICOM1_ADDRESS, ICOM2_ADDRESS};

void connect(){  
  uint8_t count = 0; 
  debug.printf("Connect to %s ...\n", SSID);  
    
  WiFi.begin(SSID, PASSWD);
  while (WiFi.status() != WL_CONNECTED) { 
    digitalWrite(WIFI_LED,!digitalRead(WIFI_LED));      
    delay(500);
    debug.print("*"); 
    count ++; 
    if (count == 40 && WiFi.status() != WL_CONNECTED){
      debug.print("\nTimeout Wifi Connect : System restart !\n");
      ESP.restart();
    }
  }

  digitalWrite(WIFI_LED,HIGH);
  debug.println("\nWifi connected."); 
  debug.println(WiFi.localIP());


  bool listening = false;
  if (udp.listen(UDP_PORT)){ 
    listening = true;  
    debug.printf("Listening on port %d\n", UDP_PORT);
  }

  if (listening) {
    udp.onPacket([](AsyncUDPPacket packet) {
      digitalWrite(UDP_LED,HIGH);
      delay(5);
      for (uint16_t i = 0; i < 2; i++){                 
        if (packet.data()[2] == icom_adress[i]){                              
          COM[i]->write(packet.data(), packet.length());
          break;
        }
      }      
      digitalWrite(UDP_LED,LOW);               
    });
  }

}

void WiFi_Disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  debug.print("\nWiFi disconnected !\n");
  connect();
}
    
void setup() { 
  delay(500); 
  Serial.begin(9600);   
  pinMode (serial_led_tx[0],OUTPUT);  
  pinMode (serial_led_tx[1],OUTPUT);  
  pinMode (UDP_LED,OUTPUT);   
  pinMode (WIFI_LED,OUTPUT); 
  digitalWrite(serial_led_tx[0],LOW);
  digitalWrite(serial_led_tx[1],LOW);  
  digitalWrite(UDP_LED,LOW);
  digitalWrite(WIFI_LED,LOW);  
 
  COM[0]->begin(UART_BAUD1, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
  COM[1]->begin(UART_BAUD2, SERIAL_PARAM2, SERIAL2_RXPIN, SERIAL2_TXPIN);

  debug.print("\n\nDual Port Icom CI-V <-> UDP\nF1RTD - 2024\n");

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.onEvent(WiFi_Disconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  connect();
  esp_err_t esp_wifi_set_max_tx_power(50);  
}

void loop() {
  for (int i = 0; i < 2; i++) {       
    if (COM[i] != NULL) {           
      if (COM[i]->available()) {
        digitalWrite(serial_led_tx[i],HIGH);               
        while (COM[i]->available()) {
          buffer[i][ix[i]] = COM[i]->read();
          ix[i]++;
          if (buffer[i][ix[i]-1] == 253){          
            udp.broadcastTo(buffer[i], ix[i], UDP_PORT);
            ix[i] = 0;                        
            memset(buffer[i], 0, sizeof(buffer[i]));           
            break;
          }         
        }
      digitalWrite(serial_led_tx[i],LOW);
      }      
    }
  }
}
