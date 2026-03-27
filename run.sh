#!/bin/bash

if [ "$#" -ne 2 ]; then
  echo "Usage: $0 <path_to_input.json> <path_to_output.json>"
  exit 1
fi

./bin/sim "$1" "$2"
