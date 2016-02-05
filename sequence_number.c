#include "freertps/sequence_number.h"

const fr_sequence_number_t g_fr_sequence_number_unknown = 
    { .high = -1, .low = 0 };

void fr_sequence_number_increment(struct fr_sequence_number *seq_num)
{
  if (seq_num->low == 0xffffffff)
    seq_num->high++;
  seq_num->low++;
}
