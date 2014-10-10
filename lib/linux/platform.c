/***************************************************************************
 *   Copyright (C) 2012 by Process Control Engineers                       *
 *   Author Kyle Hayes  kylehayes@processcontrolengineers.com              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

 /**************************************************************************
  * CHANGE LOG                                                             *
  *                                                                        *
  * 2012-11-27  KRH - Created file from old platform dependent code.       *
  *                   Genericized thread creation.                         *
  *                                                                        *
  **************************************************************************/


#include <platform.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>

#include "libplctag.h"



/***************************************************************************
 ******************************* Memory ************************************
 **************************************************************************/



/*
 * mem_alloc
 *
 * This is a wrapper around the platform's memory allocation routine.
 * It will zero out memory before returning it.
 *
 * It will return NULL on failure.
 */
extern void *mem_alloc(int size)
{
	void *res = malloc(size);

	if(res) {
		memset(res, 0, size);
	}

	return res;
}



/*
 * mem_free
 *
 * Free the allocated memory passed in.  If the passed pointer is
 * null, do nothing.
 */
extern void mem_free(const void *mem)
{
	if(mem) {
		free((void *)mem);
	}
}




/*
 * mem_set
 *
 * set memory to the passed argument.
 */
extern void mem_set(void *d1, int c, int size)
{
	memset(d1, c, size);
}





/*
 * mem_copy
 *
 * copy memory from one pointer to another for the passed number of bytes.
 */
extern void mem_copy(void *d1, void *d2, int size)
{
	memcpy(d1, d2, size);
}




/***************************************************************************
 ******************************* Strings ***********************************
 **************************************************************************/


/*
 * str_cmp
 *
 * Return -1, 0, or 1 depending on whether the first string is "less" than the
 * second, the same as the second, or "greater" than the second.  This routine
 * just passes through to POSIX strcmp.
 */
extern int str_cmp(const char *first, const char *second)
{
	return strcmp(first, second);
}




/*
 * str_cmp_i
 *
 * Returns -1, 0, or 1 depending on whether the first string is "less" than the
 * second, the same as the second, or "greater" than the second.  The comparison
 * is done case insensitive.
 *
 * It just passes this through to POSIX strcasecmp.
 */
extern int str_cmp_i(const char *first, const char *second)
{
	return strcasecmp(first,second);
}



/*
 * str_copy
 *
 * Returns
 */
extern int str_copy(char *dst, const char *src, int size)
{
	strncpy(dst,src,size);
	return 0;
}


/*
 * str_length
 *
 * Return the length of the string.  If a null pointer is passed, return
 * null.
 */
extern int str_length(const char *str)
{
	if(!str) {
		return 0;
	}

	return strlen(str);
}




/*
 * str_dup
 *
 * Copy the passed string and return a pointer to the copy.
 * The caller is responsible for freeing the memory.
 */
extern char *str_dup(const char *str)
{
	if(!str) {
		return NULL;
	}

	return strdup(str);
}



/*
 * str_to_int
 *
 * Convert the characters in the passed string into
 * an int.  Return an int in integer in the passed
 * pointer and a status from the function.
 */
extern int str_to_int(const char *str, int *val)
{
	char *endptr;
	long int tmp_val;

	tmp_val = strtol(str,&endptr,0);

	if (errno == ERANGE && (tmp_val == LONG_MAX || tmp_val == LONG_MIN)) {
		/*pdebug("strtol returned %ld with errno %d",tmp_val, errno);*/
		return -1;
	}

	if (endptr == str) {
		return -1;
	}

	/* FIXME - this will truncate long values. */
	*val = (int)tmp_val;

	return 0;
}


extern int str_to_float(const char *str, float *val)
{
	char *endptr;
	float tmp_val;

	tmp_val = strtof(str,&endptr);

	if (errno == ERANGE && (tmp_val == HUGE_VALF || tmp_val == -HUGE_VALF || tmp_val == 0)) {
		return -1;
	}

	if (endptr == str) {
		return -1;
	}

	/* FIXME - this will truncate long values. */
	*val = tmp_val;

	return 0;
}


