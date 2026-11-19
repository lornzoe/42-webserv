#!/usr/bin/python3
# Opens message.txt by relative path: works only if run from this directory.
import os

print("Content-Type: text/plain\r\n\r\n", end="")
print("cwd: " + os.getcwd())
try:
    print("message.txt: " + open("message.txt").read().strip())
except OSError as e:
    print("message.txt: " + str(e))
