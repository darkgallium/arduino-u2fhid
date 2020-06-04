#include <U2F_HID.h>
#include <string.h>

// U2F MSG spec: https://fidoalliance.org/specs/fido-u2f-v1.0-ps-20141009/fido-u2f-raw-message-formats-ps-20141009.html
// Google's implem : https://github.com/google/tock-on-titan/blob/master/userspace/u2f_app/u2f.c

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

void U2F_Register(APDU *p) {
  
}

void handleU2Fmsg(uint8_t *m, uint8_t len) {
  // CLA : always 0x0?
  APDU p = {
    .ins = m[1],
    .p1 = m[2],
    .p2 = m[3],
    .len = 0,
    .data = m + 5
  };

  /* ISO7618 LC decoding */
  if (len >= 5) {
    p.len = m[4];
  } if (p.len == 0 && len >= 7) {
    p.len = (m[5] << 8) | m[6];
    p.data += 2;
  }

  switch (p.ins) {
    case (U2F_INS_REGISTER):
      Serial.print("u2f: register");
      U2F_Register(&p);
      break;
    default:
      Serial.print("u2f: not implemented");
      break;
  }
  
}

void initChannel(U2FHID_INIT_REQ *req) {
  //dump("nonce", req->nonce, 8);
  uint32_t cid = 0xcafebabe; //todo: implement multi channel support
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
  U2FHID_FRAME f = { 0 };
  memcpy(&f, p, sizeof(U2FHID_FRAME)); 
  U2F_HID.SendReport(&f);
}

void unpackMessage() {
  uint8_t size = p->init.bcntl;
  uint8_t data[(size/64)+64] = {0};
  uint8_t expectedSeq = 0;
  uint8_t sz; 

  U2FHID_FRAME f;

  memcpy(data, p->init.data, 57);

  if (size > 57) { // content doesnt fit on a single init packet, need to read continuation
    // TODO: for now, we assume packets arrive in order which is not the case, need to do some reordering
    
    for (sz = 57; sz < size; sz += 59) {
        while(U2F_HID.RecvRaw(&f) <= 0);
        // assert that it is a continuation
        // assert f.cont.seq == expectedseq
        memcpy(data+sz, f.cont.data, 59);
    }
  }

  dump("msg dump", data, size);
  handleU2Fmsg(data, size);
}

void handlePacket() {
  // TODO: refuse continuation pack without prior init pack
  
  switch (p->init.cmd) {
    case U2FHID_MSG:
      Serial.write("type: msg\n");
      handleHIDMessage();
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
    //dump("recv", (uint8_t*)p, 64);
    //memcpy(&f,p,64);
    unpackMessage();
  }
}
