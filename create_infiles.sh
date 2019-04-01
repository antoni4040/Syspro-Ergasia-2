#!/usr/bin/env bash

# Function to generate a random alphanumerical string, of random size starting from the first parameter
# up to the second paremeter:
generate_random_string() {
    let length=$((RANDOM%=$2))+$1;
    string=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $length | head -n 1)
    echo $string
}

# Get the command line parameters:
dirname="$1"
numOfFiles="$2"
numOfDirs="$3"
levels="$4"

echo $dirname
echo $numOfFiles
echo $numOfDirs
echo $levels

# Check to see that the numOfFiles, numOfDirs and levels variables are numbers with regex:
isNum='^[0-9]+$'
if ! [[ $numOfFiles =~ $isNum ]] || ! [[ $numOfDirs =~ $isNum ]] || ! [[ $levels =~ $isNum ]]; then
    >&2 echo "Error: you didn't give the proper parameters genius..."
    exit 1
fi

# Make dirname directory if it doesn't exist:
mkdir -p $dirname
fullpath=$(readlink -f $dirname)
cd $dirname

level=0
queue=()
# For every directory that must be created:
for ((i = 0 ; i < $numOfDirs ; i++)); do

    # Keep making random dir names until we have one that doesn't already exist:
    while true; do
        dirname=$(generate_random_string 1 7)
        if [ ! -d "$dirname" ]; then
            break
        fi
    done

    # Create the directory and go in it to continue the process:
    mkdir $dirname
    queue=("${queue[@]}" $(readlink -f $dirname))
    cd $dirname
    let level+=1

    # If we've reached the last level, cd to initial dir:
    if [[ $level == $levels ]]; then
        cd $fullpath
        level=0
    fi
done

fileIndex=0
# For every file that must be created:
for ((i = 0 ; i < $numOfFiles ; i++)); do
    currentDir=${queue[$fileIndex]}
    cd $currentDir

    # Keep making random filenames until we have one that doesn't already exist:
    while true; do
        filename=$(generate_random_string 1 7)
        if [ ! -f "$filename" ]; then
            break
        fi
    done

    # Generate random text and write it to the file:
    text=$(generate_random_string 1024 131072)
    echo $text > $filename

    let fileIndex+=1

    # If we're at the end of the queue, go to the beginning:
    if [[ $fileIndex -gt ${#queue[@]} ]]; then
        fileIndex=0
    fi
done