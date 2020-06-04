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
  //r.capFlags = CAPFLAG_WINK;

  U2FHID_FRAME f = { CID_BROADCAST };
  f.type = U2FHID_INIT;
  f.init.bcntl = 17;
  memcpy(&(f.init.data), &r, 17);
  
  U2F_HID.SendReport(&f);
}

void handlePing() {
  U2FHID_FRAME f = { p->cid };
  f.type = U2FHID_PING;
  f.init.bcntl = p->init.bcntl;
  memcpy(&(f.init.data), p->init.data, p->init.bcntl);
  
  U2F_HID.SendReport(&f);
}

void handleMessage() {
  uint8_t size = p->init.bcntl;
  uint8_t data[(size%64)+64] = {0};
  uint8_t expectedSeq = 0;
  uint8_t sz; 

  U2FHID_FRAME f;

  memcpy(data, p->init.data, 57);

  if (size > 57) { // content doesnt fit on a single init packet, need to read continuation
    // TODO: for now, we assume packets arrive in order which is not the case, need to do some reordering
    
    for (sz = 57; sz < size; sz += 59) {
        while(U2F_HID.RecvRaw(&f) <= 0);
        //assert that it is a continuation
        // assert f.cont.seq == expectedseq
        memcpy(data+sz, f.cont.data, 59);
    }
  }

  dump("msg dump", data, size);
}

void handlePacket() {
  // TODO: refuse continuation pack without prior init pack
  
  switch (p->init.cmd) {
    case U2FHID_MSG:
      Serial.write("type: msg\n");
      handleMessage();
      break;
    case U2FHID_INIT:
      Serial.write("type: init\n");
      initChannel((U2FHID_INIT_REQ*)p->init.data);
      break;
    case U2FHID_PING:
      Serial.write("type: ping\n");
      handlePing();
      break;
    default:
      Serial.write("type: not implemented\n");
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
