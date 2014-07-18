target=wztelnetd

all: $(target)

$(target): Main.cpp ClientThread.cpp Properties.cpp \
TerminalServer.cpp ThreadCreator.cpp Runnable.cpp
	g++ $^ -lpthread -o $@

clean:
	rm -f $(target)

install:
	cp stelnetd.conf /etc/
	cp wztelnetd.cfg /etc/
	cp wztelnetd.sh  /etc/init.d/wztelnetd
	cp wztelnetd     /sbin/
	chmod +x /etc/init.d/wztelnetd
	chmod +x /sbin/wztelnetd
	/sbin/chkconfig --add wztelnetd
	/sbin/chkconfig --list | grep wztelnetd

uninstall:
	rm -f /etc/stelnetd.conf
	rm -f /etc/wztelnetd.cfg
	rm -f /etc/init.d/wztelnetd
	rm -f /sbin/wztelnetd
	rm -f /var/log/wztelnetd.log
	rm -f /var/run/wztelnetd.pid
	
start:
	service wztelnetd start

stop:
	service wztelnetd stop

.PHONY: all clean install uninstall