extern char **str_split(const char *str, const char *sep)
{
	int sub_str_count=0;
	int size = 0;
	const char *sub;
	const char *tmp;
	char **res;

	/* first, count the sub strings */
	tmp = str;
	sub = strstr(tmp,sep);

	while(sub && *sub) {
		/* separator could be at the front, ignore that. */
		if(sub != tmp) {
			sub_str_count++;
		}

		tmp = sub + str_length(sep);
		sub = strstr(tmp,sep);
	}

	if(tmp && *tmp && (!sub || !*sub))
		sub_str_count++;

	/* calculate total size for string plus pointers */
	size = sizeof(char *)*(sub_str_count+1)+str_length(str)+1;

	/* allocate enough memory */
	res = mem_alloc(size);
	if(!res)
		return NULL;

	/* calculate the beginning of the string */
	tmp = (char *)res + sizeof(char *)*(sub_str_count+1);

	/* copy the string */
	str_copy((char *)tmp,str,strlen(str));

	/* set up the pointers */
	sub_str_count=0;
	sub = strstr(tmp,sep);
	while(sub && *sub) {
		/* separator could be at the front, ignore that. */
		if(sub != tmp) {
			/* store the pointer */
			res[sub_str_count] = (char *)tmp;

			sub_str_count++;
		}

		/* zero out the separator chars */
		mem_set((char*)sub,0,str_length(sep));

		/* point past the separator (now zero) */
		tmp = sub + str_length(sep);

		/* find the next separator */
		sub = strstr(tmp,sep);
	}

	/* if there is a chunk at the end, store it. */
	if(tmp && *tmp && (!sub || !*sub)) {
		res[sub_str_count] = (char*)tmp;
	}

	return res;
}






/***************************************************************************
 ******************************* Mutexes ***********************************
 **************************************************************************/

struct mutex_t {
	pthread_mutex_t p_mutex;
	int initialized;
};

int mutex_create(mutex_p *m)
{
	/*pdebug("Starting.");*/

	*m = (struct mutex_t *)mem_alloc(sizeof(struct mutex_t));
	if(! *m) {
		/*pdebug("null mutex pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	if(pthread_mutex_init(&((*m)->p_mutex),NULL)) {
		mem_free(*m);
		*m = NULL;
		/*pdebug("Error initializing mutex.");*/
		return PLCTAG_ERR_MUTEX_INIT;
	}

    (*m)->initialized = 1;

	/*pdebug("Done.");*/

	return PLCTAG_STATUS_OK;
}


int mutex_lock(mutex_p m)
{
	//pdebug("Starting");

	if(!m) {
		/*pdebug("null mutex pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

    if(!m->initialized) {
        return PLCTAG_ERR_MUTEX_INIT;
    }

	if(pthread_mutex_lock(&(m->p_mutex))) {
		/*pdebug("error locking mutex.");*/
		return PLCTAG_ERR_MUTEX_LOCK;
	}

	//pdebug("Done.");

	return PLCTAG_STATUS_OK;
}



int mutex_unlock(mutex_p m)
{
	//pdebug("Starting.");

	if(!m) {
		/*pdebug("null mutex pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

    if(!m->initialized) {
        return PLCTAG_ERR_MUTEX_INIT;
    }

	if(pthread_mutex_unlock(&(m->p_mutex))) {
		/*pdebug("error unlocking mutex.");*/
		return PLCTAG_ERR_MUTEX_UNLOCK;
	}

	//pdebug("Done.");

	return PLCTAG_STATUS_OK;
}


