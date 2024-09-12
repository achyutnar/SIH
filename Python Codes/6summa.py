import socket
import time

start_time = time.time()
count = 0

if __name__ == "__main__":
    server_ip = '0.0.0.0'
    port = 6969
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    server_socket.bind((server_ip, port))
    server_socket.listen(5)
    print("Server listening on {}:{}".format(server_ip, port))

    while True:
        client_socket, addr = server_socket.accept()
        # Receive the data
        data = client_socket.recv(1024).decode('utf-8')
        client_socket.close()

        # Increment the count for each received data stream
        count += 1

        # Calculate the elapsed time
        elapsed_time = time.time() - start_time

        # Print the count and frequency every second
        if elapsed_time >= 1:
            print("Count =", count)
            print("Frequency =", count / elapsed_time)
            start_time = time.time()  # Reset start time
            count = 0  # Reset count for the next interval
