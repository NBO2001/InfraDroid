/*
 * Firmware ESP32 - RX/TX Infravermelho (Versão Compatível)
 * Arquivo: infraDroid_rxtx.ino
 */

#include <Arduino.h>
#include "iD_config.h" 

// Configurações da Biblioteca IRremote
#define DECODE_NEC          
#define DECODE_SAMSUNG      
#define DECODE_DISTANCE_WIDTH 

#include <IRremote.hpp>

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
        
        // Filtra para pegar apenas NEC (definido no config) e ignora repeticoes
        if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) && 
            IrReceiver.decodedIRData.protocol == IR_PROTOCOL) {
            
            Serial.print("CAPTURA NEC: 0x");
            Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

            Serial.println("RAW DATA (Copie os numeros abaixo para o SEND):");
            
            // --- A SOLUÇÃO DO ERRO ESTÁ AQUI ---
            // Em vez de acessar a memória manualmente (que dava erro rawDataPtr),
            // pedimos para a própria biblioteca imprimir os dados.
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
    if (!inputString.startsWith("SEND ")) return;

    String dados = inputString.substring(5);
    uint16_t rawData[400]; // Buffer seguro
    int count = 0;
    int startIndex = 0;
    int spaceIndex = dados.indexOf(' ');
    
    // 1. Pega Frequência
    int freqkHz = dados.substring(0, spaceIndex).toInt();
    startIndex = spaceIndex + 1;

    // 2. Pega os Pulsos
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
        
        // Limpa sinais de + e - que a biblioteca imprime
        valStr.replace("+", ""); 
        valStr.replace("-", "");
        
        rawData[count] = valStr.toInt();
        count++;
    }

    Serial.printf("Enviando %d pulsos pelo pino %d...\n", count, IR_TX_PIN);
    
    IrSender.sendRaw(rawData, count, freqkHz);
    IrReceiver.enableIRIn(); 
}
