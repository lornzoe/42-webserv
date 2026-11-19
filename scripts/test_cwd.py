#!/usr/bin/env python3

import os

print("Content-Type: text/plain")
print()

print("CGI working directory:", os.getcwd())
print("Script location:", os.path.dirname(os.path.abspath(__file__)))