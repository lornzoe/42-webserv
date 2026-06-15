#!/usr/bin/env python3
"""
test_webserv.py — Automated test suite for webserv
=====================================================
Run this after starting the server:

    ./webserv config/default.conf &
    python3 test_webserv.py

Tests cover:
  - Static file serving (GET)
  - 404 Not Found
  - Method Not Allowed (405)
  - Directory listing (autoindex)
  - File upload (POST)
  - File download after upload (GET)
  - File delete (DELETE)
  - HTTP redirect (301)
  - CGI execution (GET with query string)
  - Large body rejection (413)
  - Multiple simultaneous connections (basic stress)

Each test prints PASS or FAIL with a brief description.
Exit code is 0 if all tests pass, 1 if any fail.
"""

import http.client
import sys
import time
import threading
import os
import random
import string

HOST = "127.0.0.1"
PORT = 8080

passed = 0
failed = 0

def conn():
    """Open a fresh HTTP connection to the server."""
    return http.client.HTTPConnection(HOST, PORT, timeout=5)

def test(name, ok, detail=""):
    global passed, failed
    status = "\033[32mPASS\033[0m" if ok else "\033[31mFAIL\033[0m"
    print(f"  [{status}] {name}" + (f" — {detail}" if detail else ""))
    if ok:
        passed += 1
    else:
        failed += 1

def random_string(n=8):
    return ''.join(random.choices(string.ascii_lowercase, k=n))

# ---------------------------------------------------------------------------
print("\n=== webserv test suite ===\n")

# 1. Basic GET — root page
try:
    c = conn()
    c.request("GET", "/index.html")
    r = c.getresponse()
    test("GET /index.html returns 200", r.status == 200,
         f"status={r.status}")
    body = r.read().decode()
    test("Response body contains 'webserv'",
         "webserv" in body.lower(), f"length={len(body)}")
except Exception as e:
    test("GET /index.html", False, str(e))
    test("Response body check", False, "skipped")

# 2. 404 for missing file
try:
    c = conn()
    c.request("GET", "/this/does/not/exist.html")
    r = c.getresponse()
    test("GET non-existent file returns 404", r.status == 404,
         f"status={r.status}")
except Exception as e:
    test("404 test", False, str(e))

# 3. Method not allowed
try:
    c = conn()
    c.request("DELETE", "/index.html")  # /  only allows GET POST
    r = c.getresponse()
    test("DELETE on GET-only location returns 405", r.status == 405,
         f"status={r.status}")
except Exception as e:
    test("405 test", False, str(e))

# 4. Directory listing — /files (autoindex on)
try:
    c = conn()
    c.request("GET", "/files")
    r = c.getresponse()
    body = r.read().decode()
    test("GET /files returns 200 (autoindex)", r.status == 200,
         f"status={r.status}")
    test("Directory listing contains 'Index of'",
         "Index of" in body, "")
except Exception as e:
    test("Directory listing test", False, str(e))
    test("Directory listing body check", False, "skipped")

# 5. File upload via POST
filename = f"test_{random_string()}.txt"
content  = f"webserv upload test — {time.time()}\n"
try:
    c = conn()
    c.request(
        "POST", "/upload",
        body=content.encode(),
        headers={
            "Content-Type": "text/plain",
            "Content-Disposition": f'attachment; filename="{filename}"',
            "Content-Length": str(len(content)),
        }
    )
    r = c.getresponse()
    r.read()
    test("POST /upload returns 201", r.status == 201, f"status={r.status}")
except Exception as e:
    test("POST /upload", False, str(e))

# 6. Verify uploaded file is accessible
try:
    c = conn()
    c.request("GET", f"/files/{filename}")
    r = c.getresponse()
    body = r.read().decode()
    test("Uploaded file is retrievable via GET /files/…",
         r.status == 200 and content.strip() in body,
         f"status={r.status}")
except Exception as e:
    test("GET uploaded file", False, str(e))

