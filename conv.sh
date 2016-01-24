#!/bin/bash

F=`cat current_encoding`
if [[ `tr '[A-Z]' '[a-z]'<<<"$LANG"` == *utf* ]]; then 
	T=UTF-8
else
	T=GBK
fi
if [ "$1" != "" ]; then
	T=$1
fi
function encode() {
	echo $F -\> $T - $1
	iconv -f $F -t $T $1 > tmp
	cat tmp > $1
	rm tmp
}

function enc() {
    until [ -z "$1" ]
    do
	encode $1    
	shift
    done
}

enc wztelnetd.sh wztelnetd.cfg README *.h *.cpp

echo $T > current_encoding
