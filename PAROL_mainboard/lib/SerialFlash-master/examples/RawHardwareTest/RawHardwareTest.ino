// RawHardwareTest - Check if a SPI Flash chip is compatible
// with SerialFlash by performing many read and write tests
// to its memory.
//
// The chip should be fully erased before running this test.
// Use the EraseEverything to do a (slow) full chip erase.
//
// Normally you should NOT access the flash memory directly,
// as this test program does.  You should create files and
// read and write the files.  File creation allocates space
// with program & erase boundaries within the chip, to allow
// reading from any other files while a file is busy writing
// or erasing (if created as erasable).
//
// If you discover an incompatible chip, please report it here:
// https://github.com/PaulStoffregen/SerialFlash/issues
// You MUST post the complete output of this program, and
// the exact part number and manufacturer of the chip.


#include <SerialFlash.h>
#include <SPI.h>

const int FlashChipSelect = 6; // digital pin for flash chip CS pin
//const int FlashChipSelect = 21; // Arduino 101 built-in SPI Flash

SerialFlashFile file;

const unsigned long testIncrement = 4096;

void setup() {

  //uncomment these if using Teensy audio shield
  //SPI.setSCK(14);  // Audio shield has SCK on pin 14
  //SPI.setMOSI(7);  // Audio shield has MOSI on pin 7

  //uncomment these if you have other SPI chips connected
  //to keep them disabled while using only SerialFlash
  //pinMode(4, INPUT_PULLUP);
  //pinMode(10, INPUT_PULLUP);

  Serial.begin(9600);

  while (!Serial) ;
  delay(100);

  Serial.println(F("Raw SerialFlash Hardware Test"));
  SerialFlash.begin(FlashChipSelect); // proceed even if begin() fails

  if (test()) {
    Serial.println();
    Serial.println(F("All Tests Passed  :-)"));
    Serial.println();
    Serial.println(F("Test data was written to your chip.  You must run"));
    Serial.println(F("EraseEverything before using this chip for files."));
  } else {
    Serial.println();
    Serial.println(F("Tests Failed  :{"));
    Serial.println();
    Serial.println(F("The flash chip may be left in an improper state."));
    Serial.println(F("You might need to power cycle to return to normal."));
  }
}


