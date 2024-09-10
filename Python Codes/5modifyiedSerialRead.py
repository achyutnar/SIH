import serial
import time
import pyautogui

def scroll_slow_down(Accel):
    tdelay = Accel * 0.5
    sdisp = int(-1 * (Accel * 15))
    for i in range(0, 20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def scroll_slow_up(Accel):
    tdelay = Accel * 0.5
    sdisp = int(Accel * 15)
    for i in range(0, 20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def scroll_fast_down(Accel):
    tdelay = Accel * 0.025
    sdisp = int(-1 * (Accel * 25))
    for i in range(0, 20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def scroll_fast_up(Accel):
    tdelay = Accel * 0.025
    sdisp = int(Accel * 25)
    for i in range(0, 20):
        pyautogui.vscroll(sdisp)  # Fast and longer scroll up
        time.sleep(tdelay)

def move_left(Accel):
    freq = int(Accel * 3)
    tdelay = 1 / 40
    for i in range(0, freq):
        pyautogui.press('left')
        time.sleep(tdelay)

def move_right(Accel):
    freq = int(Accel * 3)
    tdelay = 1 / 40
    for i in range(0, freq):
        pyautogui.press('right')
        time.sleep(tdelay)

def zoom_in(Accel):
    pyautogui.hotkey('ctrl', '+')
    pyautogui.hotkey('ctrl', '+')

def zoom_out(Accel):
    pyautogui.hotkey('ctrl', '-')
    pyautogui.hotkey('ctrl', '-')

def open_file(file_position):
    pyautogui.doubleClick(file_position)
    time.sleep(2)

def close_file():
    pyautogui.hotkey('ctrl', 'w')
    time.sleep(2)

file_position = (607, 368)  # (x, y) coordinates of the file

def DetectGesture(thumb_angle, middle_angle, index_angle):
    if abs(middle_angle + index_angle) > 140:
        print("zoom in")
        zoom_in(max(middle_angle, index_angle))
    elif abs(index_angle + thumb_angle) > 100:
        print("zoom out")
        zoom_out(max(index_angle, thumb_angle))
    elif abs(thumb_angle) > 40:
        print("move right")
        for i in range(1):
            pyautogui.press('right')
            time.sleep(0.1)
    elif abs(index_angle) > 40:
        print("move left")
        for i in range(1):
            pyautogui.press('left')
            time.sleep(0.1)

def parse_pry_data(data):
    finger_data = data.split(';')
    thumb_pry = [0.0, 0.0, 0.0]
    index_pry = [0.0, 0.0, 0.0]
    middle_pry = [0.0, 0.0, 0.0]
    ring_pry = [0.0, 0.0, 0.0]
    pinky_pry = [0.0, 0.0, 0.0]
    palm_pry = [0.0, 0.0, 0.0]

    for finger in finger_data:
        parts = finger.split(':')
        if len(parts) == 2:
            finger_name = parts[0]
            values = parts[1].split(',')

            if len(values) == 3:
                pitch = float(values[0])
                roll = float(values[1])
                yaw = float(values[2])

                if finger_name == "Thumb":
                    thumb_pry[0], thumb_pry[1], thumb_pry[2] = pitch, roll, yaw
                elif finger_name == "Point":
                    index_pry[0], index_pry[1], index_pry[2] = pitch, roll, yaw
                elif finger_name == "Middle":
                    middle_pry[0], middle_pry[1], middle_pry[2] = pitch, roll, yaw
                elif finger_name == "Ring":
                    ring_pry[0], ring_pry[1], ring_pry[2] = pitch, roll, yaw
                elif finger_name == "Pinky":
                    pinky_pry[0], pinky_pry[1], pinky_pry[2] = pitch, roll, yaw
                elif finger_name == "Palm":
                    palm_pry[0], palm_pry[1], palm_pry[2] = pitch, roll, yaw

    return thumb_pry, index_pry, middle_pry, ring_pry, pinky_pry, palm_pry

if __name__ == "__main__":
    # Replace with your actual serial port (e.g., 'COM3' for Windows or '/dev/ttyUSB0' for Linux)
    serial_port = '/dev/ttyUSB0'  # Change this to the actual port your device is connected to
    baud_rate = 9600  # Set the correct baud rate for your device

    # Initialize serial communication
    ser = serial.Serial(serial_port, baud_rate, timeout=1)
    time.sleep(2)  # Allow time for the serial connection to initialize

    d1 = {"Thumb": [], "Middle": [], "Point": []}

    while True:
        if ser.in_waiting > 0:
            # Read data from the serial port
            data = ser.readline().decode('utf-8').strip()
            print("Received data:", data)

            # Parse PRY data from the received string
            thumb_pry, index_pry, middle_pry, ring_pry, pinky_pry, palm_pry = parse_pry_data(data)

            # Update the dictionary with the new PRY values
            d1["Thumb"] = thumb_pry
            d1["Middle"] = middle_pry
            d1["Point"] = index_pry

            print(d1)

            # Detect gestures based on the parsed angles
            DetectGesture(d1["Thumb"][0], d1["Middle"][0], d1["Point"][0])
