import socket
import time
import threading

def receive_response(sock):
    try:
        response = sock.recv(4096).decode()
        print(f"[{time.monotonic():.2f}] Received:\n{response}")
    except OSError as e:
        print(f"Receive error: {e}")

sock = socket.create_connection(("localhost", 8080))

initial_part = (
    "POST /post-test HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 11\r\n"
)

sock.sendall(initial_part.encode())

thread = threading.Thread(target=receive_response, args=(sock,))
thread.start()

print(f"[{time.monotonic():.2f}] Waiting for 10 seconds...")
time.sleep(10)

sock.close()
thread.join()