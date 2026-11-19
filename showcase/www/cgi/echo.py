#!/usr/bin/python3
# Echoes method, length and body.
import os, sys

body = sys.stdin.read()
print("Content-Type: text/plain\r\n\r\n", end="")
print("method: " + os.environ.get("REQUEST_METHOD", ""))
print("CONTENT_LENGTH: " + os.environ.get("CONTENT_LENGTH", ""))
print("body: " + (body or "(empty)"))
