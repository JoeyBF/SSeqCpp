#!/usr/bin/env bash

degree=$1
diagram=$2

# Make sure to run in benches directory!

# First, compute Adams data
scripts/res.sh $degree
scripts/prod.sh $degree
scripts/map_res.sh $degree
scripts/d2.sh $degree
scripts/export.sh $degree
scripts/export_d2.sh $degree
scripts/export_map.sh $degree

mv *.db $diagram

# Then, initialize the spectral sequence and deduce
yes | ./ss reset_ss $diagram
scripts/deduce.sh $degree $diagram

# Finally, output the results
if [ ! -d webpages/mix ]; then
    mkdir webpages/mix
fi

./ss plot_ss $diagram
