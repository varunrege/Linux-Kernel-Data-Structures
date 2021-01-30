#!/usr/bin/bash
sudo insmod pr2.ko int_str="1,2,3,4,5"
cat /proc/proj2
sudo rmmod pr2
