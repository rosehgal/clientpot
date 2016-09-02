#ifndef   FORKMONITOR_H
#define   FORKMONITOR_H

#define   FORKMONITOR_ENVNAME "FORKMONITOR_SOCKET"

#ifndef   UNIX_PATH_MAX
#define   UNIX_PATH_MAX 108
#endif

#define TYPE_EXEC       1   /* When a binary is executed */
#define TYPE_DONE       2   /* exit() or return from main() */
#define TYPE_FORK       3
#define TYPE_VFORK      4
#define TYPE_EXIT       5   /* _exit() or _Exit() */
#define TYPE_ABORT      6   /* abort() */

struct message {
    pid_t          pid;     /* Process ID */
    pid_t          ppid;    /* Parent process ID */
    pid_t          sid;     /* Session ID */
    pid_t          pgid;    /* Process group ID */
    uid_t          uid;     /* Real user ID */
    gid_t          gid;     /* Real group ID */
    uid_t          euid;    /* Effective user ID */
    gid_t          egid;    /* Effective group ID */
    unsigned short len;     /* Length of data[] */
    unsigned char  type;    /* One of the TYPE_ constants */
    char           data[0]; /* Optional payload, possibly longer */
};

#endif /* FORKMONITOR_H */
