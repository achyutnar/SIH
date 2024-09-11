import socket
import torch
import torch.nn as nn
import math
import pyttsx3

engine = pyttsx3.init()

def speak_text(text):
    """
    Convert the provided text to speech.
    
    Args:
    text (str): The text to convert to speech.
    """
    engine.say(text)
    engine.runAndWait()

def get_gesture(hand_landmarks):
    thumb_tip = hand_landmarks[4]
    index_finger_tip = hand_landmarks[8]
    middle_finger_tip = hand_landmarks[12]
    ring_finger_tip = hand_landmarks[16]
    little_finger_tip = hand_landmarks[20]

    # Check if hand is in OK gesture
    if thumb_tip[1] < index_finger_tip[1] < middle_finger_tip[1] < ring_finger_tip[1] < little_finger_tip[1]:
        speak_text("Okay")
        return "Okay"

    # Check if hand is in Dislike gesture
    elif thumb_tip[1] > index_finger_tip[1] > middle_finger_tip[1] > ring_finger_tip[1] > little_finger_tip[1]:
        speak_text("Dislike")
        return "Dislike"

    # Check if hand is in Victory gesture
    elif index_finger_tip[1] < middle_finger_tip[1] and abs(index_finger_tip[0] - middle_finger_tip[0]) < 0.2:
        speak_text("Victory")
        return "Victory"

    # Check if hand is in Stop gesture
    elif thumb_tip[0] < index_finger_tip[0] < middle_finger_tip[0]:
        if (hand_landmarks[2][0] < hand_landmarks[5][0]) and (hand_landmarks[3][0] < hand_landmarks[5][0]) and (hand_landmarks[4][0] < hand_landmarks[5][0]):
            speak_text("Stop")
            return "Stop"
        else:
            return None

# The rest of your LandmarkPredictor model and socket communication code goes here
class LandmarkPredictor(nn.Module):
    def __init__(self, input_size, hidden_size, output_size):
        super(LandmarkPredictor, self).__init__()
        self.hidden = nn.Linear(input_size, hidden_size)
        self.bn = nn.BatchNorm1d(hidden_size)
        self.output = nn.Linear(hidden_size, output_size)
        self.activation = nn.ReLU()

    def forward(self, x):
        x = self.activation(self.bn(self.hidden(x)))
        x = self.output(x)
        return x

input_size = 18  # Adjust based on the actual input size
hidden_size = 64  # Adjust these sizes based on your model configuration
output_size = 63  # Adjust based on your model configuration
model = LandmarkPredictor(input_size, hidden_size, output_size)

# Load the pre-trained model state dictionary
model.load_state_dict(torch.load("./models/landmark_predictor.pth", map_location=torch.device('cpu')))

# Set the model to evaluation mode
model.eval()

def parse_pry_data(data):
    finger_data = data.split(';')
    parsed_data = {}

    for finger in finger_data:
        parts = finger.split(':')
        if len(parts) == 2:
            finger_name = parts[0]
            values = parts[1].split(',')

            if len(values) == 3:
                pitch = float(values[0])
                roll = float(values[1])
                yaw = float(values[2])
                parsed_data[finger_name] = [pitch, roll, yaw]

    ss = ""
    for finger_name, values in parsed_data.items():
        for value in values:
            ss += f"{value:.2f},"  # Format to 2 decimal places and add comma
    
    return ss.rstrip(',')

if __name__ == "__main__":
    server_ip = '0.0.0.0'  # Listen on all interfaces
    port = 7001

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((server_ip, port))
    server_socket.listen(5)
    print(f"Server listening on {server_ip}:{port}")

    try:
        while True:
            # Establish a connection with a client
            client_socket, addr = server_socket.accept()
            print(f"Connection from {addr}")

            # Receive the data
            data = client_socket.recv(1024).decode('utf-8')

            # Parse the data
            parsed_data = parse_pry_data(data)
            print(parsed_data)
            input_list = [float(i) for i in parsed_data.split(',')]
            input_tensor = torch.tensor(input_list).unsqueeze(0)  # Add batch dimension

            with torch.no_grad():
                output = model(input_tensor)

            # Print the output
            output = torch.squeeze(output)
            output = output.view(-1, 3)
            print(get_gesture(output))

            # Close the connection
            client_socket.close()

    except KeyboardInterrupt:
        print("\nServer shutting down...")
    finally:
        server_socket.close()
