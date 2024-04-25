#define buzzer 21
#define botao 14

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(botao,INPUT);
}

void loop() {
  if(digitalRead(botao)==HIGH){
    digitalWrite(buzzer,HIGH);
  }
  else{
    digitalWrite(buzzer,LOW);
  }
}
