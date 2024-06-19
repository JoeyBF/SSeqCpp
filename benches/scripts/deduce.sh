#!/usr/bin/env bash

# Assign input arguments to variables
degree=$1
diagram=$2

# Define the expected output
expected_output="Summary - Changed differentials: 0"

# Function to execute a command, print its output, and check the output
check_command() {
    local command_output
    command_output=$("$@")
    echo "$command_output"  # Print the output
    if [[ "$command_output" != *"$expected_output"* ]]; then
        return 1
    fi
    return 0
}

echo "--- Deducing ---"
while true; do
    # Execute the commands and check the output
    check_command ./ss deduce diff 0 "$degree" "$diagram" || continue
    check_command ./ss deduce diff_v2 "$diagram" || continue
    check_command ./ss deduce cofseq 0 "$degree" "$diagram" || continue
    check_command ./ss deduce synthetic "$diagram" || continue

    # If all commands succeed, exit the loop
    break
done

echo "All commands completed successfully."
