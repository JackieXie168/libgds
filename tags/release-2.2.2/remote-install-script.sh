#!/bin/bash

declare -r LINE="--------------------------------\
-------------------------------------"

# -----[ info ]------------------------------------------------------
info()
{
    local MSG=$1
    echo -e "\033[1m$MSG\033[0m"
}

# -----[ info_part ]-------------------------------------------------
info_part()
{
    local MSG=$1
    echo $LINE
    echo -e "*** \033[33;1m$MSG\033[0m ***"
    echo $LINE
}

# -----[ error ]-----------------------------------------------------
error()
{
    local MSG=$1
    echo -e "ERROR: \033[1;31m$MSG\033[0m"
    echo "Aborting..."
    exit 2
}

# -----[ check ]-----------------------------------------------------
check()
{
    local CMD=$1
    echo "$CMD"
    eval "$CMD $REDIRECT"
    if [ $? != 0 ]; then
	error "could not run $CMD"
    fi
}

# -----[ main ]------------------------------------------------------

declare DOWNLOAD=1
declare TIME=`date +'%d%m%Y-%H%M%S'`

# *** Parse arguments ***
for arg in $@; do
  case $arg in
  -libgds=*)
    LIBGDS=${arg:8}
  ;;
  -target=*)
    HOST=${arg:8}
  ;;
  -flags=*)
    FLAGS=${arg:7}
  ;;
  -no-download)
    DOWNLOAD=0
  ;;
  *)
    echo "Error: invalid argument [ $arg ]"
    exit -1
  ;;
  esac
done

if [ -z $LIBGDS ] || [ -z $HOSTNAME ]; then
  error "LIBGDS and HOSTNAME must be defined !"
fi

info_part "Building LIBGDS on $HOSTNAME"

declare ROOTDIR=$PWD
declare REDIRECT=">$ROOTDIR/$HOSTNAME-remote-install-$TIME.log 2>&1"

if [ $DOWNLOAD = 1 ]; then
  info "Downloading packages..."
  if [ -e libgds-$LIBGDS.tar.gz ]; then
    eval "rm -f libgds-$LIBGDS.tar.gz $REDIRECT"
  fi
  check "wget -q \
      http://libgds.info.ucl.ac.be/downloads/libgds-$LIBGDS.tar.gz"
fi

info "Building libgds-$LIBGDS..."
check "gunzip -c libgds-$LIBGDS.tar.gz | tar xvf - $REDIRECT"

check "cd libgds-$LIBGDS $REDIRECT"

check "./configure --prefix=$ROOTDIR/local-$HOSTNAME $REDIRECT"

check "make clean $REDIRECT"

check "make $REDIRECT"

check "make install $REDIRECT"

check "cd $ROOTDIR $REDIRECT"
