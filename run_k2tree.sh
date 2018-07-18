#! /bin/bash

if [[ $# -lt 3 ]]; then
    echo "Usage: $0 <path_to_scripts> <BVGraph_basename> <output_basename>"
    exit 1
fi

goto=$PWD
cd $1
b++ stream_arcs_to_int_vectors.cpp & b++ int_vectors_to_k2tree.cpp
cd $goto

java it.unimi.dsi.webgraph.ArcListASCIIGraph $2 >($1/stream_arcs_to_int_vectors.out $3 $2 --no-output);

time $1/int_vectors_to_k2tree.out $3 --no-output
