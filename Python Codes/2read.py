import socket
import time

import pyautogui


# Scrolling functions
def scroll_slow(direction, accel):
    tdelay = accel * 0.5
    sdisp = int(direction * accel * 15)
    for _ in range(20):
        pyautogui.vscroll(sdisp)
        time.sleep(tdelay)

def scroll_fast(direction, accel):
    tdelay = accel * 0.025
    sdisp = int(direction * accel * 25)
    for _ in range(20):
        pyautogui.vscroll(sdisp)
        time.sleep(tdelay)

# Movement functions
def move_horizontal(direction, accel):
    freq = int(accel * 3)
    tdelay = 1 / accel
    for _ in range(freq):
        pyautogui.press(direction)
        time.sleep(tdelay)

# Zoom functions
def zoom(direction, accel):
    tdelay = 1 / accel
    pyautogui.hotkey('ctrl', direction)
    time.sleep(tdelay)
    pyautogui.hotkey('ctrl', direction)

# File operations
def open_file(file_position):
    pyautogui.doubleClick(file_position)
    time.sleep(2)

def close_file():
    pyautogui.hotkey('ctrl', 'w')
    time.sleep(2)

# Gesture detection
def detect_gesture(pitch_diffs, yaw_diffs, roll_diffs):
    val_change = 2.5
    no_change_threshold = 1.5
    
    if all(diff <= -val_change for diff in pitch_diffs):
        move_horizontal('right', max(map(abs, pitch_diffs)))
        print("Move right")
    elif pitch_diffs[0] >= val_change and all(abs(yaw) < val_change and abs(roll) < val_change for yaw, roll in zip(yaw_diffs, roll_diffs)):
        zoom('ctrl+plus', max(pitch_diffs))
        print("Zoom in")
    elif pitch_diffs[1] >= val_change and pitch_diffs[0] <= -val_change and all(abs(yaw) < val_change and abs(roll) < val_change for yaw, roll in zip(yaw_diffs, roll_diffs)):
        zoom('ctrl+minus', max(-pitch_diffs[0], pitch_diffs[1]))
        print("Zoom out")
    elif pitch_diffs[0] >= val_change and all(abs(yaw) < no_change_threshold and abs(roll) < no_change_threshold for yaw, roll in zip(yaw_diffs, roll_diffs)):
        scroll_fast(-1, pitch_diffs[0])
        print("Scroll down")
    elif pitch_diffs[0] <= -val_change and all(abs(yaw) < no_change_threshold and abs(roll) < no_change_threshold for yaw, roll in zip(yaw_diffs, roll_diffs)):
        scroll_fast(1, abs(pitch_diffs[0]))
        print("Scroll up")
    elif all(diff >= val_change for diff in pitch_diffs):
        move_horizontal('left', max(pitch_diffs))
        print("Move left")

# Data decoding
def decode(data):
    finger, values = data.split(":")
    return [float(v) for v in values.split(",")] + [finger]

# Main execution
if __name__ == "__main__":
    server_ip = '0.0.0.0'
    port = 8080

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((server_ip, port))
    server_socket.listen(5)
    print(f"Server listening on {server_ip}:{port}")

    buffer = {"Thumb": [], "Middle": [], "Point": []}
    flag = False
    count = 0

    try:
        while True:
            client_socket, addr = server_socket.accept()
            data = client_socket.recv(1024).decode('utf-8')
            client_socket.close()
            
            decoded_data = decode(data)
            finger = decoded_data[3]

            if not flag:
                if finger != "Main":
                    buffer[finger] = decoded_data[:3]
                count += 1
                if count % 4 == 0:
                    flag = True
                    count = 0
                continue

            if finger != "Main" and all(buffer[f] for f in buffer):
                pitch_diffs = [decoded_data[i] - buffer[finger][i] for i in range(3)]
                yaw_diffs = [decoded_data[1] - buffer[finger][1] for i in range(3)]
                roll_diffs = [decoded_data[2] - buffer[finger][2] for i in range(3)]
                
                detect_gesture(pitch_diffs, yaw_diffs, roll_diffs)

    except KeyboardInterrupt:
        print("Server stopped.")
    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        server_socket.close()