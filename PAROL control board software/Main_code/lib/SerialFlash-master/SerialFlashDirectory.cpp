/* SerialFlash Library - for filesystem-like access to SPI Serial Flash memory
 * https://github.com/PaulStoffregen/SerialFlash
 * Copyright (C) 2015, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this library was funded by PJRC.COM, LLC by sales of Teensy.
 * Please support PJRC's efforts to develop open source software by purchasing
 * Teensy or other genuine PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SerialFlash.h"

/* On-chip SerialFlash file allocation data structures:

  uint32_t signature = 0xFA96554C;
  uint16_t maxfiles          
  uint16_t stringssize  // div by 4
  uint16_t hashes[maxfiles]
  struct {
    uint32_t file_begin
    uint32_t file_length
    uint16_t string_index  // div4
  } fileinfo[maxfiles]
  char strings[stringssize]

A 32 bit signature is stored at the beginning of the flash memory.
If 0xFFFFFFFF is seen, the entire chip should be assumed blank.
If any value other than 0xFA96554C is found, a different data format
is stored.  This could should refuse to access the flash.

The next 4 bytes store number of files and size of the strings
section, which allow the position of every other item to be found.
The string section size is the 16 bit integer times 4, which allows
up to 262140 bytes for string data.

An array of 16 bit filename hashes allows for quick linear search
for potentially matching filenames.  A hash value of 0xFFFF indicates
no file is allocated for the remainder of the array.

Following the hashes, and array of 10 byte structs give the location
and length of the file's actual data, and the offset of its filename
in the strings section.

Strings are null terminated.  The remainder of the chip is file data.
*/

#define DEFAULT_MAXFILES      600
#define DEFAULT_STRINGS_SIZE  25560


static uint32_t check_signature(void)
{
	uint32_t sig[2];

	SerialFlash.read(0, sig, 8);
	 //Serial.printf("sig: %08X %08X\n", sig[0], sig[1]);
	if (sig[0] == 0xFA96554C) return sig[1];
	if (sig[0] == 0xFFFFFFFF) {
		sig[0] = 0xFA96554C;
		sig[1] = ((uint32_t)(DEFAULT_STRINGS_SIZE/4) << 16) | DEFAULT_MAXFILES;
		SerialFlash.write(0, sig, 8);
		while (!SerialFlash.ready()) ; // TODO: timeout
		SerialFlash.read(0, sig, 8);
		if (sig[0] == 0xFA96554C) return sig[1];
	}
	return 0;
}

static uint16_t filename_hash(const char *filename)
{
	// http://isthe.com/chongo/tech/comp/fnv/
	uint32_t hash = 2166136261;
	const char *p;

	for (p=filename; *p; p++) {
		hash ^= *p;
		hash *= 16777619;
	}
	hash = (hash % (uint32_t)0xFFFE) + 1; // all values except 0000 & FFFF
	return hash;
}

static bool filename_compare(const char *filename, uint32_t straddr)
{
	unsigned int i;
	const char *p;
	char buf[16];

	p = filename;
	while (1) {
		SerialFlash.read(straddr, buf, sizeof(buf));
		straddr += sizeof(buf);
		for (i=0; i < sizeof(buf); i++) {
			if (*p++ != buf[i]) return false;
			if (buf[i] == 0) return true;
		}
	}
}

#if 0
void pbuf(const void *buf, uint32_t len)
{
  const uint8_t *p = (const uint8_t *)buf;
  do {
    Serial.printf("%02X ", *p++);
  } while (--len > 0);
  Serial.println();
}
#endif

