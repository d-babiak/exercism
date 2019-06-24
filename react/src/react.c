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

  struct cell *nxt;
  for (struct cell *C = R->cell; C != NULL; C = nxt) {
    for (callback_t *cb = C->callbacks; cb != NULL; cb = cb->next)
      free(cb);

    //for (subscriber_t *S = C->subscribers; S != NULL; S = S->next)
    //  free(S);
    nxt = C->next;
    free(C);
  }
  free(R);
}

/*
dmb@dmb ...exercism/c/react % make memcheck # ?_? 
Compiling memcheck
AddressSanitizer:DEADLYSIGNAL
=================================================================
==5177==ERROR: AddressSanitizer: SEGV on unknown address 0x000000000000 (pc 0x55d696e9f304 bp 0x7ffed360c830 sp 0x7ffed360c800 T0)                                                                                                           
==5177==The signal is caused by a READ memory access.
==5177==Hint: address points to the zero page.
    #0 0x55d696e9f303 in destroy_reactor src/react.c:28
    #1 0x55d696ea335e in test_input_cells_have_value test/test_react.c:21
    #2 0x55d696ea3168 in UnityDefaultTestRun test/vendor/unity.c:1339
    #3 0x55d696ea5177 in main test/test_react.c:314
    #4 0x7f3b19ad9b6a in __libc_start_main (/lib/x86_64-linux-gnu/libc.so.6+0x26b6a)
    #5 0x55d696e9f1d9 in _start (/home/dmb/exercism/c/react/memcheck.out+0x31d9)

AddressSanitizer can not provide additional info.
SUMMARY: AddressSanitizer: SEGV src/react.c:28 in destroy_reactor
==5177==ABORTING
make: *** [makefile:27: memcheck] Error 1
*/

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

static int op_num = 0;

int get_cell_value(struct cell *C) {
  if (C->op_num == op_num)
    return C->data;

  C->op_num = op_num;

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

  for (subscriber_t *S = C->subscribers; S != NULL; S = S->next)
    publish((struct cell *)(S->consumer), i + 1);
}

void set_cell_value(struct cell *C, int new_value) {
  if (new_value == C->data)
    return;

  op_num++;

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
  if (!C->callbacks)
    return;
  
  if (C->callbacks->cb_id == n) {
    C->callbacks = C->callbacks->next;
    return;
  }

  callback_t *cb = C->callbacks;
  while (cb->next && cb->next->cb_id != n)
    cb = cb->next;

  if (cb->next)
    cb->next = cb->next->next;
}
