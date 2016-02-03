#include <stddef.h>
#include "freertps/container.h"
#include "freertps/iterator.h"
#include "freertps/mem.h"
#include <stdio.h>
#include <string.h>

static bool fr_container_node_set(struct fr_container_node *n,
    void *data, size_t len)
{
  if (!n)
    return false;
  if (n->state == FR_CNS_NEW)
  {
    n->data = fr_malloc(len); // allocate the requested size
    n->alloc_len = len;
  }
  else if (n->state == FR_CNS_READONLY)
  {
    printf("HEY! can't set a read-only container node...\n");
    return false;
  }
  else if (n->alloc_len < len)
  {
    printf("HEY! data object of size %d won't fit into %d bytes\n",
        (int)len, (int)n->alloc_len);
    return false;
  }
  // if we get here, we know alloc_len >= len
  memcpy(n->data, data, len);
  n->data_len = len;
  n->state = FR_CNS_VALID;
  return true;
}

static struct fr_container_node *fr_container_node_create(
    void *data, size_t len)
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

bool fr_container_append(struct fr_container *c, void *data, size_t len)
{
  if (c->type == FR_CT_LIST)
  {
    if (c->u.n == NULL) // if this list is empty. allocate the first node
    {
      c->u.n = fr_container_node_create(data, len);
      return fr_container_node_set(c->u.n, data, len);
    }
    // find the end of the list, or an invalid currently-allocated block
    // that is the same size as this object
    struct fr_container_node *n = c->u.n;
    while (n->next != NULL)
    {
      if (n->state == FR_CNS_UNUSED && n->alloc_len >= len)
        return fr_container_node_set(n, data, len);
      n = n->next;
    }
    // if we get here, we're at the tail of the list
    n->next = fr_container_node_create(data, len);
    return fr_container_node_set(n->next, data, len);
  }
  else if (c->type == FR_CT_ARRAY_LIST)
  {
    return false;
  }
  else
  {
    printf("unknown container type: %d\n", (int)c->type);
    return false;
  }
}

uint32_t fr_container_len(struct fr_container *c)
{
  if (c == NULL)
    return 0;
  uint32_t len = 0;
  if (c->type == FR_CT_LIST)
  {
    struct fr_container_node *n = c->u.n;
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
    return 0;
  }
  else
  {
    printf("unknown container type: %d\n", (int)c->type);
    return false;
  }
}

struct fr_container *fr_container_create(const uint32_t type)
{
  printf("** sizeof(struct fr_container) = %d\n", (int)sizeof(struct fr_container));
  struct fr_container *c =
      (struct fr_container *)fr_malloc(sizeof(struct fr_container));
  c->type = type;
  c->u.n = NULL;
  return c;
}

