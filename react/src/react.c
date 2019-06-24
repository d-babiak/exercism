#include "react.h"
#include <assert.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#define UNPOSSIBLE 0

/*
struct reactor;
struct cell;

typedef int (*compute1) (int);
typedef int (*compute2) (int, int);
*/

struct reactor *create_reactor() {
  struct reactor *R = malloc(sizeof(struct reactor));
  return R;
}

// destroy_reactor should free all cells created under that reactor.
void destroy_reactor(struct reactor *R) {
  free(R);
}

struct cell *create_input_cell(struct reactor *R, int initial_value) {
  struct cell *C = malloc(sizeof(struct cell));
  memset(C, 0, sizeof(struct cell));

  C->next  = R->cell;
  C->arity = 0;
  C->data  = initial_value;

  return R->cell = C;
}

struct cell *create_compute1_cell(
  struct reactor *R, struct cell *in0, compute1 f
) {
  struct cell *C = malloc(sizeof(struct cell));
  memset(C, 0, sizeof(struct cell));

  C->next    = R->cell;
  C->arity   = 1;
  C->f1      = f;
  C->args[0] = in0;

  return R->cell = C;
}

struct cell *create_compute2_cell(
  struct reactor *R, struct cell *in0, struct cell *in1, compute2 f
) {
  struct cell *C = malloc(sizeof(struct cell));
  memset(C, 0, sizeof(struct cell));

  C->next    = R->cell;
  C->arity   = 2;
  C->f2      = f;
  C->args[0] = in0;
  C->args[1] = in1;

  return R->cell = C;
}

int get_cell_value(struct cell *C) {
  switch (C->arity) {
    case 0: 
      return C->data;

    case 1: 
      return (C->f1)(get_cell_value(C->args[0]));

    case 2: 
      return (C->f2)(
        get_cell_value(C->args[0]), 
        get_cell_value(C->args[1])
      );
  }
  assert(UNPOSSIBLE);
}

void set_cell_value(struct cell *C, int new_value) {
  C->data = new_value;

  for (callback_t *cb = C->callbacks; cb != NULL; cb = cb->next)
    (cb->f)(cb->arg, new_value);
}

callback_id add_callback(struct cell *C, void * arg, callback f) {
  callback_t *cb = malloc(sizeof(callback_t));
  memset(cb, 0, sizeof(callback_t));

  cb->next  = C->callbacks;
  cb->cb_id = C->next_id;
  cb->f     = f;
  cb->arg   = arg; 

  C->callbacks = cb;
  C->next_id  += 1;

  return cb->cb_id;
}

void remove_callback(struct cell *C, callback_id n) {
  printf("lol - fuck off %d %d\n", C->arity, n);
}

static int N_callbacks = 0;
static void print_plus(void *arg, int i) {
  int x = *((int*) arg);
  printf(
   "N_callbacks: %d, %d + %d = %d\n", 
    N_callbacks++, x, i, x + i
  );
}

static void print_minus(void *arg, int i) {
  int x = *((int*) arg);
  printf(
   "N_callbacks: %d, %d - %d = %d\n", 
    N_callbacks++, x, i, x - i
  );
}

int main() {
  struct reactor *R  = create_reactor();
  struct cell *input = create_input_cell(R, 0);
  int r1 = get_cell_value(input);
  printf("r1 = %d\n", r1);

  int X = 42;

  add_callback(input, &X, print_plus);

  set_cell_value(input, 7);

  add_callback(input, &X, print_minus);
  set_cell_value(input, 7);

  int r2 = get_cell_value(input);
  printf("r2 = %d\n", r2);
}
