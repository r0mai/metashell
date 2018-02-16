#!/bin/bash

set -ex

brew update >/dev/null

# Oclint installs something under /usr/local/include/c++ which
# conflicts with files installed by gcc
brew cask uninstall oclint

brew install p7zip

if [ "$CXX" = "g++" ]; then
  brew install gcc@4.9
  export CXX="g++-4.9"
fi

# Test that the download version links are correct

git fetch --tags
git tag
egrep $(tools/latest_release --no_dots --prefix=version-) README.md
egrep $(tools/latest_release --no_dots --prefix=version-) docs/index.md

# Get the templight binary

cd 3rd/templight
  wget https://github.com/r0mai/templight_binary/releases/download/templight_d2be281/templight_osx10.13.2_x86_64.tar.bz2
  tar -xvjf templight_osx10.13.2_x86_64.tar.bz2
cd ../..

# Test the code

BUILD_THREADS=2 NO_TEMPLIGHT=1 METASHELL_NO_DOC_GENERATION=1 ./build.sh
