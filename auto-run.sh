#!/bin/bash

clear

echo "Compiling the program..."
g++ -std=c++17 hw6.cpp -o hw6
echo "Compiling completed."
echo "Current PWD: /home/assignments/changeme"

if [ $? -eq 0 ]; then
    echo -e "Running the program...\n"
    ./hw6
else
    echo "Compilation failed."
fi

# Drop back to the shell
exec /bin/bash
