#ifndef DAEMON_H
#define DAEMON_H

#define for_each_daemon(d) \
        for (d = daemons; d; d = d->next)

#include <semaphore.h>      // sem_t

#include "daemon_request.h" // struct daemon_request

struct daemon {
    int                     socket;
    /* We need to lock the socket to send atomic messages */
    sem_t                   socket_lock;
    char                    *addr;  /* IPv4, IPv6, whatever... */
    int                     port;   /* Host short, use htons () if needed */
    /*
     * Many different requests will try and modify "requests", using
     * request_add () or request_remove (). That's why we need a semaphore.
     */
    struct daemon_request   *requests;
    int                     nb_requests;
    sem_t                   req_lock;
    struct daemon           *next;
    struct daemon           *prev;
};

int
daemon_count ();

struct daemon *
daemon_find_by_ip_port (struct daemon *daemons,
                        const char *ip,
                        int port);

struct daemon *
daemon_new  (int socket,
            const char *addr,
            int port);

void
daemon_free (struct daemon *d);

struct daemon *
daemon_add  (struct daemon *l,
            struct daemon *d);

struct daemon *
daemon_remove   (struct daemon *l,
                struct daemon *c);

int
daemon_send (struct daemon *d,
            const char *msg);

#endif/*DAEMON_H*/
