#!/usr/bin/env bash

# improvised bash script that takes vector, as pritned by print_vec, in stdin
# and prints a bar chart of it in stdout
# because matplotlib is for losers

# usage: ./build/main | ./bar.bash
# please note: only workks with the current iteration of main

print_bar () {
	local n
	local stripn

	while read n; do
		stripn="${n%%.*}"
		printf "%03d :" "$stripn"
		for ((i = 0; i<stripn; ++i)); do
			echo -n '#'
		done
	echo ""
	done
}

# ignore first two lines, they are status whatevers printed by ./build/main
read
read

# `xargs echo' is a very... suboptimal way to remove excess whitespace
# and does not make it viable to use this script in a streaming fashion
# but it works for this usecase so fuck it, why not
tr '[' ' ' | tr ']' ' ' | tr ',' ' ' | xargs echo | tr ' ' '\n' | print_bar
