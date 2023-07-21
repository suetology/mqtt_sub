#include "daemon.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>	
#include <stdlib.h>

int make_daemon(int flags)
{
        switch(fork()) {
		case -1: 
			return -1;
		case 0: 
			break;                 
		default:
			closelog(); 
			_exit(EXIT_SUCCESS);   
        }

        if(setsid() == -1)                
                return -1;

        switch(fork()) {
		case -1: 
			return -2;
		case 0: 
			break;                  
		default:
			closelog(); 
			_exit(EXIT_SUCCESS);   
        }

	int fd, maxfd;

        if(!(flags & NO_UMASK0))
    		umask(0);                      

 	if(!(flags & NO_CHDIR))
    		chdir("/");                     

	if(!(flags & NO_CLOSE_FILES)) {
		maxfd = sysconf(_SC_OPEN_MAX);

		if(maxfd == -1)
			maxfd = MAX_CLOSE;  

		for(fd = 0; fd < maxfd; fd++)
			close(fd);
	}

	if(!(flags & NO_REOPEN_STD_FDS)) {
		close(STDIN_FILENO);

		fd = open("/dev/null", O_RDWR);
		if(fd != STDIN_FILENO)
			return -3;
		if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			return -4;
		if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -5;
	}
        return 0;
}