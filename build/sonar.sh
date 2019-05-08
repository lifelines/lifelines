#!/bin/sh

# NOTE: This script assumes that:
# - the lifelines project is set up in sonarcloud
# - the lifelines project configuration is in sonar-project.properties
# - the sonar-scanner is installed and in $PATH
#   (see https://docs.sonarqube.org/display/SCAN/Analyzing+with+SonarQube+Scanner)

# Check for configuration file
if [ ! -f sonar-project.properties ]
then
  echo "ERROR: Could not find sonar-project.properties file!"
  exit
fi

# Get and unzip build wrapper
wget https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
unzip build-wrapper-linux-x86.zip

# Build with build wrapper
(cd ..; build/build-wrapper-linux-x86/build-wrapper-linux-x86-64 --out-dir build/sonar_build_wrapper_output make clean all)

# Run analysis (on current branch, assuming that the target branch will be master)
MYBRANCH=`git status | head -1 | cut -f4 -d' '`
(cd ..; echo sonar-scanner -Dproject.settings=build/sonar-project.properties -Dsonar.branch.target=master -Dsonar.branch.name=${MYBRANCH})

# Remove build wrapper and scanner files
rm -rf sonar_build_wrapper_output
rm -rf ../.scannerwork

# Remove build wrapper
rm -rf build-wrapper-linux-x86
rm -f build-wrapper-linux-x86.zip
