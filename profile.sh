#!/bin/bash

cd build

gprof FMCL gmon.out > profile.txt