# 7. Delete uploaded file
try:
    c = conn()
    c.request("DELETE", f"/upload/{filename}")
    r = c.getresponse()
    r.read()
    test("DELETE uploaded file returns 204", r.status == 204,
         f"status={r.status}")
except Exception as e:
    test("DELETE file", False, str(e))

# 8. File gone after delete
try:
    c = conn()
    c.request("GET", f"/files/{filename}")
    r = c.getresponse()
    r.read()
    test("File is gone after DELETE (404)", r.status == 404,
         f"status={r.status}")
except Exception as e:
    test("File gone after delete", False, str(e))

# 9. Redirect
try:
    c = conn()
    c.request("GET", "/old-page")
    r = c.getresponse()
    r.read()
    location = r.getheader("Location", "")
    test("GET /old-page returns 301", r.status == 301,
         f"status={r.status}")
    test("Redirect Location header is /index.html",
         location == "/index.html", f"Location: {location}")
except Exception as e:
    test("Redirect test", False, str(e))
    test("Redirect location header", False, "skipped")

# 10. CGI — GET with query string
try:
    c = conn()
    c.request("GET", "/cgi-bin/info.py?name=tester&year=2024")
    r = c.getresponse()
    body = r.read().decode()
    test("CGI GET returns 200", r.status == 200, f"status={r.status}")
    test("CGI output contains query param value",
         "tester" in body, f"length={len(body)}")
except Exception as e:
    test("CGI GET test", False, str(e))
    test("CGI output check", False, "skipped")

# 11. CGI — POST echo
try:
    payload = b'{"hello": "webserv"}'
    c = conn()
    c.request(
        "POST", "/cgi-bin/echo.py",
        body=payload,
        headers={
            "Content-Type": "application/json",
            "Content-Length": str(len(payload)),
        }
    )
    r = c.getresponse()
    body = r.read().decode()
    test("CGI POST returns 200", r.status == 200, f"status={r.status}")
    test("CGI POST echoes body",
         "webserv" in body, f"response={body[:80]}")
except Exception as e:
    test("CGI POST test", False, str(e))
    test("CGI POST echo check", False, "skipped")

# 12. Body too large
try:
    big = b"X" * (11 * 1024 * 1024)   # 11 MB — over the 10 MB limit
    c = conn()
    c.request(
        "POST", "/upload",
        body=big,
        headers={
            "Content-Type": "application/octet-stream",
            "Content-Length": str(len(big)),
        }
    )
    r = c.getresponse()
    r.read()
    test("POST body > limit returns 413", r.status == 413,
         f"status={r.status}")
except Exception as e:
    test("413 body-too-large test", False, str(e))

# 13. Stress: many sequential requests
try:
    errors = 0
    for _ in range(50):
        c = conn()
        c.request("GET", "/index.html")
        r = c.getresponse()
        r.read()
        if r.status != 200:
            errors += 1
    test("50 sequential GET requests all return 200",
         errors == 0, f"errors={errors}")
except Exception as e:
    test("Sequential stress test", False, str(e))

# 14. Stress: concurrent connections
try:
    results = []
    def fetch():
        try:
            c = conn()
            c.request("GET", "/index.html")
            r = c.getresponse()
            r.read()
            results.append(r.status)
        except Exception:
            results.append(0)

    threads = [threading.Thread(target=fetch) for _ in range(20)]
    for t in threads: t.start()
    for t in threads: t.join()
    ok = all(s == 200 for s in results)
    test("20 concurrent connections all return 200",
         ok, f"results={results}")
except Exception as e:
    test("Concurrent stress test", False, str(e))

# 15. Second server on port 8081
try:
    c = http.client.HTTPConnection(HOST, 8081, timeout=5)
    c.request("GET", "/")
    r = c.getresponse()
    r.read()
    test("Second server (port 8081) returns 200", r.status == 200,
         f"status={r.status}")
except Exception as e:
    test("Second server port 8081", False, str(e))

# ---------------------------------------------------------------------------
print(f"\n  Passed: {passed}   Failed: {failed}\n")
sys.exit(0 if failed == 0 else 1)