bool test() {
  unsigned char buf[256], sig[256], buf2[8];
  unsigned long address, count, chipsize, blocksize;
  unsigned long usec;
  bool first;

  // Read the chip identification
  Serial.println();
  Serial.println(F("Read Chip Identification:"));
  SerialFlash.readID(buf);
  Serial.print(F("  JEDEC ID:     "));
  Serial.print(buf[0], HEX);
  Serial.print(' ');
  Serial.print(buf[1], HEX);
  Serial.print(' ');
  Serial.println(buf[2], HEX);
  Serial.print(F("  Part Number: "));
  Serial.println(id2chip(buf));
  Serial.print(F("  Memory Size:  "));
  chipsize = SerialFlash.capacity(buf);
  Serial.print(chipsize);
  Serial.println(F(" bytes"));
  if (chipsize == 0) return false;
  Serial.print(F("  Block Size:   "));
  blocksize = SerialFlash.blockSize();
  Serial.print(blocksize);
  Serial.println(F(" bytes"));


  // Read the entire chip.  Every test location must be
  // erased, or have a previously tested signature
  Serial.println();
  Serial.println(F("Reading Chip..."));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  address = 0;
  count = 0;
  first = true;
  while (address < chipsize) {
    SerialFlash.read(address, buf, 8);
    //Serial.print("  addr = ");
    //Serial.print(address, HEX);
    //Serial.print(", data = ");
    //printbuf(buf, 8);
    create_signature(address, sig);
    if (is_erased(buf, 8) == false) {
      if (equal_signatures(buf, sig) == false) {
        Serial.print(F("  Previous data found at address "));
        Serial.println(address);
        Serial.println(F("  You must fully erase the chip before this test"));
        Serial.print(F("  found this: "));
        printbuf(buf, 8);
        Serial.print(F("     correct: "));
        printbuf(sig, 8);
        return false;
      }
    } else {
      count = count + 1; // number of blank signatures
    }
    if (first) {
      address = address + (testIncrement - 8);
      first = false;
    } else {
      address = address + 8;
      first = true;
    }
  }


  // Write any signatures that were blank on the original check
  if (count > 0) {
    Serial.println();
    Serial.print(F("Writing "));
    Serial.print(count);
    Serial.println(F(" signatures"));
    memset(buf, 0, sizeof(buf));
    memset(sig, 0, sizeof(sig));
    memset(buf2, 0, sizeof(buf2));
    address = 0;
    first = true;
    while (address < chipsize) {
      SerialFlash.read(address, buf, 8);
      if (is_erased(buf, 8)) {
        create_signature(address, sig);
        //Serial.printf("write %08X: data: ", address);
        //printbuf(sig, 8);
        SerialFlash.write(address, sig, 8);
        while (!SerialFlash.ready()) ; // wait
        SerialFlash.read(address, buf, 8);
        if (equal_signatures(buf, sig) == false) {
          Serial.print(F("  error writing signature at "));
          Serial.println(address);
          Serial.print(F("  Read this: "));
          printbuf(buf, 8);
          Serial.print(F("  Expected:  "));
          printbuf(sig, 8);
          return false;
        }
      }
      if (first) {
        address = address + (testIncrement - 8);
        first = false;
      } else {
        address = address + 8;
        first = true;
      }
    }
  } else {
    Serial.println(F("  all signatures present from prior tests"));
  }


  // Read all the signatures again, just to be sure
  // checks prior writing didn't corrupt any other data
  Serial.println();
  Serial.println(F("Double Checking All Signatures:"));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  count = 0;
  address = 0;
  first = true;
  while (address < chipsize) {
    SerialFlash.read(address, buf, 8);
    create_signature(address, sig);
    if (equal_signatures(buf, sig) == false) {
      Serial.print(F("  error in signature at "));
      Serial.println(address);
      Serial.print(F("  Read this: "));
      printbuf(buf, 8);
      Serial.print(F("  Expected:  "));
      printbuf(sig, 8);
      return false;
    }
    count = count + 1;
    if (first) {
      address = address + (testIncrement - 8);
      first = false;
    } else {
      address = address + 8;
      first = true;
    }
  }
  Serial.print(F("  all "));
  Serial.print(count);
  Serial.println(F(" signatures read ok"));


  // Read pairs of adjacent signatures
  // check read works across boundaries
  Serial.println();
  Serial.println(F("Checking Signature Pairs"));
  memset(buf, 0, sizeof(buf));
  memset(sig, 0, sizeof(sig));
  memset(buf2, 0, sizeof(buf2));
  count = 0;
  address = testIncrement - 8;
  first = true;
  while (address < chipsize - 8) {
    SerialFlash.read(address, buf, 16);
    create_signature(address, sig);
    create_signature(address + 8, sig + 8);
    if (memcmp(buf, sig, 16) != 0) {
      Serial.print(F("  error in signature pair at "));
      Serial.println(address);
      Serial.print(F("  Read this: "));
      printbuf(buf, 16);
      Serial.print(F("  Expected:  "));
      printbuf(sig, 16);
      return false;
    }
    count = count + 1;
    address = address + testIncrement;
  }
  Serial.print(F("  all "));
  Serial.print(count);
  Serial.println(F(" signature pairs read ok"));


  // Write data and read while write in progress
  Serial.println();
  Serial.println(F("Checking Read-While-Write (Program Suspend)"));
  address = 256;
  while (address < chipsize) { // find a blank space
    SerialFlash.read(address, buf, 256);
    if (is_erased(buf, 256)) break;
    address = address + 256;
  }
  if (address >= chipsize) {
    Serial.println(F("  error, unable to find any blank space!"));
    return false;
  }
  for (int i=0; i < 256; i += 8) {
    create_signature(address + i, sig + i);
  }
  Serial.print(F("  write 256 bytes at "));
  Serial.println(address);
  Serial.flush();
  SerialFlash.write(address, sig, 256);
  usec = micros();
  if (SerialFlash.ready()) {
    Serial.println(F("  error, chip did not become busy after write"));
    return false;
  }
  SerialFlash.read(0, buf2, 8); // read while busy writing
  while (!SerialFlash.ready()) ; // wait
  usec = micros() - usec;
  Serial.print(F("  write time was "));
  Serial.print(usec);
  Serial.println(F(" microseconds."));
  SerialFlash.read(address, buf, 256);
  if (memcmp(buf, sig, 256) != 0) {
    Serial.println(F("  error writing to flash"));
    Serial.print(F("  Read this: "));
    printbuf(buf, 256);
    Serial.print(F("  Expected:  "));
    printbuf(sig, 256);
    return false;
  }
  create_signature(0, sig);
  if (memcmp(buf2, sig, 8) != 0) {
    Serial.println(F("  error, incorrect read while writing"));
    Serial.print(F("  Read this: "));
    printbuf(buf2, 256);
    Serial.print(F("  Expected:  "));
    printbuf(sig, 256);
    return false;
  }
  Serial.print(F("  read-while-writing: "));
  printbuf(buf2, 8);
  Serial.println(F("  test passed, good read while writing"));



  // Erase a block and read while erase in progress
  if (chipsize >= 262144 + blocksize + testIncrement) {
    Serial.println();
    Serial.println(F("Checking Read-While-Erase (Erase Suspend)"));
    memset(buf, 0, sizeof(buf));
    memset(sig, 0, sizeof(sig));
    memset(buf2, 0, sizeof(buf2));
    SerialFlash.eraseBlock(262144);
    usec = micros();
    delayMicroseconds(50);
    if (SerialFlash.ready()) {
      Serial.println(F("  error, chip did not become busy after erase"));
      return false;
    }
    SerialFlash.read(0, buf2, 8); // read while busy writing
    while (!SerialFlash.ready()) ; // wait
    usec = micros() - usec;
    Serial.print(F("  erase time was "));
    Serial.print(usec);
    Serial.println(F(" microseconds."));
    // read all signatures, check ones in this block got
    // erased, and all the others are still intact
    address = 0;
    first = true;
    while (address < chipsize) {
      SerialFlash.read(address, buf, 8);
      if (address >= 262144 && address < 262144 + blocksize) {
        if (is_erased(buf, 8) == false) {
          Serial.print(F("  error in erasing at "));
          Serial.println(address);
          Serial.print(F("  Read this: "));
          printbuf(buf, 8);
          return false;
        }
      } else {
        create_signature(address, sig);
        if (equal_signatures(buf, sig) == false) {
          Serial.print(F("  error in signature at "));
          Serial.println(address);
          Serial.print(F("  Read this: "));
          printbuf(buf, 8);
          Serial.print(F("  Expected:  "));
          printbuf(sig, 8);
          return false;
        }
      }
      if (first) {
        address = address + (testIncrement - 8);
        first = false;
      } else {
        address = address + 8;
        first = true;
      }
    }
    Serial.print(F("  erase correctly erased "));
    Serial.print(blocksize);
    Serial.println(F(" bytes"));
    // now check if the data we read during erase is good
    create_signature(0, sig);
    if (memcmp(buf2, sig, 8) != 0) {
      Serial.println(F("  error, incorrect read while erasing"));
      Serial.print(F("  Read this: "));
      printbuf(buf2, 256);
      Serial.print(F("  Expected:  "));
      printbuf(sig, 256);
      return false;
    }
    Serial.print(F("  read-while-erasing: "));
    printbuf(buf2, 8);
    Serial.println(F("  test passed, good read while erasing"));

  } else {
    Serial.println(F("Skip Read-While-Erase, this chip is too small"));
  }




  return true;
}


