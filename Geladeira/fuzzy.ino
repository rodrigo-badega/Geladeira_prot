#include <Fuzzy.h>

//int temperatura=70;  //Valor lido do XML do site
//int maturacao=0;  //Valor lido do XML do site

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


void setup()
{
  // Set the Serial output
  Serial.begin(9600);
  // Set a random seed
  randomSeed(analogRead(0));

  // Every setup must occur in the function setup()

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

void loop()
{
  int temperatura = random(0,50);//simulando temperatura
  int maturacao = random(0,10);//simulando maturacao
  
  fuzzy->setInput(1, temperatura);
  fuzzy->setInput(2, maturacao);

  fuzzy->fuzzify();
  delay(1000);
  Serial.println("Input: ");
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
  // wait 12 seconds
  delay(12000);
}