SerialFlashFile SerialFlashChip::open(const char *filename)
{
	uint32_t maxfiles, straddr;
	uint16_t hash, hashtable[8];
	uint32_t i, n, index=0;
	uint32_t buf[3];
	SerialFlashFile file;

	maxfiles = check_signature();
	 //Serial.printf("sig: %08X\n", maxfiles);
	if (!maxfiles) return file;
	maxfiles &= 0xFFFF;
	hash = filename_hash(filename);
	 //Serial.printf("hash %04X for \"%s\"\n", hash, filename);
	while (index < maxfiles) {
		n = 8;
		if (n > maxfiles - index) n = maxfiles - index;
		SerialFlash.read(8 + index * 2, hashtable, n * 2);
		 //Serial.printf(" read %u: ", 8 + index * 2);
		 //pbuf(hashtable, n * 2);
		for (i=0; i < n; i++) {
			if (hashtable[i] == hash) {
				 //Serial.printf("  hash match at index %u\n", index+i);
				buf[2] = 0;
				SerialFlash.read(8 + maxfiles * 2 + (index+i) * 10, buf, 10);

				 //Serial.printf("  maxf=%d, index=%d, i=%d\n", maxfiles, index, i);
				 //Serial.printf("  read %u: ", 8 + maxfiles * 2 + (index+i) * 10);
				 //pbuf(buf, 10);
				straddr = 8 + maxfiles * 12 + buf[2] * 4;
				 //Serial.printf("  straddr = %u\n", straddr);
				if (filename_compare(filename, straddr)) {
					 //Serial.printf("  match!\n");
					 //Serial.printf("  addr = %u\n", buf[0]);
					 //Serial.printf("  len =  %u\n", buf[1]);
					file.address = buf[0];
					file.length = buf[1];
					file.offset = 0;
					file.dirindex = index + i;
					return file;
				}
			} else if (hashtable[i] == 0xFFFF) {
				return file;
			}
		}
		index += n;
	}
	return file;
}

bool SerialFlashChip::exists(const char *filename)
{
	SerialFlashFile file = open(filename);
	return (bool)file;
}

bool SerialFlashChip::remove(const char *filename)
{
	SerialFlashFile file = open(filename);
	return remove(file);
}

bool SerialFlashChip::remove(SerialFlashFile &file)
{
	// To "remove" a file, we simply zero its hash in the lookup
	// table, so it can't be found by open().  The space on the
	// flash memory is not freed.
	if (!file) return false;
	uint16_t hash;
	SerialFlash.read(8 + file.dirindex * 2, &hash, 2);
	 //Serial.printf("remove hash %04X at %d index\n", hash, file.dirindex);
	hash ^= 0xFFFF;  // write zeros to all ones
	SerialFlash.write(8 + file.dirindex * 2, &hash, 2);
	while (!SerialFlash.ready()) ; // wait...  TODO: timeout
	SerialFlash.read(8 + file.dirindex * 2, &hash, 2);
	if (hash != 0)  {
		 //Serial.printf("remove failed, hash %04X\n", hash);
		return false;
	}
	file.address = 0;
	file.length = 0;
	return true;
}

static uint32_t find_first_unallocated_file_index(uint32_t maxfiles)
{
	uint16_t hashtable[8];
	uint32_t i, n, index=0;

	do {
		n = 8;
		if (index + n > maxfiles) n = maxfiles - index;
		SerialFlash.read(8 + index * 2, hashtable, n * 2);
		for (i=0; i < n; i++) {
			if (hashtable[i] == 0xFFFF) return index + i;
		}
		index += n;
	} while (index < maxfiles);
	return 0xFFFFFFFF;
}

static uint32_t string_length(uint32_t addr)
{
	char buf[16];
	const char *p;
	uint32_t len=0;

	while (1) {
		SerialFlash.read(addr, buf, sizeof(buf));
		for (p=buf; p < buf + sizeof(buf); p++) {
			len++;
			if (*p == 0) return len;
		}
		addr += sizeof(buf);
	}
}

//  uint32_t signature = 0xFA96554C;
//  uint16_t maxfiles          
//  uint16_t stringssize  // div by 4
//  uint16_t hashes[maxfiles]
//  struct {
//    uint32_t file_begin
//    uint32_t file_length
//    uint16_t string_index  // div 4
//  } fileinfo[maxfiles]
//  char strings[stringssize]

