#include "routes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Route *initRoute(char *key, char *value) {
  Route *root = (Route *)malloc(sizeof(Route));

  root->key = key;
  root->value = value;

  root->left = root->right = NULL;

  return root;
}

Route *addRoute(Route *root, char *key, char *value) {

  if (root == NULL) {
    printf("[ROUTE LOG] Root not exist, creating route for %s -> %s\n", key,
           value);
    Route *tmp = initRoute(key, value);
    return tmp;
  }

  if (strcmp(key, root->key) == 0)
    fprintf(stderr, "Root already exist\n");
  else if (strcmp(key, root->key) > 0)
    root->right = addRoute(root->right, key, value);
  else
    root->left = addRoute(root->left, key, value);

  return root;
}

Route *search(Route *root, char *key) {
  printf("[ROUTE LOG] Seaching routes...\n");

  if (root == NULL) {
    printf("[ROUTE LOG] Root does not exist for key: %s\n", key);
    return NULL;
  }

  if (strcmp(key, root->key) == 0) {
    printf("[ROUTE LOG] Root found for key %s\n", key);
    return root;
  } else if (strcmp(key, root->key) > 0)
    return search(root->right, key);
  else
    return search(root->left, key);
}

void print_inorder(Route *root) {
  if (root != NULL) {
    print_inorder(root->left);
    printf("%s -> %s\n", root->key, root->value);
    print_inorder(root->right);
  }
}
