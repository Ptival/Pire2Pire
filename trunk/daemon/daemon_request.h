#ifndef DAEMON_REQUEST_H
#define DAEMON_REQUEST_H

#include <pthread.h>    // pthread_t

struct daemon;

struct daemon_request {
    pthread_t               thread_id;
    char                    *cmd;
    struct daemon           *daemon;
    struct daemon_request   *prev;
    struct daemon_request   *next;
};

struct daemon_request
*daemon_request_new (char *cmd, struct daemon *daemon);

void
daemon_request_free (struct daemon_request *r);

struct daemon_request *
daemon_request_add (struct daemon_request *l, struct daemon_request *r);

struct daemon_request *
daemon_request_remove (struct daemon_request *l, pthread_t pt);

int
daemon_request_count (struct daemon_request *l);

#endif//DAEMON_REQUEST_H
