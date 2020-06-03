#include <U2F_HID.h>
#include <string.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.write("up!");
  Serial.println();
}

U2FHID_FRAME *p = malloc(sizeof(U2FHID_FRAME));

void dump(String d, uint8_t *p, uint8_t len) {
  uint8_t i;
  Serial.print(d);
  Serial.write(": ");
  for (i = 0; i < len; i++) {
    Serial.print(p[i], HEX);
    Serial.write(" ");
  }
  Serial.println();
}

void initChannel(U2FHID_INIT_REQ *req) {
  //dump("nonce", req->nonce, 8);
  uint32_t cid = 0xcafebabe;
  U2FHID_INIT_RESP r = {0};
  memcpy(&(r.cid), &cid, 4);
  memcpy(&(r.nonce), req->nonce, 8);

  U2FHID_FRAME f = { CID_BROADCAST };
  f.type = U2FHID_INIT;
  f.init.bcntl = 17;
  memcpy(&(f.init.data), &r, 17);
  
  U2F_HID.SendReport(&f);
}

void handlePacket() {
  switch (p->init.cmd) {
    case U2FHID_INIT:
      Serial.write("init\n");
      initChannel((U2FHID_INIT_REQ*)p->init.data);
      break;
    default:
      Serial.write("default\n");
      break;
  }
}

void loop() {
  U2F_HID.begin();
  memset(p, 0, sizeof(U2FHID_FRAME));
  if(U2F_HID.RecvRaw(p) > 0) {
    dump("recv", (uint8_t*)p, 64);
    //memcpy(&f,p,64);
    handlePacket();
  }
}
