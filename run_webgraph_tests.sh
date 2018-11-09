#! /bin/bash

graphs=( "enwiki-2015" "enwiki-2016" "enwiki-2017" "enwiki-2018" "eu-2015-host" "facebook" "gsh-2015-host" "twitter-2010" "uk-2014-host" "eu-2015" "clueweb12" "uk-2014")
algorithms=( "BVGraph" "EFGraph")
WD=$(pwd)

for graph in "${graphs[@]}"
do
    mkdir $graph
    cd $graph
    rm -rf *
    for algo in "${algorithms[@]}"
    do
        java com.gioaudino.thesis.WebGraphTester $algo $DATASETS/$graph/$graph-hc $graph
    done
    cd $WD
done
