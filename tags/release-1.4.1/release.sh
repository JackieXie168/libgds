#!/bin/bash
# ===================================================================
# Release script for libgds
# Last modified: 29/09/2006 (bqu)
# ===================================================================

. release_functions

#####################################################################
# ------------->>>> PARAMETERS UPDATE NEEDED HERE <<<<---------------
#####################################################################
#
# MODULE (mandatory)
#   is the module name (or the project name)
#
MODULE="libgds"
#
# VC (mandatory)
#   is the version control system (cvs / svn)
#
VC=svn
#
# SVN_URL (optional)
#   is the SVN URL of the project (this needs only be configured
#   for SVN projects)
#
SVN_URL="https://gforge.info.ucl.ac.be/svn/$MODULE"
#
# MAIN_VERSION (mandatory)
#   is the project main version (major)
#
MAIN_VERSION=1
#
# RELEASE_VERSION (mandatory)
#   is the release main version (minor)
#
RELEASE_VERSION=4
#
# BUILD_VERSION (mandatory)
#   is the build number
#
BUILD_VERSION=1
#
# LIBRARY_VERSION (optional)
#   is the library version
#
#LIBRARY_VERSION=`grep "LDFLAGS" src/libgds/Makefile.am | sed 's:^.*LDFLAGS.*=.*-version-info \([0-9]*\)\:\([0-9]*\)\:\([0-9]*\)$:\1.\2:'`
LIBRARY_VERSION=`grep "LT_RELEASE" configure.in | sed 's:^AC_SUBST(LT_RELEASE, \[\(.*\)\])$:\1:'`
#
# VERSION
#   
VERSION="$MAIN_VERSION.$RELEASE_VERSION.$BUILD_VERSION"
#
# CONF_OPTIONS[] (mandatory)
#   is an array of configure options. The project is built once
#   for each element of the array.
#
CONF_OPTIONS[0]=""
#
# CONF_OPTIONS_COMMON (optional)
#   is a set of common configure options
#                            
CONF_OPTIONS_COMMON=
#
# VALID_EXEC (optional)
#   is the name of a command used to validate the project.
#
VALID_EXEC="make check"
#
# PUBLISH_URL (optional)
#   is the SCP URL where the distribution will be published
#
PUBLISH_URL="libgds@libgds.info.ucl.ac.be:/var/www/libgds/downloads"
#
# PUBLISH_FILES (optional)
#   is the list of files that needs to be published
#
PUBLISH_FILES="$MODULE-$VERSION.tar.gz"
#
#####################################################################
# ----->>>> YOU SHOULDN'T CHANGE THE SCRIPT PAST THIS LINE <<<<------
#####################################################################

make_release $@
