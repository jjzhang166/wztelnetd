#!/bin/sh -e
# chkconfig:345 89 17
# description: wztelnetd

PIDFILE=/var/run/wztelnetd.pid
LOGFILE=/var/log/wztelnetd.log

TELNETD_PID=
[ -r "$PIDFILE" ]  && {
    [ -f "$PIDFILE" ] || {
        echo "'$PIDFILE' is not a regular file" 1>&2
        exit 6
    }
    [ -w "$PIDFILE" ] || {
        echo "'$PIDFILE' is not writable" 1>&2
        exit 6
    }
    TELNETD_PID="`cat $PIDFILE`" || {
        echo "Failed to read pid file '$PIDFILE'" 1>&2
        exit 6
    }
    case "$TELNETD_PID" in
        *[a-zA-Z/!@#$%*+=_~]*) TELNETD_PID=;;
        *'^'*) TELNETD_PID=;;
    esac
    [ -n "$TELNETD_PID" ] || {
        echo "Pid file '$PIDFILE' does not contain a valid process identifier" 1>&2
        exit 6
    }
    kill -0 "$TELNETD_PID" > /dev/null 2>&1 || {
        echo 'Removing stale pid file'
        rm -f "$PIDFILE" || {
            echo "Failed to remove pid file '$PIDFILE'" 1>&2
            exit 6
        }
        TELNETD_PID=
    }
}

#echo "PID is ($TELNETD_PID)"

COMMAND="$1"; shift

case "$COMMAND" in
    start)
        [ -n "$TELNETD_PID" ] && {
        	echo "�󶨷������Ѿ������� $TELNETD_PID." 1>&2
			exit 1
        }
        touch "$PIDFILE" || {
			echo "�������̺��ļ�ʧ�ܣ�'$PIDFILE'" 1>&2
			exit 1
        }
		nohup sh -c "echo "'$$'" > '$PIDFILE' && exec /sbin/wztelnetd" >> "$LOGFILE" 2>&1 &
		echo "�󶨷���ɹ�������"
		exit 0
	;;
	stop)
        [ -n "$TELNETD_PID" ] || {
            echo "û�м�⵽�������еİ󶨷���." 1>&2
            exit 0
        }
        kill "$TELNETD_PID" > /dev/null 2>&1 && {
            echo "���棺�󶨷����������У�" 1>&2
            exit 0
        }
        rm -f "$PIDFILE" || {
            echo "�޷�ɾ�����̺��ļ��� '$PIDFILE'" 1>&2
            exit 1
        }
        echo "����ɹ�ֹͣ��"
        exit 0
    ;;
esac

