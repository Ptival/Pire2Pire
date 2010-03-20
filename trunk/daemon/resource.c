#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../util/logger.h"
#include "resource.h"

extern FILE *log_file;

#define MIN(a,b)    (((a)>(b))?(b):(a))

struct resource *
resource_new (const char *filename, const char *key,
                res_size_t size, const char *ip_port) {
    struct resource *resource;
    int filename_size;

    resource = (struct resource *)malloc (sizeof (struct resource));

    if (!resource) {
        printf ("Error allocating resource\n");
        exit (EXIT_FAILURE);
    }

    filename_size = MIN (RESOURCE_FILENAME_SIZE, strlen (filename)) + 1;
    strncpy (resource->filename, filename, filename_size);
    resource->filename[filename_size - 1] = '\0';

    strncpy (resource->key, key, RESOURCE_KEY_SIZE + 1);
    resource->key[RESOURCE_KEY_SIZE] = '\0';

    resource->size = size;

    resource->seeders = (struct seeder *)malloc (sizeof (struct seeder));
    strncpy (resource->seeders->ip_port, ip_port, RESOURCE_IP_PORT_SIZE + 1);

    resource->nb_seeders = 1;

    sem_init (&resource->seeders_lock, 0, 1);

    return resource;
}

void
resource_free (struct resource *resource) {
    free (resource->seeders);
    sem_destroy (&resource->seeders_lock);
    free (resource);
}

void
resource_add_seeder (struct resource *resource, const char *ip_port) {
    sem_wait (&resource->seeders_lock);
    for (int i = 0; i < resource->nb_seeders; i++)
        if (strncmp (ip_port,
                    resource->seeders[i].ip_port,
                    RESOURCE_IP_PORT_SIZE +1))
            return;
    resource->seeders = (struct seeder *)realloc (resource->seeders,
                        (resource->nb_seeders + 1) * sizeof (struct seeder));
    strncpy (resource->seeders[resource->nb_seeders].ip_port,
                ip_port,
                RESOURCE_IP_PORT_SIZE + 1);
    resource->nb_seeders++;
    sem_post (&resource->seeders_lock);
}

struct resource_tree *
resource_tree_add (struct resource_tree *tree, struct resource *resource) {
    int bias = 0;
    if (tree == NULL) {
        tree = (struct resource_tree *)malloc (sizeof (struct resource_tree));
        tree->this = resource;
        tree->left = NULL;
        tree->right = NULL;
    }
    else {
        bias = strcmp (resource->key, tree->this->key);
        if (bias < 0) {
            tree->left = resource_tree_add (tree->left, resource);
        }
        else if (bias > 0) {
            tree->right = resource_tree_add (tree->right, resource);
        }
        else {
            printf ("Error : key already used\n");
        }
    }
    return tree;
}

struct resource *
resource_tree_get_by_key (struct resource_tree *tree, const char *key) {
    int compare;

    if (!tree)
        return NULL;

    compare = strncmp (tree->this->key, key, RESOURCE_KEY_SIZE + 1);
    if (!compare)
        return tree->this;
    else if (compare < 0)
        return resource_tree_get_by_key (tree->left, key);
    else
        return resource_tree_get_by_key (tree->right, key);
}

void
resource_tree_to_client (struct resource_tree *tree, struct client *c) {
    char line[RESOURCE_KEY_SIZE + RESOURCE_FILENAME_SIZE + 27];

    if (tree) {
        sprintf (line,
                " < %s %s %20llu\n",
                tree->this->key,
                tree->this->filename,
                tree->this->size);
        resource_tree_to_client (tree->left, c);
        client_send (c, line);
        resource_tree_to_client (tree->right, c);
    }
}

void
resource_tree_free (struct resource_tree *tree) {
    if (tree != NULL) {
        if (tree->left != NULL) {
            resource_tree_free (tree->left);
            tree->left = NULL;
        }
        if (tree->right != NULL) {
            resource_tree_free (tree->right);
            tree->right = NULL;
        }
        resource_free (tree->this);
        tree->this = NULL;
        free (tree);
    }
}
