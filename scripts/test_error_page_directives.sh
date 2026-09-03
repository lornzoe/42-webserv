#!/bin/bash
# needs bash for /dev/tcp (the malformed-request test); dash/sh won't work.

SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(cd -- "$SCRIPT_DIR/.." && pwd)
WEBSERV="$ROOT_DIR/webserv"
CONF="$ROOT_DIR/configs/error_page_test.conf"

# helper functions
linebreak() {
    echo "----------------------------------------------------------------------"
}

title() {
    linebreak
    echo "$1"
    linebreak
}

# --- launch webserv with configs/error_page_test.conf ----------------------
# On Linux we start it ourselves and kill it on exit. 
# On macOS webserv can't be built/run natively (epoll), so we assume you started it in a container.
WS_PID=""
if [ "$(uname)" = "Darwin" ]; then
    echo "[macOS] not launching webserv — expecting it to be running separately"
else
    if [ ! -x "$WEBSERV" ]; then
        echo "error: $WEBSERV not found or not executable (run 'make' first)" >&2
        exit 1
    fi
    "$WEBSERV" "$CONF" >/dev/null 2>&1 &
    WS_PID=$!
    trap '[ -n "$WS_PID" ] && kill "$WS_PID" 2>/dev/null' EXIT INT TERM

    # wait for the listening socket (max ~5s)
    for _ in $(seq 1 50); do
        curl -s -o /dev/null http://127.0.0.1:8080/ && break
        kill -0 "$WS_PID" 2>/dev/null || { echo "error: webserv exited during startup" >&2; exit 1; }
        sleep 0.1
    done
fi

echo "testing error_page directives;"

# curl :8080/override/x	404 → errors/location_404.html (override wins)
# curl :8080/broken/x	404 → built-in default body (missing file falls back)
# curl -X PUT :8080/	501 → errors/server.html
# bad HTTP version on :8080	505 → errors/server.html
# curl :8081/nope	404 → built-in default body (control)

# run a bunch of curl reequests and see responses:
title "curl :8080/override/x -- 404 → errors/location_404.html"
curl -i localhost:8080/override/x

title "curl :8080/broken/x -- 404 → built-in default body"
curl -i localhost:8080/broken/x

title "curl -X PUT :8080/ -- 501 → errors/server.html"
curl -i -X PUT localhost:8080/

title "bad HTTP version on :8080 -- 505 → errors/server.html"
exec 3<>/dev/tcp/127.0.0.1/8080
printf 'GET / HTTP/9.9\r\nHost: x\r\n\r\n' >&3
timeout 2 cat <&3
exec 3<&-

title "curl :8081/nope -- 404 → built-in default body (control)"
curl -i localhost:8081/nope

# webserv is killed by the EXIT trap above (Linux only).