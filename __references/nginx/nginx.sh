#!/bin/sh
# Control the reference nginx instance. Run from the PROJECT ROOT, inside the
# webserv dev container (nginx-light is installed via .devcontainer/Dockerfile).
#
#   ./__references/nginx/nginx.sh start    - start in the background
#   ./__references/nginx/nginx.sh run      - run in the foreground (Ctrl-C to quit)
#   ./__references/nginx/nginx.sh stop     - stop it
#   ./__references/nginx/nginx.sh reload   - reload config without dropping conns
#   ./__references/nginx/nginx.sh test     - check config syntax
#
# Listens on http://localhost:8084 with document root ./html
#
# From the macOS host you can reach the same commands with:
#   docker exec -w /workspaces/42-webserv 42-webserv ./__references/nginx/nginx.sh start

set -e

# find the nginx binary (PATH, then the usual Debian/Ubuntu location)
NGINX="${NGINX:-$(command -v nginx || echo /usr/sbin/nginx)}"
PREFIX="$(pwd)"
CONF="__references/nginx/nginx.conf"

mkdir -p __references/nginx/logs __references/nginx/tmp

case "$1" in
    start)   exec "$NGINX" -p "$PREFIX/" -c "$CONF" -g "daemon on;" ;;
    run)     exec "$NGINX" -p "$PREFIX/" -c "$CONF" -g "daemon off;" ;;
    stop)    exec "$NGINX" -p "$PREFIX/" -c "$CONF" -s stop ;;
    reload)  exec "$NGINX" -p "$PREFIX/" -c "$CONF" -s reload ;;
    test|t)  exec "$NGINX" -p "$PREFIX/" -c "$CONF" -t ;;
    *)       echo "usage: $0 {start|run|stop|reload|test}" >&2; exit 1 ;;
esac
