#!/usr/bin/env python3
"""
echo.py — CGI POST echo script
--------------------------------
Reads the POST body from stdin and echoes it back as JSON.
Useful for testing that chunked / regular POST bodies arrive correctly.

Example:
  curl -X POST http://localhost:8080/cgi-bin/echo.py \
       -H "Content-Type: text/plain" \
       -d "Hello from curl"
"""

import os
import sys
import json

method = os.environ.get("REQUEST_METHOD", "GET")
content_length = int(os.environ.get("CONTENT_LENGTH", "0") or "0")
content_type   = os.environ.get("CONTENT_TYPE", "")

body = ""
if content_length > 0:
    body = sys.stdin.read(content_length)

response = {
    "method":         method,
    "content_type":   content_type,
    "content_length": content_length,
    "body":           body,
    "query_string":   os.environ.get("QUERY_STRING", ""),
}

output = json.dumps(response, indent=2)

sys.stdout.write("Content-Type: application/json\r\n")
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("\r\n")
sys.stdout.write(output)
