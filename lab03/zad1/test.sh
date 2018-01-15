#!/bin/bash

SIZES="4 512 4096 8192"
COUNT="500 1000"

FN=data.dat

for X in $SIZES; do
	echo ""
	echo ""
	for Y in $COUNT; do
		echo ""
		echo "    struct size = $X, number of structs = $Y"
		./lprog "generate" $X $Y $FN
		echo "    Library functions:"
		./lprog "sort" $FN
		echo "    System functions:"
		./sprog "sort" copy_$FN
	done
done
echo "    Temporary files deleting..."
rm -f *.dat copy_$FN

