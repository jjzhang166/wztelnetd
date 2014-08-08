#!/bin/bash

F=`cat current_encoding`

if [ "$F" = "GBK" ]; then  
    T=UTF-8
else
	T=GBK
fi

function encode() {
	echo $F -\> $T - $1
	iconv -f $F -t $T $1 > tmp
	cat tmp > $1
	rm tmp
}

encode Messages.h
encode wztelnetd.sh
encode wztelnetd.cfg

echo $T > current_encoding
