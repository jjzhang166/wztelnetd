target=wztelnetd

all: $(target)

$(target): Main.cpp ClientThread.cpp Properties.cpp \
TerminalServer.cpp ThreadCreator.cpp Runnable.cpp Util.cpp
	g++ $^ -lpthread -o $@

clean:
	rm -f $(target)

install: $(target)
	if [ -f /etc/$(target).cfg.bak ]; then mv /etc/$(target).cfg.bak  /etc/$(target).cfg ; else cp $(target).cfg /etc/ ; fi
	cp $(target).sh  /etc/init.d/$(target)
	cp $(target)     /sbin/
	chmod +x /etc/init.d/$(target)
	chmod +x /sbin/$(target)
	/sbin/chkconfig --add $(target)
	/sbin/chkconfig --list | grep $(target)

uninstall: stop
	if [ -e /etc/$(target).cfg ]; then mv /etc/$(target).cfg /etc/$(target).cfg.bak; fi
	rm -f /etc/init.d/$(target)
	rm -f /sbin/$(target)
	rm -f /var/log/$(target).log
	rm -f /var/run/$(target).pid

start:
	service $(target) start

stop:
	-killall wztelnetd

enable:
	chkconfig $(target) on
	
disable:
	chkconfig $(target) off

reinstall: uninstall clean $(target) install start

.PHONY: all clean install uninstall
