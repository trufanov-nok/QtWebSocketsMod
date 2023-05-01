#!/usr/bin/env bash

SCRIPT_PATH=$(dirname -- "$( readlink -f -- "$0"; )";)
cd "${SCRIPT_PATH}/.."

qmake
make -j4

cd "${SCRIPT_PATH}"

if [ ! -d "./build/" ]
then
     mkdir "./build/"
fi

cd "build"

cmake ..
make package

mv *.deb ../
