#ifndef Routes_H
#define Routes_H

typedef struct Route Route;

struct Route {
  char *key;
  char *value;
  struct Route *left;
  struct Route *right;
};

Route *initRoute(char *key, char *value);
Route *addRoute(Route *root, char *key, char *value);
Route *search(Route *root, char *key);
void print_inorder(Route *root);

#endif