int mutex_destroy(mutex_p *m)
{
	/*pdebug("Starting.");*/

	if(!m) {
		/*pdebug("null mutex pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	if(pthread_mutex_destroy(&((*m)->p_mutex))) {
		/*pdebug("error while attempting to destroy mutex.");*/
		return PLCTAG_ERR_MUTEX_DESTROY;
	}

	mem_free(*m);

	*m = NULL;

	/*pdebug("Done.");*/

	return PLCTAG_STATUS_OK;
}







/***************************************************************************
 ******************************* Threads ***********************************
 **************************************************************************/

struct thread_t {
	pthread_t p_thread;
	int initialized;
};

/*
 * thread_create()
 *
 * Start up a new thread.  This allocates the thread_t structure and starts
 * the passed function.  The arg argument is passed to the function.
 *
 * FIXME - use the stacksize!
 */

extern int thread_create(thread_p *t, thread_func_t func, int stacksize, void *arg)
{
	/*pdebug("Starting.");*/

	if(!t) {
		/*pdebug("null thread pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	*t = (thread_p)mem_alloc(sizeof(struct thread_t));

	if(! *t) {
		/*pdebug("null thread pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	/* create a pthread.  0 means success. */
    if(pthread_create(&((*t)->p_thread), NULL, func, arg)) {
		/*pdebug("error creating thread.");*/
        return PLCTAG_ERR_THREAD_CREATE;
    }

	/* mark as initialized */
	(*t)->initialized = 1;

	/*pdebug("Done.");*/

	return PLCTAG_STATUS_OK;
}


/*
 * platform_thread_stop()
 *
 * Stop the current thread.  Does not take arguments.  Note: the thread
 * ends completely and this function does not return!
 */
void thread_stop(void)
{
	pthread_exit((void*)0);
}


/*
 * thread_join()
 *
 * Wait for the argument thread to stop and then continue.
 */

int thread_join(thread_p t)
{
	void *unused;

	/*pdebug("Starting.");*/

	if(!t) {
		/*pdebug("null thread pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	if(pthread_join(t->p_thread,&unused)) {
		/*pdebug("Error joining thread.");*/
		return PLCTAG_ERR_THREAD_JOIN;
	}

	/*pdebug("Done.");*/

	return PLCTAG_STATUS_OK;
}

/*
 * thread_destroy
 *
 * This gets rid of the resources of a thread struct.  The thread in
 * question must be dead first!
 */
extern int thread_destroy(thread_p *t)
{
	/*pdebug("Starting.");*/

	if(!t || ! *t) {
		/*pdebug("null thread pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	mem_free(*t);

	*t = NULL;

	return PLCTAG_STATUS_OK;
}






/***************************************************************************
 ******************************* Atomic Ops ********************************
 **************************************************************************/

/*
 * lock_acquire
 *
 * Tries to write a non-zero value into the lock atomically.
 *
 * Returns non-zero on success.
 *
 * Warning: do not pass null pointers!
 */

#define ATOMIC_LOCK_VAL (1)

extern int lock_acquire(lock_t *lock)
{
	int rc = __sync_lock_test_and_set((int*)lock, ATOMIC_LOCK_VAL);

	if(rc != ATOMIC_LOCK_VAL) {
		/* we got the lock */
		/*pdebug("got lock");*/
		return 1;
	} else {
		/* we did not get the lock */
		/*pdebug("did not get lock");*/
		return 0;
	}
}


extern void lock_release(lock_t *lock)
{
	__sync_lock_release((int*)lock);
	/*pdebug("released lock");*/
}


/***************************************************************************
 ******************************* Sockets ***********************************
 **************************************************************************/

struct sock_t {
	int fd;
	int port;
	int is_open;
};


#define MAX_IPS (8)

extern int socket_create(sock_p *s)
{
	/*pdebug("Starting.");*/

	if(!s) {
		/*pdebug("null socket pointer.");*/
		return PLCTAG_ERR_NULL_PTR;
	}

	*s = (sock_p)mem_alloc(sizeof(struct sock_t));

	if(! *s) {
		/*pdebug("memory allocation failure.");*/
		return PLCTAG_ERR_NO_MEM;
	}

	return PLCTAG_STATUS_OK;
}


extern int socket_connect_tcp(sock_p s, const char *host, int port)
{
	in_addr_t ips[MAX_IPS];
	int num_ips = 0;
	struct sockaddr_in gw_addr;
    int sock_opt = 1;
    int i = 0;
    int done = 0;
	int fd;
    int flags;
    struct timeval timeout; /* used for timing out connections etc. */

	/*pdebug("Starting.");*/

    /* Open a socket for communication with the gateway. */
    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    /* check for errors */
    if(fd < 0) {
        /*pdebug("Socket creation failed, errno: %d",errno);*/
        return PLCTAG_ERR_OPEN;
    }

    /* set up our socket to allow reuse if we crash suddenly. */
    sock_opt = 1;

    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&sock_opt,sizeof(sock_opt))) {
		close(fd);
        /*pdebug("Error setting socket reuse option, errno: %d",errno);*/
        return PLCTAG_ERR_OPEN;
    }

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout))) {
		close(fd);
        /*pdebug("Error setting socket receive timeout option, errno: %d",errno);*/
        return PLCTAG_ERR_OPEN;
    }

    if(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout))) {
		close(fd);
        /*pdebug("Error setting socket set timeout option, errno: %d",errno);*/
        return PLCTAG_ERR_OPEN;
    }

    /* figure out what address we are connecting to. */

    /* try a numeric IP address conversion first. */
    if(inet_pton(AF_INET,host,(struct in_addr *)ips) > 0) {
        /*pdebug("Found numeric IP address: %s",host);*/
        num_ips = 1;
    } else {
        struct hostent *h=NULL;

    	/* not numeric, try DNS */
        h = gethostbyname(host);

        if(!h) {
            /*pdebug("Call to gethostbyname() failed, errno: %d!", errno);*/
            return PLCTAG_ERR_OPEN;
        }

        /* copy the IP list */
        for(num_ips = 0; h->h_addr_list[num_ips] && num_ips < MAX_IPS; num_ips++) {
            ips[num_ips] = *((in_addr_t *)h->h_addr_list[num_ips]);
        }

        free(h);
    }


    /* now try to connect to the remote gateway.  We may need to
     * try several of the IPs we have.
     */

    i = 0;
    done = 0;

    memset((void *)&gw_addr,0, sizeof(gw_addr));
    gw_addr.sin_family = AF_INET ;
    gw_addr.sin_port = htons(port);

    do {
    	int rc;
        /* try each IP until we run out or get a connection. */
        gw_addr.sin_addr.s_addr = ips[i];

        /*pdebug("Attempting to connect to %s",inet_ntoa(*((struct in_addr *)&ips[i])));*/

        rc = connect(fd,(struct sockaddr *)&gw_addr,sizeof(gw_addr));

        if( rc == 0) {
            /*pdebug("Attempt to connect to %s succeeded.",inet_ntoa(*((struct in_addr *)&ips[i])));*/
            done = 1;
        } else {
            /*pdebug("Attempt to connect to %s failed, errno: %d",inet_ntoa(*((struct in_addr *)&ips[i])),errno);*/
            i++;
        }
    } while(!done && i < num_ips);

    if(!done) {
		close(fd);
        /*pdebug("Unable to connect to any gateway host IP address!");*/
        return PLCTAG_ERR_OPEN;
    }


    /* FIXME
     * connect() is a little easier to handle in blocking mode, for now
     * we make the socket non-blocking here, after connect(). */
    flags=fcntl(fd,F_GETFL,0);

    if(flags<0) {
      /*pdebug("Error getting socket options, errno: %d", errno);*/
      close(fd);
      return PLCTAG_ERR_OPEN;
    }

    flags |= O_NONBLOCK;

    if(fcntl(fd,F_SETFL,flags)<0) {
        /*pdebug("Error setting socket to non-blocking, errno: %d", errno);*/
        close(fd);
        return PLCTAG_ERR_OPEN;
    }

    /* save the values */
	s->fd = fd;
	s->port = port;

	return PLCTAG_STATUS_OK;
}