void loop() {
  // do nothing after the test
}

const char * id2chip(const unsigned char *id)
{
	if (id[0] == 0xEF) {
		// Winbond
		if (id[1] == 0x40) {
			if (id[2] == 0x14) return "W25Q80BV";
			if (id[2] == 0x15) return "W25Q16DV";
			if (id[2] == 0x17) return "W25Q64FV";
			if (id[2] == 0x18) return "W25Q128FV";
			if (id[2] == 0x19) return "W25Q256FV";
		}
	}
	if (id[0] == 0x01) {
		// Spansion
		if (id[1] == 0x02) {
			if (id[2] == 0x16) return "S25FL064A";
			if (id[2] == 0x19) return "S25FL256S";
			if (id[2] == 0x20) return "S25FL512S";
		}
		if (id[1] == 0x20) {
			if (id[2] == 0x18) return "S25FL127S";
		}
	}
	if (id[0] == 0xC2) {
		// Macronix
		if (id[1] == 0x20) {
			if (id[2] == 0x18) return "MX25L12805D";
		}
	}
	if (id[0] == 0x20) {
		// Micron
		if (id[1] == 0xBA) {
			if (id[2] == 0x20) return "N25Q512A";
			if (id[2] == 0x21) return "N25Q00AA";
		}
		if (id[1] == 0xBB) {
			if (id[2] == 0x22) return "MT25QL02GC";
		}
	}
	if (id[0] == 0xBF) {
		// SST
		if (id[1] == 0x25) {
			if (id[2] == 0x02) return "SST25WF010";
			if (id[2] == 0x03) return "SST25WF020";
			if (id[2] == 0x04) return "SST25WF040";
			if (id[2] == 0x41) return "SST25VF016B";
			if (id[2] == 0x4A) return "SST25VF032";
		}
		if (id[1] == 0x25) {
			if (id[2] == 0x01) return "SST26VF016";
			if (id[2] == 0x02) return "SST26VF032";
			if (id[2] == 0x43) return "SST26VF064";
		}
	}
  	if (id[0] == 0x1F) {
    		// Adesto
   		if (id[1] == 0x89) {
      			if (id[2] == 0x01) return "AT25SF128A";
    		}  
  	} 	
	return "(unknown chip)";
}

