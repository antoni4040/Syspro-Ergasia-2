#!/usr/bin/env bash

ids=()
bytesSent=0
bytesReceived=0
filesSent=0
filesReceived=0
clientsDeleted=0

# Read log files line by line:
while read line; do
    words=( $line )
    numWords=${#words[@]}
    
    if [[ $numWords -eq 1 ]]; then
        ids=("${ids[@]}" $line)
    elif [[ $numWords -eq 2 ]]; then
        let clientsDeleted+=1
    else
        if [[ ${words[0]} = "S" ]]; then
            let filesSent+=1
            let bytesSent+=${words[2]}
        else
            let filesReceived+=1
            let bytesReceived+=${words[2]}
        fi
    fi
done

# Find min-max ids:
min=${ids[0]}
max=0
for i in "${ids[@]}"; do
    if [[ $i -gt max ]]; then
        max=$i
    fi
    if [[ $i -lt max ]]; then
        min=$i
    fi
done

echo Number of clients: ${#ids[@]}
echo Clients: ${ids[@]}
echo Min ID: $min, Max ID: $max
echo Bytes sent: $bytesSent
echo Bytes received: $bytesReceived
echo Files sent: $filesSent
echo Files received: $filesReceived
echo Clients that left the system: $clientsDeleted