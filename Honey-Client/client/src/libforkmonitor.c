#define  _POSIX_C_SOURCE 200809L
#define  _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "forkmonitor.h"

static pid_t (*actual_fork)(void)  = NULL;
static pid_t (*actual_vfork)(void) = NULL;
static void  (*actual_abort)(void) = NULL;
static void  (*actual__exit)(int)  = NULL;
static void  (*actual__Exit)(int)  = NULL;
static int     commfd = -1;

#define MINIMUM_COMMFD  31

static void notify(const int type, struct message *const msg, const size_t extra)
{
    const int    saved_errno = errno;

    msg->pid  = getpid();
    msg->ppid = getppid();
    msg->sid  = getsid(0);
    msg->pgid = getpgrp();
    msg->uid  = getuid();
    msg->gid  = getgid();
    msg->euid = geteuid();
    msg->egid = getegid();
    msg->len  = extra;
    msg->type = type;

    /* Since we don't have any method of dealing with send() errors
     * or partial send()s, we just fire one off and hope for the best. */
    send(commfd, msg, sizeof (struct message) + extra, MSG_EOR | MSG_NOSIGNAL);

    errno = saved_errno;
}

void libforkmonitor_init(void) __attribute__((constructor));
void libforkmonitor_init(void)
{
    const int saved_errno = errno;
    int       result;

    /* Save the actual fork() call pointer. */
    if (!actual_fork)
        *(void **)&actual_fork = dlsym(RTLD_NEXT, "fork");

    /* Save the actual vfork() call pointer. */
    if (!actual_vfork)
        *(void **)&actual_vfork = dlsym(RTLD_NEXT, "vfork");

    /* Save the actual abort() call pointer. */
    if (!actual_abort)
        *(void **)&actual_abort = dlsym(RTLD_NEXT, "abort");

    /* Save the actual _exit() call pointer. */
    if (!actual__exit)
        *(void **)&actual__exit = dlsym(RTLD_NEXT, "_exit");
    if (!actual__exit)
        *(void **)&actual__exit = dlsym(RTLD_NEXT, "_Exit");

    /* Save the actual abort() call pointer. */
    if (!actual__Exit)
        *(void **)&actual__Exit = dlsym(RTLD_NEXT, "_Exit");
    if (!actual__Exit)
        *(void **)&actual__Exit = dlsym(RTLD_NEXT, "_exit");

    /* Open an Unix domain datagram socket to the observer. */
    if (commfd == -1) {
        const char *address;

        /* Connect to where? */
        address = getenv(FORKMONITOR_ENVNAME);
        if (address && *address) {
            struct sockaddr_un addr;

            memset(&addr, 0, sizeof addr);
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, address, sizeof addr.sun_path - 1);

            /* Create and bind the socket. */
            commfd = socket(AF_UNIX, SOCK_DGRAM, 0);
            if (commfd != -1) {
                if (connect(commfd, (const struct sockaddr *)&addr, sizeof (addr)) == -1) {
                    /* Failed. Close the socket. */
                    do {
                        result = close(commfd);
                    } while (result == -1 && errno == EINTR);
                    commfd = -1;
                }
            }

            /* Move commfd to a high descriptor, to avoid complications. */
            if (commfd != -1 && commfd < MINIMUM_COMMFD) {
                const int newfd = MINIMUM_COMMFD;
                do {
                    result = dup2(commfd, newfd);
                } while (result == -1 && errno == EINTR);
                if (!result) {
                    do {
                        result = close(commfd);
                    } while (result == -1 && errno == EINTR);
                    commfd = newfd;
                }
            }
        }
    }

    /* Send an init message, listing the executable path. */
    if (commfd != -1) {
        size_t          len = 128;
        struct message *msg = NULL;

        while (1) {
            ssize_t n;

            free(msg);
            msg = malloc(sizeof (struct message) + len);
            if (!msg) {
                len = 0;
                break;
            }

            n = readlink("/proc/self/exe", msg->data, len);
            if (n > (ssize_t)0 && (size_t)n < len) {
                msg->data[n] = '\0';
                len = n + 1;
                break;
            }

            len = (3 * len) / 2;
            if (len >= 65536U) {
                free(msg);
                msg = NULL;
                len = 0;
                break;
            }
        }

        if (len > 0) {
            /* INIT message with executable name */
            notify(TYPE_EXEC, msg, len);
            free(msg);
        } else {
            /* INIT message without executable name */
            struct message msg2;
            notify(TYPE_EXEC, &msg2, sizeof msg2);
        }
    }

    /* Restore errno. */
    errno = saved_errno;
}

