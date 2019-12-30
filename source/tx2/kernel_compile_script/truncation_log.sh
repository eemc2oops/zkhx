#!/bin/bash

CONFIG_FILE=config.sh
CURRENT_DIR=`pwd`

if [ ! -f $CURRENT_DIR/$CONFIG_FILE ]
then
    echo "$CONFIG_FILE file not exist."
    exit 1
fi

source $CURRENT_DIR/$CONFIG_FILE

function cut_line()
{
    local num=0
    local last_tag=0
    local curr_tag=0
    local cut_num=0

    while true
    do
        cut_num=0
        num=0
        last_tag=0
        curr_tag=0
        while read line
        do
            num=`expr $num + 1`
            curr_tag=`echo $line | cut -d '[' -f2|cut -d ']' -f1 | awk '{sub(/^ */, "");sub(/ *$/, "")}1'`
            #echo "$num   [$curr_tag]  [$last_tag]  $line"
            local state=`echo "$curr_tag<$last_tag" | bc`
            if [ $state -eq 1 ]
            then
                cut_num=$num
                #echo "big     $num   [$curr_tag]  [$last_tag]  $line"
                #echo $cut_num
                break
            fi
            last_tag=$curr_tag
        done < ./$LAST_ONCE_TX2_KERNEL_LOG

        #echo "check cut_num  $cut_num"

        if [ $cut_num -eq 0 ]
        then
            #echo "file is ok"
            break
        fi

        echo "cut log file at line $cut_num"
        if [ -f ./$LAST_ONCE_TX2_KERNEL_LOG.tmp ]
        then
            rm ./$LAST_ONCE_TX2_KERNEL_LOG.tmp
        fi
	sed -n "$cut_num,\$p" ./$LAST_ONCE_TX2_KERNEL_LOG >> ./$LAST_ONCE_TX2_KERNEL_LOG.tmp
        mv ./$LAST_ONCE_TX2_KERNEL_LOG.tmp ./$LAST_ONCE_TX2_KERNEL_LOG
    done

    #echo "return"
    return 0
}


if [ ! -f ./$TX2_KERNEL_LOG ]
then
    echo "$TX2_KERNEL_LOG not exist."
    exit 1
fi

if [ -f ./$LAST_ONCE_TX2_KERNEL_LOG ]
then
    mv ./$LAST_ONCE_TX2_KERNEL_LOG ./$LAST_ONCE_TX2_KERNEL_LOG.previous
fi

boot_flag=`grep -n "Booting Linux on physical CPU 0x100" ./$TX2_KERNEL_LOG`
if [ $? != 0 ]
then
    echo "./$TX2_KERNEL_LOG log file error."
    exit 1
fi

line_start=`grep -n "Booting Linux on physical CPU 0x100" ./$TX2_KERNEL_LOG | tail -1 | cut -d : -f 1`
if [ ! -n $line_start ]
then
    echo "./$TX2_KERNEL_LOG log file error."
    exit 1
fi

sed -n "$line_start,\$p" ./$TX2_KERNEL_LOG >> ./$LAST_ONCE_TX2_KERNEL_LOG

cut_line

exit 0