extern int socket_read(sock_p s, uint8_t *buf, int size)
{
    int rc;

    if(!s || !buf) {
    	return PLCTAG_ERR_NULL_PTR;
    }

    /* The socket is non-blocking. */
    rc = read(s->fd,buf,size);

    if(rc < 0) {
    	if(errno == EAGAIN || errno == EWOULDBLOCK) {
    		return PLCTAG_ERR_NO_DATA;
    	} else {
    		return PLCTAG_ERR_READ;
    	}
    }

    return rc;
}


extern int socket_write(sock_p s, uint8_t *buf, int size)
{
    int rc;

    if(!s || !buf) {
    	return PLCTAG_ERR_NULL_PTR;
    }

    /* The socket is non-blocking. */
    rc = write(s->fd,buf,size);

    if(rc < 0) {
    	if(errno == EAGAIN || errno == EWOULDBLOCK) {
    		return PLCTAG_ERR_NO_DATA;
    	} else {
    		return PLCTAG_ERR_READ;
    	}
    }

    return rc;
}



extern int socket_close(sock_p s)
{
	if(!s)
		return PLCTAG_ERR_NULL_PTR;

	return close(s->fd);
}


extern int socket_destroy(sock_p *s)
{
	if(!s || !*s)
		return PLCTAG_ERR_NULL_PTR;

	socket_close(*s);

	mem_free(*s);

	*s = 0;

	return PLCTAG_STATUS_OK;
}