void libforkmonitor_done(void) __attribute__((destructor));
void libforkmonitor_done(void)
{
    const int saved_errno = errno;
    int       result;

    /* Send an exit message, no data. */
    if (commfd != -1) {
        struct message msg;
        notify(TYPE_DONE, &msg, sizeof msg);
    }

    /* If commfd is open, close it. */
    if (commfd != -1) {
        do {
            result = close(commfd);
        } while (result == -1 && errno == EINTR);
    }

    /* Restore errno. */
    errno = saved_errno;
}

/*
 * Hooked C library functions.
*/

pid_t fork(void)
{
    pid_t result;
    if (!actual_fork) {
        const int saved_errno = errno;

        *(void **)&actual_fork = dlsym(RTLD_NEXT, "fork");
        if (!actual_fork) {
            errno = EAGAIN;
            return (pid_t)-1;
        }

        errno = saved_errno;
    }

    result = (*actual_fork)();
    if (!result && commfd != -1) {
        struct message msg;
        notify(TYPE_FORK, &msg, sizeof msg);
    }

    printf("\n%d",result);
    fflush(stdout);
    return result;
}

pid_t vfork(void)
{
    pid_t result;

    if (!actual_vfork) {
        const int saved_errno = errno;

        *(void **)&actual_vfork = dlsym(RTLD_NEXT, "vfork");
        if (!actual_vfork) {
            errno = EAGAIN;
            return (pid_t)-1;
        }

        errno = saved_errno;
    }

    result = actual_vfork();
    if (!result && commfd != -1) {
        struct message msg;
        notify(TYPE_VFORK, &msg, sizeof msg);
    }

    return result;
}

void _exit(const int code)
{
    if (!actual__exit) {
        const int saved_errno = errno;
        *(void **)&actual__exit = dlsym(RTLD_NEXT, "_exit");
        if (!actual__exit)
            *(void **)&actual__exit = dlsym(RTLD_NEXT, "_Exit");
        errno = saved_errno;
    }

    if (commfd != -1) {
        struct {
            struct message  msg;
            int             extra;
        } data;

        memcpy(&data.msg.data[0], &code, sizeof code);
        notify(TYPE_EXIT, &(data.msg), sizeof (struct message) + sizeof (int));
    }

    if (actual__exit)
        actual__exit(code);

    exit(code);
}

void _Exit(const int code)
{
    if (!actual__Exit) {
        const int saved_errno = errno;
        *(void **)&actual__Exit = dlsym(RTLD_NEXT, "_Exit");
        if (!actual__Exit)
            *(void **)&actual__Exit = dlsym(RTLD_NEXT, "_exit");
        errno = saved_errno;
    }

    if (commfd != -1) {
        struct {
            struct message  msg;
            int             extra;
        } data;

        memcpy(&data.msg.data[0], &code, sizeof code);
        notify(TYPE_EXIT, &(data.msg), sizeof (struct message) + sizeof (int));
    }

    if (actual__Exit)
        actual__Exit(code);

    exit(code);
}

void abort(void)
{
    if (!actual_abort) {
        const int saved_errno = errno;
        *(void **)&actual_abort = dlsym(RTLD_NEXT, "abort");
        errno = saved_errno;
    }

    if (commfd != -1) {
        struct message msg;
        notify(TYPE_ABORT, &msg, sizeof msg);
    }

    actual_abort();
    exit(127);
}
