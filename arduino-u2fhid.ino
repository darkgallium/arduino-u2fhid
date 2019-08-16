#include <U2F_HID.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.write("up!");
  Serial.println();
}

uint8_t *p = malloc(64*sizeof(uint8_t));

void dump(uint8_t *p, uint8_t len) {
  uint8_t i;
  for (i = 0; i < len; i++) {
    Serial.print(p[i], HEX);
    Serial.write(" ");
  }
  Serial.println();
}

void loop() {
  U2F_HID.begin();
  if(U2F_HID.RecvRaw(&p) > 0) {
    dump(p, 64);
  }
}
