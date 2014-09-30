#! /bin/bash

# This file can be executed by calling "bash run_tests.sh"
# It will then test tube1 against the reference_implementation for
# each tube file in the test-suite
make
if [ ! -f tube3 ]; then 
	echo "tube3 not correctly compiled";
	exit 1;
fi;
if [ ! -f example.tube ]; then 
	echo "example.tube doesn't exist";
	exit 1;
fi;

function run_test {
    desired_output=Test_Suite/$(basename $1 .tube).out;
    ./tube3 $1 intermediate.tic > /dev/null;
    if [ ! -f intermediate.tic ]; then
	echo "no intermediate code generated";
	exit 1;
    fi;
    diff $desired_output <(Test_Suite/TubeIC intermediate.tic);
    result=$?;
    rm intermediate.tic;
    if [ $result -ne 0 ]; then
	echo $1 "failed";
	exit 1;
    else
	echo $1 "passed";
    fi;
}

for F in Test_Suite/*.tube; do 
	run_test $F
done

