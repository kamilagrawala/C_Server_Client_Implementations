#!/bin/bash 

# R Jesse Chaney
# chaneyr@eecs.orst.edu
# CS344-001
# Oregon State University
# School of EECS

# $Author: chaneyr $
# $Date: 2015/02/11 09:41:17 $
# $RCSfile: test-oscar.bash,v $
# $Revision: 1.25 $

CLASS=cs344-001
TERM=winter2015
HWK=Homework2
HDIR=/usr/local/classes/eecs/${TERM}/${CLASS}/src/${HWK}/
TDIR=~chaneyr/test_files/
IPROG=${HDIR}/oscar
SPROG=./myoscar
PPROG=Problem3.py

# This is either I for Instructor file or S for Student file.
# It should be very uncommon to use the S value, using the -s
# command line option.
EX=I

TOTAL_POINTS=0
TOTAL_AVAIL=0
EC_POINTS=0
PYTHON_POINTS=0
PYTHON_ONLY=0
TOC_ERR_PCT=40
TO=4
EC_PERCENT=70

MAKE_POINTS=0
WARN_POINTS=0

VERBOSE=0
SUM_ONLY=0
CLEANUP=1

function signalCaught {
    echo "++++ caught signal while running script ++++"
}

function signalCtrlC {
    echo "Caught Control-C"
    echo "You will neeed to clean up some files"
    exit
}

function signalSegFault {
    echo "+++++++ Caught Segmentation Fault from program  ++++++++"
}

function copyTestFiles {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "  Copying test files into current directory"
    fi

    rm -f ./latin[1-5].txt
    rm -f ./[1-5]-s.txt

    cp ${TDIR}/latin?.txt .
    chmod g+rw,o+rx latin[1-5].txt
    cp ${TDIR}/?-s.txt .
    chmod g+rw,o+r [1-5]-s.txt
    for FILE in numbers_100000.txt numbers_10000.txt numbers_1000.txt numbers_100.txt words.txt
    do
	rm -f ./${FILE}
	cp ${TDIR}/${FILE} .
	chmod g+x,o+x ${FILE}
    done
}

function timeStampTestFiles {

    TIME_STAMP=1303031503

    touch -t ${TIME_STAMP} ./[1-5]-s.txt
    touch -t ${TIME_STAMP} latin[1-5].txt
    touch -t ${TIME_STAMP} numbers_10*.txt words.txt

    sync
}

function cleaupTestFiles {
    rm -f latin[1-5].txt 
    rm -f [1-5]-s.txt 
    rm -f numbers_10*.txt words.txt
}

function diffTimeFiles {
    FAIL=0

    # I should make this a here document.
    FAIL=`paste ${1}_header_${3} ${2}_header_${3} | awk 'function abs(x){return ((x < 0.0) ? -x : x)} BEGIN {Fail=0;} {if (abs($1 - $2) > 2) Fail++;} END {print Fail;}'`

    echo ${FAIL}
}

function memberHeaderPortion {
    { grep '.txt' ${1}_${2}.oscar | cut -c${3} > ${2}_header_${4} 2> /dev/null; } >> grep.log 2>&1
}

