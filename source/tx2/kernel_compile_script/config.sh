#!/bin/bash

KERNEL_DIR=/home/zerg/tx2_src/with_rapidio/kernel/kernel-4.9
PLATFORM=arm64

VERSION_FILE=ver

# compress dir tree
COMPRESS_DIR=obj
COMPRESS_BOOT_DIR=boot
COMPRESS_DTB_DIR=dtb
COMPRESS_MODULES_DIR=lib

OS_PACKAGE=kernel_package.tar.gz



TX2_IP=172.16.202.60
#TX2_IP=172.16.202.100
TX2_PORT=22
#TX2_PORT=30022
TX2_USER=zkhx
TX2_PW=123

TX2_PATCH_USER=root

TX2_DIR=/home/zkhx/tx2/spc000

PATCH_SCRIPT=patch.sh

TX2_KERNEL_LOG=tx2_kern.log

GET_LOG_SCRIPT=fetch_tx2_log.sh

CUT_LOG_SCRIPT=truncation_log.sh
LAST_ONCE_TX2_KERNEL_LOG=last_once_tx2_kern.log
