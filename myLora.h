#include <SX126x-Arduino.h>
// http://librarymanager/All#SX126x-Arduino

// Define LoRa parameters
uint32_t myFreq = 868125000; // Hz
int8_t TxPower = 22; // dBm
uint32_t myBW = 0; // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
double BWs[10] = {
  125.0, 250.0, 500.0, 62.5, 41.7, 31.25, 20.8, 15.6, 10.4, 7.8
};
uint32_t mySF = 12; // [SF7..SF12]
uint8_t myCR = 1; // [1: 4/5, 2: 4/6,  3: 4/7,  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8  // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0 // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false
#define RX_TIMEOUT_VALUE 15013 // 013 ~ 877
#define TX_TIMEOUT_VALUE 3000

static RadioEvents_t RadioEvents;
time_t cadTime;
uint32_t lastPing = 0;

/**@brief Function that fills a buffer with size random bytes
          Bytes come from the SX1262's TRNG.
          Radio.Random() returns a uint32_t, split over 4 bytes
          Make sure buf's size is a multiple of 4
*/
void fillRandom(uint8_t *buf, uint16_t size) {
  uint8_t ix = 0, ln = size / 4;
  for (uint8_t i = 0; i < ln; i++) {
    uint32_t v = Radio.Random();
    buf[ix++] = (v >> 24) & 0xff;
    buf[ix++] = (v >> 16) & 0xff;
    buf[ix++] = (v >> 8) & 0xff;
    buf[ix++] = v & 0xff;
  }
#ifdef NEED_DEBUG
  hexDump(buf, size);
#endif // NEED_DEBUG
}

/**@brief Function that initializes the LoRa chip
*/
void initLora(void) {
  // Initialize the LoRa chip
  // Start LoRa
#ifdef NEED_DEBUG
  Serial.println("Initialize RAK4631 LoRa");
#endif // NEED_DEBUG
  lora_rak4630_init();
  // Initialize the Radio callbacks
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.RxDone = OnRxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  RadioEvents.RxTimeout = OnRxTimeout;
  RadioEvents.RxError = OnRxError;
  RadioEvents.CadDone = OnCadDone;
  // Initialize the Radio
  Radio.Init(&RadioEvents);
  // Set Radio channel
  Radio.SetChannel(myFreq);
  // Set Radio TX configuration
  // Fill the random buffer first, as after calling this function,
  // either Radio.SetRxConfig or Radio.SetTxConfig functions must be called.
  fillRandom(RandomStock, 256);
  Radio.SetTxConfig(
    MODEM_LORA, TxPower, 0, myBW,
    mySF, myCR,
    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
    true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
  // Set Radio RX configuration
  Radio.SetRxConfig(
    MODEM_LORA, myBW, mySF,
    myCR, 0, LORA_PREAMBLE_LENGTH,
    LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
    0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
  Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Done event
*/
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  Serial.println("OnRxDone");
  delay(10);
  Serial.printf("RssiValue=%d dBm, SnrValue=%d\n", rssi, snr);
  hexDump(payload, size);
  Serial.println("");
  if (needEncryption) {
#ifdef NEED_DEBUG
    Serial.println("\n . Decrypting...");
#endif // NEED_DEBUG
    int16_t packetSize = decryptECB(payload, size);
    if (packetSize > -1) {
      hexDump(RcvBuffer, packetSize);
      Serial.println("");
    } else {
      Serial.println("Error while decrypting");
      return;
    }
  }
  Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Tx Timeout event
*/
void OnTxTimeout(void) {
  // Radio.Sleep();
#ifdef NEED_DEBUG
  Serial.println("OnTxTimeout");
#endif // NEED_DEBUG
  digitalWrite(LED_BUILTIN, LOW);
  Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Timeout event
*/
void OnRxTimeout(void) {
#ifdef NEED_DEBUG
  Serial.println("OnRxTimeout");
#endif // NEED_DEBUG
  digitalWrite(LED_BUILTIN, LOW);
  Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Radio Rx Error event
*/
void OnRxError(void) {
#ifdef NEED_DEBUG
  Serial.println("OnRxError");
#endif // NEED_DEBUG
  digitalWrite(LED_BUILTIN, LOW);
  // Wait 500ms before sending the next package
  delay(500);
  // Check if our channel is available for sending
  Radio.Standby();
  Radio.SetCadParams(LORA_CAD_08_SYMBOL, mySF + 13, 10, LORA_CAD_ONLY, 0);
  cadTime = millis();
  Radio.StartCad();
  // Sending the ping will be started when the channel is free
}

/**@brief Function to be executed on Radio Tx Done event
*/
void OnTxDone(void) {
#ifdef NEED_DEBUG
  Serial.println("OnTxDone\n");
#endif // NEED_DEBUG
  digitalWrite(LED_BLUE, LOW);
  Radio.Rx(RX_TIMEOUT_VALUE);
}

/**@brief Function to be executed on Cad Done event
*/
void OnCadDone(bool cadResult) {
  time_t duration = millis() - cadTime;
  if (cadResult) {
    Radio.Rx(RX_TIMEOUT_VALUE);
  } else {
#ifdef NEED_DEBUG
    Serial.println("RAK4630 sending a PING in OnCadDone");
#endif // NEED_DEBUG
    sprintf((char*)TxdBuffer, "{\"UUID\":\"01234567\",\"from\":\"RAK4631\",\"cmd\":\"ping\",\"SIV\":%d,\"lat\":%.7f,\"long\":%.7f}", g_SIV, g_latitude, g_longitude);
    // fillRandom(TxdBuffer, 128);
    Radio.SetTxConfig(
      MODEM_LORA, TxPower, 0, myBW, mySF, myCR,
      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
      true, 0, 0, LORA_IQ_INVERSION_ON, TX_TIMEOUT_VALUE);
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    // demo version – we are sending 128 bytes of random data
    // HMAC'ed
    uint16_t flen = encryptECB(TxdBuffer, strlen((char*)TxdBuffer));
    Radio.Send(EncBuffer, flen);
  }
}

void prepareJSONPacket(char *) {
}

void sendJSONPacket() {
}

uint8_t calcMaxPayload() {
  // https://www.thethingsnetwork.org/airtime-calculator
  if (myFreq > 900.0) {
    if (mySF == 7 && myBW == 0) return 242;
    if (mySF == 7 && myBW == 2) return 222;
    if (mySF == 8 && myBW == 2) return 222;
    if (mySF == 8 && myBW == 0) return 125;
    if (mySF == 9 && myBW == 2) return 222;
    if (mySF == 9 && myBW == 0) return 53;
    if (mySF == 10 && myBW == 2) return 222;
    if (mySF == 10 && myBW == 0) return 11;
    if (mySF == 11 && myBW == 2) return 109;
    if (mySF == 12 && myBW == 2) return 33;
  } else if (myFreq >= 860.9) {
    if (mySF == 7 && (myBW == 0 || myBW == 1)) return 222;
    if (mySF == 8 &&  myBW == 0) return 222;
    if (mySF == 9 &&  myBW == 1) return 115;
    if ((mySF == 10 || mySF == 11 || mySF == 12) & (myBW == 0)) return 51;
    return 0;
  } else return 0;
}
