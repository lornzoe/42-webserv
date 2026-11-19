#!/usr/bin/python3
# ?code=N -> "Status: N" header; ?go=/path -> "Location:" only (server sends 302).
import os
from urllib.parse import parse_qs

q = parse_qs(os.environ.get("QUERY_STRING", ""))
if "go" in q:
    print("Location: %s\r\n\r\n" % q["go"][0], end="")
else:
    code = q.get("code", ["200"])[0]
    if code not in ("200", "201", "202", "404", "409", "500"):
        code = "400"
    print("Status: %s\r\nContent-Type: text/plain\r\n\r\nstatus %s" % (code, code), end="")