/***************************************************************************
 ********************************* Endian **********************************
 **************************************************************************/


/*
 * FIXME
 *
 * Note that these will not work if __BYTE_ORDER__ is not defined.  They
 * will also not work if the CPU has some other form of byte order.
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
/* little endian */
extern uint16_t h2le16(uint16_t v)
{
	return v;
}



extern uint16_t le2h16(uint16_t v)
{
	return v;
}



extern uint16_t h2be16(uint16_t v)
{
	uint8_t bytes[2];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);

	return ((uint32_t)(bytes[0]) << 8)
		   |((uint32_t)(bytes[1]));
}



extern uint16_t be2h16(uint16_t v)
{
	uint8_t bytes[2];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);

	return ((uint32_t)(bytes[0]) << 8)
		   |((uint32_t)(bytes[1]));
}



extern uint32_t h2le32(uint32_t v)
{
	return v;
}

extern uint32_t le2h32(uint32_t v)
{
	return v;
}


extern uint32_t h2be32(uint32_t v)
{
	uint8_t bytes[4];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);
	bytes[2] = ((v >> 16) & 0xFF);
	bytes[3] = ((v >> 24) & 0xFF);

	return ((uint32_t)(bytes[0]) << 24)
		   |((uint32_t)(bytes[1]) << 16)
		   |((uint32_t)(bytes[2]) << 8)
		   |((uint32_t)(bytes[3]));
}


extern uint32_t be2h32(uint32_t v)
{
	uint8_t bytes[4];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);
	bytes[2] = ((v >> 16) & 0xFF);
	bytes[3] = ((v >> 24) & 0xFF);

	return ((uint32_t)(bytes[0]) << 24)
		   |((uint32_t)(bytes[1]) << 16)
		   |((uint32_t)(bytes[2]) << 8)
		   |((uint32_t)(bytes[3]));
}


#else
/* big endian */
extern uint16_t h2le16(uint16_t v)
{
	uint8_t bytes[2];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);

	return  ((uint32_t)(bytes[0]) << 8)
		   |((uint32_t)(bytes[1]));
}