bool SerialFlashChip::create(const char *filename, uint32_t length, uint32_t align)
{
	uint32_t maxfiles, stringsize;
	uint32_t index, buf[3];
	uint32_t address, straddr, len;
	SerialFlashFile file;

	// check if the file already exists
	if (exists(filename)) return false;

	// first, get the filesystem parameters
	maxfiles = check_signature();
	if (!maxfiles) return false;
	stringsize = (maxfiles & 0xFFFF0000) >> 14;
	maxfiles &= 0xFFFF;

	// find the first unused slot for this file
	index = find_first_unallocated_file_index(maxfiles);
	if (index >= maxfiles) return false;
	 //Serial.printf("index = %u\n", index);
	// compute where to store the filename and actual data
	straddr = 8 + maxfiles * 12;
	if (index == 0) {
		address = straddr + stringsize;
	} else {
		buf[2] = 0;
		SerialFlash.read(8 + maxfiles * 2 + (index-1) * 10, buf, 10);
		address = buf[0] + buf[1];
		straddr += buf[2] * 4;
		straddr += string_length(straddr);
		straddr = (straddr + 3) & 0x0003FFFC;
	}
	 //Serial.printf("straddr = %u\n", straddr);
	 //Serial.printf("address = %u\n", address);
	 //Serial.printf("length = %u\n", length);
	if (align > 0) {
		// for files aligned to sectors, adjust addr & len
		address += align - 1;
		address /= align;
		address *= align;
		 //Serial.printf("align address = %u\n", address);
		length += align - 1;
		length /= align;
		length *= align;
		 //Serial.printf("align length = %u\n", length);
	} else {
		// always align every file to a page boundary
		// for predictable write latency and to guarantee
		// write suspend for reading another file can't
		// conflict on the same page (2 files never share
		// a write page).
		address = (address + 255) & 0xFFFFFF00;
	}
	 //Serial.printf("address = %u\n", address);
	// last check, if enough space exists...
	len = strlen(filename);
	// TODO: check for enough string space for filename
	uint8_t id[5];
	SerialFlash.readID(id);
	if (address + length > SerialFlash.capacity(id)) return false;

	SerialFlash.write(straddr, filename, len+1);
	buf[0] = address;
	buf[1] = length;
	buf[2] = (straddr - (8 + maxfiles * 12)) / 4;
	SerialFlash.write(8 + maxfiles * 2 + index * 10, buf, 10);
	 //Serial.printf("  write %u: ", 8 + maxfiles * 2 + index * 10);
	 //pbuf(buf, 10);
	while (!SerialFlash.ready()) ;  // TODO: timeout
	 
	buf[0] = filename_hash(filename);
	 //Serial.printf("hash = %04X\n", buf[0]);
	SerialFlash.write(8 + index * 2, buf, 2);
	while (!SerialFlash.ready()) ;  // TODO: timeout
	return true;
}

bool SerialFlashChip::readdir(char *filename, uint32_t strsize, uint32_t &filesize)
{
	uint32_t maxfiles, index, straddr;
	uint32_t i, n;
	uint32_t buf[2];
	uint16_t hash;
	char str[16], *p=filename;

	filename[0] = 0;
	maxfiles = check_signature();
	if (!maxfiles) return false;
	maxfiles &= 0xFFFF; 
	index = dirindex;
	while (1) {
		if (index >= maxfiles) return false;
		 //Serial.printf("readdir, index = %u\n", index);
		SerialFlash.read(8 + index * 2, &hash, 2);
		if (hash != 0) break;
		index++;  // skip deleted entries
	}
	dirindex = index + 1;
	buf[1] = 0;
	SerialFlash.read(8 + 4 + maxfiles * 2 + index * 10, buf, 6);
	if (buf[0] == 0xFFFFFFFF) return false;
	filesize = buf[0];
	straddr = 8 + maxfiles * 12 + buf[1] * 4;
	 //Serial.printf("  length = %u\n", buf[0]);
	 //Serial.printf("  straddr = %u\n", straddr);

	while (strsize) {
		n = strsize;
		if (n > sizeof(str)) n = sizeof(str);
		SerialFlash.read(straddr, str, n);
		for (i=0; i < n; i++) {
			*p++ = str[i];
			if (str[i] == 0) {
				 //Serial.printf("  name = %s\n", filename);
				return true;
			}
		}
		strsize -= n;
		straddr += n;
	}
	*(p - 1) = 0;
	 //Serial.printf("  name(overflow) = %s\n", filename);
	return true;
}


void SerialFlashFile::erase()
{
	uint32_t i, blocksize;

	blocksize = SerialFlash.blockSize();
	if (address & (blocksize - 1)) return; // must begin on a block boundary
	if (length & (blocksize - 1)) return;  // must be exact number of blocks
	for (i=0; i < length; i += blocksize) {
		SerialFlash.eraseBlock(address + i);
	}
}

