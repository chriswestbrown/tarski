#!/bin/bash

#PBS -l select=1:ncpus=80:mpiprocs=80
#PBS -l place=scatter:excl
#PBS -A MHPCC96650N19
#PBS -q standard
#PBS -l walltime=012:00:00
#PBS -j oe
#PBS -N EvalTenThousand
#PBS -r y
#PBS -m be
#PBS -M m201362@usna.edu

cd ${WORKDIR}/eval
module purge
module load anaconda3/5.2.0
source activate mpi4py
module load gcc/5.3.0 openmpi/2.1.1/gnu/5.3.0 tensorflow/1.11.0
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/cuda-local-hdd/cuda-9.2/targets/ppc64le-linux/lib/"
export PATH=$PATH:$WORKDIR/tarski/bin
export PYTHONPATH="$PYTHONPATH:$WORKDIR/tarski"
mpirun  python mpievaltarski.py eval_tenthousand /gpfs/scratch/m201362/tarski/bin
