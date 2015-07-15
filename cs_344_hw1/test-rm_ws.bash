#!/bin/bash

# R Jesse Chaney
# chaneyr@eecs.orst.edu
# CS344-001
# Oregon State University
# School of EECS

# $Author: chaneyr $
# $Date: 2015/02/13 22:20:33 $
# $RCSfile: test-rm_ws.bash,v $
# $Revision: 1.10 $

CLASS=cs344-001
TERM=winter2015
HWK=Homework1
HDIR=/usr/local/classes/eecs/${TERM}/${CLASS}/src/${HWK}/
TDIR=~chaneyr/test_files/
IPROG=${HDIR}/rm_ws
SPROG=./rm_ws
POINTS_AVAIL=90
EXTRA_CREDIT=0

TOTAL_FAIL_COUNT=0
TOTAL_PASS_COUNT=0
TOTAL_TEST_COUNT=0
TOTAL_ACTUAL_TESTS=0
MAKE_PIONTS=0
WARN_POINTS=0
NO_BUILD=0

VERBOSE=0
SUM_ONLY=0

function build {

    make clean > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=5))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed \"make clean\""
	fi
    fi

    make clean > /dev/null 2>&1
    make rm_ws.o > /dev/null 2>&1 
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=5))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed \"make rm_ws.o\""
	fi
    fi

    make clean > /dev/null 2>&1
    make rm_ws > /dev/null 2>&1 
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=5))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed \"make rm_ws\""
	fi
    fi

    make clean > /dev/null 2>&1
    make all 2> WARN > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=7))
    fi

    grep -c CC ?akefile > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=2))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed CC"
	fi
    fi

    grep -c CFLAGS ?akefile > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=2))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed CFLAGS"
	fi
    fi

    grep -c PROGS ?akefile > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=2))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed PROGS"
	fi
    fi

    grep -c SOURCES ?akefile > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=2))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> missed SOURCES"
	fi
    fi

    NUM_BYTES=`cat WARN | wc -c`
    if [ ${NUM_BYTES} -eq 0 ]
    then
	WARN_POINTS=10
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo ">> had warnings messages"
	fi
    fi
}

