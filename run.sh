#!/bin/sh

spack load cmake ucx+cuda
spack find --loaded

rm -rf $HOME/ucx_cuda_demo_build

cmake -S $(dirname $0) \
    -D CMAKE_CUDA_ARCHITECTURES=80 \
    -B $HOME/ucx_cuda_demo_build

cmake --build $HOME/ucx_cuda_demo_build -j
cmake --build $HOME/ucx_cuda_demo_build -t test
cat $HOME/ucx_cuda_demo_build/Testing/Temporary/LastTest.log
