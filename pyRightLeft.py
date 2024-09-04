import pyautogui
import time

# Press left arrow key 10 times
for _ in range(10):
    pyautogui.press('left')
    time.sleep(0.1)  # Adding a small delay between key presses

# Press right arrow key 10 times
for _ in range(10):
    pyautogui.press('right')
    time.sleep(0.1)  # Adding a small delay between key presses