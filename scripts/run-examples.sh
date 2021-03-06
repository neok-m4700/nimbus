#!/usr/bin/env bash

#
#  Copyright 2013 Stanford University.
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
# 
#  - Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 
#  - Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
# 
#  - Neither the name of the copyright holders nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
#  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
#  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
#  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
#  OF THE POSSIBILITY OF SUCH DAMAGE.
#

# Starts Nimbus controller on the machine this script is executed on.

# Author: Omid Mashayekhi <omidm@stanford.edu>

# **************************
# Text Reset
RCol='\x1B[0m'    
# Regular           
Bla='\x1B[0;90m';
Red='\x1B[0;91m';
Gre='\x1B[0;92m';
Yel='\x1B[0;93m';
Blu='\x1B[0;94m';
Pur='\x1B[0;95m';
Cya='\x1B[0;96m';
Whi='\x1B[0;97m';
# **************************

function print_usage {
  echo -e "${Cya}Usage:"
  echo -e "./scripts/run-examples.sh"
  echo -e "                    <example-name>"
  echo -e "                    [example options]"
  echo -e ">> example-name is one of the following: [stencil_1d, job_spawner, lr, k-means, page-rank]."
  echo -e ">> to get available options for each example pass -h."
  echo -e "${RCol}"
}


if [ -z "${NIMBUS_HOME}" ]; then
  export NIMBUS_HOME="$(cd "`dirname "$0"`"/..; pwd)"
fi

if [ -z "${DBG}" ]; then
  export DBG="error"
fi

if [ -z "${TTIMER}" ]; then
  export TTIMER="l1"
fi

if [[ "$#" == "0" ]] || [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
  print_usage
  exit 1
fi

EXAMPLE="$1"
shift
ARGS="$@"


if [[ ${ARGS} != *--help* ]] && [[ ${ARGS} != *-h* ]]; then
  ${NIMBUS_HOME}/scripts/stop-workers.sh > /dev/null
  ${NIMBUS_HOME}/scripts/stop-controller.sh > /dev/null
  ${NIMBUS_HOME}/scripts/start-controller.sh
fi

if [[ ${EXAMPLE} == "stencil_1d" ]]; then
  ${NIMBUS_HOME}/scripts/start-workers.sh 1 --fg -l applications/simple/stencil_1d/libstencil_1d.so ${ARGS}
elif [[ ${EXAMPLE} == "job_spawner" ]]; then
  ${NIMBUS_HOME}/scripts/start-workers.sh 1 --fg -l applications/simple/job_spawner/libjob_spawner.so ${ARGS}
elif [[ ${EXAMPLE} == "lr" ]]; then
  ${NIMBUS_HOME}/scripts/start-workers.sh 1 --fg -l applications/ml/logistic_regression/liblr.so ${ARGS}
elif [[ ${EXAMPLE} == "k-means" ]]; then
  ${NIMBUS_HOME}/scripts/start-workers.sh 1 --fg -l applications/ml/k_means/libk_means.so ${ARGS}
elif [[ ${EXAMPLE} == "page-rank" ]]; then
  ${NIMBUS_HOME}/scripts/start-workers.sh 1 --fg -l applications/graph/page_rank/libpagerank.so ${ARGS}
else
  echo -e "${Red}ERROR: unknown application name!${RCol}"
  ${NIMBUS_HOME}/scripts/stop-controller.sh
  print_usage
  exit 1
fi
