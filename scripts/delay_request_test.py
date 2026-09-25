import socket
import time

# 1. First part of the request (incomplete headers)
initial_part = (
    "POST /post-test HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 11\r\n"
)

# 2. Final part to complete the request (header terminator + body)
remaining_part = (
    "\r\n"
    "hello world"
)

sock = socket.create_connection(("localhost", 8080))
sock.settimeout(15.0)  # Ensure socket won't block forever on recv

print("[+] Sending initial incomplete request...")
sock.sendall(initial_part.encode())

print("[+] Waiting for 35 seconds before sending the rest...")
time.sleep(35)

print("[+] Sending remaining part of the request...")
sock.sendall(remaining_part.encode())

try:
    response = sock.recv(4096).decode()
    if response:
        print("[+] Received response:\n", response)
    else:
        print("[-] Server closed connection during the pause.")
except socket.timeout:
    print("[-] Read timed out: Server did not respond within the time limit.")
finally:
    sock.close()