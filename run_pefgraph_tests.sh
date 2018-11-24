#! /bin/bash

graphs=( "enwiki-2015" "enwiki-2016" "enwiki-2017" "enwiki-2018" "eu-2015-host" "facebook" "gsh-2015-host" "twitter-2010" "uk-2014-host" "uk-2014")
WD=$(pwd)

for graph in "${graphs[@]}"
do
    mkdir $graph
    cd $graph
    rm -rf *
    java -Xmx200G com.gioaudino.thesis.PEFGraphTester $DATASETS/$graph/$graph-hc $graph
    cd $WD
done
