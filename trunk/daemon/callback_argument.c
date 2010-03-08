#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "../util/logger.h"
#include "callback_argument.h"
extern FILE* log_file;

struct callback_argument*
cba_new (char *cmd, int client_socket) {
    struct callback_argument *cba;
    cba = malloc (sizeof (struct callback_argument));
    if (!cba)
        return NULL;
    cba->cmd           = strdup (cmd);
    cba->client_socket = client_socket;
    return cba;
}

void
cba_free (struct callback_argument *cba) {
    if (!cba)
        return;
    log_success (log_file, "epic win");

/*
 * FIXME : Weird bug is weird.
 *
 * When the following code is not commented, doing :
 *
 * > set
 * > quit
 *
 * leads to the daemon exiting. Must be some kind of segfault :(
 *
 * That is definitely weird, since we test cba->cmd.
 *
 * I will look into that later.
 * Cyril.
 */

/*
    if (cba->cmd) {
        free (cba->cmd);
        cba->cmd = NULL;
    }
*/

    log_success (log_file, "long cat");
    free (cba);
    cba = NULL;
}
