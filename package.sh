#!/usr/bin/env bash

rm -f docs/proj$1.zip
mkdir -p docs/proj$1
cp -r client server lib README.md Makefile autodep .clang-format docs/proj$1
cd docs
rm -rf proj$1/server/assets/hints
zip -r proj$1.zip proj$1
