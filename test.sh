#!/usr/bin/zsh
OUT=ext/out
for file in ext/{,forlorenz/}*.xml; do
	echo "Testing with file $file at `date`"
	./test $file
	echo ""
done | tee $1