extern uint16_t le2h16(uint16_t v)
{
	uint8_t bytes[2];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);

	return  ((uint32_t)(bytes[0]) << 8)
		   |((uint32_t)(bytes[1]));
}


extern uint16_t h2be16(uint16_t v)
{
	return v;
}

extern uint16_t be2h16(uint16_t v)
{
	return v;
}




extern uint32_t h2le32(uint32_t v)
{
	uint8_t bytes[4];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);
	bytes[2] = ((v >> 16) & 0xFF);
	bytes[3] = ((v >> 24) & 0xFF);

	return ((uint32_t)(bytes[0]) << 24)
		   |((uint32_t)(bytes[1]) << 16)
		   |((uint32_t)(bytes[2]) << 8)
		   |((uint32_t)(bytes[3]));
}

extern uint32_t le2h32(uint32_t v)
{
	uint8_t bytes[4];

	bytes[0] = (v & 0xFF);
	bytes[1] = ((v >> 8) & 0xFF);
	bytes[2] = ((v >> 16) & 0xFF);
	bytes[3] = ((v >> 24) & 0xFF);

	return ((uint32_t)(bytes[0]) << 24)
		   |((uint32_t)(bytes[1]) << 16)
		   |((uint32_t)(bytes[2]) << 8)
		   |((uint32_t)(bytes[3]));
}


extern uint32_t h2be32(uint32_t v)
{
	return v;
}

extern uint32_t be2h32(uint32_t v)
{
	return v;
}

#endif









/***************************************************************************
 ***************************** Miscellaneous *******************************
 **************************************************************************/




/*
 * sleep_ms
 *
 * Sleep the passed number of milliseconds.  Note that signals
 * will cause this to terminate early!  Check the time before
 * you assume that the total time has passed.
 */
int sleep_ms(int ms)
{
    struct timeval tv;

    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms % 1000)*1000;

    return select(0,NULL,NULL,NULL, &tv);
}


/*
 * time_ms
 *
 * Return the current epoch time in milliseconds.
 */
int64_t time_ms(void)
{
    struct timeval tv;

    gettimeofday(&tv,NULL);

    return  ((int64_t)tv.tv_sec*1000)+ ((int64_t)tv.tv_usec/1000);
}


/*
 * Debugging support.
 */

extern void pdebug_impl(const char *func, int line_num, const char *templ, ...)
{
    va_list va;
    struct tm t;
    time_t epoch;
    char prefix[2048];

    /* build the prefix */
    /* get the time parts */
    epoch = time(0);

    /* FIXME - should capture error return! */
    localtime_r(&epoch,&t);

    /* create the prefix and format for the file entry. */
    snprintf(prefix, sizeof prefix,"%04d-%02d-%02d %02d:%02d:%02d %s:%d %s\n",
                                    t.tm_year+1900,t.tm_mon,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec,
                                    func,line_num,templ);

    /* print it out. */
    va_start(va,templ);
    vfprintf(stderr,prefix,va);
    va_end(va);
}




extern void pdebug_dump_bytes_impl(uint8_t *data,int count)
{
    int i;
    int end;
    char buf[2048];

    snprintf(buf,sizeof buf,"Dumping bytes:\n");

    end = str_length(buf);

    for(i=0; i<count; i++) {
        if((i%10) == 0) {
            snprintf(buf+end,sizeof(buf)-end,"%05d",i);

            end = strlen(buf);
        }

        snprintf(buf+end,sizeof(buf)-end," %02x",data[i]);

        end = strlen(buf);

        if((i%10) == 9) {
            snprintf(buf+end,sizeof(buf)-end,"\n");

            end = strlen(buf);
        }
    }

    /*if( ((i%10)!=9) || (i>=count && (i%10)==9))
        snprintf(buf+end,sizeof(buf)-end,"\n");*/

    pdebug("%s",buf);
    fflush(stderr);
}




