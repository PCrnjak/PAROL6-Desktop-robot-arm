import can
import time
import struct
# Initialize the CAN bus


def combine_2_can_id(id2, can_command, error_bit):
    # Combine components into an 11-bit CAN ID
    can_id = 0

    # Add ID2 (first 4 MSB)
    can_id |= (id2 & 0xF) << 7

    # Add CAN Command (next 6 bits)
    can_id |= (can_command & 0x3F) << 1

    # Add Error Bit (last bit)
    can_id |= (error_bit & 0x1)

    return can_id

def extract_from_can_id(can_id):
    # Extracting ID2 (first 4 MSB)
    id2 = (can_id >> 7) & 0xF

    # Extracting CAN Command (next 6 bits)
    can_command = (can_id >> 1) & 0x3F

    # Extracting Error Bit (last bit)
    error_bit = can_id & 0x1
    
    return id2, can_command, error_bit


bus = can.interface.Bus(bustype='slcan', channel='COM44', bitrate=1000000)

#arbitration_id_val = combine_2_can_id(0,60,0)
# Send a CAN message
#msg = can.Message(arbitration_id=arbitration_id_val, data=[30, 0, 1, 39], is_extended_id=False)



while True:


    #bus.send(msg)
    #time.sleep(0.01)
    message = bus.recv()
  

    print(f"Received message: {message}")
    print(f"Received message ID is: {message.arbitration_id}")
    print(" ")
    a,b,c = extract_from_can_id(message.arbitration_id)
    print(f"Node ID is: {a}")
    print(f"Command ID is: {b}")

    time.sleep(0.05)
# Don't forget to close the CAN bus when you're done
bus.shutdown()


