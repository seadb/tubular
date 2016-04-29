#! /bin/bash

# This file can be executed by calling "bash run_tests.sh"
# It will then test tube1 against the reference_implementation for
# each tube file in the test-suite
project=tube8
chmod a+x Test_Suite/reference_$project
chmod a+x Test_Suite/TubeIC
chmod a+x Test_Suite/tubecode

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
    ./$project $1 $project.tca > $project.cout;
    Test_Suite/reference_$project $1 ref.tca > ref.cout;
    grep -Fq "ERROR" ref.cout
    ref_error=$?
    grep -Fq "ERROR" $project.cout
    proj_error=$?
    #Check the extra credit
    if [ $2 -eq "1" ] ; then
 	 diff -w ref.cout $project.cout > /dev/null
	 res=$?
	 rm -f ref.cout $project.cout
	 if [ $res -ne 0 ]; then
		echo $1 " gave a different error than the reference";
		summary=$summary"\n"$1" gave a different error than the reference";
		if [ -e $project.tca ] ; then
			rm $project.tca
		fi
		if [ -e ref.tca ] ; then
			rm ref.tca;
		fi
		return 1
	 fi
    else
	 rm -f ref.cout $project.cout

	    if  [ $proj_error -eq "0" ] ; then
		if [ $ref_error -ne "0" ] ; then
			echo $1 " gave an error when there should not be an error";
			summary=$summary"\n"$1" gave an error when there should not be an error";
			rm $project.tca;
			if [ -e ref.tca ] ; then
			    rm ref.tca;
			fi
			return 1;

		fi
	    else
		if [ $ref_error -eq "0" ] ; then
			echo $1 " did not give an error when it should have";
			summary=$summary"\n"$1" did not give an error when it should have";
			rm $project.tca;
			if [ -e ref.tca ] ; then
			    rm ref.tca;
			fi
			return 1;

		fi

	    fi
    fi
    if [ -e ref.tca ] ; then
    	    # Pull out the execution limit from the filename
	    IFS='-' read -a array <<< "$1"
	    IFS='.' read -a array2 <<< "${array[2]}"
	    limit=${array2[0]};
	    Test_Suite/tubecode -t " $limit " $project.tca > $project.out
	    Test_Suite/tubecode ref.tca > ref.out
	    rm  ref.tca $project.tca;
	    diff -w ref.out $project.out;
	    result=$?;
	    rm $project.out ref.out;
	    if [ $result -ne 0 ]; then
		echo $1 "failed different executed result on tubecode";
		summary=$summary"\n"$1" failed different executed result on tubecode"
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
	run_error_test $F "0"
done
for F in Test_Suite/fail*.tube; do
	run_error_test $F "0"
done

echo Extra Credit Results:

for F in Test_Suite/extra.*.tube; do
	run_error_test $F "1"
done


echo -e $summary
