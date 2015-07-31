# Korebot 2
# environment variables for programming
# K-Team S.A.

# --- DESCRIPTION -------------------------------------------------------------
  # This file must be sourced before starting the cross-compiler to set
  # the environment variables.

# --- VERSION -----------------------------------------------------------------

# version 1.0: April 2009: first release

# --- ENVIRONMENT VARIABLES DEFINITIONS ---------------------------------------

# Modify this!
export KTEAM_HOME=/home/viki/Downloads/khepera

# And maybe these
export LIBKOREBOT_ROOT=$KTEAM_HOME/libkorebot-1.19-kb1

export KTEAM_KERNEL_VERSION=2.6.24

export KTEAM_KERNEL_HOME=/usr/local/korebot2-oetools-1.0/tmp/work/gumstix-custom-verdex-angstrom-linux-gnueabi/gumstix-kernel-$KTEAM_KERNEL_VERSION-r1/linux-$KTEAM_KERNEL_VERSION

# And don't touch these
export ARCH="arm"
export CROSS_COMPILE="arm-angstrom-linux-gnueabi-"
export PATH=$PATH:$KTEAM_HOME/cross/bin
export CC=arm-angstrom-linux-gnueabi-gcc
# added CXX
export CXX=arm-angstrom-linux-gnueabi-g++
export LD=arm-angstrom-linux-gnueabi-ld
export AR=arm-angstrom-linux-gnueabi-ar
export AS=arm-angstrom-linux-gnueabi-as


export INCPATH="$LIBKOREBOT_ROOT/build-korebot-2.6/include"


export LIBPATH="$LIBKOREBOT_ROOT/build-korebot-2.6/lib"




