

void initAnalogSwitches() {
  pinMode(VIDswitch, OUTPUT);
  pinMode(OSDswitch, OUTPUT);
  pinMode(progPin, INPUT);

  digitalWrite(VIDswitch, HIGH);
  digitalWrite(OSDswitch, HIGH);
}


bool IRAM_ATTR SetVIDswitch(bool value) {

  if (value) {
    digitalWrite(VIDswitch, HIGH);
  } else {
    digitalWrite(VIDswitch, LOW);
  }


}

bool IRAM_ATTR setOSDswitch(bool value) {
  
  if (value) {
    digitalWrite(OSDswitch, HIGH);
  } else {
    digitalWrite(OSDswitch, LOW);
  }
  
}
