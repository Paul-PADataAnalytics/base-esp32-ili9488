#!/usr/bin/env python3
"""
Simple ESP32 Serial Monitor Script
Reads and prints serial output at 115200 baud rate.
"""

import serial
import time
import sys

PORT = '/dev/ttyUSB0'
BAUD = 115200

def main():
    port = sys.argv[1] if len(sys.argv) > 1 else PORT
    print(f"Connecting to ESP32 on {port} at {BAUD} baud...")
    try:
        ser = serial.Serial(port, BAUD, timeout=1)
        # Pulse RTS to reset ESP32
        ser.dtr = False
        ser.rts = True
        time.sleep(0.1)
        ser.rts = False
        print("ESP32 reset. Listening for output (Press Ctrl+C to stop)...")
        print("-" * 50)
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='replace')
                print(line, end='')
            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\nStopped monitor.")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()
