#include "lxml.h"

void setup() {
  // put your setup code here, to run once:
  XMLDocument doc;
    if (XMLDocument_load(&doc, "test.xml")) {
        XMLNode* str = XMLNode_child(doc.root, 0);

        XMLNodeList* fields = XMLNode_children(str, "field");
        for (int i = 0; i < fields->size; i++) {
            XMLNode* field = XMLNodeList_at(fields, i);
            XMLAttribute* type = XMLNode_attr(field, "type");
            type->value = "";
        }

        XMLDocument_write(&doc, "out.xml", 4);
        XMLDocument_free(&doc);
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
