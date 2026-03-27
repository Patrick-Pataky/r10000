#!/bin/bash

cd ..

./build.sh

for tnum in ./given_tests/*
do	
	# Check if it's a number
	base_num=$(basename "$tnum")
	if ! [[ "$base_num" =~ ^[0-9]+.*$ ]]; then
		continue
	fi

  ./run.sh ${tnum}/input.json ${tnum}/user_output.json

	base_num_decimal=$(echo $base_num | sed 's/^0*//')
	if [[ $base_num_decimal -gt 13 ]]; then
		mv ${tnum}/user_output.json ${tnum}/output.json
	fi
done
