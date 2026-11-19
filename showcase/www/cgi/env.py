#!/usr/bin/python3
# Prints every CGI variable and the body.
import os, sys

body = sys.stdin.read()
print("Content-Type: text/plain\r\n\r\n", end="")
for k in sorted(os.environ):
    print("%s=%s" % (k, os.environ[k]))
print("\nbody: " + (body or "(empty)"))
