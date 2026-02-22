import serial
import time

class PicoSender:
    def __init__(self, port='COM3', baudrate=115200): # <--- CHANGE COM PORT HERE
        self.ser = None
        try:
            self.ser = serial.Serial(port, baudrate, timeout=1)
            time.sleep(2) # Wait for connection to settle
            # Flush any existing data in the buffer
            self.ser.reset_input_buffer()
            self.ser.reset_output_buffer()
            print(f"Connected to Pico on {port} at {baudrate} baud")
        except serial.SerialException as e:
            print(f"Error connecting to Pico: {e}")
            print("Make sure your Pico is plugged in and you are using the correct COM port.")

    def send_digit(self, digit):
        if self.ser and self.ser.is_open:
            try:
                # Convert digit to bytes and send
                data = str(digit).encode('utf-8')
                self.ser.write(data)
                print(f"Sent to Pico: {digit}")
                return True
            except Exception as e:
                print(f"Failed to send data: {e}")
                return False
        else:
            print("Serial port not open. Cannot send.")
            return False

    def send_success(self, digit):
        """Send SUCCESS message to Pico with digit number"""
        if self.ser and self.ser.is_open:
            try:
                message = f'SUCCESS{digit}\n'
                self.ser.write(message.encode('utf-8'))
                self.ser.flush()
                print(f"Sent SUCCESS{digit} to Pico")
                return True
            except Exception as e:
                print(f"Failed to send SUCCESS: {e}")
                return False
        return False

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
            print("Serial connection closed.")

# Simple test if run directly
if __name__ == "__main__":
    # You can change the port here for testing too
    sender = PicoSender(port='COM3') 
    
    if sender.ser and sender.ser.is_open:
        print("Sending test digits...")
        sender.send_digit(1)
        time.sleep(1)
        sender.send_digit(6)
        time.sleep(1)
        sender.send_digit(2)
        sender.close()