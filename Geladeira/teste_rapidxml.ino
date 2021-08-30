#include <tinyxml2.h>

using namespace tinyxml2;

char *testDocument = "<root><element>7</element></root>";
XMLDocument xml;
XMLDocument doc;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  xml.LoadFile("a.xml");
  
//  if(doc.Parse(xml) != XML_SUCCESS)
//    {
//    Serial.println("Error Parsing"); 
//    }
//  XMLNode *root = xml.FirstChild();
//  XMLElement *element = root->FirstChildElement("element");
//  int val;
//  element->QueryIntText(&val);
//  Serial.print(val);
// Text is just another Node to TinyXML-2. The more
  // general way to get to the XMLText:
  const char *retorno;
  XMLText* textNode = doc.FirstChildElement( "project" )->FirstChildElement( "gaveta" )->FirstChild()->ToText();
  retorno = textNode->Value();
  Serial.print(retorno);
}
  
void loop() {
  // put your main code here, to run repeatedly:

}
