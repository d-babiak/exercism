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

static void add_subscriber(struct cell *producer, struct cell *consumer) {
  subscriber_t *S = malloc(sizeof(subscriber_t));
  memset(S, 0, sizeof(subscriber_t));

  S->next = producer->subscribers;
  S->consumer = (void *)consumer;

  producer->subscribers = S;
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
  C->data    = get_cell_value(C);

  add_subscriber(in0, C);
  
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
  C->data    = get_cell_value(C);

  add_subscriber(in0, C);
  add_subscriber(in1, C);

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

static void publish(struct cell *C, int i) {
  int new_value = get_cell_value(C);

  if (!(i == 0 || C->data != new_value))
    return;

  C->data = new_value;

  for (callback_t *cb = C->callbacks; cb != NULL; cb = cb->next)
    (cb->f)(cb->arg, new_value);

  for (subscriber_t *S = C->subscribers; S != NULL; S = S->next) {
    struct cell *consumer = (struct cell *)(S->consumer);
    if (consumer == C) {
      printf("WTF\n");
      continue;
    }
    publish(consumer, i + 1);
  }
}

void set_cell_value(struct cell *C, int new_value) {
  if (new_value == C->data)
    return;

  C->data = new_value;

  publish(C, 0);
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

/*
static int big_if_three(int x) {
   return x < 3 ? 111 : 222;
}

void console_log(void *y, int x) {
  int i = *((int *) y);
  printf("console_log: %d %d\n", i, x);
}

int main() {
   struct reactor *r = create_reactor();
   struct cell *input = create_input_cell(r, 1);
   struct cell *output = create_compute1_cell(r, input, big_if_three);

   int Y = 42; 
   add_callback(output, &Y, console_log);

   set_cell_value(input, 2);

   set_cell_value(input, 4);
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

static int x_plus_1(int x) {
  return x + 1;
}

static int x_plus_y_plus_3(int x, int y) {
  return x + y + 3;
}

int main() {
  struct reactor *R  = create_reactor();
  struct cell *input = create_input_cell(R, 0);
  int r1 = get_cell_value(input);
  printf("r1 = %d\n", r1);

  int X = 42;

  add_callback(input, &X, print_plus);

  set_cell_value(input, 7);

  int r2 = get_cell_value(input);
  printf("r2 = %d\n", r2);

  int Y = 100;
  struct cell *C1 = create_compute1_cell(R, input, x_plus_1);
  add_callback(C1, &Y, print_minus);

  set_cell_value(input, 8);

  int Z = 200;
  struct cell *C2 = create_compute2_cell(R, input, C1, x_plus_y_plus_3);
  add_callback(C2, &Z, print_plus);
  add_callback(C2, &Z, print_minus);

  set_cell_value(input, 8);
}
*/
