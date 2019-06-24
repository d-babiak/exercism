#ifndef REACT_H
#define REACT_H

#include <stdint.h>

typedef int (*compute1) (int);
typedef int (*compute2) (int, int);

struct cell {
  struct cell *next;
  uint8_t     arity;
  union {
    compute1  f1;
    compute2  f2;
  };
  struct cell *args[2];
  int         data;
};

struct reactor {
  struct cell *cell;
};

/*
struct reactor;
struct cell;
*/


struct reactor *create_reactor();
// destroy_reactor should free all cells created under that reactor.
void destroy_reactor(struct reactor *);

struct cell *create_input_cell(struct reactor *, int initial_value);
struct cell *create_compute1_cell(struct reactor *, struct cell *, compute1);
struct cell *create_compute2_cell(struct reactor *, struct cell *,
                                  struct cell *, compute2);

int get_cell_value(struct cell *);
void set_cell_value(struct cell *, int new_value);

typedef void (*callback) (void *, int);
typedef int callback_id;

// The callback should be called with the same void * given in add_callback.
callback_id add_callback(struct cell *, void *, callback);
void remove_callback(struct cell *, callback_id);

#endif
