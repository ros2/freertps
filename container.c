#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"

static fr_rc_t fr_container_node_set(struct fr_container_node *n,
    void *data, const size_t len, const uint32_t flags)
{
  if (!n)
    return FR_RC_BAD_ARGUMENT;
  if (flags & FR_CFLAGS_TAKE_OWNERSHIP)
  {
    if (n->state != FR_CNS_NEW)
    {
      printf("woah! memory leak in fr_container_node_set\n");
      return FR_RC_OH_NOES;
    }
    n->data = data;
  }
  else
  {
    if (n->state == FR_CNS_NEW)
    {
      n->data = fr_malloc(len); // allocate the requested size
      n->alloc_len = len;
    }
    else if (n->alloc_len < len)
    {
      printf("HEY! data object of size %d won't fit into %d bytes\n",
          (int)len, (int)n->alloc_len);
      return FR_RC_BAD_ARGUMENT;
    }
    // if we get here, we know alloc_len >= len
    memcpy(n->data, data, len);
  }
  n->data_len = len;
  n->state = FR_CNS_VALID;
  return FR_RC_OK;
}

static struct fr_container_node *fr_container_node_create()
{
  struct fr_container_node *n = fr_malloc(sizeof(struct fr_container_node));
  if (!n)
    return NULL;
  n->state = FR_CNS_NEW;
  n->next = NULL;
  n->data_len = 0;
  n->alloc_len = 0;
  return n;
}

static fr_rc_t fr_container_append_array_block(
    struct fr_container_array_list *c,
    struct fr_container_array_node *parent,
    uint32_t block_len)
{
  // add an extra byte to each element so we can have a state field for
  // each element (in a separate array, at the end of the block)
  struct fr_container_array_node *a = 
      fr_malloc(sizeof(struct fr_container_array_node) + 
                (1 + c->element_size) * block_len);
  // use the array length of the first array in the chain, or zero
  a->length = block_len;
  a->state = &a->data[block_len * c->element_size];
  for (uint32_t i = 0; i < a->length; i++)
    a->state[i] = FR_CANS_INVALID;
  a->next = NULL;
  parent->next = a;
  return FR_RC_OK;
}

fr_rc_t fr_container_append(struct fr_container *c, void *data, size_t len,
    uint32_t flags)
{
  printf("fr_container_append(type=%d, size=%d, flags=%d)\n",
      (int)c->type, (int)len, (int)flags);
  if (c->type == FR_CT_ARRAY_LIST)
  {
    struct fr_container_array_list *ca = (struct fr_container_array_list *)c;
    // this data structure is only intended for fixed-length elements.
    // verify that we're writing the right size!
    if (len != ca->element_size)
    {
      printf("woah! tried to write %d-byte element into %d-byte array cell\n",
          (int)len, (int)ca->element_size);
      return FR_RC_BAD_ARGUMENT;
    }
    // spin through the array list to find the next available element,
    // allocating a new array block if needed. This could be sped up if 
    // desired by holding a tail pointer in the container struct.
    for (struct fr_container_array_node *a = &ca->head; a; a = a->next)
    {
      for (uint32_t i = 0; i < a->length; i++)
        if (a->state[i] == FR_CANS_INVALID)
        {
          uint8_t *p =
              (uint8_t *)(&(((uint8_t *)a->data)[i * ca->element_size]));
          memcpy(p, data, ca->element_size);
          a->state[i] = FR_CANS_VALID;
          //printf("stuffed into existing array @ cell %d\n", (int)i);
          return FR_RC_OK;
        }
      // if we get here, we couldn't find a place for this element in
      // the last array block of the chain. so we need to allocate a new
      // array block.
      if (!a->next)
      {
        // todo: some flag to allow only pre-allocated arrays, and not
        // chain into another array block like this?
        fr_container_append_array_block(ca, a, ca->head.length);
        struct fr_container_array_node *n = a->next;
        //n->state = (uint8_t *)fr_malloc(n->length);
        //for (uint32_t i = 0; i < n->length; i++)
        //  n->state[i] = FR_CANS_INVALID;
        //n->data = fr_malloc(n->length * c->element_size);
        memcpy(n->data, data, ca->element_size); // stuff first element
        n->state[0] = FR_CANS_VALID;
        n->next = NULL;
        a->next = n;
        return FR_RC_OK;
      }
    }
    // shouldn't ever get here, unless we're out of memory, but we've 
    // probably segfaulted by now if that was the case :)
    printf("woah! hit end of array-list chain. how did this happen?\n");
    return FR_RC_CONTAINER_FULL;
  }
  else if (c->type == FR_CT_LIST)
  {
    struct fr_container_list *cl = (struct fr_container_list *)c;
    if (!cl->head) // if this list is empty. allocate the first node
    {
      cl->head = fr_container_node_create();
      return fr_container_node_set(cl->head, data, len, flags);
    }
    // find the end of the list, or an unused already-allocated block
    // that is the same size as this object, unless we are taking ownership
    // of the data, in which case we need to make a new node for it
    struct fr_container_node *n = cl->head;
    while (n->next != NULL)
    {
      if (!(flags & FR_CFLAGS_TAKE_OWNERSHIP) &&
          n->state == FR_CNS_UNUSED && 
          n->alloc_len >= len)
        return fr_container_node_set(n, data, len, flags);
      n = n->next;
    }
    // if we get here, we're at the tail of the list
    n->next = fr_container_node_create(data, len);
    return fr_container_node_set(n->next, data, len, flags);
  }
  else
  {
    printf("unknown container type: %d\n", (int)c->type);
    return FR_RC_BAD_ARGUMENT;
  }
}

