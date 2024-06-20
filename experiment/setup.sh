#!/bin/bash

# Configure sandbox
cp ~/sandbox.tgz /tmp
cd /tmp
tar xfv sandbox.tgz

# Disable ASLR - https://askubuntu.com/a/318476
echo 0 | sudo tee /proc/sys/kernel/randomize_va_space

# Increase open file limit
ulimit -n 50000
