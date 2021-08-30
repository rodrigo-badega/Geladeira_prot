/**
/*Geladeira Inteligente 
 * Autor: Rodrigo Badega
 * Ultima modificacao: 01/12/20 14:10
 * Descricao: programa com o objetivo de requisitar do site uma informacao(xlm) 
 * para a analise da data de validade dos alimentos. Consegue acessar sites
 * com conexao TLS.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <tinyxml2.h>
 
using namespace tinyxml2;
// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
//const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

ESP8266WiFiMulti WiFiMulti;

void setup() {

  Serial.begin(9600);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Dejanice", "deja1843");

//  XMLDocument xmlDoc;
//     if(xmlDoc.Parse(payload) != XML_SUCCESS){
//      Serial.println("Error parsing! YOU ARE JUST STUPID!!");
//      return;
//      }
//     XMLNode *root = xmlDoc.FirstChild();
}

void parserXml(String payload){
  
  XMLDocument xmlDoc;
  if(xmlDoc.Parse(payload.c_str()) != XML_SUCCESS){
    Serial.println("Error parsing! YOU ARE JUST STUPID!!");
    return;
    }
  XMLNode *root = xmlDoc.LastChild();
  XMLElement *element = root->FirstChildElement("gaveta");
  float num_gaveta;
  const char* status_gav;
  int validade;
  int restante;
  while(element != NULL){
    if(element->QueryFloatAttribute("num", &num_gaveta) != XML_SUCCESS){
      Serial.println("COULD'NT OBTAIN THE ATTRIBUTE! YOU ARE JUST STUPID!!");
      return;
    }
    if(element->QueryStringAttribute("status", &status_gav) != XML_SUCCESS){
      Serial.println("COULD'NT OBTAIN THE ATTRIBUTE! YOU ARE JUST STUPID!!");
      return;
    }
    if(element->QueryIntAttribute("validade", &validade) != XML_SUCCESS){
      Serial.println("COULD'NT OBTAIN THE ATTRIBUTE! YOU ARE JUST STUPID!!");
      return;
    }
    if(element->QueryIntAttribute("restante", &restante) != XML_SUCCESS){
      Serial.println("COULD'NT OBTAIN THE ATTRIBUTE! YOU ARE JUST STUPID!!");
      return;
    }
    Serial.printf("Numero da gaveta: %f\n",num_gaveta);
    Serial.printf("Status: %s\n",status_gav);
    Serial.printf("Prazo de validade: %d\n",validade);
    Serial.printf("Tempo restante: %d\n",restante);
    element = element->NextSiblingElement("gaveta");
  }
  
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    
//    client->setFingerprint(fingerprint); Serve  para validar o certificado do site
    client->setInsecure();
    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://webgelateljava.herokuapp.com/appalimentos?username=admin@admin&password=123admin123")) {  // HTTPS
 
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
          parserXml(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}
