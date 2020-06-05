#include <uECC.h>
#include <types.h>
#include <uECC_vli.h>

#include <U2F_HID.h>
#include <string.h>

#define DEBUG

extern "C" {

static int RNG(uint8_t *dest, unsigned size);

static int RNG(uint8_t *dest, unsigned size) {
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of 
  // random noise). This can take a long time to generate random data if the result of analogRead(0) 
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      int init = analogRead(0);
      int count = 0;
      while (analogRead(0) == init) {
        ++count;
      }
      
      if (count == 0) {
         val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

}  // extern "C"

// U2F MSG spec: https://fidoalliance.org/specs/fido-u2f-v1.0-ps-20141009/fido-u2f-raw-message-formats-ps-20141009.html
// Google's implem : https://github.com/google/tock-on-titan/blob/master/userspace/u2f_app/u2f.c

// attestation stuff: https://github.com/solokeys/solo/blob/530e175ad13e065767f277314f821d702532447b/fido2/device.c

const uint8_t attestation_cert_der[] PROGMEM = 
"\x30\x82\x01\xfb\x30\x82\x01\xa1\xa0\x03\x02\x01\x02\x02\x01\x00\x30\x0a\x06\x08"
"\x2a\x86\x48\xce\x3d\x04\x03\x02\x30\x2c\x31\x0b\x30\x09\x06\x03\x55\x04\x06\x13"
"\x02\x55\x53\x31\x0b\x30\x09\x06\x03\x55\x04\x08\x0c\x02\x4d\x44\x31\x10\x30\x0e"
"\x06\x03\x55\x04\x0a\x0c\x07\x54\x45\x53\x54\x20\x43\x41\x30\x20\x17\x0d\x31\x38"
"\x30\x35\x31\x30\x30\x33\x30\x36\x32\x30\x5a\x18\x0f\x32\x30\x36\x38\x30\x34\x32"
"\x37\x30\x33\x30\x36\x32\x30\x5a\x30\x7c\x31\x0b\x30\x09\x06\x03\x55\x04\x06\x13"
"\x02\x55\x53\x31\x0b\x30\x09\x06\x03\x55\x04\x08\x0c\x02\x4d\x44\x31\x0f\x30\x0d"
"\x06\x03\x55\x04\x07\x0c\x06\x4c\x61\x75\x72\x65\x6c\x31\x15\x30\x13\x06\x03\x55"
"\x04\x0a\x0c\x0c\x54\x45\x53\x54\x20\x43\x4f\x4d\x50\x41\x4e\x59\x31\x22\x30\x20"
"\x06\x03\x55\x04\x0b\x0c\x19\x41\x75\x74\x68\x65\x6e\x74\x69\x63\x61\x74\x6f\x72"
"\x20\x41\x74\x74\x65\x73\x74\x61\x74\x69\x6f\x6e\x31\x14\x30\x12\x06\x03\x55\x04"
"\x03\x0c\x0b\x63\x6f\x6e\x6f\x72\x70\x70\x2e\x63\x6f\x6d\x30\x59\x30\x13\x06\x07"
"\x2a\x86\x48\xce\x3d\x02\x01\x06\x08\x2a\x86\x48\xce\x3d\x03\x01\x07\x03\x42\x00"
"\x04\x45\xa9\x02\xc1\x2e\x9c\x0a\x33\xfa\x3e\x84\x50\x4a\xb8\x02\xdc\x4d\xb9\xaf"
"\x15\xb1\xb6\x3a\xea\x8d\x3f\x03\x03\x55\x65\x7d\x70\x3f\xb4\x02\xa4\x97\xf4\x83"
"\xb8\xa6\xf9\x3c\xd0\x18\xad\x92\x0c\xb7\x8a\x5a\x3e\x14\x48\x92\xef\x08\xf8\xca"
"\xea\xfb\x32\xab\x20\xa3\x62\x30\x60\x30\x46\x06\x03\x55\x1d\x23\x04\x3f\x30\x3d"
"\xa1\x30\xa4\x2e\x30\x2c\x31\x0b\x30\x09\x06\x03\x55\x04\x06\x13\x02\x55\x53\x31"
"\x0b\x30\x09\x06\x03\x55\x04\x08\x0c\x02\x4d\x44\x31\x10\x30\x0e\x06\x03\x55\x04"
"\x0a\x0c\x07\x54\x45\x53\x54\x20\x43\x41\x82\x09\x00\xf7\xc9\xec\x89\xf2\x63\x94"
"\xd9\x30\x09\x06\x03\x55\x1d\x13\x04\x02\x30\x00\x30\x0b\x06\x03\x55\x1d\x0f\x04"
"\x04\x03\x02\x04\xf0\x30\x0a\x06\x08\x2a\x86\x48\xce\x3d\x04\x03\x02\x03\x48\x00"
"\x30\x45\x02\x20\x18\x38\xb0\x45\x03\x69\xaa\xa7\xb7\x38\x62\x01\xaf\x24\x97\x5e"
"\x7e\x74\x64\x1b\xa3\x7b\xf7\xe6\xd3\xaf\x79\x28\xdb\xdc\xa5\x88\x02\x21\x00\xcd"
"\x06\xf1\xe3\xab\x16\x21\x8e\xd8\xc0\x14\xaf\x09\x4f\x5b\x73\xef\x5e\x9e\x4b\xe7"
"\x35\xeb\xdd\x9b\x6d\x8f\x7d\xf3\xc4\x3a\xd7"
;

const uint8_t attestation_key[] PROGMEM = 
"\xcd\x67\xaa\x31\x0d\x09\x1e\xd1\x6e\x7e\x98\x92\xaa"
"\x07\x0e\x19\x94\xfc\xd7\x14\xae\x7c\x40\x8f\xb9\x46"
"\xb7\x2e\x5f\xe7\x5d\x30";

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.write("up!");
  Serial.println();

  // init RNG
  uECC_set_rng(&RNG);
}

U2FHID_FRAME *p = malloc(sizeof(U2FHID_FRAME));


#ifdef DEBUG
void generateBuffer(void *buf, uint8_t len) {
  uint32_t i;
  uint32_t o = 0xcafebabe;
  for (i=0;i<len;i+=4) {
    memcpy(buf+i,&o,4);
  }
}
#endif

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
  U2F_REGISTER_REQ *req = (U2F_REGISTER_REQ *)p->data;
  U2F_REGISTER_RESP r = {0x65};

  const struct uECC_Curve_t *curve = uECC_secp256r1();
  uint8_t privk[32];
  uint8_t pubk[65];
  uint8_t payload[163] = {0};
  uint8_t signature[MAX_ECDSA_SIG_SIZE] = {0};

  Serial.print("generating ECDSA key");
  uECC_make_key(r.pubKey, r.keyHandle, curve);

  memcpy(payload+1, req->appId, U2F_APPID_SIZE);
  memcpy(payload+U2F_APPID_SIZE+2, req->nonce, U2F_NONCE_SIZE);
  memcpy(payload+U2F_APPID_SIZE+U2F_NONCE_SIZE+2, privk, 32);
  memcpy(payload+U2F_APPID_SIZE+U2F_NONCE_SIZE+34, pubk, 65);

  Serial.print("computing payload signature with attestation key");
  uECC_sign(attestation_key, payload, 163, r.signature, curve);

  r.keyHandleLen = 32;
  memcpy(r.attestationCert,attestation_cert_der,sizeof(attestation_cert_der));

  sendMessageToHost(&r, sizeof(U2F_REGISTER_RESP));

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

void sendMessageToHost(void *payload, uint8_t payload_size) {
  uint32_t sz, seq = 0; 
  U2FHID_FRAME f = {0} ,c = {0};

  f.cid = 0xcafebabe;
  f.type = U2FHID_MSG;
  f.init.bcntl = payload_size;
  
  memcpy(f.init.data, payload, 57);
  
  dump("init",(uint8_t*) &f, sizeof(U2FHID_FRAME));
  U2F_HID.SendReport(&f);

  if (payload_size > 57) { // content doesnt fit on a single init packet, need to write continuation

    c.cid = 0xcafebabe;
    
    for (sz = 57; sz < payload_size; sz += 59) {
      memset(&c, 0, sizeof(U2FHID_FRAME));
      c.cont.seq = seq;
      /*while(U2F_HID.RecvRaw(&f) <= 0);
      // assert that it is a continuation
      // assert f.cont.seq == expectedseq
      memcpy(data+sz, f.cont.data, 59);*/
      memcpy(c.cont.data, payload+sz, 59);
      dump("pak",(uint8_t*) &c, sizeof(U2FHID_FRAME));
      U2F_HID.SendReport(&c);
      seq++;
    }
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

void handleHID() {
  // TODO: refuse continuation pack without prior init pack
  
  switch (p->init.cmd) {
    case U2FHID_MSG:
      Serial.write("type: msg\n");
      unpackMessage();
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

uint8_t *b = malloc(64*sizeof(uint8_t));
void loop() {
  U2F_HID.begin();
  /*memset(p, 0, sizeof(U2FHID_FRAME));
  if(U2F_HID.RecvRaw(p) > 0) {
    //dump("recv", (uint8_t*)p, 64);
    //memcpy(&f,p,64);
    handleHID();
  }*/
  generateBuffer(b,128);
  dump("buffer", b, 128);
  sendMessageToHost(b,128);
  delay(10000);
}
