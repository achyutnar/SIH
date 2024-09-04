import socket


server_ip = '0.0.0.0'  # Listen on all interfaces
port = 8080

# Create a socket object
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind to the port
server_socket.bind((server_ip, port))

# Start listening for connections (max 5 queued connections)
server_socket.listen(5)
print("Server listening on {}:{}".format(server_ip, port))

while True:
    # Establish a connection with a client
    client_socket, addr = server_socket.accept()
    print("Got connection from", addr)

    # Receive the data
    data = client_socket.recv(1024).decode('utf-8')
    print("Received data:", data)

    # Close the connection
    client_socket.close()