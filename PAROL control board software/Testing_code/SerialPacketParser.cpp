// SerialPacketParser.cpp
#include "SerialPacketParser.h"

SerialPacketParser::SerialPacketParser()
{
  bufferIndex = 0;
  packetComplete = false;
}

bool SerialPacketParser::parse(char c, char *command, char *argument)
{

  if (c == '#')
  {
    bufferIndex = 0;
    packetComplete = false;
  }

  if (bufferIndex < sizeof(buffer) - 1)
  {
    buffer[bufferIndex++] = c;
    buffer[bufferIndex] = '\0'; // Null-terminate the buffer

    if (c == '\n')
    {
      packetComplete = true;
    }
  }

  if (packetComplete)
  {
    // Reset the buffer and flag after successfully processing a command
    bufferIndex = 0;
    packetComplete = false;

    // Extract the command and argument
    if (sscanf(buffer, "#%19s %19s", command, argument) == 2)
    {
      // Dispatch based on the parsed command
      if (strlen(command) + strlen(argument) <= 20)
      {
// Dispatch based on the parsed command
        if (strcmp(command, "start") == 0)
        {
          return handleStart(NULL); // No argument
        }
        else if (strcmp(command, "STOP") == 0)
        {
          return handleEnd(NULL); // No argument
        }
        else if (strcmp(command, "CAN") == 0)
        {
          return handleGo(NULL); // No argument
        }
        else if (strcmp(command, "IO") == 0)
        {
          return handleTest(NULL); // No argument
        }
        else if (strcmp(command, "ONOFF") == 0)
        {
          return HandlePoweronoff(NULL); // No argument
        }
        else if (strcmp(command, "LED1TOGGLE") == 0)
        {
          return handleLED1TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "LED2TOGGLE") == 0)
        {
          return handleLED2TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "FLASH") == 0)
        {
          return handleTESTFLASH(NULL); // No argument
        }
        else if (strcmp(command, "SUPPLYV") == 0)
        {
          return handleSUPPLYV(NULL); // No argument
        }
        else if (strcmp(command, "LIMIT") == 0)
        {
          return handleLIMIT(NULL); // No argument
        }
        else if (strcmp(command, "OUT1TOGGLE") == 0)
        {
          return handleOUT1TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "OUT2TOGGLE") == 0)
        {
          return handleOUT2TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "INPUT") == 0)
        {
          return handleINPUT(NULL); // No argument
        }
        else if (strcmp(command, "ESTOP") == 0)
        {
          return handleESTOP(NULL); // No argument
        }
        else if (strcmp(command, "STATUS") == 0)
        {
          return handleSTATUS(NULL); // No argument
        }
        else if (strcmp(command, "STEP1") == 0)
        {
          return handleSTEP1(NULL); // No argument
        }
        else if (strcmp(command, "STEP2") == 0)
        {
          return handleSTEP2(NULL); // No argument
        }
        else if (strcmp(command, "STEP3") == 0)
        {
          return handleSTEP3(NULL); // No argument
        }
        else if (strcmp(command, "STEP4") == 0)
        {
          return handleSTEP4(NULL); // No argument
        }
        else if (strcmp(command, "STEP5") == 0)
        {
          return handleSTEP5(NULL); // No argument
        }
        else if (strcmp(command, "STEP6") == 0)
        {
          return handleSTEP6(NULL); // No argument
        }
        else
        {
          // Unknown command
          resetCommandAndArgument(command, argument);
          return false;
        }
      }
      else
      {
        // Combined length of command and argument exceeds 20 characters
        resetCommandAndArgument(command, argument);
        return false;
      }
    }
    else if (sscanf(buffer, "#%19s", command) == 1)
    {
      if (strlen(command) <= 20)
      {
        // Dispatch based on the parsed command
        if (strcmp(command, "start") == 0)
        {
          return handleStart(NULL); // No argument
        }
        else if (strcmp(command, "STOP") == 0)
        {
          return handleEnd(NULL); // No argument
        }
        else if (strcmp(command, "CAN") == 0)
        {
          return handleGo(NULL); // No argument
        }
        else if (strcmp(command, "IO") == 0)
        {
          return handleTest(NULL); // No argument
        }
        else if (strcmp(command, "ONOFF") == 0)
        {
          return HandlePoweronoff(NULL); // No argument
        }
        else if (strcmp(command, "LED1TOGGLE") == 0)
        {
          return handleLED1TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "LED2TOGGLE") == 0)
        {
          return handleLED2TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "FLASH") == 0)
        {
          return handleTESTFLASH(NULL); // No argument
        }
        else if (strcmp(command, "SUPPLYV") == 0)
        {
          return handleSUPPLYV(NULL); // No argument
        }
        else if (strcmp(command, "LIMIT") == 0)
        {
          return handleLIMIT(NULL); // No argument
        }
        else if (strcmp(command, "OUT1TOGGLE") == 0)
        {
          return handleOUT1TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "OUT2TOGGLE") == 0)
        {
          return handleOUT2TOGGLE(NULL); // No argument
        }
        else if (strcmp(command, "INPUT") == 0)
        {
          return handleINPUT(NULL); // No argument
        }
        else if (strcmp(command, "ESTOP") == 0)
        {
          return handleESTOP(NULL); // No argument
        }
        else if (strcmp(command, "STATUS") == 0)
        {
          return handleSTATUS(NULL); // No argument
        }
        else if (strcmp(command, "STEP1") == 0)
        {
          return handleSTEP1(NULL); // No argument
        }
        else if (strcmp(command, "STEP2") == 0)
        {
          return handleSTEP2(NULL); // No argument
        }
        else if (strcmp(command, "STEP3") == 0)
        {
          return handleSTEP3(NULL); // No argument
        }
        else if (strcmp(command, "STEP4") == 0)
        {
          return handleSTEP4(NULL); // No argument
        }
        else if (strcmp(command, "STEP5") == 0)
        {
          return handleSTEP5(NULL); // No argument
        }
        else if (strcmp(command, "STEP6") == 0)
        {
          return handleSTEP6(NULL); // No argument
        }
        else
        {
          // Unknown command
          resetCommandAndArgument(command, argument);
          return false;
        }
      }
      else
      {
        // Command length exceeds 20 characters
        resetCommandAndArgument(command, argument);
        return false;
      }
    }
    else
    {
      // Invalid command or argument
      resetCommandAndArgument(command, argument);
      return false;
    }
  }
  return false; // No valid packet yet
}

bool SerialPacketParser::handleStart(char *argument)
{
  // Your code to handle the "start" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}

bool SerialPacketParser::handleEnd(char *argument)
{
  // Your code to handle the "end" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}

bool SerialPacketParser::handleGo(char *argument)
{
  // Your code to handle the "go" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}

bool SerialPacketParser::handleTest(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}

bool SerialPacketParser::HandlePoweronoff(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleLED1TOGGLE(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleLED2TOGGLE(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleTESTFLASH(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSUPPLYV(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleLIMIT(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleOUT1TOGGLE(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleOUT2TOGGLE(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleINPUT(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleESTOP(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTATUS(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTEP1(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTEP2(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTEP3(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTEP4(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTEP5(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}
bool SerialPacketParser::handleSTEP6(char *argument)
{
  // Your code to handle the "test" command
  // Use 'argument' for any additional data related to this command

  return true; // Successfully handled
}

void SerialPacketParser::resetCommandAndArgument(char *command, char *argument)
{
  // Reset the command and argument arrays
  memset(command, 0, sizeof(command));
  memset(argument, 0, sizeof(argument));
}