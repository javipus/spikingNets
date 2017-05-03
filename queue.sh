#!/bin/bash
#PBS -N task
#PBS -q workq
#PBS -l nodes=1:ppn=24
#PBS -j oe
echo "Running on Node `hostname`"
date
mount
LD_LIBRARY_PATH=/home/lib/armadillo/usr/local/lib64:/usr/lib64:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH
/home/javier/spikingNets/bin/tmpxor.x