uint32_t fr_container_len(struct fr_container *c)
{
  if (c == NULL)
    return 0;
  uint32_t len = 0;
  if (c->type == FR_CT_LIST)
  {
    struct fr_container_list *cl = (struct fr_container_list *)c;
    struct fr_container_node *n = cl->head;
    while (n)
    {
      if (n->state != FR_CNS_UNUSED)
        len++;
      n = n->next;
    }
    return len;
  }
  else if (c->type == FR_CT_ARRAY_LIST)
  {
    printf("need to implement array list container length!\n");
    exit(1);
    return 0;
  }
  else
  {
    printf("unknown container type: %d\n", (int)c->type);
    return false;
  }
}

struct fr_container *fr_container_create(
    const uint32_t element_size,
    uint32_t array_block_length)
{
  //printf("** sizeof(struct fr_container) = %d\n",
  //    (int)sizeof(struct fr_container));
  if (!element_size) // allocate a normal linked-list if ele sizes are dynamic
  {
    struct fr_container_list *c = fr_malloc(sizeof(struct fr_container_list));
    c->type = FR_CT_LIST;
    c->head = NULL; // it's a simple linked-list; no complex init necessary.
    return (struct fr_container *)c;
  }
  else
  {
    if (array_block_length == 0)
    {
      printf("woah! defaulting container array block length to 1.\n");
      array_block_length = 1;
    }
    struct fr_container_array_list *c =
        fr_malloc(sizeof(struct fr_container_array_list) +
                  (1 + element_size) * array_block_length);
    c->type = FR_CT_ARRAY_LIST;
    c->element_size = element_size;
    c->head.length = array_block_length;
    c->head.state = &c->head.data[array_block_length * c->element_size];
    for (uint32_t i = 0; i < c->head.length; i++)
      c->head.state[i] = FR_CANS_INVALID;
    c->head.next = NULL;
    //fr_container_append_array_block(c, NULL, array_block_length);
    return (struct fr_container *)c;
  }
}

void fr_container_free(struct fr_container *c)
{
  printf("fr_container_free()\n");
  if (c->type == FR_CT_ARRAY_LIST)
  {
    struct fr_container_array_list *ca = (struct fr_container_array_list *)c;
    // cruise down the list, deleting as we go
    for (struct fr_container_array_node *n = ca->head.next; n;)
    {
      struct fr_container_array_node *n_next = n->next;
      fr_free(n->data);
      fr_free(n);
      n = n_next;
    }
  }
  else
  {
    struct fr_container_list *cl = (struct fr_container_list *)c;
    for (struct fr_container_node *n = cl->head; n;)
    {
      struct fr_container_node *n_next = n->next;
      fr_free(n->data);
      fr_free(n);
      n = n_next;
    }
  }
}

