#!/bin/bash
cd data
for file in `ls -A1` 
do 
    echo ${file}
    curl -F "file=@$PWD/${file}" myLoc.local/edit
done
