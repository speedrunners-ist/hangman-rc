#!/usr/bin/env bash

rm -f docs/proj_$1.zip
mkdir -p docs/proj_$1
cp -r client server lib README.md Makefile autodep docs/proj_$1
cd docs
zip -r proj_$1.zip proj_$1