#!/bin/bash

# -----[ usage ]-----------------------------------------------------
usage()
{
    echo
    echo 'Usage:'
    echo '  remote-install.sh -libgds=<version> \'
    echo '                    -target=<host> -rdir=<directory> \'
    echo '                    [ -flags=<flags> ] [ -push ]'
    echo
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
    local MSG=$1
    if [ $? != 0 ]; then
	error "$1"
    fi
}

declare PUSH=0
declare RDIR='~'

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
  -push)
    PUSH=1
  ;;
  -rdir=*)
    RDIR=${arg:6}
  ;;
  *)
    echo "Error: invalid argument [ $arg ]"
    exit -1
  ;;
  esac
done

if [ -z $LIBGDS ] || [ -z $HOST ]; then
  echo "Error: missing argument(s)"
  usage
  exit -1
fi

SCRIPT=remote-install-script.sh

scp $SCRIPT $USER@$HOST:$RDIR/$HOST-$SCRIPT
check "could not copy remote install script"

if [ $PUSH = 1 ]; then
  scp libgds-$LIBGDS.tar.gz $USER@$HOST:$RDIR/libgds-$LIBGDS.tar.gz
  check "could not copy libgds-$LIBGDS.tar.gz"
  OPTIONS="-no-download"
fi

ssh $USER@$HOST "cd $RDIR; chmod +x $HOST-$SCRIPT; ./$HOST-$SCRIPT -libgds=$LIBGDS -flags=$FLAGS $OPTIONS"
check "could not execute remote install script"
