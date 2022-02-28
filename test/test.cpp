#include <sys/syslog.h>

int main() {
    openlog(">>>>>> a test program", LOG_PID | LOG_CONS | LOG_NDELAY, LOG_USER);
    setlogmask(LOG_UPTO(LOG_INFO));
    syslog(LOG_INFO, ">>>>>> this is a info test message");
    syslog(LOG_ERR, ">>>>>> this is a err test message");
    syslog(LOG_CRIT, ">>>>>> this is a crit test message");
    closelog();

    return 1;
}