void print_signature(const unsigned char *data)
{
	Serial.print(F("data="));
	for (unsigned char i=0; i < 8; i++) {
		Serial.print(data[i]);
		Serial.print(' ');
	}
	Serial.println();
}

void create_signature(unsigned long address, unsigned char *data)
{
	data[0] = address >> 24;
	data[1] = address >> 16;
	data[2] = address >> 8;
	data[3] = address;
	unsigned long hash = 2166136261ul;
	for (unsigned char i=0; i < 4; i++) {
		hash ^= data[i];
		hash *= 16777619ul;
	}
	data[4] = hash;
	data[5] = hash >> 8;
	data[6] = hash >> 16;
	data[7] = hash >> 24;
}

bool equal_signatures(const unsigned char *data1, const unsigned char *data2)
{
	for (unsigned char i=0; i < 8; i++) {
		if (data1[i] != data2[i]) return false;
	}
	return true;
}

bool is_erased(const unsigned char *data, unsigned int len)
{
	while (len > 0) {
		if (*data++ != 255) return false;
		len = len - 1;
	}
	return true;
}


void printbuf(const void *buf, uint32_t len)
{
  const uint8_t *p = (const uint8_t *)buf;
  do {
    unsigned char b = *p++;
    Serial.print(b >> 4, HEX);
    Serial.print(b & 15, HEX);
    //Serial.printf("%02X", *p++);
    Serial.print(' ');
  } while (--len > 0);
  Serial.println();
}

