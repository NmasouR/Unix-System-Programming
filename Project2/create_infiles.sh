#!/bin/bash

inputFile=$1


if [ ! "$#" -eq 3 ]; then
	echo "Wrong args"
	exit 1
fi

if [ ! -f "$inputFile" ]; then
	echo "Error: ${virusesFile} is not a file"
	exit 1
fi

if [ -d $2 ]; then
    echo "Error dir $2 already exist"
    exit 1
fi

mkdir $2

countries=()
roundrobin=()

index=0

while read line ; do
	country=$(echo ${line} | cut -d " " -f 4)
	if [[ ! " ${countries[@]} " =~ " ${country} " ]]; then #if new country found
		countries[$index]=${country}
		index=`expr ${index} + 1`
	fi
done < $1

index=0

#create subdirs
for each in "${countries[@]}"
do
	mkdir ./$2/${each}
done

#create files
for each in "${countries[@]}"
do
	for (( i=1; i<=$3; i++ ))
	do
		filename="${each}-$i.txt"
		touch ./$2/${each}/${filename}
	done
done

for each in "${countries[@]}"
do
	roundrobin[${index}]=1
	index=`expr ${index} + 1`
done

#break input file to multiple files
while read line ; do
	country=$(echo ${line} | cut -d " " -f 4)
	for i in "${!countries[@]}"; do #$i index ${countries[$i]} value
		if [ "$country" = "${countries[$i]}" ]; then
			filename="${countries[$i]}-${roundrobin[$i]}.txt"
    		echo -n "${line}" >> ./$2/${countries[$i]}/${filename}
			echo >> ./$2/${countries[$i]}/${filename}
			roundrobin[$i]=`expr ${roundrobin[$i]} + 1`
			if [ ${roundrobin[$i]} -gt $3 ]
    		then
				roundrobin[$i]=1
			fi
		fi
	done
done < $1
