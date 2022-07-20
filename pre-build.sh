#!/bin/bash
##################################################################
##                                                              ##
##      SPDX-License-Identifier: GPL-2.0-or-later               ##
##                                                              ##
##	Copyright (C) 2018-2022   Starfive Technology           ##  
##                                                              ##
##	Author:	jason zhou  <jason.zhou@starfivetech.com>       ##
##                                                              ##
##################################################################

COLOR_NORMAL="\033[0m"
COLOR_GREEN="\033[1;32m"
COLOR_YELLOW="\033[1;33m"
COLOR_RED="\033[1;31m"
COLOR_GREY="\033[1;30m"

printf ${COLOR_GREEN}
echo ""
echo "prepare for release build, make sure repo synced, working tree was clean"
echo "please checking if proper tagged for release!"
echo ""
printf ${COLOR_NORMAL}

git submodule sync --recursive
git pull
git submodule foreach "git pull"
git branch && git status
git submodule foreach "git branch && git status"

printf $COLOR_GREEN
git tag | grep JH7110_51
git submodule foreach "git tag | grep JH7110_51"
printf $COLOR_NORMAL
