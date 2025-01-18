#!/bin/bash

scons -Q -j 20
if [ $? -eq 0 ]; then
  (perf stat build/Release/renderer --maxticks 512) 2>&1 | grep instructions
fi