T=UTF-8
F=GBK
iconv -f $F -t $T Messages.h > tmp ; cat tmp > Messages.h
iconv -f $F -t $T wztelnetd.sh > tmp ; cat tmp > wztelnetd.sh
iconv -f $F -t $T wztelnetd.cfg > tmp ; cat tmp > wztelnetd.cfg
rm tmp
