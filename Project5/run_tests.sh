#! /bin/bash

# This file can be executed by calling "bash run_tests.sh"
# It will then test tube1 against the reference_implementation for
# each tube file in the test-suite
project=tube5
begin_="\n\n<<< Begin <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
end_="\n>>> End >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n"
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

summary="\n\n\n\n:::::::::::::::::::::::::::::::::::::::::::::::::\n::: Summary:\n::::::::::::::::::::::::::::::::::::::::::::::::";
function run_error_test {
    ./$project $1 $project.tic > $project.cout;
    Test_Suite/reference_$project $1 ref.tic > ref.cout;
    grep -Fq "ERROR" ref.cout
    ref_error=$?
    grep -Fq "ERROR" $project.cout
    proj_error=$?    
    #Check the extra credit
    if [ $2 -eq "1" ] ; then 
 	 diff -w ref.cout $project.cout > /dev/null
	 if [ $?  -ne 0 ]; then
		echo -e $1 "\tTest Failed : gave a different error than the reference";
                echo -e $begin_ 
                echo $1 
                echo '==================='
                echo -e '\n'
                cat $1
                echo -e $end_ 
		summary=$summary"\n"$1"\tTest Failed : gave a different error than the reference";
		if [ -e $project.tic ] ; then
			rm $project.tic
		fi
		if [ -e ref.tic ] ; then
			rm ref.tic;
		fi
		return 1
	 fi
    else

	    if  [ $proj_error -eq "0" ] ; then
		if [ $ref_error -ne "0" ] ; then
			echo -e $1 "\tTest Failed : gave an error when there should not be an error";
                        echo -e $begin_ 
                        echo $1
                        echo '==================='
                        echo -e '\n'
                        cat $1
                        echo -e $end_ 
			summary=$summary"\n"$1"\tTest Failed : gave an error when there should not be an error";
			rm $project.tic;
			if [ -e ref.tic ] ; then 
			    rm ref.tic;
			fi
			return 1;

		fi
	    else
		if [ $ref_error -eq "0" ] ; then
			echo -e $1 "\tTest Failed : did not give an error when it should have";
                        echo -e $begin_ 
                        echo $1
                        echo '==================='
                        echo -e '\n'
                        cat $1
                        echo -e $end_ 
			summary=$summary"\n"$1"\t Test Failed : did not give an error when it should have";
			rm $project.tic;
			if [ -e ref.tic ] ; then 
			    rm ref.tic;
			fi
			return 1;

		fi
		
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
		echo -e $1 "\tTest Failed : different executed result on TubeIC";
                echo -e $begin_ 
                echo $1
                echo -e '\n'
                cat $1
                echo -e $end_ 
		summary=$summary"\n"$1"\tTest Failed : different executed result on TubeIC"
		return 1;
	    else
		echo $1 "\tTest Passed :  ";
		summary=$summary"\n"$1"\tTest Passed"
	    fi;
    else 
		summary=$summary"\n"$1"\tTest Passed"
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
