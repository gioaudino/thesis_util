#! /bin/bash

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <path_to_scripts>"
    exit 1
fi

graphs=("enwiki-2015")
WD=$(pwd)

for graph in "${graphs[@]}":
do
    mkdir $graph
    cd $graph
    $1/run_k2tree $1 $DATASETS/$graph/$graph-hc $graph
    cd $WD
done
