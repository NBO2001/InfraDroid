
#include <Arduino.h>
#include "iD_config.h"

//biblioteca do IR
#include <IRremote.hpp>


#define DECODE_NEC          
#define DECODE_SAMSUNG      
#define DECODE_DISTANCE_WIDTH 


String inputString = "";
bool stringComplete = false;

void setup() {
    Serial.begin(115200);
    
    IrReceiver.begin(IR_RX_PIN, ENABLE_LED_FEEDBACK);
    IrSender.begin(IR_TX_PIN);
    
    Serial.println("--- SISTEMA ONLINE ---");
    Serial.printf("RX: %d | TX: %d\n", IR_RX_PIN, IR_TX_PIN);
    Serial.println("----------------------");
}

void loop() {
    // --- RECEPÇÃO (RX) ---
    if (IrReceiver.decode()) {
        
        // filtra pra pegar apenas NEC (definido no config) e ignora repeticoes
        if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) && 
            IrReceiver.decodedIRData.protocol == IR_PROTOCOL) {
            
            Serial.print("CAPTURA NEC: 0x");
            Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

            
            
            IrReceiver.printIRResultRawFormatted(&Serial, true); 
            
            Serial.println();
            Serial.println("----------------");
        } 
        IrReceiver.resume(); 
    }

    // --- TRANSMISSÃO (TX) ---
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '\n') {
            stringComplete = true;
        } else {
            inputString += inChar;
        }
    }

    if (stringComplete) {
        processarEnvio();
        inputString = "";
        stringComplete = false;
    }
}

void processarEnvio() {
    
    //METRALHAR o sinal...
    // if (inputString.startsWith("NEC ")) {
    //     String hexCodeStr = inputString.substring(4);
    //     uint32_t hexCode = strtoul(hexCodeStr.c_str(), NULL, 16);
        
    //     Serial.printf("METRALHANDO NEC: 0x%08X\n", hexCode);
        
        
    //     for (int i = 0; i < 5; i++) {
    //         IrSender.sendNEC(hexCode, 32); 
    //         delay(40); 
    //     }
        
    //     Serial.println("Rajada enviada!");
    //     IrReceiver.enableIRIn();
    //     return;
    // }



    //envio do HEX !!!!!! (testanto ainda)
    if (inputString.startsWith("NEC ")) {
        String hexCodeStr = inputString.substring(4);
        uint32_t hexCode = strtoul(hexCodeStr.c_str(), NULL, 16);
        
        Serial.printf("enviando nec: 0x%06X\n", hexCode);
        
        IrSender.sendNEC(hexCode, 32); 
        
        IrReceiver.enableIRIn();
        return;
    }

    //CONSIDERAR ESSE:
    //envio RAW    
    if (inputString.startsWith("SEND ")) {
        String dados = inputString.substring(5);
        uint16_t rawData[400]; 
        int count = 0;
        int startIndex = 0;
        int spaceIndex = dados.indexOf(' ');
        
        int freqkHz = dados.substring(0, spaceIndex).toInt();
        startIndex = spaceIndex + 1;

        while (startIndex < dados.length() && count < 400) {
            spaceIndex = dados.indexOf(' ', startIndex);
            String valStr;
            if (spaceIndex == -1) {
                valStr = dados.substring(startIndex);
                startIndex = dados.length();
            } else {
                valStr = dados.substring(startIndex, spaceIndex);
                startIndex = spaceIndex + 1;
            }
            valStr.replace("+", ""); 
            valStr.replace("-", "");
            rawData[count] = valStr.toInt();
            count++;
        }
        Serial.printf("Enviar RAW (%d pulsos)...\n", count);
        IrSender.sendRaw(rawData, count, freqkHz);
        IrReceiver.enableIRIn(); 
    }
}
