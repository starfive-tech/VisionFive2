#!/bin/sh
app=$1
tftp -p  -l  $app -b 2800 192.168.120.145
