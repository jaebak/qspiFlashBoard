#!/bin/sh
echo 'Kill the below running programms'
ps aux | grep continuousReadAndValidate | grep -v grep
ps aux | grep readAndValidate | grep -v grep
ps aux | grep checkProm | grep -v grep
ps aux | grep checkUsb | grep -v grep
ps aux | grep eraseProm | grep -v grep
ps aux | grep readAndValidate | grep -v grep
ps aux | grep writeProm | grep -v grep
