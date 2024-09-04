import pyautogui
import time
time.sleep(1)
# Press left arrow key 10 times
for _ in range(10):
    pyautogui.press('a')
    print("left")
    time.sleep(0.1)  # Adding a small delay between key presses

# Press right arrow key 10 times
for _ in range(10):
    pyautogui.press('d')
    print("right")
    time.sleep(0.1)  # Adding a small delay between key presses
