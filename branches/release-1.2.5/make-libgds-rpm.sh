#!/bin/bash
# -------------------------------------------------------------------
# Make libgds RPM packages
# author: Bruno Quoitin (bqu@info.ucl.ac.be)
# -------------------------------------------------------------------

MODULE="libgds"
VERSION="1.2.1"

RPM_PKG_RELEASE="0.FC4"
RPM_SPEC_FILE="/tmp/$MODULE-rpm.spec"
RPM_RPMS_DIR=`rpm --eval %{_rpmdir}`"/i386"
RPM_SOURCE_DIR=`rpm --eval %{_sourcedir}`

SOURCE_PKG="$MODULE-$VERSION.tar.gz"
SOURCE_URL="http://libgds.info.ucl.ac.be/downloads/$SOURCE_PKG"

PUBLISH_URL="bqu@libgds.info.ucl.ac.be:/var/www/libgds/downloads"
PUBLISH_FILES=" \
    libgds-$VERSION-$RPM_PKG_RELEASE.i386.rpm \
    libgds-devel-$VERSION-$RPM_PKG_RELEASE.i386.rpm \
    "

REDIRECT='>/dev/null 2>&1'
#REDIRECT=''

# -----[ info ]------------------------------------------------------
info()
{
    MSG=$1
    echo -e "\033[1m$MSG\033[0m"
}

# -----[ error ]-----------------------------------------------------
error()
{
    MSG=$1
    echo -e "ERROR: \033[1;31m$MSG\033[0m"
    echo "Aborting..."
    exit -1
}

# -----[ redhat_version ]--------------------------------------------
redhat_version()
{
    REDHAT_RELEASE_FILE="/etc/redhat-release"
    if [ ! -e $REDHAT_RELEASE_FILE ]; then
	error "not a redhat-based distribution"
    fi
    REDHAT_RELEASE=`cat $REDHAT_RELEASE_FILE`
    echo "  redhat-release: $REDHAT_RELEASE"
}

# -----[ rpm_init ]--------------------------------------------------
rpm_init()
{
	echo ""
}

# -----[ download_sources ]------------------------------------------
download_sources()
{
    info "Downloading source package..."
    echo "  url        : $SOURCE_URL"
    echo "  rpm-src-dir: $RPM_SOURCE_DIR"
    if [ -e $SOURCE_PKG ]; then
	error "source file already exists."
    fi
    eval "wget $SOURCE_URL $REDIRECT"
    if [ $? != 0 ] || [ ! -e $SOURCE_PKG ]; then
	error "could not download \"$SOURCE_PKG\"."
    fi
    mv $SOURCE_PKG $RPM_SOURCE_DIR
    if [ $? != 0 ]; then
	error "could not move \"$SOURCE_PKG\" to \"$RPM_SOURCE_DIR\"."
    fi
}

redhat_version
rpm_init

# -------------------------------------------------------------------
# Download source package
# -------------------------------------------------------------------
download_sources

# -------------------------------------------------------------------
# Build RPM spec file
# -------------------------------------------------------------------
info "Building RPM spec file..."
echo "  version: $VERSION"
echo "  release: $RPM_PKG_RELEASE"
cat > $RPM_SPEC_FILE <<EOF
Summary: libgds, a library of generic data structures
Name: libgds
Version: $VERSION
Release: $RPM_PKG_RELEASE
License: GPL/LGPL
Group: Development/Libraries
Packager: Bruno Quoitin (bqu@info.ucl.ac.be)
URL: http://openresources.info.ucl.ac.be/libgds
ExclusiveArch: i386
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_builddir}/%{name}-buildroot
%define _unpackaged_files_terminate_build 0
%description
No description for now...
%prep
%setup -q
%build
./configure
make
%install
rm -rf \$RPM_BUILD_ROOT
make DESTDIR=\$RPM_BUILD_ROOT install
%clean
rm -rf \$RPM_BUILD_ROOT
%files
%defattr(-,root,root)
/usr/local/lib/%{name}.so*
%package devel
Summary: libgds development package
License: GPL/LGPL
Group: Development/Libraries
Requires: %{name} = %{version}
%description devel
libgds development package
%files devel
%defattr(-,root,root)
/usr/local/include/*
/usr/local/lib/%{name}.a
/usr/local/lib/%{name}.la
EOF

# -------------------------------------------------------------------
# Build RPM packages
# -------------------------------------------------------------------
info "Building package (rpmbuild)..."
eval "rpmbuild -ba $RPM_SPEC_FILE $REDIRECT"
if [ $? != 0 ]; then
    error "could not build RPM."
fi

# -------------------------------------------------------------------
# Info
# -------------------------------------------------------------------
echo "You should find the RPM packages in the following directory"
echo "  $RPM_RPMS_DIR"

for f in $PUBLISH_FILES; do
    RPM_FILE="$RPM_RPMS_DIR/$f"
    if [ ! -e $RPM_FILE ]; then
	error "could not find \"$f\"."
    fi
    scp $RPM_FILE $PUBLISH_URL
    if [ $? != 0 ]; then
	error "could not publish \"$f\"."
    fi
done
