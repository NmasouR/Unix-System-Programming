#!/bin/bash

virusesFile=$1
countriesFile=$2
numLines=$3
dublicatesAllowed=$4

if [ ! "$#" -eq 4 ]; then
	echo "Wrong args"
	exit 1
fi

if [ ! -f "$virusesFile" ]; then
	echo "Error: ${virusesFile} is not a file"
	exit 1
fi

if [ ! -f "$countriesFile" ]; then
	echo "Error: ${countriesFile} is not a file"
	exit 1
fi

if [ $3 -gt 10000 ] && [ $4 -eq 0 ] ; then
	echo "cannot create unique IDs"
	exit 1
fi

let index=0
countries=()
viruses=()

while read line ; do
	countries[$index]="$line"
	index=`expr ${index} + 1`
done < $2

index=0

while read line ; do
	viruses[$index]="$line"
	index=`expr ${index} + 1`
done < $1

let l=1

touch inputFile
rm inputFile
touch inputFile

while [ $l -le ${numLines} ] ; do
	id=$((1 + $RANDOM % 9999))
	if grep -q "$id" "inputFile" ; then #found
		citizen=($(grep ${id} -m1 "inputFile"))
		if [ "$dublicatesAllowed" -eq 1 ] ; then
			date=$(($RANDOM % 101))
			if [ "$date" -ge 50 ] ; then
				citizen[6]="YES"
				day=$((1 + $RANDOM % 30))
				if [ "$day" -lt 10 ] ; then #make in format dd
					day="0${day}"
				fi
				month=$((1 + $RANDOM % 12))
				if [ "$month" -lt 10 ] ; then
					month="0${month}"
				fi
				year=$((1950 +  $RANDOM % 71))
				date="${day}-${month}-${year}"
				citizen[7]=${date}
			else
				citizen[6]="NO"
				unset citizen[7]
			fi
			virus=$(($RANDOM % ${#viruses[@]}))
			citizen[5]=${viruses[virus]}
			for ((i=0; i<=${#citizen[@]}-1; i++))
			do
				echo -n "${citizen[i]}" >> inputFile
				echo -n " " >> inputFile
			done
			echo >> inputFile
			l=`expr $l + 1`
			unset citizen
			continue
		elif [ "$dublicatesAllowed" -eq 0 ] ; then
			unset citizen
			continue
		fi
	else
		citizen=()
		citizen[0]=${id}
		namelenght=$((3 +  $RANDOM % 10))
		surnamelenght=$((3 +  $RANDOM % 10))
		citizen[1]=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w "${namelenght}" | head -n 1)
		citizen[2]=$(cat /dev/urandom | tr -dc 'a-zA-Z' | fold -w "${surnamelenght}" | head -n 1)
		country=$(($RANDOM % ${#countries[@]}))
		citizen[3]=${countries[${country}]}
		age=$((1 +$RANDOM % 120))
		citizen[4]=${age}
		virus=$(($RANDOM % ${#viruses[@]}))
		citizen[5]=${viruses[${virus}]}
		date=$(($RANDOM % 101))
		if [ "${date}" -ge 50 ] ; then
			citizen[6]="YES"
			day=$((1 + $RANDOM % 30))
			if [ "$day" -lt 10 ] ; then
				day="0${day}"
			fi
			month=$((1 + $RANDOM % 12))
			if [ "$month" -lt 10 ] ; then
				month="0${month}"
			fi
			year=$((1950 +  $RANDOM % 71))
			date="${day}-${month}-${year}"
			citizen[7]=${date}
		else
			citizen[6]="NO"
			unset citizen[7]
		fi
		for ((i=0; i<=${#citizen[@]}-1; i++))
		do
			echo -n "${citizen[i]}" >> inputFile
			echo -n " " >> inputFile
		done
		echo >> inputFile
		l=`expr $l + 1`
		unset citizen
	fi 
done
			