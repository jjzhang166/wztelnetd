target=wztelnetd

all: $(target)

$(target): Main.cpp ClientThread.cpp Properties.cpp \
TerminalServer.cpp ThreadCreator.cpp Runnable.cpp Util.cpp
	g++ $^ -lpthread -o $@

clean:
	rm -f $(target)

install:
	cp $(target).cfg /etc/
	cp $(target).sh  /etc/init.d/$(target)
	cp $(target)     /sbin/
	chmod +x /etc/init.d/$(target)
	chmod +x /sbin/$(target)
	/sbin/chkconfig --add $(target)
	/sbin/chkconfig --list | grep $(target)

uninstall:
	rm -f /etc/$(target).cfg
	rm -f /etc/init.d/$(target)
	rm -f /sbin/$(target)
	rm -f /var/log/$(target).log
	rm -f /var/run/$(target).pid

#	ngcbs yjdlzyyd
start:
	service $(target) start

stop:
	service $(target) stop

enable:
	chkconfig $(target) on
	
disable:
	chkconfig $(target) off


rebuild: stop uninstall $(target) install start

.PHONY: all clean install uninstall
