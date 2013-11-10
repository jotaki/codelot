#! /bin/bash

server="$1"
nickname="$2"
port=6667

if [ -z "$server" -o -z "$nickname" ]; then
    echo "Usage: $0 <server>[:port] <nickname>"
    exit 1
fi

if [[ "$server" =~ ":" ]]; then
    server=$(echo $1 | cut -f1 -d':')
    port=$(echo $1 | cut -f2 -d':')
fi

echo "Connecting to $server:$port as $nickname"
exec 9<>/dev/tcp/$server/$port

echo "NICK $nickname" >&9
echo "USER $nickname 0 0 :$nickname" >&9

while true; do
    read -r str <&9
    echo "$str"

    if [[ "$str" =~ ^PING ]]; then
        echo 'Ping? Pong!'
        echo "PONG :$(echo "$str" | cut -f2 -d':')" >&9
    fi
done
