 if [ -d "build" ]; then
      echo "build exists"
      echo "purging build directory"
      rm -rf build
fi

for p in debug relwithdebinfo release aarch64-debug; do
    echo "========================"
    echo "making preset: $p"

    configure_output=$(cmake --preset $p 2>&1)
    if ! echo "$configure_output" | grep -q "Configuring done" || ! echo "$configure_output" | grep -q "Generating done"; then
        echo "$p: configure FAILED"
        echo "$configure_output"
        exit 1
    fi
        echo "$p: configure OK"

    if ! cmake --build --preset $p; then
        echo "$p: build FAILED"
        exit 1
    fi
        echo "$p: build OK"
done

GOOD_FILE=good_data.txt
BAD_FILE=bad_data.txt

echo -e "0 1 24.8 3.1 41.0 1 14\n100 2 24.7 3.2 41.4 1 14" > $GOOD_FILE
echo -e "0 1 24.8 3.1 41.0 1\n100 2 24.7 3.2 41.4 1 14" > $BAD_FILE

run() {
    FILE=$1
    EXPECTED_CODE=$2

    for d in debug relwithdebinfo release; do
        ./build/"$d"/homework_05/telemetry_check $FILE
    
        code=$?
    
        if [ "$code" -eq $EXPECTED_CODE ]; then
            echo "================"
            echo "$d sanity passed"
            echo "================"
        else
            echo "================"
            echo "$d sanity failed"
            echo "================"

            rm -rf $FILE
            exit 1
        fi
    done
    rm -rf $FILE
}

run $GOOD_FILE 0
run $BAD_FILE 1

echo "All tests passed"

exit 0