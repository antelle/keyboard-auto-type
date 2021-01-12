#!/bin/bash
set -euo pipefail

mkdir -p build/test
:>build/test/file.txt

open /System/Applications/TextEdit.app build/test/file.txt

for i in {1..100}
do
    win=$(osascript -e 'tell application "System Events" to set frontAppName to name of first process whose frontmost is true')
    if [[ "$win" == "TextEdit" ]]; then
        break
    fi
    sleep 0.1
done

if [[ "$i" -eq 100 ]]; then
    echo "TextEdit didn't start"
    exit 1
fi

sleep 1

echo Testing...

./build/output/example

echo "Done, waiting for TextEdit to exit..."

for i in {1..100}
do
    if [[ ! $(pgrep TextEdit) ]]; then
        break
    fi
    sleep 0.1
done

if [[ "$i" -eq 100 ]]; then
    echo "TextEdit didn't exit"
    killall TextEdit
    exit 1
fi

echo "File contents:"

cat build/test/file.txt
