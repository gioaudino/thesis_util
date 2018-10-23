#! /bin/bash

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <path_to_scripts>"
    exit 1
fi

graphs=("enwiki-2016" "enwiki-2017")
WD=$(pwd)

for graph in "${graphs[@]}"
do
    mkdir $graph
    cd $graph
    rm -rf *
    $1/run_k2tree.sh $1 $DATASETS/$graph/$graph-hc $graph
    cd $WD
done
