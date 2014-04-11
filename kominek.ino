/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example RF Radio Ping Pair
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two different nodes,
 * connect the role_pin to ground on one.  The ping node sends the current time to the pong node,
 * which responds by sending the value back.  The ping node can then see how long the whole cycle
 * took.
 */
 
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT11
#define ONOFFPIN 6
DHT dht(DHTPIN, DHTTYPE);

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9, 10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {
  0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL
};

int tempset = 20;
int lasttemp = 0;
bool on = false;

void setup(void) {

  //
  // Print preamble
  //

  Serial.begin(9600);
  //printf_begin();
  //printf("\n\rRF24/examples/pingpair/\n\r");
  pinMode(ONOFFPIN, OUTPUT);
  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(sizeof(int));
  radio.setPALevel(RF24_PA_HIGH);
  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)


  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);


  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  dht.begin();

}

void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //



  // if there is data ready
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    int rec = 0;
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &rec, sizeof(int) );
      printf("rec: %u\n\r", rec);
      // Spew it
      Serial.println((rec));
      if (rec > 15 && rec < 35) {
        tempset = rec;
      }
      else if (rec = 48) {
        digitalWrite(ONOFFPIN, LOW);
        if (on) {
          radio.stopListening();
          int tt = 0;
          bool ok = radio.write( &tt, sizeof(int) );
          Serial.print("wysylam: ");
          Serial.println(tt);
          delay(50);
          radio.startListening();
          on = false;
        }
      }
      else if (rec = 49) {
        digitalWrite(ONOFFPIN, HIGH);
        if (!on) {
          radio.stopListening();
          int tt = 1;
          bool ok = radio.write( &tt, sizeof(int) );
          Serial.print("wysylam: ");
          Serial.println(tt);
          delay(50);
          on = true;
          radio.startListening();
        }
      }
      // Delay just a little bit to let the other unit
      // make the transition to receiver
      delay(100);
    }

    // First, stop listening so we can talk
    radio.stopListening();

    // Send the final one back.
    int h = dht.readHumidity();
    int t = dht.readTemperature();
    int tmp = t * 100 + h;
    
    Serial.print("wysylam: ");
    Serial.println(tmp);
    bool ok = radio.write( &tmp, sizeof(int) );
    delay(200);
    radio.startListening();
  }
  // Now, resume listening so we catch the next packets.

  int t = dht.readTemperature();
  int h = dht.readHumidity();
  if (abs(lasttemp - t) > 1) {
    int tt = t * 100 + (h);
    radio.stopListening();
    bool ok = radio.write( &tt, sizeof(int) );
    Serial.print("wysylam: ");
    Serial.println(tt);
    delay(50);
    radio.startListening();
  }
  if (tempset > 1 + t) {
    digitalWrite(ONOFFPIN, HIGH);
    if (!on) {
      radio.stopListening();
      int tt = 1;
      bool ok = radio.write( &tt, sizeof(int) );
      Serial.print("wysylam: ");
      Serial.println(tt);
      delay(50);
      on = true;
      radio.startListening();
    }
  }
  if (tempset < t - 1) {
    digitalWrite(ONOFFPIN, LOW);
    if (on) {
      radio.stopListening();
      int tt = 0;
      bool ok = radio.write( &tt, sizeof(int) );
      Serial.print("wysylam: ");
      Serial.println(tt);
      delay(50);
      radio.startListening();
      on = false;
    }
  }
  //printf("tempset: %u\n\r",tempset);
  delay(50);
  lasttemp = t;
}
// vim:cin:ai:sts=2 sw=2 ft=cpp

