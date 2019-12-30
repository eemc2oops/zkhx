#!/bin/bash

CONFIG_FILE=config.sh
CURRENT_DIR=`pwd`


function kernel_patch()
{
    if [ ! -f ./$OS_PACKAGE ]
    then
        echo "$OS_PACKAGE not exist."
        return 1
    fi

    if [ -d ./$COMPRESS_DIR ]
    then
        rm -rf ./$COMPRESS_DIR
    fi

    tar zxvf ./$OS_PACKAGE > /dev/null 2>&1
    if [ $? != 0 ]
    then
        echo "tar failed."
        return 1
    fi

    if [ ! -f ./$COMPRESS_DIR/$VERSION_FILE ]
    then
        echo "$VERSION_FILE not exist."
        return 1
    fi

    KERNEL_VER=`cat ./$COMPRESS_DIR/$VERSION_FILE`

    # check file
    IMAGE_FILE=./$COMPRESS_DIR/$COMPRESS_BOOT_DIR/Image
    if [ ! -f "$IMAGE_FILE" ]
    then
        echo "$IMAGE_FILE not exist"
        return 1
    fi

    local dtb_dir=./$COMPRESS_DIR/$COMPRESS_DTB_DIR
    for file in /boot/*.dtb
    do
        dtbfile=`basename $file`
        echo "check $dtb_dir/$dtbfile"
        if [ ! -f "$dtb_dir/$dtbfile" ]
        then
            echo "$dtb_dir/$dtbfile not exist."
            return 1
        fi
    done

    for file in /boot/dtb/*.dtb
    do
        dtbfile=`basename $file`
        echo "check $dtb_dir/$dtbfile"
        if [ ! -f "$dtb_dir/$dtbfile" ]
        then
            echo "$dtb_dir/$dtbfile not exist."
            return 1
        fi
    done

    local module_dir=./$COMPRESS_DIR/lib/modules/$KERNEL_VER
    for file in /lib/modules/$KERNEL_VER/modules*
    do
        mod_name=`basename $file`
        echo "check $module_dir/$mod_name"
        if [ ! -f "$module_dir/$mod_name" ]
        then
            echo "$module_dir/$mod_name not exist"
            return 1
        fi
    done

    echo "begin replace file"

    replace_file
    if [ $? != 0 ]
    then
        echo "replace kernel failed."
        return 1
    fi

    return 0
}

function replace_file()
{
    echo "cp ./$COMPRESS_DIR/$COMPRESS_BOOT_DIR/Image /boot/Image"
    cp ./$COMPRESS_DIR/$COMPRESS_BOOT_DIR/Image /boot/Image
    if [ $? != 0 ]
    then
        echo "cp ./$COMPRESS_DIR/$COMPRESS_BOOT_DIR/Image /boot/Image failed."
        return 1
    fi

    local dtb_dir=./$COMPRESS_DIR/$COMPRESS_DTB_DIR
    for file in /boot/*.dtb
    do
        dtbfile=`basename $file`
        echo "cp $dtb_dir/$dtbfile /boot/"
        cp $dtb_dir/$dtbfile /boot/
        if [ $? != 0 ]
        then
            echo "cp $dtb_dir/$dtbfile /boot/ failed."
            return 1
        fi
    done

    for file in /boot/dtb/*.dtb
    do
        dtbfile=`basename $file`
        echo "cp $dtb_dir/$dtbfile /boot/dtb/"
        cp $dtb_dir/$dtbfile /boot/dtb/
        if [ $? != 0 ]
        then
            echo "cp $dtb_dir/$dtbfile /boot/dtb/ failed."
            return 1
        fi
    done

    local module_dir=./$COMPRESS_DIR/lib/modules/$KERNEL_VER
    for file in /lib/modules/$KERNEL_VER/modules*
    do
        mod_name=`basename $file`
        echo "cp $module_dir/$mod_name /lib/modules/$KERNEL_VER/"
        cp $module_dir/$mod_name /lib/modules/$KERNEL_VER/
        if [ $? != 0 ]
        then
            echo "cp $module_dir/$mod_name /lib/modules/$KERNEL_VER/ failed."
            return 1
        fi
    done

    local mod_kernel_dir=$module_dir/kernel

    #echo "rm -rf /lib/modules/$KERNEL_VER/kernel/*"
    rm -rf /lib/modules/$KERNEL_VER/kernel/*
    echo "cp -r $mod_kernel_dir/* /lib/modules/$KERNEL_VER/kernel/"
    cp -r $mod_kernel_dir/* /lib/modules/$KERNEL_VER/kernel/
    if [ $? != 0 ]
    then
        echo "cp -r $mod_kernel_dir/* /lib/modules/$KERNEL_VER/kernel/ failed."
        return 1
    fi

    return 0
}


RUN_DIR=`dirname $0`

cd $RUN_DIR

if [ ! -f ./$CONFIG_FILE ]
then
    echo "$CONFIG_FILE file not exist."
    cd $CURRENT_DIR
    exit 1
fi

source ./$CONFIG_FILE

echo $TX2_DIR

if [ ! -d $TX2_DIR ]
then
    echo "$TX2_DIR not exist."
    exit 1
fi

cd $TX2_DIR

if [ $? != 0 ]
then
    echo "cd $TX2_DIR failed."
    exit 1
fi

kernel_patch
if [ $? != 0 ]
then
    cd $CURRENT_DIR
    exit 1
fi

cd $CURRENT_DIR

echo "PATCH OK!!!!!!!!!!!!!!!!"

printf "reboot tx2 to make new kernel effect."

for((i=1;i<=5;i++));
do
    sleep 1
    printf "."
done

echo ""

shutdown -r -t 5

echo "tx2 rebooting....."

exit 0
