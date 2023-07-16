#!/usr/bin/python
#
# Uploads raw audio files to Teensy + Audio board with SPI Flash on board.  To use this program, first 
# load the 'CopyFromSerial' example sketch.  When it first runs, it will format the SPI flash chip
# (this may take a long time for larger chips; a 128MB chip that I am using can take almost 10 minutes,
# but smaller 16MB ones should be faster).
#
# While the chip is being formatted, the LED (pin 13) will toggle at 1Hz rate.  When the formatting is
# done, it flashes quickly (10Hz) for one second, then stays on solid.  When nothing has been received
# for 3 seconds, the upload is assumed to be completed, and the light goes off.
#
# You can start this program immediately upon plugging in the Teensy.  It will buffer and wait until
# the Teensy starts to read the serial data from USB.
#
###################

import serial, sys, os, time

if (len(sys.argv) <= 2):
	print("Usage: '" + sys.argv[0] + " <port> <files>' where:\n\t<port> is the TTY USB port connected to Drum Master\n\t<files> is a list of .RAW files (bash globs work).")
	sys.exit()

#Special bytes
BYTE_START = "\x7e"
BYTE_ESCAPE = "\x7d"
BYTE_SEPARATOR = "\x7c"

#Flash size (in MB).  Change this to match how much space you have on your chip.
FLASH_SIZE = 16

totalFileSize = 0;
for i, filename in enumerate(sys.argv):
	if (i >= 2):
		totalFileSize = totalFileSize + os.path.getsize(filename)

flashSizeBytes = FLASH_SIZE * 1024 * 1024
if (totalFileSize > flashSizeBytes):
	print("Too many files selsected.\n\tTotal flash size:\t" + "{:>14,}".format(flashSizeBytes) + " bytes\n\tTotal file size:\t" + "{:>14,}".format(totalFileSize) + " bytes")
	sys.exit()

ser = serial.Serial(sys.argv[1])
print("Uploading " + str(len(sys.argv) - 2) + " files...")
for i, filename in enumerate(sys.argv):
	if (i >= 2):
		startTime = time.time();
		sys.stdout.write(str(i - 1) + ": ")
		sys.stdout.write(filename)
		sys.stdout.flush()
		

		f = open(filename, "rb")
		fileLength = os.path.getsize(filename)
		try:
			encoded = []
			#Start byte
			encoded.append(BYTE_START)
			#Filename
			for byte in os.path.basename(filename):
				encoded.append(byte)
			#End of filename
			encoded.append(BYTE_SEPARATOR)
			
			#File length (uint32_t)
			encoded.append(chr((fileLength >> 24) & 0xFF));
			encoded.append(chr((fileLength >> 16) & 0xFF));
			encoded.append(chr((fileLength >> 8) & 0xFF));
			encoded.append(chr((fileLength >> 0) & 0xFF));
			encoded.append(BYTE_SEPARATOR)
			
			#Binary data, with escaping
			for byte in f.read():
				if byte == BYTE_START or byte == BYTE_ESCAPE:
					encoded.append(BYTE_ESCAPE)
					encoded.append(chr(ord(byte) ^ 0x20))
				else:
					encoded.append(byte);
			
			#Write end of data byte
			encoded.append(BYTE_START)
			ser.write("".join(encoded))
		 
		finally:
			f.close()
			
		endTime = time.time();
		print(" (" + str(round(fileLength / 1024 / (endTime - startTime), 2)) + " KB/s)");

print("All files uploaded")
