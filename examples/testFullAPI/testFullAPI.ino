/**
 * MIT License
 *
 * Copyright (c) 2018 Infineon Technologies AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE
 *
 * Demonstrates use of the 
 * Infineon Technologies AG OPTIGA™ Trust X Arduino library
 */

#include "OPTIGATrustX.h"

#define MAXCMD_LEN		255
#define CERT_LENGTH		512
#define RND_LENGTH		64
#define HASH_LENGTH		32
#define SIGN_LENGTH		80
#define PUBKEY_LENGTH	70
#define UID_LENGTH    27

#define SUPPRESSCOLLORS
#include "fprint.h"

#define ASSERT(err)   if (ret) { printlnRed("Failed"); while (true); }

/*
 * Allocating buffers for further use in loop()
 */
uint8_t *cert = new uint8_t[CERT_LENGTH];
uint16_t certLen = CERT_LENGTH;
uint8_t *rnd = new uint8_t[RND_LENGTH];
uint16_t rndLen = RND_LENGTH;
uint8_t *hash = new uint8_t[HASH_LENGTH];
uint16_t hashLen = HASH_LENGTH;
uint8_t *rawSign = new uint8_t[SIGN_LENGTH];
uint8_t *formSign = new uint8_t[SIGN_LENGTH];
uint16_t signLen = SIGN_LENGTH;
uint8_t *pubKey = new uint8_t[PUBKEY_LENGTH];
uint16_t pubKeyLen = PUBKEY_LENGTH;
uint8_t *uid = new uint8_t[UID_LENGTH];
  



static void output_result(char* tag, uint8_t* in, uint16_t in_len)
{
  printlnGreen("OK"); 
  printMagenta(tag); 
  printMagenta(" Length: ");
  Serial.println(in_len);
  printMagenta(tag); 
  printlnMagenta(":");
  HEXDUMP(in, in_len);
}

void loop() 
{
  uint32_t ret = 0;
  uint8_t  cntr = 10;
  uint8_t ifxPublicKey[68];

  /* 
   * Getting co-processor Unique ID
   */
  printGreen("Get co-processor UID ... ");
  uint16_t uidLength = UID_LENGTH;
  ret = trustX.getUniqueID(uid, uidLength);
  ASSERT(ret);
  output_result("Co-processor UID", uid, uidLength);

  /* 
   * Getting primary certificate
   */
  printGreen("Reading cert ... ");
  ret = trustX.getCertificate(cert, certLen);
  ASSERT(ret);
  output_result("Certificate", cert, certLen);

  /* 
   * Generate a Keypair
   */
  printGreen("Generate Key Pair ... ");
  uint16_t ctx = 0;
  ret = trustX.generateKeypair(pubKey, pubKeyLen, ctx);
  ASSERT(ret);
  output_result("Public key", pubKey, pubKeyLen);

  /* 
   * Get random value of RND_LENGTH length
   */
  printGreen("Get random value ... ");
  ret = trustX.getRandom(RND_LENGTH, rnd);
  ASSERT(ret);
  output_result("Random", rnd, RND_LENGTH);

  /* 
   * Calculate SHA256 value
   */
  printGreen("Calculate Hash ... ");
  ret = trustX.sha256(rnd, RND_LENGTH, hash);
  hashLen = 32;
  ASSERT(ret);
  output_result("SHA256", hash, hashLen);

  /* 
   * Generate a signature NIST-P256
   */
  printGreen("Generate Signature ... ");
  ret = trustX.calculateSignature(hash, hashLen, formSign, signLen);
  ASSERT(ret);
  output_result("Signature", formSign, signLen);

  /* 
   * Verify just geberated signature
   */
  trustX.getPublicKey(ifxPublicKey);
   
  printGreen("Verify Signature ... ");
  ret = trustX.verifySignature(hash, hashLen, formSign, signLen, ifxPublicKey,
      sizeof(ifxPublicKey) / sizeof(ifxPublicKey[0]));
  ASSERT(ret);
  printlnGreen("OK");

  /*
   * Count down 10 seconds and restart the application
   */
  while(cntr) {
    Serial.print(cntr);
    Serial.println(" seconds untill restart.");
    delay(1000);
    cntr--;
  }
}

void setup()
{
	uint32_t ret = 0;

  /*
   * Initialise serial output
   */
	Serial.begin(38400);
	Serial.println("Initializing ... ");

  /*
   * Initialise OPTIGA™ Trust X
   */
	printGreen("Begin Trust ... ");
	ret = trustX.begin();
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Speed up the chip (min is 6ma, maximum is 15ma)
   */
  printGreen("Setting Current Limit... ");
	ret = trustX.setCurrentLimit(15);
	ASSERT(ret);
	printlnGreen("OK");

  /*
   * Check the return value which we just set
   */
  printGreen("Checking Power Limit... ");
  uint8_t current_lim = 0;
  ret = trustX.getCurrentLimit(current_lim);
  ASSERT(ret);
  if (current_lim == 15) {
    printlnGreen("OK");
  } else {
    printlnRed("Failed");
    while(1);
  }
}
