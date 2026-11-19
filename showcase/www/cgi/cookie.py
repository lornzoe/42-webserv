#!/usr/bin/python3
# Increments a visits cookie.
import os, re

m = re.search(r"(?:^|;\s*)visits=(\d+)", os.environ.get("HTTP_COOKIE", ""))
visits = int(m.group(1)) + 1 if m else 1
print("Content-Type: text/plain\r\nSet-Cookie: visits=%d; Path=/\r\n\r\nvisits=%d" % (visits, visits), end="")
