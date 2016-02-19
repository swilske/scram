#!/usr/bin/env bash

set -ev

sudo apt-get install -qq libboost-program-options-dev
sudo apt-get install -qq libboost-filesystem-dev
sudo apt-get install -qq libboost-system-dev
sudo apt-get install -qq libboost-math-dev
sudo apt-get install -qq libxml2-dev
sudo apt-get install -qq libxml++2.6-dev
sudo apt-get install -qq libgoogle-perftools-dev
sudo apt-get install -qq qt5-default
sudo apt-get install -qq python-lxml
sudo apt-get install -qq graphviz
sudo pip install nose

[[ -z "${RELEASE}" && "$CXX" = "g++" ]] || exit 0
sudo apt-get install -qq valgrind
sudo apt-get install -qq doxygen
sudo pip install cpp-coveralls
sudo pip install lizard
sudo pip install codecov
sudo pip install coverage
sudo pip install cpplint
