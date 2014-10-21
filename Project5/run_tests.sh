#! /bin/bash

# This file can be executed by calling "bash run_tests.sh"
# It will then test tube1 against the reference_implementation for
# each tube file in the test-suite
project=tube5
chmod a+x Test_Suite/reference_$project
chmod a+x Test_Suite/TubeIC

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
summary="Summary:\n";
function run_error_test {
    ./$project $1 $project.tic > $project.cout;
    Test_Suite/reference_$project $1 ref.tic > ref.cout;
    grep -Fq "ERROR" ref.cout
    ref_error=$?
    grep -Fq "ERROR" $project.cout
    proj_error=$?    
    echo $ref_error
    echo $proj_error
    echo [ $proj_error ] && [ $ref_error ]
    if  [ $proj_error -eq "0" ] ; then
    	if [ $ref_error -ne "0" ] ; then
		echo $1 " gave an error when there should not be an error";
		summary=$summary"\n"$1" gave an error when there should not be an error";
		rm $project.tic;
		if [ -e ref.tic ] ; then 
		    rm ref.tic;
		fi
		return 1;

	fi
    else
    	if [ $ref_error -eq "0" ] ; then
		echo $1 " did not give an error when it should have";
		summary=$summary"\n"$1" did not give an error when it should have";
		rm $project.tic;
		if [ -e ref.tic ] ; then 
		    rm ref.tic;
		fi
		return 1;

	fi
    	
    fi

    if [ -e ref.tic ] ; then
	    Test_Suite/TubeIC -t 1000000 $project.tic > $project.out
	    Test_Suite/TubeIC -t 1000000 ref.tic > ref.out
	    rm  ref.tic $project.tic;
	    diff -w ref.out $project.out;
	    result=$?;
	    rm $project.out ref.out;
	    if [ $result -ne 0 ]; then
		echo $1 "failed different executed result on TubeIC";
		summary=$summary"\n"$1" failed different executed result on TubeIC"
		return 1;
	    else
		echo $1 "passed";
		summary=$summary"\n"$1" passed"
	    fi;
    else 
		summary=$summary"\n"$1" passed"
    	echo $1 "passed";
    fi

}
for F in Test_Suite/good*.tube; do 
	run_error_test $F
done
for F in Test_Suite/fail*.tube; do 
	run_error_test $F
done

echo Extra Credit Results:

for F in Test_Suite/extra.*.tube; do 
	run_error_test $F
done


echo -e $summary
