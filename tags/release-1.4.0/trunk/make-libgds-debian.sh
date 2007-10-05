#!/bin/bash

###
# Automate the debian package building process
#
# In order to use it, don't forget to provides accurate information in debian_package.config
#
###

. make-package-lib
. make-package-debian.config
. make-package.config


if [[ -z $MODULE ]]; then
  error "No module defined. Impossible to build the debian package."
fi

if [[ -z $VERSION ]]; then
  error "No version defined. Impossible to build the debian package."
fi

DebianPackageName="$MODULE-$VERSION"
DebianDir="debian"

ControlFilename="control"
ChangelogFilename="changelog"

TemplateDir="$DebianDir/template"
TemplatesSuffix=".copyright .dirs .install"
TemplatesControlFiles="$ControlFilename"
TemplatePackageName="$MODULE""packagename"

PackagesType="-doc -dev"
function copy_package_templates
{
  if [[ -z $1 || -z $2 || -z $3 || -z $4 || -z $5 ]]; then
    return 1
  fi

  DirSource=$1
  shift
  DirDest=$1
  shift
  FakePackageName=$1
  shift
  RealPackageName=$1
  shift

  for suffix in $@
  do
    cp $DirSource/$FakePackageName$suffix $DirDest/$RealPackageName$suffix
#    echo $DirSource/$FakePackageName$suffix $DirDest/$RealPackageName$suffix
  done

  return 0
}

function copy_control_files
{
  if [[ -z $1 || -z $2 || -z $3 ]]; then
    return 1
  fi

  DirSource=$1
  shift
  DirDest=$1
  shift

  for file in $@
  do
    cp $DirSource/$file $DirDest/$file
  done

  return 0
}

function adapt_control
{
  if [[ -z $1 || -z $2 || -z $3 ]]; then
    return 1
  fi
  ControlFile=$1
  PackageNamePattern=$2
  PackageName=$3

  cat $ControlFile | sed -e s/\%$PackageNamePattern\%/$PackageName/ > $ControlFile.tmp
  mv $ControlFile.tmp $ControlFile

  return 0
}

function add_changelog_entry
{
  if [[ -z $1 || -z $2 || -z $3 || -z $4 || -z $5 || -z $6 ]]; then
    return 1
  fi

  Changelog=$1
  shift
  DebianDir=$1
  shift
  TemplateDir=$1
  shift
  Module=$1
  shift
  Version=$1
  shift
  DebianVersion=$1
  shift
  MaintainerMail=$1
  shift

  MaintainerName=""
  for parts in $@
  do
    MaintainerName="$MaintainerName$parts "
  done

  echo "$Module ($Version-$DebianVersion) unstable; urgency=low"  >   changelog_entry
  echo								  >>  changelog_entry
  echo "  * $PACKAGE_REASON"					  >>  changelog_entry
  echo								  >>  changelog_entry
  echo " -- $MaintainerName<$MaintainerMail>  `822-date`"	  >>  changelog_entry
  echo								  >>  changelog_entry


  cat changelog_entry $TemplateDir/$Changelog > $DebianDir/$Changelog
  rm changelog_entry

  return 0
}


# copy the mandatory package (without extension)
copy_package_templates $TemplateDir $DebianDir $TemplatePackageName $DebianPackageName $TemplatesSuffix 
if [[ $? -ne 0 ]]; then
  error "unable to copy the templates files"
fi
  

# copy additional packages templates
for packagetype in $PackagesType
do
   copy_package_templates $TemplateDir $DebianDir $TemplatePackageName"$packagetype" $DebianPackageName"$packagetype" $TemplatesSuffix
   if [[ $? -ne 0 ]]; then
      error "unable to copy the templates files for the package type : $packagetype"
   fi
done

# Copy the control files we want to update for the debian building process
copy_control_files $TemplateDir $DebianDir $TemplatesControlFiles
if [[ $? -ne 0 ]]; then
  error "unable to copy the control file"
fi

# Change all template names to the wished debian package name
adapt_control $DebianDir/$ControlFilename $TemplatePackageName $DebianPackageName
if [[ $? -ne 0 ]]; then
  error "unable to adapt the control file"
fi

# Add an entry to the changelog ... this changelog is copied back to the template repository
add_changelog_entry $ChangelogFilename $DebianDir $TemplateDir $MODULE $VERSION $DEBIAN_VERSION $DEBIAN_MAINTAINER_EMAIL $DEBIAN_MAINTAINER 
if [[ $? -ne 0 ]]; then
  error "unable add an entry to the changelog"
fi

dpkg-buildpackage -us -uc -rfakeroot
if [[ $? -ne 0 ]]; then
  error "the packaging has not been completely built"
fi

##save the changelod information
cp $DebianDir/$ChangelogFilename $TemplateDir/$ChangelogFilename
