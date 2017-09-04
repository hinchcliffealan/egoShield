#include <egoShield.h>

// REQUIRES JUMPERS INSTALLED FOR 1/16 MICROSTEPPING!

egoShield ego;

void setup() {
  // put your setup code here, to run once:
  ego.setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  ego.loop();
}
