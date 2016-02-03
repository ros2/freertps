#include <stdio.h>
#include "freertps/container.h"
#include "freertps/iterator.h"

bool fr_iterator_next(struct fr_iterator *it)
{
  if (!it)
    return false;
  if (it->c->type == FR_CT_LIST)
  {
    if (!it->u.n)
      return false;
    while (it->u.n->next)
    {
      it->u.n = it->u.n->next;
      if (it->u.n->state == FR_CNS_VALID)
      {
        it->data = it->u.n->data;
        it->data_len = it->u.n->data_len;
        return true;
      }
    }
    // if we get here, we hit the end of the list
    it->u.n = NULL;
    return false;
  }
  else if (it->c->type == FR_CT_ARRAY_LIST)
  {
    for (uint32_t i = it->array_idx + 1; i < it->u.a->length; i++)
    {
      if (it->u.a->state[i] == FR_CANS_VALID)
      {
        it->data = 
            (void *)(&(((uint8_t *)it->u.a->data)[i * it->c->element_size]));
        it->data_len = c->element_size;
        it->array_idx = i;
        return it;
      }
    }
     
  }
  else
    printf("unknown container type: %d\n", (int)it->c->type);
  return false;
}

struct fr_iterator fr_iterator_begin(struct fr_container *c)
{
  struct fr_iterator it;
  it.c = c;
  it.array_idx = 0;
  it.data = NULL;
  it.data_len = 0;
  it.u.n = NULL;

  if (!c || !c->u.n)
    return it;

  if (c->type == FR_CT_LIST)
  {
    it.u.n = c->u.n;
    it.data = c->u.n->data;
    it.data_len = c->u.n->data_len;
    return it;
  }
  else if (c->type == FR_CT_ARRAY_LIST)
  {
    it.u.a = c->u.a;
    for (uint32_t i = 0; i < c->u.a->length; i++)
    {
      if (c->u.a->state[i] == FR_CANS_VALID)
      {
        it.data = (void *)(&(((uint8_t *)c->u.a->data)[i*c->element_size]));
        it.data_len = c->element_size;
        it.array_idx = i;
        return it;
      }
    }
    // if we get here, we made it through the first array node without
    // ever finding a valid element. todo: chase through all array nodes
    // so we correctly handle pathological cases.
  }
  // if we get here, we have an unknown container type!
  printf("unknown container type: %d\n", (int)c->type);
  return it;
}

