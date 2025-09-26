#!/bin/bash

# Directory for test files
BIMDC_DIR="./BIMDC"
RECEIVED_DIR="./received"

# Make sure the received directory exists
mkdir -p "$RECEIVED_DIR"

# Array of file sizes to test
file_sizes=("1M" "10M" "50M" "100M")  

# Results file
RESULTS="transfer_times.txt"
echo -e "File Size\tTransfer Time (s)\tStatus" > $RESULTS

# Loop through each file size
for size in "${file_sizes[@]}"; do
    FILENAME="test_$size.bin"
    FILEPATH="$BIMDC_DIR/$FILENAME"

    # Create the test file
    truncate -s $size "$FILEPATH"
    echo "Created $FILEPATH of size $size"

    # Remove old received file if exists
    rm -f "$RECEIVED_DIR/$FILENAME"

    # Measure transfer time silently, capturing time
    TIME=$( { /usr/bin/time -f "%e" ./client -f "$FILENAME" >/dev/null; } 2>&1 )

    # Check if the transfer worked by comparing files
    if diff "$FILEPATH" "$RECEIVED_DIR/$FILENAME" >/dev/null 2>&1; then
        STATUS="Success"
    else
        STATUS="Failed"
    fi

    # Save result
    echo -e "$size\t$TIME\t$STATUS" | tee -a $RESULTS
    echo "$size took $TIME seconds to run"
done

echo "All tests done. Results saved in $RESULTS"