function testWithFiles {

    FAIL_COUNT=0
    PASS_COUNT=0
    TOTAL_TESTS=0
    OPTION=$1

    if [ -z "$2" ]
    then
	SCALE=1
    else
	SCALE=$2
    fi

    if [ ${SUM_ONLY} -eq 0 ]
    then
	echo "Testing with options: \"${OPTION}\"  Scale: ${SCALE}"
    fi

    for F in ${TDIR}/latin?.txt
    do

	BASE=`basename ${F} .txt`
	IOP=${BASE}.iop
	SOP=${BASE}.sop

	${IPROG} ${OPTION} < ${F} > ${IOP} 2> /dev/null
	${SPROG} ${OPTION} < ${F} > ${SOP} 2> /dev/null

	diff ${IOP} ${SOP} > /dev/null
	# Save the exit code from diff.
	RES=$?

	# If the exit code from diff is 0, there were no
	# differences in the compared files.  If the exit
	# code is 1, there were differecnes.
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      File ${BASE} fails match with \"${OPTION}\""
	    fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      File ${BASE} passes match with \"${OPTION}\""
	    fi
	    ((PASS_COUNT++))
	fi
	((TOTAL_TESTS++))
	rm -f ${IOP} ${SOP}

    done

    # This tests will a larger input.
    cat ${TDIR}/latin?.txt | ${IPROG} ${OPTION} > iBigTest 2> /dev/null
    cat ${TDIR}/latin?.txt | ${SPROG} ${OPTION} > sBigTest 2> /dev/null

    # See comments above about exit codes for diff.
    diff iBigTest sBigTest > /dev/null 2> /dev/null
    RES=$?

    if [ ${RES} -ne 0 ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
            echo "      Big file test fails match with \"${OPTION}\""
	fi
	((FAIL_COUNT++))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      Big file test passes match with \"${OPTION}\""
	fi
	((PASS_COUNT++))
    fi
    ((TOTAL_TESTS++))
    rm -f iBigTest sBigTest

    if [ ${SUM_ONLY} -ne 1 ]
    then
	if [ ${FAIL_COUNT} -gt 0 ]
	then
	    echo "  * Failed ${FAIL_COUNT} of ${TOTAL_TESTS} tests"
	else
	    echo "  All ${PASS_COUNT} tests passed"
	fi
    fi

    # Add the global variables.
    ((TOTAL_FAIL_COUNT+=${FAIL_COUNT} * ${SCALE}))
    ((TOTAL_PASS_COUNT+=${PASS_COUNT} * ${SCALE}))
    ((TOTAL_TEST_COUNT+=${TOTAL_TESTS} * ${SCALE}))
    ((TOTAL_ACTUAL_TESTS+=${TOTAL_TESTS}))
}

function runSingleTests {
    testWithFiles "-b" 50
    testWithFiles "-i" 50
    testWithFiles "-e" 50
}

function runDoubleTests {
    testWithFiles "-bi" 10
    testWithFiles "-be" 10

    testWithFiles "-ie" 10
    testWithFiles "-ib" 10

    testWithFiles "-eb" 10
    testWithFiles "-ei" 10

    testWithFiles "-b -i" 10
    testWithFiles "-b -e" 10

    testWithFiles "-i -e" 10
    testWithFiles "-i -b" 10

    testWithFiles "-e -b" 10
    testWithFiles "-e -i" 10
}

function runTrippleTests {
    testWithFiles "-bie"
    testWithFiles "-bei"

    testWithFiles "-eib"
    testWithFiles "-ebi"

    testWithFiles "-ibe"
    testWithFiles "-ieb"

    testWithFiles "-b -i -e"
    testWithFiles "-b -e -i"

    testWithFiles "-e -i -b"
    testWithFiles "-e -b -i"

    testWithFiles "-i -e -b"
    testWithFiles "-i -b -e"
}

# Not a lot of command line options.  The getopts function 
# is a bash built-in.  It differs from the getopt command
# line program.
while getopts "evsP:" opt
do
    case $opt in
	P)
	    # Change the name of the student program.  The
	    # default is ./rm_ws.  You can probably make this
	    # "python rm_ws.py" to test extra credit.
	    SPROG=$OPTARG
	    ;;
	e)
	    EXTRA_CREDIT=1
	    NO_BUILD=1
	    POINTS_AVAIL=30
	    ;;
	v)
	    # Print extra messages.
	    VERBOSE=1
	    ;;
	s)
	    # This will eliminate verbose.
	    SUM_ONLY=1
	    ;;
	\?)
	    echo "Invalid option" >&2
	    exit 1
	    ;;
	:)
	    echo "Option -$OPTARG requires an argument." >&2
	    exit 1
	    ;;
    esac
done

echo "Running with program \"${SPROG}\""
echo ""

if [ ${NO_BUILD} -eq 0 ]
then
    build
fi

# Test all single options.
runSingleTests

# Test all 2 option combinations.
runDoubleTests

# Test all 3 option combinations.
runTrippleTests

# This tests the code when no characters are removed from
# the input data at all.
testWithFiles ""

PASS_PERCENT=`awk "BEGIN {print ${TOTAL_PASS_COUNT} / ${TOTAL_TEST_COUNT} * 100;}"`
FAIL_PERCENT=`awk "BEGIN {print ${TOTAL_FAIL_COUNT} / ${TOTAL_TEST_COUNT} * 100;}"`
POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`

if [ ${EXTRA_CREDIT} -eq 0 ]
then
    echo ""
    echo "** Problem 8 Makefile points: ${WARN_POINTS} of 10 **"
    echo "** Problem 9 Makefile points: ${MAKE_POINTS} of 30 **"
else
    echo ""
    echo "++ EXTRA CREDIT POINTS ++ EXTRA CREDIT POINTS ++ EXTRA CREDIT POINTS ++"
fi

echo ""
#echo "** Passed ${TOTAL_PASS_COUNT} of ${TOTAL_TEST_COUNT} tests ${PASS_PERCENT}% **"
#echo "** Failed ${TOTAL_FAIL_COUNT} of ${TOTAL_TEST_COUNT} tests ${FAIL_PERCENT}% **"
echo "** Total tests ${TOTAL_ACTUAL_TESTS} **"
echo "** Passed percent ${PASS_PERCENT} **"
echo "** Problem 8 rm_ws points awarded ${POINTS} of ${POINTS_AVAIL} **"

GRADED_BY=`finger ${LOGNAME} | grep gecos: | awk '{print $2,$3,$4,$5;}'`
DATE=`date`
HOST=`hostname`
SCRIPT=`basename $0`
echo ""
echo "Graded by ${GRADED_BY}  (${LOGNAME})"
echo "    ${DATE} ${HOST}"
echo "    grading script ${SCRIPT}" ' $Revision: 1.10 $'
