#include <sys/socket.h>     // send ()

#include <errno.h>
#include <pthread.h>        // pthread_equal ()
#include <signal.h>         // pthread_kill ()
#include <stdlib.h>         // malloc ()
#include <string.h>         // strdup ()

#include "../util/logger.h" // log_failure ()
#include "daemon.h"         // struct daemon

extern FILE *log_file;

struct daemon *
daemon_new (int socket, const char *addr, int port) {
    struct daemon *d;
    d = malloc (sizeof (struct daemon));
    if (d == NULL)
        return NULL;

    d->socket   = socket;
    d->addr     = strdup (addr);
    d->port     = port;
    d->next     = NULL;
    d->prev     = NULL;
    d->requests = NULL;
    memset (&d->thread_id, 0, sizeof (d->thread_id));
    sem_init (&d->req_lock, 0, 1);
    sem_init (&d->socket_lock, 0, 1);

    return d;
}

void
daemon_free (struct daemon *d) {
    if (!d)
        return;

    /*
     * TODO: Shouldn't we pthread_kill the remaining requests threads ?
     */

    if (d->addr) {
        free (d->addr);
        d->addr = NULL;
    }
    sem_destroy (&d->req_lock);
    sem_destroy (&d->socket_lock);
    free (d);
}

struct daemon *
daemon_add (struct daemon *l, struct daemon *d) {
    if (!d) {
        return l;
    }
    if (!l) {
        return d;
    }
    d->prev = NULL;
    d->next = l;
    l->prev = d;
    return d;
}

int
daemon_numbers (struct daemon *l) {
    if (!l)
        return 0;
    struct daemon *tmp;
    int sum;
    for (tmp = l, sum = 0; tmp; tmp = tmp->next, sum++);
    return sum;
}

struct daemon *
daemon_remove (struct daemon *l, pthread_t pt) {
    if (!l)
        return NULL;
    struct daemon *tmp;
    for (tmp = l; pthread_equal (tmp->thread_id, pt) == 0; tmp = tmp->next);

    /* This was the last element */
    if (tmp->prev == NULL && tmp->next == NULL) {
        return NULL;
    }

    /* Was the first element */
    if (!tmp->prev) {
        tmp->next->prev = NULL;
        return tmp->next;
    }

    tmp->prev->next = tmp->next;
    return l;
}

int
daemon_send (struct daemon *d, const char *msg) {
    int     dest_sock;
    int     nb_sent;
    int     nb_sent_sum;
    int     send_size;
//    char    ending_char;

    // Log this (see ../util/logger.c to disactivate this)
    log_send (log_file, msg);

    dest_sock = d->socket;
    if (sem_wait (&d->socket_lock) < 0) {
        log_failure (log_file,
                    "failed to sem_wait socket_lock, error: %s",
                    strerror (errno));
        return -1;
    }

    // Now the socket is locked for us, let's send!
    send_size = strlen (msg);

    // Send the command
    nb_sent_sum = 0;
    while (nb_sent_sum < send_size) {
        nb_sent = send (dest_sock,
                        msg + nb_sent_sum,
                        send_size - nb_sent_sum,
                        0);
        if (nb_sent < 0) {
            log_failure (log_file,
                        "couldn't send to daemon socket, error: %s",
                        strerror (errno));
            return -1;
        }
        nb_sent_sum += nb_sent;
    }

    if (sem_post (&d->socket_lock) < 0) {
        log_failure (log_file,
                    "failed to sem_post socket_lock, error: %s",
                    strerror (errno));
    }

    return 0;
}
