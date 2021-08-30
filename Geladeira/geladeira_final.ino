/**
/*Geladeira Inteligente 
 * Autor: Rodrigo Badega
 * Ultima modificacao: 06/08/21 17:20
 * Descricao: programa com o objetivo de requisitar do site uma informacao(xlm) 
 * para a analise da data de validade dos alimentos. Consegue acessar sites
 * com conexao TLS. É capaz de processar os dados a partir de um controlador fuzzy.
 * Além disso é coordenado em tempo real por meio de um protocolo NTP
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <tinyxml2.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <TimeAlarms.h>
#include <Fuzzy.h>

using namespace tinyxml2;

ESP8266WiFiMulti WiFiMulti;

// Fuzzy
Fuzzy *fuzzy = new Fuzzy();

// FuzzyInput: Temperatura
FuzzySet *tempbaixa = new FuzzySet(0, 0, 10, 20);
FuzzySet *tempmedia = new FuzzySet(10, 20, 20, 30);
FuzzySet *tempalta = new FuzzySet(20, 30, 50, 50);

// FuzzyInput: Maturacao
FuzzySet *matbaixa = new FuzzySet(0, 0, 2, 4);
FuzzySet *matmedia = new FuzzySet(2, 4, 4, 6);
FuzzySet *matalta = new FuzzySet(4, 6, 10, 10);

// FuzzyOutput: Tempo de Conservacao
FuzzySet *tempobaixo = new FuzzySet(0, 5, 10, 15);
FuzzySet *tempomedio = new FuzzySet(10, 15, 15, 20);
FuzzySet *tempoalto = new FuzzySet(15, 20, 25, 30);
////////////////////////////////////////////////////////////////////////////////

const char ssid[] = "Dejanice";  //  your network SSID (name)
const char pass[] = "deja1843";       // your network password

// NTP Servers:
static const char ntpServerName[] = "a.st1.ntp.br";
//static const char ntpServerName[] = "gps.ntp.br";
//static const char ntpServerName[] = "us.pool.ntp.org";

const int timeZone = -3;     //Horario Brasilia

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
////////////////////////////////////////////////////////////////////////////////



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //while (!Serial) ; // Needed for Leonardo only
  delay(250);
  Serial.println("Geladeira Final");
  Serial.print("Connecting to Wi-Fi");
  //Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

//  Serial.print("IP number assigned by DHCP is ");
//  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
//  Serial.print("Local port: ");
//  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  Serial.println("Conectando ao Site");
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Dejanice", "deja1843");

  randomSeed(analogRead(0));//set random seed

  // FuzzyInput
  FuzzyInput *temperatura = new FuzzyInput(1);

  temperatura->addFuzzySet(tempbaixa);
  temperatura->addFuzzySet(tempmedia);
  temperatura->addFuzzySet(tempalta);
  fuzzy->addFuzzyInput(temperatura);

  // FuzzyInput
  FuzzyInput *maturacao = new FuzzyInput(2);

  maturacao->addFuzzySet(matbaixa);
  maturacao->addFuzzySet(matmedia);
  maturacao->addFuzzySet(matalta);
  fuzzy->addFuzzyInput(maturacao);
  
//SAIDA
  // FuzzyOutput
  FuzzyOutput *tempo = new FuzzyOutput(1);

  tempo->addFuzzySet(tempobaixo);
  tempo->addFuzzySet(tempomedio);
  tempo->addFuzzySet(tempoalto);
  fuzzy->addFuzzyOutput(tempo);

  // Building FuzzyRule
  //mat alta => tempo baixo(1)
  FuzzyRuleAntecedent *input_matalta = new FuzzyRuleAntecedent();
  input_matalta->joinSingle(matalta);

  FuzzyRuleConsequent *output_thentempobaixo = new FuzzyRuleConsequent();
  output_thentempobaixo->addOutput(tempobaixo);

  FuzzyRule *fuzzyRule1 = new FuzzyRule(1, input_matalta, output_thentempobaixo);
  fuzzy->addFuzzyRule(fuzzyRule1);
  
  //mat e temp baixa => tempo alto(2)
  FuzzyRuleAntecedent *input_tempbaixaandmatbaixa = new FuzzyRuleAntecedent();
  input_tempbaixaandmatbaixa->joinWithAND(tempbaixa, matbaixa);
  
  FuzzyRuleConsequent *output_thentempoalto = new FuzzyRuleConsequent();
  output_thentempoalto->addOutput(tempoalto);  

  FuzzyRule *fuzzyRule2 = new FuzzyRule(2, input_tempbaixaandmatbaixa, output_thentempoalto);
  fuzzy->addFuzzyRule(fuzzyRule2);
  
  //mat baixa e temp media => tempo medio(3)
  FuzzyRuleAntecedent *input_tempmediaandmatbaixa = new FuzzyRuleAntecedent();
  input_tempmediaandmatbaixa->joinWithAND(tempmedia, matbaixa);

  FuzzyRuleConsequent *output_thentempomedio = new FuzzyRuleConsequent();
  output_thentempomedio->addOutput(tempomedio);  

  FuzzyRule *fuzzyRule3 = new FuzzyRule(3, input_tempmediaandmatbaixa, output_thentempomedio);
  fuzzy->addFuzzyRule(fuzzyRule3);

  //mat baixa e temp alta => tempo baixo(4)
  FuzzyRuleAntecedent *input_tempaltaandmatbaixa = new FuzzyRuleAntecedent();
  input_tempaltaandmatbaixa->joinWithAND(tempalta, matbaixa);

  FuzzyRule *fuzzyRule4 = new FuzzyRule(4, input_tempaltaandmatbaixa, output_thentempobaixo);
  fuzzy->addFuzzyRule(fuzzyRule4);

  //mat media e temp baixa => tempo alto(5)
  FuzzyRuleAntecedent *input_tempbaixaandmatmedia = new FuzzyRuleAntecedent();
  input_tempbaixaandmatmedia->joinWithAND(tempbaixa, matmedia);

  FuzzyRule *fuzzyRule5 = new FuzzyRule(5, input_tempbaixaandmatmedia, output_thentempoalto);
  fuzzy->addFuzzyRule(fuzzyRule5);

  //mat media e temp med => tempo medio(6)
  FuzzyRuleAntecedent *input_tempmediaandmatmedia = new FuzzyRuleAntecedent();
  input_tempmediaandmatmedia->joinWithAND(tempmedia, matmedia);

  FuzzyRule *fuzzyRule6 = new FuzzyRule(6, input_tempmediaandmatmedia, output_thentempomedio);
  fuzzy->addFuzzyRule(fuzzyRule6);

  //mat media e temp alta => tempo baixo(7)
  FuzzyRuleAntecedent *input_tempaltaandmatmedia = new FuzzyRuleAntecedent();
  input_tempaltaandmatmedia->joinWithAND(tempalta, matmedia);

  FuzzyRule *fuzzyRule7 = new FuzzyRule(7, input_tempaltaandmatmedia, output_thentempobaixo);
  fuzzy->addFuzzyRule(fuzzyRule7);
  
}

time_t prevDisplay = 0; // when the digital clock was displayed

void parserXml(String payload){
  
  XMLDocument xmlDoc;
  if(xmlDoc.Parse(payload.c_str()) != XML_SUCCESS){
    Serial.println("Error parsing!");
    return;
    }
  XMLNode *root = xmlDoc.LastChild();
  XMLElement *element = root->FirstChildElement("gaveta");
  float num_gaveta;
  const char* status_gav;
  int validade;
  int restante;
  while(element != NULL){
    if(element->QueryFloatAttribute("num", &num_gaveta) != XML_SUCCESS){//armazena qual o num da gaveta
      Serial.println("COULDN'T OBTAIN THE ATTRIBUTE!");
      return;
    }
    if(element->QueryStringAttribute("status", &status_gav) != XML_SUCCESS){//armazena qual o status da gaveta
      Serial.println("COULDN'T OBTAIN THE ATTRIBUTE!");
      return;
    }
    if(element->QueryIntAttribute("validade", &validade) != XML_SUCCESS){//armazena qual a validade do produto da gaveta
      Serial.println("COULDN'T OBTAIN THE ATTRIBUTE!");
      return;
    }
    if(element->QueryIntAttribute("restante", &restante) != XML_SUCCESS){//armazena qual o tempo restante do produto
      Serial.println("COULD'NT OBTAIN THE ATTRIBUTE! YOU ARE JUST STUPID!!");
      return;
    }
    //exibe os valores no monitor serial
    Serial.printf("Numero da gaveta: %f\n",num_gaveta);
    Serial.printf("Status: %s\n",status_gav);
    Serial.printf("Prazo de validade: %d\n",validade);
    Serial.printf("Tempo restante: %d\n",restante);
    element = element->NextSiblingElement("gaveta");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
  Alarm.delay(1000); // wait one second between clock display

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
          //Serial.println(payload);
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

//  Serial.println("Wait 10s before next round...");
//  delay(10000);
int temperatura = random(0,50);//simulando temperatura
  int maturacao = random(0,10);//simulando maturacao
  
  fuzzy->setInput(1, temperatura);
  fuzzy->setInput(2, maturacao);

  fuzzy->fuzzify();
  delay(1000);
  Serial.println("\n\nInput: ");
//  Serial.print("\tTemperatura: Baixa-> ");
//  Serial.print(tempbaixa->getPertinence());
//  Serial.print(", Media-> ");
//  Serial.print(tempmedia->getPertinence());
//  Serial.print(", Alta-> ");
//  Serial.println(tempalta->getPertinence());
//
//  Serial.print("\tMaturacao: Baixa-> ");
//  Serial.print(matbaixa->getPertinence());
//  Serial.print(", Media-> ");
//  Serial.print(matmedia->getPertinence());
//  Serial.print(", Alta-> ");
//  Serial.println(matalta->getPertinence());
    Serial.print("Temperatura:");
    Serial.println(temperatura);
    Serial.print("Maturacao:");
    Serial.println(maturacao);
  float output1 = fuzzy->defuzzify(1);
  float pertinencetempobaixo, pertinencetempomedio, pertinencetempoalto;
  pertinencetempobaixo = tempobaixo->getPertinence();
  pertinencetempomedio = tempomedio->getPertinence();
  pertinencetempoalto = tempoalto->getPertinence();
  
  Serial.println("Result: ");
  Serial.print("\t\t\tTempo de Conservacao do Alimento: ");
  Serial.print(output1);
  if(pertinencetempoalto > pertinencetempomedio){
    if(pertinencetempoalto > pertinencetempobaixo){
      Serial.print("(Verde)");  
    }
    else{
      Serial.print("(Vermelho)");
    }
  }
  else{
    if(pertinencetempomedio > pertinencetempobaixo){
      Serial.print("(Amarelo)");  
    }
    else{
      Serial.print("(Vermelho)");
    }
  }
  Serial.print("\n");
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void OnceOnly() {
  Serial.println("Alarme -----------------------------------------------------");
  // use Alarm.free() to disable a timer and recycle its memory.
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
