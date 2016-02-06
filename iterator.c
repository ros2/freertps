#include <stdio.h>
#include "freertps/container.h"
#include "freertps/iterator.h"

bool fr_iterator_next(struct fr_iterator *it)
{
  if (!it || !it->u.n)
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
    it->data = NULL;
    it->data_len = 0;
    return false;
  }
  else if (it->c->type == FR_CT_ARRAY_LIST)
  {
    struct fr_container_array_list *ca =
        (struct fr_container_array_list *)it->c;
    do
    {
      for (uint32_t i = it->array_idx + 1; i < it->u.a->length; i++)
      {
        if (it->u.a->state[i] == FR_CANS_VALID)
        {
          it->data = 
            (void *)(&(((uint8_t *)it->u.a->data)[i * ca->element_size]));
          it->data_len = ca->element_size;
          it->array_idx = i;
          return true;
        }
      }
      // if we get here, we made it to the end of the current array
      // without finding the next valid element. Try to chain to the
      // next array block...
      it->array_idx = 0;
      it->u.a = it->u.a->next;
    } while (it->u.a);
    it->data = NULL;
    it->data_len = 0;
    return false;
  }
  else
    printf("unknown container type: %d\n", (int)it->c->type);
  it->data = NULL;
  it->data_len = 0;
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

  if (!c)
    return it;

  if (c->type == FR_CT_LIST)
  {
    struct fr_container_list *cl = (struct fr_container_list *)c;
    it.u.n = cl->head;
    it.data = cl->head->data;
    it.data_len = cl->head->data_len;
    return it;
  }
  else if (c->type == FR_CT_ARRAY_LIST)
  {
    struct fr_container_array_list *ca = (struct fr_container_array_list *)c;
    it.u.a = &ca->head;
    for (uint32_t i = 0; i < ca->head.length; i++)
    {
      if (ca->head.state[i] == FR_CANS_VALID)
      {
        it.data = (void *)(&(((uint8_t *)ca->head.data)[i*ca->element_size]));
        it.data_len = ca->element_size;
        it.array_idx = i;
        //printf("  iterator initial array cell = %d\n", (int)i);
        return it;
      }
    }
    // if we get here, we made it through the first array node without
    // ever finding a valid element. todo: chase through all array nodes
    // so we correctly handle pathological cases.
    return it;
  }
  // if we get here, we have an unknown container type!
  printf("unknown container type: %d\n", (int)c->type);
  return it;
}

