#!/bin/bash


PARENT_FILE_PATH=$(cd "$(dirname "$0")/.."; pwd)
FILE_PATH=$(cd "$(dirname "$0")"; pwd)


sudo rmmod Mantis

