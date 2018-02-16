#!/bin/bash

set -ex

./install_build_dependencies.sh

[ "${COVERAGE}" = "true" ] && sudo pip install cpp-coveralls==0.3.12

UBUNTU_VERSION="$(tools/detect_platform.sh --version)"

# Test that the download version links are correct

git fetch --tags
git tag
egrep $(tools/latest_release --no_dots --prefix=version-) README.md
egrep $(tools/latest_release --no_dots --prefix=version-) docs/index.md

# Do static validations

tools/validate/all_static.sh

# Build the docs

mkdocs build

# Get the templight binary

cd 3rd/templight
  wget http://sinkovics.hu/templight/templight_ubuntu14.04_x86_64.tar.bz2
  tar -xvjf templight_ubuntu14.04_x86_64.tar.bz2
cd ../..

# Test the code

export CXXFLAGS="-Werror"

# If we check for coverage, than add an extra compiler flag
[ "${COVERAGE}" = "true" ] && export CXXFLAGS="${CXXFLAGS} --coverage"
[ "${COVERAGE}" = "true" ] && export BUILD_TYPE="Debug"

BUILD_THREADS=2 NO_TEMPLIGHT=1 METASHELL_NO_DOC_GENERATION=1 ./build.sh

# Collect and upload coverage data
[ "${COVERAGE}" = "true" ] && coveralls \
  -b "bin" \
  --exclude "3rd" \
  --exclude "test" \
  --exclude "bin/3rd" \
  --exclude "bin/test" \
  --exclude "bin/app/include/metashell/" \
  --exclude "bin/_CPack_Packages" \
  --exclude "windows_headers" \
  --gcov gcov-4.8 --gcov-options '\-lp'

cd bin
  ../tools/clang_tidy.sh | tee clang_tidy_output.txt
  [ ! -s clang_tidy_output.txt ]
cd ..

# Test the demo server's code

tools/demo_server/test/test_git_clone
tools/demo_server/test/test_deploy

SRC_ROOT=. \
  CONFIG=tools/demo_server/config/metashell.json \
  tools/demo_server/test/test_config
