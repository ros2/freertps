#include "freertps/container.h"
#include "freertps/locator.h"
#include "freertps/mem.h"
#include "freertps/reader_locator.h"

struct fr_reader_locator *fr_reader_locator_create()
{
  struct fr_reader_locator *rl = fr_malloc(sizeof(struct fr_reader_locator));
  fr_reader_locator_init(rl);
  return rl;
}

void fr_reader_locator_init(struct fr_reader_locator *rl)
{
  rl->locator.kind = FR_LOCATOR_KIND_INVALID;
  rl->highest_seq_num_sent = FR_SEQUENCE_NUMBER_UNKNOWN;
  rl->lowest_requested_change = FR_SEQUENCE_NUMBER_UNKNOWN;
  rl->expects_inline_qos = false;
}