function createOscarFiles {

    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "  Creating oscar files for testing"
    fi

    rm -f *.oscar

    timeStampTestFiles
    ${IPROG} -a 1_I.oscar [1-5]-s.txt > I.log 2>&1
    timeStampTestFiles
    { timeout ${TO} ${SPROG} -a 1_S.oscar [1-5]-s.txt > S.log 2>&1 ; } >> coreDump.log 2>&1
    coreDumpMessage $? "creating number 1 archive"

    timeStampTestFiles
    ${IPROG} -a 2_I.oscar latin[1-5].txt > I.log 2>&1
    timeStampTestFiles
    { timeout ${TO} ${SPROG} -a 2_S.oscar latin[1-5].txt > S.log 2>&1 ; } >> coreDump.log 2>&1
    coreDumpMessage $? "creating number 2 archive"

    timeStampTestFiles
    ${IPROG} -a 3_I.oscar numbers_100000.txt numbers_10000.txt \
	numbers_1000.txt numbers_100.txt words.txt > I.log 2>&1
    timeStampTestFiles
    { timeout ${TO} ${SPROG} -a 3_S.oscar numbers_100000.txt numbers_10000.txt \
	numbers_1000.txt numbers_100.txt words.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "creating number 3 archive"

    timeStampTestFiles
    ${IPROG} -a 4_I.oscar [1-5]-s.txt latin[1-5].txt \
	numbers_100000.txt numbers_10000.txt numbers_1000.txt \
	numbers_100.txt words.txt > I.log 2>&1
    timeStampTestFiles
    { timeout ${TO} ${SPROG} -a 4_S.oscar [1-5]-s.txt latin[1-5].txt \
	numbers_100000.txt numbers_10000.txt numbers_1000.txt \
	numbers_100.txt words.txt > S.log 2>&1 ; } >> coreDump.log 2>&1
    coreDumpMessage $? "creating number 4 archive"

    #### Many files test, 100 files in the archive
    if [ ${VERBOSE} -eq 1 ]
    then
	echo "    Starting to create many files test - 100 members in archive"
    fi
    timeStampTestFiles
    for I in `seq 10`
    do
	${IPROG} -a 5_I.oscar [1-5]-s.txt latin[1-5].txt > I.log 2>&1
    done
    timeStampTestFiles
    for I in `seq 10`
    do
    	{ timeout ${TO} ${SPROG} -a 5_S.oscar [1-5]-s.txt latin[1-5].txt > S.log 2>&1; } >> coreDump.log 2>&1
	if [ $? -eq 139 ]
	then
	    coreDumpMessage 139 "creating loooooong archive"
	    #echo "      >>>> ${SPROG} core dump creating loooooong archive"
	    break;
	fi
    done

    #### Big boy file test
    if [ ${VERBOSE} -eq 1 ]
    then
	echo "    Starting to create big boy file test"
    fi
    timeStampTestFiles
    for I in `seq 20`
    do
	${IPROG} -a 6_I.oscar numbers_100000.txt > I.log 2>&1
    done
    timeStampTestFiles
    for I in `seq 20`
    do
    	{ timeout ${TO} ${SPROG} -a 6_S.oscar numbers_100000.txt > S.log 2>&1; } >> coreDump.log 2>&1
	if [ $? -eq 139 ]
	then
	    coreDumpMessage 139 "creating big boy archive"
	    #echo "      >>>> ${SPROG} core dump creating big boy archive"
	    break;
	fi
    done
}

function testLittleH {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -h"
    fi

    { timeout ${TO} ${SPROG} -h > HelpFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-h"

    POINTS=0
    POINTS_AVAIL=10
    NUM_BYTES=`grep "invalid option" HelpFile.txt | wc -c` 
    if [ ${NUM_BYTES} -eq 0 ]
    then
	POINTS=10
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo " >> no help message"
	fi
    fi
    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f HelpFile.txt
    fi
    echo "** Points for -h   ${POINTS} of ${POINTS_AVAIL} **"
    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function testBigV {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -V"
    fi

    { timeout ${TO} ${SPROG} -V > VersionFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-V"

    POINTS=0
    POINTS_AVAIL=10
    NUM_BYTES=`grep "invalid option" VersionFile.txt | wc -c`
    if [ ${NUM_BYTES} -eq 0 ]
    then
	POINTS=10
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo " >> no help message"
	fi
    fi
    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f VersionFile.txt
    fi
    echo "** Points for -V   ${POINTS} of ${POINTS_AVAIL} **"

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function coreDumpMessage {
    if [ $1 -eq 139 ]
    then
	echo "      >>>> ${SPROG} core dump during $2 testing"
    fi
    if [ $1 -eq 134 ]
    then
	echo "      >>>> ${SPROG} abort during $2 testing"
    fi
    if [ $1 -eq 124 ]
    then
	echo "      >>>> ${SPROG} timeout during $2 testing"
    fi
}

function testLittleV {
    # This is a bit of a whild guess as to how I can hunt down
    # a verbose message __somplace__ in the studnet's output.
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -v"
    fi

    POINTS=0
    POINTS_AVAIL=10

    { timeout ${TO} ${SPROG} -vh > VerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -vV >> VerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -ve 1_S.oscar >> VerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -vE 1_S.oscar >> VerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -veo 1_S.oscar >> VerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -vEo 1_S.oscar >> VerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'

    { timeout ${TO} ${SPROG} -h > NoVerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -V >> NoVerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -e 1_S.oscar >> NoVerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -E 1_S.oscar >> NoVerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -eo 1_S.oscar >> NoVerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'
    { timeout ${TO} ${SPROG} -Eo 1_S.oscar >> NoVerboseFile.txt 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? '-v'

    NUM_BYTES=`grep "invalid option" VerboseFile.txt | wc -c`
    diff VerboseFile.txt NoVerboseFile.txt > /dev/null
    RES=$?
    if [ ${NUM_BYTES} -eq 0 -a ${RES} -ne 0 ]
    then
	POINTS=10
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo " >> verbose flag failed"
	fi
    fi
    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f VerboseFile.txt NoVerboseFile.txt
    fi
    echo "** Points for -v   ${POINTS} of ${POINTS_AVAIL} **"

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function testLitleEoBigEo {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -eo/-oe/-E0/-oE"
	if [ "${EX}" = "I" ]
	then
	    echo "   Using instructor generated archive for extraction"
	else
	    echo "   Using student generated archive for extraction"
	fi
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`
    POINTS_AVAIL=40
    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0

    ###  -eo  ###
    cleaupTestFiles
    ((TEST_COUNT++))
    ${IPROG} -e 1_${EX}.oscar > I.log 2>&1
    touch [1-5]-s.txt
    ls -s1 [1-5]-s.txt > 1_IE_files.txt
    ls -o -g [1-5]-s.txt > 1_IE_dates.txt
    { timeout ${TO} ${SPROG} -eo 1_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-eo"
    ls -s1 [1-5]-s.txt > 1_IS_files.txt
    ls -o -g [1-5]-s.txt > 1_IS_dates.txt

    diff 1_IE_files.txt 1_IS_files.txt > /dev/null
    RES_files=$?
    diff 1_IE_dates.txt 1_IS_dates.txt > /dev/null
    RES_dates=$?

    if [ ${RES_files} -eq 1 -o ${RES_dates} -eq 0  ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	     echo "      >> File 1_${EX}.oscar fails -eo match"
        fi
	((FAIL_COUNT++))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      File 1_${EX}.oscar passes -eo match"
        fi
        ((PASS_COUNT++))
    fi

    ###  -oe  ###
    cleaupTestFiles
    ((TEST_COUNT++))
    ${IPROG} -e 1_${EX}.oscar > I.log 2>&1
    touch [1-5]-s.txt
    ls -s1 [1-5]-s.txt > 1_IE_files.txt
    ls -o -g [1-5]-s.txt > 1_IE_dates.txt
    { timeout ${TO} ${SPROG} -oe 1_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-oe"
    ls -s1 [1-5]-s.txt > 1_IS_files.txt
    ls -o -g [1-5]-s.txt > 1_IS_dates.txt

    diff 1_IE_files.txt 1_IS_files.txt > /dev/null
    RES_files=$?
    diff 1_IE_dates.txt 1_IS_dates.txt > /dev/null
    RES_dates=$?

    if [ ${RES_files} -eq 1 -o ${RES_dates} -eq 0  ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	     echo "      >> File 1_${EX}.oscar fails -oe match"
        fi
	((FAIL_COUNT++))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      File 1_${EX}.oscar passes -oe match"
        fi
        ((PASS_COUNT++))
    fi

    ###  -Eo  ###
    cleaupTestFiles
    ((TEST_COUNT++))
    ${IPROG} -e 1_${EX}.oscar > I.log 2>&1
    ls -s1 [1-5]-s.txt > 1_IE_files.txt
    ls -o -g [1-5]-s.txt > 1_IE_dates.txt
    { timeout ${TO} ${SPROG} -Eo 1_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-Eo"
    ls -s1 [1-5]-s.txt > 1_IS_files.txt
    ls -o -g [1-5]-s.txt > 1_IS_dates.txt

    diff 1_IE_files.txt 1_IS_files.txt > /dev/null
    RES_files=$?
    diff 1_IE_dates.txt 1_IS_dates.txt > /dev/null
    RES_dates=$?

    if [ ${RES_files} -eq 1 -o ${RES_dates} -eq 0  ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	     echo "      >> File 1_${EX}.oscar fails -Eo match"
        fi
	((FAIL_COUNT++))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      File 1_${EX}.oscar passes -Eo match"
        fi
        ((PASS_COUNT++))
    fi

    ###  -oE  ###
    cleaupTestFiles
    ((TEST_COUNT++))
    ${IPROG} -e 1_${EX}.oscar > I.log 2>&1
    ls -s1 [1-5]-s.txt > 1_IE_files.txt
    ls -o -g [1-5]-s.txt > 1_IE_dates.txt
    { timeout ${TO} ${SPROG} -oE 1_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-oE"
    ls -s1 [1-5]-s.txt > 1_IS_files.txt
    ls -o -g [1-5]-s.txt > 1_IS_dates.txt

    diff 1_IE_files.txt 1_IS_files.txt > /dev/null
    RES_files=$?
    diff 1_IE_dates.txt 1_IS_dates.txt > /dev/null
    RES_dates=$?

    if [ ${RES_files} -eq 1 -o ${RES_dates} -eq 0  ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	     echo "      >> File 1_${EX}.oscar fails -oE match"
        fi
	((FAIL_COUNT++))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      File 1_${EX}.oscar passes -oE match"
        fi
        ((PASS_COUNT++))
    fi
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f 1_IE_files.txt 1_IE_dates.txt 1_IS_files.txt 1_IS_dates.txt 
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -eo -oe -Eo -oE ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test"
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function testLitleE {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -e"
	if [ "${EX}" = "I" ]
	then
	    echo "   Using instructor generated archive for extraction"
	else
	    echo "   Using student generated archive for extraction"
	fi
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`
    POINTS_AVAIL=60
    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0

    cleaupTestFiles

    ###   1 all   ###
    ((TEST_COUNT++))
    ${IPROG} -e 1_${EX}.oscar > I.log 2>&1
    ls -lF [1-5]-s.txt > 1_IE.txt
    rm -f [1-5]-s.txt
    { timeout ${TO} ${SPROG} -e 1_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e all archive 1"
    ls -lF [1-5]-s.txt > 1_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 1_IE.txt 1_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 1_${EX}.oscar fails extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 1_${EX}.oscar passes extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 1_${EX}.oscar fails to extract all"
        fi
	((FAIL_COUNT++))
    fi

    ###   2 all  ###
    ((TEST_COUNT++))
    ${IPROG} -e 2_${EX}.oscar > I.log 2>&1
    ls -lF latin[1-5].txt > 2_IE.txt 2> /dev/null
    rm -f latin[1-5].txt
    { timeout ${TO} ${SPROG} -e 2_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e all archive 2"
    ls -lF latin[1-5].txt > 2_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 2_IE.txt 2_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 2_${EX}.oscar fails extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 2_${EX}.oscar passes extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 2_${EX}.oscar fails to extract all"
        fi
	((FAIL_COUNT++))
    fi

    ###   3 all   ###
    ((TEST_COUNT++))
    ${IPROG} -e 3_${EX}.oscar > I.log 2>&1
    ls -lF numbers_10*.txt words.txt > 3_IE.txt
    rm -f numbers_10*.txt words.txt
    { timeout ${TO} ${SPROG} -e 3_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e all archive 3"
    ls -lF numbers_10*.txt words.txt > 3_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 3_IE.txt 3_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 3_${EX}.oscar fails extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 3_${EX}.oscar passes extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 3_${EX}.oscar fails to extract all"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 all  ###
    ((TEST_COUNT++))
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ${IPROG} -e 4_${EX}.oscar > I.log 2>&1
    ls -lF latin[1-5].txt latin[1-5].txt numbers_10*.txt words.txt > 4_IE.txt
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    { timeout ${TO} ${SPROG} -e 4_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e all archive 4"
    ls -lF latin[1-5].txt latin[1-5].txt numbers_10*.txt words.txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract all"
        fi
	((FAIL_COUNT++))
    fi

    ##############################################################
    ##############################################################

    ###   1 some   ###
    cleaupTestFiles
    ((TEST_COUNT++))
    ${IPROG} -e 1_${EX}.oscar 2-s.txt 4-s.txt > I.log 2>&1
    ls -lF [1-5]-s.txt > 1_IE.txt
    rm -f [1-5]-s.txt
    { timeout ${TO} ${SPROG} -e 1_${EX}.oscar 2-s.txt 4-s.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e some archive 1"
    ls -lF [1-5]-s.txt > 1_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 1_IE.txt 1_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 1_${EX}.oscar fails extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 1_${EX}.oscar passes extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 1_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   2 some   ###
    ((TEST_COUNT++))
    ${IPROG} -e 2_${EX}.oscar latin1.txt latin5.txt > I.log 2>&1
    ls -lF latin[1-5].txt > 2_IE.txt
    rm -f latin[1-5].txt
    { timeout ${TO} ${SPROG} -e 2_${EX}.oscar latin1.txt latin5.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e some archive 2"
    ls -lF latin[1-5].txt > 2_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 2_IE.txt 2_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 2_${EX}.oscar fails extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 2_${EX}.oscar passes extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 2_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   3 some   ###
    ((TEST_COUNT++))
    ${IPROG} -e 3_${EX}.oscar numbers_100000.txt words.txt > I.log 2>&1
    ls -lF numbers_10*.txt words.txt > 3_IE.txt
    rm -f numbers_10*.txt words*.txt
    { timeout ${TO} ${SPROG} -e 3_${EX}.oscar numbers_100000.txt words.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e some archive 3"
    ls -lF numbers_10*.txt words*.txt > 3_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 3_IE.txt 3_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 3_${EX}.oscar fails extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 3_${EX}.oscar passes extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 3_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 some   ###
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ((TEST_COUNT++))
    ${IPROG} -e 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt 5-s.txt latin1.txt > I.log 2>&1
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IE.txt
    rm -f numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt
    { timeout ${TO} ${SPROG} -e 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt \
	5-s.txt latin1.txt > S.log 2>&1 ; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e some archive 4"
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 all no overwrite   ###
    ((TEST_COUNT++))
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ${IPROG} -e 4_${EX}.oscar > I.log 2>&1
    touch [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IE.txt
    { timeout ${TO} ${SPROG} -e 4_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e all archive 4 no overwite"
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails extract no overwite all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes extract no overwrite all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 some no overwrite   ###
    ((TEST_COUNT++))
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt 2> /dev/null
    ${IPROG} -e 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt 5-s.txt latin1.txt > I.log 2>&1
    touch [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IE.txt
    { timeout ${TO} ${SPROG} -e 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt \
	5-s.txt latin1.txt > S.log 2>&1 ; } >> coreDump.log 2>&1
    coreDumpMessage $? "-e some archive 4 no overwite"
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails extract no overwite some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes extract no overwrite some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm [1-6]_I?.txt > /dev/null 2>&1
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -e ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test"
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}


function testBigE {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -E"
	if [ "${EX}" = "I" ]
	then
	    echo "   Using instructor generated archive for extraction"
	else
	    echo "   Using student generated archive for extraction"
	fi
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`
    POINTS_AVAIL=40
    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0

    cleaupTestFiles

    ###   1 all   ###
    ((TEST_COUNT++))
    ${IPROG} -E 1_${EX}.oscar > I.log 2>&1
    ls -lF [1-5]-s.txt > 1_IE.txt
    rm -f [1-5]-s.txt
    { timeout ${TO} ${SPROG} -E 1_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E all 1"
    ls -lF [1-5]-s.txt > 1_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 1_IE.txt 1_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 1_${EX}.oscar fails Extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 1_${EX}.oscar passes Extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 1_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   2 all  ###
    ((TEST_COUNT++))
    ${IPROG} -E 2_${EX}.oscar > I.log 2>&1
    ls -lF latin[1-5].txt > 2_IE.txt
    rm -f latin[1-5].txt
    { timeout ${TO} ${SPROG} -E 2_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E all 2"
    ls -lF latin[1-5].txt > 2_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 2_IE.txt 2_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 2_${EX}.oscar fails Extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 2_${EX}.oscar passes Extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 2_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   3 all   ###
    ((TEST_COUNT++))
    ${IPROG} -E 3_${EX}.oscar > I.log 2>&1
    ls -lF numbers_10*.txt words.txt > 3_IE.txt
    rm -f numbers_10*.txt words.txt
    { timeout ${TO} ${SPROG} -E 3_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E all 3"
    ls -lF numbers_10*.txt words.txt > 3_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 3_IE.txt 3_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 3_${EX}.oscar fails Extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 3_${EX}.oscar passes Extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 3_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 all  ###
    ((TEST_COUNT++))
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ${IPROG} -E 4_${EX}.oscar > I.log 2>&1
    ls -lF latin[1-5].txt latin[1-5].txt numbers_10*.txt words.txt > 4_IE.txt
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    { timeout ${TO} ${SPROG} -E 4_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E all 4"
    ls -lF latin[1-5].txt latin[1-5].txt numbers_10*.txt words.txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails Extract all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes Extract all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ##############################################################
    ##############################################################

    ###   1 some   ###
    cleaupTestFiles
    ((TEST_COUNT++))
    ${IPROG} -E 1_${EX}.oscar 2-s.txt 4-s.txt > I.log 2>&1
    ls -lF [1-5]-s.txt > 1_IE.txt
    rm -f [1-5]-s.txt
    { timeout ${TO} ${SPROG} -E 1_${EX}.oscar 2-s.txt 4-s.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E some 1"
    ls -lF [1-5]-s.txt > 1_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 1_IE.txt 1_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 1_${EX}.oscar fails Extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 1_${EX}.oscar passes Extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 1_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   2 some   ###
    ((TEST_COUNT++))
    ${IPROG} -E 2_${EX}.oscar latin1.txt latin5.txt > I.log 2>&1
    ls -lF latin[1-5].txt > 2_IE.txt
    rm -f latin[1-5].txt
    { timeout ${TO} ${SPROG} -E 2_${EX}.oscar latin1.txt latin5.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E some 2"
    ls -lF latin[1-5].txt > 2_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 2_IE.txt 2_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 2_${EX}.oscar fails Extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 2_${EX}.oscar passes Extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 2_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   3 some   ###
    ((TEST_COUNT++))
    ${IPROG} -E 3_${EX}.oscar numbers_100000.txt words.txt > I.log 2>&1
    ls -lF numbers_10*.txt words.txt > 3_IE.txt
    rm -f numbers_10*.txt words.txt
    { timeout ${TO} ${SPROG} -E 3_${EX}.oscar numbers_100000.txt words.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E some 3"
    ls -lF numbers_10*.txt words.txt > 3_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 3_IE.txt 3_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 3_${EX}.oscar fails Extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 3_${EX}.oscar passes Extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 3_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 some   ###
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ((TEST_COUNT++))
    ${IPROG} -E 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt 5-s.txt latin1.txt > I.log 2>&1
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IE.txt
    rm -f numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt
    { timeout ${TO} ${SPROG} -E 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt \
	5-s.txt latin1.txt > S.log 2>&1 ; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E some 4"
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails Extract some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes Extract some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi


    ###   4 no overwrite   ###
    ((TEST_COUNT++))
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ${IPROG} -e 4_${EX}.oscar > I.log 2>&1
    touch [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IE.txt
    { timeout ${TO} ${SPROG} -E 4_${EX}.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E all no overwrite"
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails Extract no overwite all match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes Extract no overwrite all match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi

    ###   4 no overwrite   ###
    ((TEST_COUNT++))
    rm -f [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ${IPROG} -e 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt 5-s.txt latin1.txt > I.log 2>&1
    touch [1-5]-s.txt latin[1-5].txt numbers_10*.txt words.txt
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IE.txt
    { timeout ${TO} ${SPROG} -E 4_${EX}.oscar numbers_100000.txt words.txt 3-s.txt latin4.txt \
	5-s.txt latin1.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-E some no overwrite"
    ls -lF numbers_10*.txt words.txt [1-5]-s.txt latin[1-5].txt > 4_IS.txt 2> /dev/null
    if [ $? -eq 0 ]
    then
	diff 4_IE.txt 4_IS.txt > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File 4_${EX}.oscar fails Extract no overwite some match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File 4_${EX}.oscar passes Extract no overwrite some match"
            fi
            ((PASS_COUNT++))
	fi
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> File 4_${EX}.oscar fails to extract some"
        fi
	((FAIL_COUNT++))
    fi
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm [1-6]_I?.txt
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -E ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test"
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}


function testLittleD {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -d"
	if [ "${EX}" = "I" ]
	then
	    echo "   Using instructor generated archive for extraction"
	else
	    echo "   Using student generated archive for extraction"
	fi
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`
    POINTS_AVAIL=60

    cp 1_${EX}.oscar 1_SD1.oscar
    cp 1_${EX}.oscar 1_SD2.oscar
    ${IPROG} -d 1_SD1.oscar 2-s.txt 4-s.txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -d 1_SD2.oscar 2-s.txt 4-s.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-d 1"

    cp 2_${EX}.oscar 2_SD1.oscar
    cp 2_${EX}.oscar 2_SD2.oscar
    ${IPROG} -d 2_SD1.oscar latin1.txt latin3.txt latin5.txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -d 2_SD2.oscar latin1.txt latin3.txt latin5.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-d 2"

    cp 3_${EX}.oscar 3_SD1.oscar
    cp 3_${EX}.oscar 3_SD2.oscar
    ${IPROG} -d 3_SD1.oscar numbers_1000.txt words.txt numbers_100000.txt numbers_10000.txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -d 3_SD2.oscar numbers_1000.txt words.txt numbers_100000.txt \
	numbers_10000.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-d 3"

    cp 4_${EX}.oscar 4_SD1.oscar
    cp 4_${EX}.oscar 4_SD2.oscar
    ${IPROG} -d 4_SD1.oscar latin[1-5].txt numbers_100000.txt 2-s.txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -d 4_SD2.oscar latin[1-5].txt numbers_100000.txt 2-s.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-d 4"

    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0
    for I in `seq 4`
    do
	memberHeaderPortion ${I} SD1 "1-32" file
	memberHeaderPortion ${I} SD1 "33-42" atime
	memberHeaderPortion ${I} SD1 "43-52" mtime
	memberHeaderPortion ${I} SD1 "53-62" ctime

	memberHeaderPortion ${I} SD2 "1-32" file
	memberHeaderPortion ${I} SD2 "33-42" atime
	memberHeaderPortion ${I} SD2 "43-52" mtime
	memberHeaderPortion ${I} SD2 "53-62" ctime

	FailA=$(diffTimeFiles SD1 SD2 atime)
	FailM=$(diffTimeFiles SD1 SD2 mtime)
	FailC=$(diffTimeFiles SD1 SD2 ctime)

	grep -v '.txt' ${I}_SD1.oscar > SD1_data 2> /dev/null
	grep -v '.txt' ${I}_SD2.oscar > SD2_data 2> /dev/null

	((TEST_COUNT++))
	((TEST_COUNT++))

	if [ ${FailA} -gt 0 -o ${FailM} -gt 0 -o ${FailC} -gt 0 ]
	then
	    ((FAIL_COUNT++))
	    if [ ${VERBOSE} -eq 1 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails delete header match"
	    fi
	else
	    ((PASS_COUNT++))
	    if [ ${VERBOSE} -eq 1 ]
            then
                echo "      File ${I}_${EX}.oscar passes delete header match"
            fi
	fi

	diff SD1_data SD2_data > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
                echo "      >> File ${I}_${EX}.oscar fails delete data match"
            fi
            ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
	 	echo "      File ${I}_${EX}.oscar passes delete data match"
	    fi
	    ((PASS_COUNT++))
	fi
    done
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f SD[12]_data SD[12]_header_[acm]time SD[12]_header_file [1-4]_SD[12].oscar
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -d ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test"
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function testSha {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -S"
    fi

    POINTS_AVAIL=40
    POINTS=0

    ${IPROG} -e 2_I.oscar > I.log 2>&1

    ${IPROG} -aS Sha_I.oscar latin[1-5].txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -aS Sha_S.oscar latin[1-5].txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-S"
    memberHeaderPortion Sha I 96- sha
    memberHeaderPortion Sha S 96- sha
    diff I_header_sha S_header_sha > /dev/null 2>&1
    RES=$?
    if [ ${RES} -ne 0 ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      >> SHA archive fails data match"
        fi
	POINTS=0
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      SHA archive passes data match"
	fi
	POINTS=40
    fi

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f Sha_[IS].oscar [IS]_header_sha
    fi

    echo "** Points for -S ${POINTS} of ${POINTS_AVAIL} ** EXTRA CREDIT **"

    ((EC_POINTS+=${POINTS}))
}

function testMark {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -m, -u and -C"
    fi

    POINTS_AVAIL=60
    POINTS=0

    # Need to make sure the data files are there.
    ${IPROG} -e 2_I.oscar > I.log 2>&1

    ${IPROG} -a Mark_I.oscar latin[1-5].txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -a Mark_S.oscar latin[1-5].txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-m"

    ${IPROG} -m Mark_I.oscar latin1.txt latin3.txt latin5.txt > I.log 2>&1
    { timeout ${TO} ${SPROG} -m Mark_S.oscar latin1.txt latin3.txt latin5.txt > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-m"
    
    memberHeaderPortion Mark I 95 mark
    memberHeaderPortion Mark S 95 mark
    diff I_header_mark S_header_mark > /dev/null 2>&1
    RES=$?
    if [ ${RES} -ne 0 ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      >> Mark fails data match"
        fi
	POINTS=0
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      Mark passes data match"
	fi
	POINTS=20

	# Now test the unmark option
	${IPROG} -u Mark_I.oscar latin1.txt latin3.txt latin5.txt > I.log 2>&1
	{ timeout ${TO} ${SPROG} -u Mark_S.oscar latin1.txt latin3.txt latin5.txt > S.log 2>&1; } >> coreDump.log 2>&1
	coreDumpMessage $? "-u"

	memberHeaderPortion Mark I 95 unmark
	memberHeaderPortion Mark S 95 unmark
	diff I_header_unmark S_header_unmark > /dev/null 2>&1
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      >> Unmark fails data match"
            fi
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      Unmark passes data match"
	    fi
	    ((POINTS+=20))
	fi

	# Now test the cleanse.
	${IPROG} -m Mark_I.oscar latin1.txt latin3.txt latin5.txt > I.log 2>&1
	{ timeout ${TO} ${SPROG} -m Mark_S.oscar latin1.txt latin3.txt latin5.txt > S.log 2>&1; } >> coreDump.log 2>&1
	coreDumpMessage $? "-m before -C"

	${IPROG} -C Mark_I.oscar > I.log 2>&1
	{ timeout ${TO} ${SPROG} -C Mark_S.oscar > S.log 2>&1; } >> coreDump.log 2>&1
	coreDumpMessage $? "-C"
	memberHeaderPortion Mark S "1-32" cleanse
	memberHeaderPortion Mark I "1-32" cleanse

	diff I_header_cleanse S_header_cleanse > /dev/null 2>&1
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      >> Cleanse fails data match"
            fi
	    POINTS=0
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      Cleanse passes data match"
	    fi
	    ((POINTS+=20))
	fi
    fi

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f Mark_[IS].oscar [IS]_header_mark [IS]_header_unmark [IS]_header_cleanse
    fi

    echo "** Points for -m, -u, and -C ${POINTS} of ${POINTS_AVAIL} ** EXTRA CREDIT **"

    ((EC_POINTS+=${POINTS}))
}

function testBigA {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -A"
    fi

    POINTS_AVAIL=20
    POINTS=0

    rm -f oscar.h

    { timeout ${TO} ${SPROG} -A All_S.oscar > S.log 2>&1; } >> coreDump.log 2>&1
    coreDumpMessage $? "-A"
    ${IPROG} -A All_I.oscar > I.log 2>&1

    ${IPROG} -d All_I.oscar All_S.oscar > I.log 2>&1

    memberHeaderPortion All S "1-32" files
    memberHeaderPortion All I "1-32" files

    diff I_header_files S_header_files > /dev/null 2>&1
    RES=$?
    if [ ${RES} -ne 0 ]
    then
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
        then
            echo "      >> File All_${EX}.oscar fails data match"
        fi
	POINTS=0
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      File All_${EX}.oscar passes data match"
	fi
	POINTS=${POINTS_AVAIL}
    fi

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f All_${EX}.oscar All_${EX}.oscar I_header_files S_header_files
    fi

    echo "** Points for -A ${POINTS} of ${POINTS_AVAIL} ** EXTRA CREDIT **"

    ((EC_POINTS+=${POINTS}))
}

function testLittleA {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -a"
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`

    POINTS_AVAIL=60
    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0

    copyTestFiles
    createOscarFiles

    for I in `seq 6`
    do
	memberHeaderPortion ${I} I "1-32" file
	memberHeaderPortion ${I} I "33-42" atime
	memberHeaderPortion ${I} I "43-52" mtime
	memberHeaderPortion ${I} I "53-62" ctime

	memberHeaderPortion ${I} S "1-32" file
	memberHeaderPortion ${I} S "33-42" atime
	memberHeaderPortion ${I} S "43-52" mtime
	memberHeaderPortion ${I} S "53-62" ctime

	FailA=$(diffTimeFiles I S atime)
	FailM=$(diffTimeFiles I S mtime)
	FailC=$(diffTimeFiles I S ctime)

	grep -v '.txt' ${I}_${EX}.oscar > I_data 2> /dev/null
	grep -v '.txt' ${I}_${EX}.oscar > S_data 2> /dev/null

	((TEST_COUNT++))

	if [ ${FailA} -gt 0 -o ${FailM} -gt 0 -o ${FailC} -gt 0 ]
	then
	    ((FAIL_COUNT++))
	    if [ ${VERBOSE} -eq 1 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails header match."
		echo "         Check the ${I}_[IS]_header files."
		mv I_header_atime ${I}_I_header_atime
		mv I_header_mtime ${I}_I_header_mtime
		mv I_header_ctime ${I}_I_header_ctime
		mv I_header_file ${I}_I_header_file

		mv S_header_atime ${I}_S_header_atime
		mv S_header_mtime ${I}_S_header_mtime
		mv S_header_ctime ${I}_S_header_ctime
		mv S_header_file ${I}_S_header_file
	    fi
	else
	    ((PASS_COUNT++))
	    if [ ${VERBOSE} -eq 1 ]
            then
                echo "      File ${I}_${EX}.oscar passes header match"
            fi
	fi

	((TEST_COUNT++))
	diff I_data S_data > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
                echo "      >> File ${I}_${EX}.oscar fails data match"
            fi
            ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
	 	echo "      File ${I}_${EX}.oscar passes data match"
	    fi
	    ((PASS_COUNT++))
	fi

	((TEST_COUNT++))
	ls -l ${I}_${EX}.oscar | cut -d' ' -f 1 > I_perms
	ls -l ${I}_${EX}.oscar | cut -d' ' -f 1 > S_perms
	diff I_perms S_perms > /dev/null
	RES=$?
	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
                echo "      >> File ${I}_${EX}.oscar fails perms match"
		echo "         Check the ${I}_[IS]_perms file"
		mv I_perms ${I}_I_perms
		mv S_perms ${I}_S_perms
            fi
            ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
	 	echo "      File ${I}_${EX}.oscar passes perms match"
	    fi
	    ((PASS_COUNT++))
	fi
    done
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f [IS]_header_* [IS]_header_file [IS]_files [IS]_data [IS]_perms
	rm -f [1-6]_[IS]_perms [1-6]_[IS]_header_[acm]time [1-6]_[IS]_header_file
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -a ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test"
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function testLittleT {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -t"
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`
    POINTS_AVAIL=50
    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0

    for I in `seq 6`
    do
	${IPROG} -t ${I}_${EX}.oscar > ${I}_It.txt 2>/dev/null
	{ timeout ${TO} ${SPROG} -t ${I}_${EX}.oscar > ${I}_St.txt 2>/dev/null; } >> coreDump.log 2>&1
	coreDumpMessage $? "-t ${I}"

	((TEST_COUNT++))
	diff -B ${I}_It.txt ${I}_St.txt > /dev/null 2>&1
	RES=$?

	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails short TOC match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File ${I}_${EX}.oscar passes short TOC match"
            fi
            ((PASS_COUNT++))
	fi

	((TEST_COUNT++))
	diff -b -B ${I}_It.txt ${I}_St.txt > /dev/null 2>&1
	RES=$?

	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails short TOC match, ignore ws"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File ${I}_${EX}.oscar passes short TOC match, ignore ws"
            fi
            ((PASS_COUNT++))
	fi
	########################################################
	((TEST_COUNT++))
	diff -b -B --suppress-common-lines -y ${I}_It.txt ${I}_St.txt > ${I}_Sterr.txt  2> /dev/null
	TOC_LINES=`wc -l < ${I}_It.txt`
	ERR_LINES=`wc -l < ${I}_Sterr.txt`
	((TOC_LINES++))
	ERR_COUNT=`echo ${ERR_LINES} ${TOC_LINES} | awk '{print int($1 / $2 * 100)}'`

	if [ ${ERR_COUNT} -gt ${TOC_ERR_PCT} ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails short TOC match, error count"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File ${I}_${EX}.oscar passes short TOC match, error count"
            fi
            ((PASS_COUNT++))
	fi
	########################################################

    done
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f [1-6]_[IS]t.txt [1-6]_Sterr.txt
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -t ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test"
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function testBigT {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing -T"
    fi

    BCORE_DUMPS=`wc -l < coreDump.log`
    POINTS_AVAIL=80
    FAIL_COUNT=0
    PASS_COUNT=0
    TEST_COUNT=0

    for I in `seq 6`
    do
	${IPROG} -T ${I}_${EX}.oscar > ${I}_IT.txt 2>/dev/null
	{ timeout ${TO} ${SPROG} -T ${I}_${EX}.oscar > ${I}_ST.txt 2>/dev/null; } >> coreDump.log 2>&1
	coreDumpMessage $? "-T ${I}"

	((TEST_COUNT++))
	diff -B ${I}_IT.txt ${I}_ST.txt > /dev/null 2>&1
	RES=$?

	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails long TOC match"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File ${I}_${EX}.oscar passes long TOC match"
            fi
            ((PASS_COUNT++))
	fi

	((TEST_COUNT++))
	diff -b -B ${I}_IT.txt ${I}_ST.txt > /dev/null 2>&1
	RES=$?

	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails long TOC match, ignore ws"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File ${I}_${EX}.oscar passes long TOC match, ignore ws"
            fi
            ((PASS_COUNT++))
	fi

	########################################################
	((TEST_COUNT++))
	diff -b -B --suppress-common-lines -y ${I}_IT.txt ${I}_ST.txt > ${I}_STerr.txt  2> /dev/null
	TOC_LINES=`wc -l < ${I}_IT.txt`
	ERR_LINES=`wc -l < ${I}_STerr.txt`
	((TOC_LINES++))
	ERR_COUNT=`echo ${ERR_LINES} ${TOC_LINES} | awk '{print int($1 / $2 * 100)}'`

	if [ ${ERR_COUNT} -gt ${TOC_ERR_PCT} ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> File ${I}_${EX}.oscar fails long TOC match, error count"
            fi
	    ((FAIL_COUNT++))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      File ${I}_${EX}.oscar passes long TOC match, error count"
            fi
            ((PASS_COUNT++))
	fi
	########################################################

    done
    ACORE_DUMPS=`wc -l < coreDump.log`
    ((DUMPS=${ACORE_DUMPS}-${BCORE_DUMPS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f [1-6]_[IS]T.txt [1-6]_STerr.txt
    fi

    PASS_PERCENT=`awk "BEGIN {print ${PASS_COUNT} / ${TEST_COUNT} * 100;}"`
    POINTS=`awk "BEGIN {printf \"%4.0f\", (${PASS_PERCENT} * ${POINTS_AVAIL} / 100) + 0.5}"`
    echo "** Points for -T ${POINTS} of ${POINTS_AVAIL} **"
    if [ ${DUMPS} -gt 0 ]
    then
	echo "     Your ${SPROG} program did a core/abort dump ${DUMPS} during this test."
    fi

    ((TOTAL_POINTS+=${POINTS}))
    ((TOTAL_AVAIL+=${POINTS_AVAIL}))
}

function buildPart {
    make clean > /dev/null 2>&1
    make $1 > /dev/null 2>&1 
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=10))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> Failed \"make $1\""
	fi
    fi
}

function build {
    if [ ${VERBOSE} -eq 1 ]
    then
	echo ""
	echo "Testing Makefile"
    fi

    rm -f oscar.h
    #rm -f ${SPROG}
    ln -fs ${HDIR}/oscar.h .
    ln -fs ${HDIR}/oscar .

    make clean > /dev/null
    if [ $? -eq 0 ]
    then
	((MAKE_POINTS+=10))
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo "      >> Failed \"make clean\""
	fi
    fi

    buildPart myoscar.o
    buildPart myoscar
    buildPart all

    make clean > /dev/null 2>&1
    make 2> WARN.err > WARN.out
#    if [ $? -eq 0 ]
#    then
#	((MAKE_POINTS+=10))
#    fi
    NUM_BYTES=`cat WARN.err | wc -c`
    if [ ${NUM_BYTES} -eq 0 ]
    then
	WARN_POINTS=10
    else
	if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	then
	    echo " >> had warnings messages"
	fi
    fi

    FLAG_POINTS=0
    for FLAG in -Wall -Wshadow -Wunreachable-code \
	-Wredundant-decls -Wmissing-declarations \
	-Wold-style-definition -Wmissing-prototypes \
	-Wdeclaration-after-statement
    do
	grep -c -- ${FLAG} WARN.out > /dev/null
	if [ $? -eq 0 ]
	then
	    ((FLAG_POINTS+=1))
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "  >> Failed gcc flag ${FLAG}"
	    fi
	fi
    done
    if [ ${FLAG_POINTS} -eq 8 ]
    then
	((WARN_POINTS+=10))
    fi

    echo "** Makefile points (section 1.6) : ${MAKE_POINTS} of 40 **"
    echo "** Points for no compiler warnings (section 1.6.2): ${WARN_POINTS} of 20 **"

    ((TOTAL_POINTS+=${MAKE_POINTS}))
    ((TOTAL_AVAIL+=40))
    ((TOTAL_POINTS+=${WARN_POINTS}))
    ((TOTAL_AVAIL+=20))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f ./WARN*
    fi
}

function testPythonPopen {
    POINTS_AVAIL=30
    POINTS=0

    echo ""
    echo "Testing Python Popen"

    if [ -e ${PPROG} ]
    then
	python ${PPROG} > Popen_S.txt
	who > Popen_I.txt

	diff -b -B Popen_I.txt Popen_S.txt > /dev/null 2>&1
	RES=$?

	if [ ${RES} -ne 0 ]
	then
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
	    then
		echo "      >> Python Popen fails match"
            fi
	    POINTS=0
	else
	    if [ ${VERBOSE} -eq 1 -a ${SUM_ONLY} -eq 0 ]
            then
		echo "      Python Popen passes match"
            fi
	    POINTS=${POINTS_AVAIL}
	fi
    fi

    echo "**** Points for Python Popen ${POINTS} of ${POINTS_AVAIL} **"

    #PYTHON_POINTS=${POINTS}
    ((TOTAL_POINTS+=${POINTS}))

    if [ ${CLEANUP} -eq 1 ]
    then
	rm -f ./Popen_[IS].txt
    fi
}


if [ ! -e ${PPROG} ]
then
    PPROG=problem3.py
fi

trap 'signalCaught;' SIGTERM SIGQUIT SIGKILL SIGSEGV
trap 'signalCtrlC;' SIGINT
trap 'signalSegFault;' SIGCHLD
#trap -p

rm -f coreDump.log
touch coreDump.log

# Not a lot of command line options.  The getopts function 
# is a bash built-in.  It differs from the getopt command
# line program.
while getopts "hCxvsS:P:pE:" opt
do
    case $opt in
	h)
	    echo "-E arg : Change the value at which Extra Credit is checked."
	    echo "-S arg : Change the name of the students myoscar program"
	    echo "-P arg : Change the name of the students Python program"
	    echo "-p     : Only run the Python portion of the assignment"
	    echo "-C     : Do not clean up the created files"
	    echo "-s     : Use the students archive file on extraction tests"
	    echo "-v     : Be verbose (reasonably)"
	    echo "-x     : Be REALLY verbose (does a set -x)"
	    exit
	    ;;
	E)
	    # Change the value at which Extra Credit is checked.
	    EC_PERCENT=$OPTARG
	    ;;
	S)
	    # Change the name of the student program.  The
	    # default is ./myoscar.
	    SPROG=$OPTARG
	    ;;
	P)
	    # Change the name of the student Python file.  The
	    # default is Problem3.py.
	    PPROG=$OPTARG
	    ;;
	p)
	    # only run the Python code.
	    PYTHON_ONLY=1
	    ;;
	C)
	    # Skip removal of data files
	    CLEANUP=0
	    ;;
	s)
	    # Use the students archive file on extraction tests.
	    EX=S
	    ;;
	v)
	    # Print extra messages.
	    VERBOSE=1
	    ;;
	x)
	    # If you really, really, REALLY want to watch what is going on.
	    set -x
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

echo "Running with student oscar program \"${SPROG}\""
echo "Running with student Python file \"${PPROG}\""
echo ""

if [ ${PYTHON_ONLY} -eq 1 ]
then
    testPythonPopen
    rm -f coreDump.log
    exit
fi

# This must preceed any of the oscar tests
build

# This NEEEEEDs to be the first testing function.  It is where
# all the files are copied in and built.
testLittleA

testLittleH
testBigV
testLittleV
testLittleD
testLitleE
testBigE
testLitleEoBigEo
testLittleT
testBigT

OSCAR_PERCENT=`awk "BEGIN {print int(${TOTAL_POINTS} / ${TOTAL_AVAIL} * 100);}"`

if [ ${OSCAR_PERCENT} -gt ${EC_PERCENT} ]
then
    echo "Checking for Extra Credit"
    echo "  received ${OSCAR_PERCENT} percent of base oscar points"
    echo ""
    testBigA
    testMark
    testSha
else
    echo "Extra Credit not available"
    echo "  received ${OSCAR_PERCENT} percent of base oscar points"
    echo ""
fi

echo ""
echo "**** oscar total points ${TOTAL_POINTS} (without Extra Credit)"
echo "**** oscar avail points ${TOTAL_AVAIL} (without Extra Credit)"

if [ ${EC_POINTS} -gt 0 ]
then
    echo "**** Total extra credit points earned ${EC_POINTS}"
    ((TOTAL_POINTS+=${EC_POINTS}))
fi
echo "**** Total oscar points ${TOTAL_POINTS} (includes Extra Credit)"
CORE_DUMPS=`wc -l < coreDump.log`
if [ ${CORE_DUMPS} -gt 0 ]
then
    echo "     Your ${SPROG} program did a core/abort dump ${CORE_DUMPS} times during testing"
fi

testPythonPopen

echo ""
echo "*******************************************************"
echo "**** TOTAL ${HWK} points ${TOTAL_POINTS} (includes any Extra Credit)"
echo "**** Deductions for late assignments will be done separately"
echo "**** Deductions for omitting file header info is done separately"
echo "**** Points for the oscar plan show up in a seperate column in Blackboard"

GRADED_BY=`finger ${LOGNAME} | grep gecos: | awk '{print $2,$3,$4,$5;}'`
DATE=`date`
HOST=`hostname`
SCRIPT=`basename $0`
echo ""
echo "Graded by ${GRADED_BY}  (${LOGNAME})"
echo "    ${DATE} ${HOST}"
echo "    grading script ${SCRIPT}" ' $Revision: 1.25 $'


if [ ${CLEANUP} -eq 1 ]
then
    cleaupTestFiles
    rm -f *.oscar coreDump.log I.log S.log ls.log oscar.h oscar grep.log
fi
