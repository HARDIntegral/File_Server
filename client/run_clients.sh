#!/bin/sh

#run from project dir not client dir

files=($(pwd)/data/test_files/*)

for N in {1..100000}
do
    file=${files[RANDOM % ${#files[@]}]}
    ruby client/client.rb ${file} &
done
wait