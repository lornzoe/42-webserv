import socket

request = (
    "POST /post-test HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Content-Type: text/html\r\n"
)

sock = socket.create_connection(("localhost", 8080))
sock.sendall(request.encode())
print(sock.recv(4096).decode())
sock.close()
