#! /bin/bash

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <path_to_scripts>"
    exit 1
fi

graphs=( "enwiki-2015" "enwiki-2016" "enwiki-2017" "enwiki-2018" "eu-2015-host" "facebook" "gsh-2015-host" "twitter-2010" "uk-2014-host")
WD=$(pwd)

for graph in "${graphs[@]}"
do
    mkdir $graph
    cd $graph
    rm -rf *
    $1/run_k2tree.sh $1 $DATASETS/$graph/$graph-hc $graph
    cd $WD
done
