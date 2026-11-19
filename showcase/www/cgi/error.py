#!/usr/bin/python3
# Prints headers, then crashes (exit status 1).
print("Content-Type: text/plain\r\n\r\n", end="")
print(1 / 0)
