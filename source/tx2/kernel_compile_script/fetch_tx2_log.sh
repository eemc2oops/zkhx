#!/bin/bash

CONFIG_FILE=config.sh
CURRENT_DIR=`pwd`
DATEPATTERN='^22.*?ssh$'

if [ ! -f $CURRENT_DIR/$CONFIG_FILE ]
then
    echo "$CONFIG_FILE file not exist."
    exit 1
fi

source $CURRENT_DIR/$CONFIG_FILE

function tx2_status()
{
    nmap -p $TX2_PORT $TX2_IP | while read line
    do
        if [[ "$line" =~ $DATEPATTERN ]]
        then
            state=`echo $line | awk '{print $2}'`
            if [ "$state" = "open" ]
            then
                return 201    
            fi
        fi
    done

    if [ $? = 201 ]
    then
        return 0
    fi

    return 1
}

echo ""
printf "wait tx2 running."

while true
do
    ping $TX2_IP -c 1 > /dev/null
    if [ $? = 0 ]
    then
        break
    fi

    sleep 1
    printf "."
done


while true
do
    tx2_status
    if [ $? = 0 ]
    then
        break    
    fi

    sleep 1
    printf "*"
done

for((i=1;i<=5;i++));
do
    sleep 1
    printf "+"
done

echo ""

echo "tx2 is running."


if [ -f ./$TX2_KERNEL_LOG.previous ]
then
    rm ./$TX2_KERNEL_LOG.previous
fi

if [ -f ./$TX2_KERNEL_LOG ]
then
    mv ./$TX2_KERNEL_LOG ./$TX2_KERNEL_LOG.previous
fi

scp -P $TX2_PORT $TX2_PATCH_USER@$TX2_IP:/var/log/kern.log ./$TX2_KERNEL_LOG

if [ ! -f ./$TX2_KERNEL_LOG ]
then
    echo "scp tx2 kern log failed."
    exit 1
fi

echo "tx2 log file name $TX2_KERNEL_LOG"

exit 0



