import os
import sys

body = sys.stdin.read()

print("Content-Type: text/plain\r\n\r\n")
print("Received body:", body)

print("CGI Environment Variables")
print("=========================")

for key, value in os.environ.items():
    print(f"{key} = {value}")
