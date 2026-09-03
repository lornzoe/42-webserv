import socket

request = (
    "POST /post-test HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Transfer-Encoding: chunked\r\n"
    "Content-Type: application/x-www-form-urlencoded\r\n"
    "\r\n"
    "5\r\n"
    "hello\r\n"
    "6\r\n"
    " world\r\n"
    "0\r\n"
    "\r\n"
)

sock = socket.create_connection(("localhost", 8080))
sock.sendall(request.encode())
print(sock.recv(4096).decode())
sock.close()