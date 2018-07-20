#! /bin/bash

if [[ $# -lt 3 ]]; then
    echo "Usage: $0 <path_to_scripts> <BVGraph_basename> <output_basename>"
    exit 1
fi

goto=$PWD
cd $1
git pull;
rm stream_arcs_to_k2_tree.out
b++ stream_arcs_to_k2_tree.cpp
compile=$?
cd $goto
if [[compile -eq 0]]; then
    java it.unimi.dsi.webgraph.ArcListASCIIGraph $2 >($1/stream_arcs_to_k2_tree.out $3 $2);
    time $1/int_vectors_to_k2tree.out $3 --no-output
fi
