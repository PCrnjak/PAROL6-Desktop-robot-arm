// SerialPacketParser.h
#ifndef SerialPacketParser_h
#define SerialPacketParser_h

#include <Arduino.h>

class SerialPacketParser {
public:
  SerialPacketParser();
  bool parse(char c, char *command, char *argument);
  bool handleStart(char *argument);
  bool handleEnd(char *argument);
  bool handleGo(char *argument);
  bool handleTest(char *argument);

  bool HandlePoweronoff(char *argument);
  bool handleLED1TOGGLE(char *argument);
  bool handleLED2TOGGLE(char *argument);
  bool handleTESTFLASH(char *argument);
  bool handleSUPPLYV(char *argument);
  bool handleLIMIT(char *argument);
  bool handleOUT1TOGGLE(char *argument);
  bool handleOUT2TOGGLE(char *argument);
  bool handleINPUT(char *argument);
  bool handleESTOP(char *argument);
  bool handleSTATUS(char *argument);
  bool handleSTEP1(char *argument);
  bool handleSTEP2(char *argument);
  bool handleSTEP3(char *argument);
  bool handleSTEP4(char *argument);
  bool handleSTEP5(char *argument);
  bool handleSTEP6(char *argument);
  

  void resetCommandAndArgument(char *command, char *argument);

private:
  char buffer[64];
  int bufferIndex;
  bool packetComplete;
};

#endif