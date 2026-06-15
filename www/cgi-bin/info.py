#!/usr/bin/env python3
"""
info.py — CGI demonstration script
-----------------------------------
A CGI script must write its response to stdout in two parts:

  1. HTTP headers (one per line, ending with a blank line)
  2. The response body

The web server (webserv) sets environment variables before exec'ing this
script. The most important ones we use here:

  REQUEST_METHOD   — "GET" or "POST"
  QUERY_STRING     — URL query string, e.g. "name=foo&year=2024"
  CONTENT_LENGTH   — byte count of POST body (empty string if no body)
  HTTP_HOST        — value of the Host header
  SERVER_NAME      — server hostname from config
  SCRIPT_NAME      — URI path of this script

For POST requests, the request body arrives on stdin.

The CGI reads all this, generates a response, and writes:
  Content-Type: text/html\r\n
  \r\n
  <html>...</html>
"""

import os
import sys
import cgi       # for parsing query string / POST body
import cgitb     # for pretty error tracebacks (dev only)
cgitb.enable()

# ------------------------------------------------------------------
# Parse query string from the environment (works for GET and POST)
# ------------------------------------------------------------------
form = cgi.FieldStorage()

# Collect some environment variables to display
env_vars = [
    "REQUEST_METHOD",
    "QUERY_STRING",
    "CONTENT_LENGTH",
    "CONTENT_TYPE",
    "HTTP_HOST",
    "HTTP_USER_AGENT",
    "SCRIPT_NAME",
    "PATH_INFO",
    "SERVER_NAME",
]

# Read POST body if present
body = ""
content_length = os.environ.get("CONTENT_LENGTH", "0")
if content_length and int(content_length) > 0:
    body = sys.stdin.read(int(content_length))

# Build the response HTML
name = form.getvalue("name", "World")
year = form.getvalue("year", "2024")

html = """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<title>CGI Info</title>
<style>
  body {{ font-family: 'Courier New', monospace; background: #0f0f0f;
          color: #e0e0e0; padding: 40px 20px; }}
  .container {{ max-width: 700px; margin: 0 auto; }}
  h1 {{ color: #00ff88; margin-bottom: 24px; }}
  h2 {{ color: #888; font-size: 0.9rem; text-transform: uppercase;
        letter-spacing: 2px; margin: 24px 0 8px; }}
  table {{ width: 100%; border-collapse: collapse; }}
  td {{ padding: 6px 10px; border-bottom: 1px solid #222;
        font-size: 0.85rem; }}
  td:first-child {{ color: #888; width: 40%; }}
  td:last-child {{ color: #00ff88; word-break: break-all; }}
  .body-box {{ background: #1a1a1a; padding: 12px; border-left: 3px solid #00ff88;
               font-size: 0.85rem; color: #aaa; margin-top: 4px; }}
  a {{ color: #00ff88; }}
</style>
</head>
<body>
<div class="container">
<h1>CGI Script Output</h1>
<p style="color:#555">Script: <code>{script}</code></p>

<h2>Query Parameters</h2>
<table>
  <tr><td>name</td><td>{name}</td></tr>
  <tr><td>year</td><td>{year}</td></tr>
</table>

<h2>Environment Variables</h2>
<table>
{env_rows}
</table>

<h2>Request Body (POST)</h2>
<div class="body-box">{body}</div>

<p style="margin-top:32px"><a href="/">← Back</a></p>
</div>
</body>
</html>""".format(
    script=os.environ.get("SCRIPT_NAME", "/cgi-bin/info.py"),
    name=name,
    year=year,
    env_rows="\n".join(
        "  <tr><td>{}</td><td>{}</td></tr>".format(
            v, os.environ.get(v, "<not set>")
        )
        for v in env_vars
    ),
    body=body if body else "<em>(none)</em>",
)

# ------------------------------------------------------------------
# Write the CGI response: headers first, then blank line, then body
# ------------------------------------------------------------------
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("\r\n")
sys.stdout.write(html)
