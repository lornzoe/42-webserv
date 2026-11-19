#!/bin/bash

# script directory
SCRIPTDIR=$(cd -- "$(dirname -- "$0")" && pwd)
echo "SCRIPTDIR: $SCRIPTDIR"

#root directory
ROOTDIR=$(cd -- "$SCRIPTDIR/.." && pwd)
echo "ROOTDIR: $ROOTDIR"

# binary to run under valgrind (can be checker or webserv)
if [ -n "$1" ]; then
    WEBSERV="$1"
else
    WEBSERV="$ROOTDIR/webserv"
fi
echo "WEBSERV: $WEBSERV"

if ! command -v valgrind >/dev/null 2>&1; then
    echo "error: valgrind not found" >&2
    exit 1
fi
if [ ! -x "$WEBSERV" ]; then
    # run make
    echo "No binary found, building..."
    echo "Silently running 'make fclean'..."
    make -C "$ROOTDIR" fclean > /dev/null 2>&1 || exit 1
    echo "Silently running 'make debug'..."
    make -C "$ROOTDIR" debug  > /dev/null 2>&1 || exit 1
    echo "$WEBSERV binary built."
fi

# configs directory
CONFIGSDIR="$ROOTDIR/configs"
echo "CONFIGSDIR: $CONFIGSDIR"

# landing dir for valgrind logs
LOGDIR="$ROOTDIR/build/valgrind"
echo "LOGDIR: $LOGDIR"

VALGRIND_OPTS=(--leak-check=full --show-leak-kinds=all)
echo "VALGRIND_OPTS: ${VALGRIND_OPTS[@]}" "--log-file=$LOGDIR/(config).log"

rm -rf "$LOGDIR" || exit 1
mkdir -p "$LOGDIR" || exit 1
echo ""

for conf in "$CONFIGSDIR"/*.conf; do
    name=$(basename "$conf" .conf)
    echo "=== Running valgrind on config: $name ==="
    log="$ROOTDIR/build/valgrind/$name.log"
    out="$ROOTDIR/build/valgrind/$name.out"

    # run valgrind with the config
    if  [ "$(basename "$WEBSERV")" != "webserv" ]; then
        timeout 5s valgrind \
        ${VALGRIND_OPTS[@]} --log-file="$log" "$WEBSERV" "$conf" > "$out" 2>&1
    else
        (sleep 3; echo "exit") | timeout 10s valgrind \
        ${VALGRIND_OPTS[@]} --log-file="$log" "$WEBSERV" "$conf" > "$out" 2>&1
    fi

    exitcode=$?
    case $exitcode in
        124) echo "valgrind timed out after 5 seconds (normal behavior)." ;;
        137) echo "valgrind was killed (did not exit on SIGINT)." ;;
        *)   echo "valgrind exited with code $exitcode." ;;
    esac

    if [ ! -s "$log" ]; then
        echo "NO REPORT: $run"
    elif grep -q "All heap blocks were freed" "$log"; then
        echo "OK: $run"
    else
        echo "LEAKED: $run"
        grep -A5 "LEAK SUMMARY:" "$log"
    fi
    echo ""

done