#! /bin/bash

# This file can be executed by calling "bash run_tests.sh"
# It will then test tube1 against the reference_implementation for
# each tube file in the test-suite
project=tube4
make clean
make
if [ ! -f $project ]; then 
	echo $project "not correctly compiled";
	exit 1;
fi;
if [ ! -f example.tube ]; then 
	echo "example.tube doesn't exist";
	exit 1;
fi;

function run_error_test {
    ./$project $1 $project.tic > $project.cout;
    Test_Suite/reference_$project $1 ref.tic > ref.cout;
    diff -w ref.cout $project.cout;
    result=$?;
    rm $project.cout ref.cout;
    if [ $result -ne 0 ]; then
	echo $1 "failed different error messages";
	rm $project.tic;
	return 1;
    fi;

    if [ -e ref.tic ] ; then
	    Test_Suite/TubeIC $project.tic > $project.out
	    Test_Suite/TubeIC ref.tic > ref.out
	    rm  ref.tic $project.tic;
	    diff -w ref.out $project.out;
	    result=$?;
	    rm $project.out ref.out;
	    if [ $result -ne 0 ]; then
		echo $1 "failed different executed result on TubeIC";
		return 1;
	    else
		echo $1 "passed";
	    fi;
    else 
    	echo $1 "passed";
    fi

}
for F in Test_Suite/good*.tube; do 
	run_error_test $F
done
for F in Test_Suite/fail*.tube; do 
	run_error_test $F
done
for F in Test_Suite/test.*.tube; do 
	run_error_test $F
done


echo Extra Credit Results:

for F in Test_Suite/extra.*.tube; do 
	run_error_test $F
done



