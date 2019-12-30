#!/bin/bash

CONFIG_FILE=config.sh
CURRENT_DIR=`pwd`
KERNEL_BUILD_LOG_FILE=log_build_kernel.log
MODULE_BUILD_LOG_FILE=log_build_module.log
DTB_BUILD_LOG_FILE=log_build_dtb.log
MODULE_INSTALL_LOG_FILE=log_install_module.log
COMPRESS_DIR=obj

if [ ! -f $CURRENT_DIR/$CONFIG_FILE ]
then
    echo "$CONFIG_FILE file not exist."
    exit 1
fi

source $CURRENT_DIR/$CONFIG_FILE

function build_linux()
{
    #return 0
    echo "begin build image, wait a moment."
    make LOCALVERSION=-tegra ARCH=arm64 -j4 zImage > $CURRENT_DIR/$KERNEL_BUILD_LOG_FILE 2>&1
    #make LOCALVERSION=-tegra ARCH=arm64 -j4 zImage 2>&1 | tee  $CURRENT_DIR/$KERNEL_BUILD_LOG_FILE
    if [ $? != 0 ]
    then
        tail -n 50 $CURRENT_DIR/$KERNEL_BUILD_LOG_FILE
        echo "build image failed."
        return 1
    fi

    echo "build image ok. begin build drivers. be patient."

    make LOCALVERSION=-tegra ARCH=arm64 -j4 modules > $CURRENT_DIR/$MODULE_BUILD_LOG_FILE 2>&1
    #make LOCALVERSION=-tegra ARCH=arm64 -j4 modules 2>&1 | tee  $CURRENT_DIR/$MODULE_BUILD_LOG_FILE
    if [ $? != 0 ]
    then
        tail -n 50 $CURRENT_DIR/$MODULE_BUILD_LOG_FILE
        echo "build modules failed."
        return 1
    fi

    echo "build drivers ok. begin build dtbs. use a little time."

    make LOCALVERSION=-tegra ARCH=arm64 -j4 dtbs > $CURRENT_DIR/$DTB_BUILD_LOG_FILE 2>&1
    #make LOCALVERSION=-tegra ARCH=arm64 -j4 dtbs 2>&1 | tee  $CURRENT_DIR/$DTB_BUILD_LOG_FILE
    if [ $? != 0 ]
    then
        tail -n 50 $CURRENT_DIR/$DTB_BUILD_LOG_FILE
        echo "build dtbs failed."
        return 1
    fi

    make LOCALVERSION=-tegra ARCH=$PLATFORM kernelrelease

    echo "build dtbs ok."

    return 0
}

function cp_obj()
{
    local image_file=$KERNEL_DIR/arch/$PLATFORM/boot/Image
    local dtb_dir=$KERNEL_DIR/arch/$PLATFORM/boot/dts
    local install_log=$CURRENT_DIR/$MODULE_INSTALL_LOG_FILE

    echo "begin copy obj."

    if [ -d $CURRENT_DIR/$COMPRESS_DIR ]
    then
        rm -rf $CURRENT_DIR/$COMPRESS_DIR
    fi

    mkdir -p $CURRENT_DIR/$COMPRESS_DIR
    mkdir -p $CURRENT_DIR/$COMPRESS_DIR/$COMPRESS_BOOT_DIR
    mkdir -p $CURRENT_DIR/$COMPRESS_DIR/$COMPRESS_DTB_DIR

    echo $KERNEL_VERSION > $CURRENT_DIR/$COMPRESS_DIR/$VERSION_FILE

    cp $image_file $CURRENT_DIR/$COMPRESS_DIR/$COMPRESS_BOOT_DIR
    if [ $? != 0 ]
    then
        echo "cp $image_file failed"
        return 1
    fi

    cp $dtb_dir/*.dtb $CURRENT_DIR/$COMPRESS_DIR/$COMPRESS_DTB_DIR
    if [ $? != 0 ]
    then
        echo "cp $dtb_dir failed"
        return 1
    fi

    make LOCALVERSION=-tegra  ARCH=arm64 INSTALL_MOD_PATH=$CURRENT_DIR/$COMPRESS_DIR modules_install > $install_log 2>&1
    #make LOCALVERSION=-tegra  ARCH=arm64 INSTALL_MOD_PATH=$CURRENT_DIR/$COMPRESS_DIR modules_install  2>&1 | tee $install_log
    if [ $? != 0 ]
    then
        echo "cp modules failed"
        return 1
    fi

    return 0
}

function make_package()
{
    if [ ! -d $KERNEL_DIR ]
    then
        echo "$KERNEL_DIR not exist."
        return 1
    fi

    cd $KERNEL_DIR

    build_linux

    if [ $? != 0 ]
    then
        echo "build linux failed."
        cd $CURRENT_DIR
        return 1
    fi

    KERNEL_VERSION=`make LOCALVERSION=-tegra ARCH=$PLATFORM kernelrelease`

    cp_obj

    if [ $? != 0 ]
    then
        echo "compress linux failed."
        cd $CURRENT_DIR
        return 1
    fi

    cd $CURRENT_DIR

    echo "begin tar obj."
    if [ -f $CURRENT_DIR/$OS_PACKAGE ]
    then
        rm $CURRENT_DIR/$OS_PACKAGE
    fi

    tar zcvf $CURRENT_DIR/$OS_PACKAGE ./$COMPRESS_DIR > /dev/null 2>&1
    #tar zcvf $CURRENT_DIR/$OS_PACKAGE ./$COMPRESS_DIR
    if [ $? != 0 ]
    then
        echo "tar obj failed"
        return 1
    fi

    return 0
}

echo "build kernel & module & dtb for NVIDIA Jetson TX2"

make_package
if [ $? != 0 ]
then
    exit 1
fi

ssh -p $TX2_PORT $TX2_USER@$TX2_IP "mkdir -p $TX2_DIR"
if [ $? != 0 ]
then
    echo "ssh failed."
    exit 1
fi

ssh -p $TX2_PORT $TX2_USER@$TX2_IP "rm -f $TX2_DIR/$OS_PACKAGE"
if [ $? != 0 ]
then
    echo "ssh failed."
    exit 1
fi

#scp to tx2
scp -P $TX2_PORT $CURRENT_DIR/$OS_PACKAGE $TX2_USER@$TX2_IP:$TX2_DIR
if [ $? != 0 ]
then
    echo "scp failed."
    exit 1
fi

scp -P $TX2_PORT $CURRENT_DIR/$PATCH_SCRIPT $TX2_USER@$TX2_IP:$TX2_DIR
if [ $? != 0 ]
then
    echo "scp failed."
    exit 1
fi

scp -P $TX2_PORT $CURRENT_DIR/$CONFIG_FILE $TX2_USER@$TX2_IP:$TX2_DIR
if [ $? != 0 ]
then
    echo "scp failed."
    exit 1
fi

ssh -p $TX2_PORT $TX2_PATCH_USER@$TX2_IP $TX2_DIR/$PATCH_SCRIPT

echo ""
printf "wait tx2 reboot."

while true
do
    ping $TX2_IP -c 1 > /dev/null
    if [ $? = 0 ]
    then
        sleep 1
        printf "."
        continue
    fi
    break
done

echo ""

echo "tx2 is restarting."

./$GET_LOG_SCRIPT
if [ $? != 0 ]
then
    echo "get tx2 log failed."
    exit 1
fi

./$CUT_LOG_SCRIPT
if [ $? != 0 ]
then
    echo "cut log failed."
    exit 1
fi

exit 0



