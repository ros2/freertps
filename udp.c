#include "freertps/udp.h"
#include <limits.h>
#include <string.h>

static frudp_subscription_t g_frudp_subs[FRUDP_MAX_SUBSCRIPTIONS];
static unsigned g_frudp_subs_used = 0;

static bool fu_rx_submsg(fu_receiver_state_t *rcvr, const fu_submsg_t *submsg);

#define RX_MSG_ARGS fu_receiver_state_t *rcvr, const fu_submsg_t *submsg

static bool fu_rx_acknack       (RX_MSG_ARGS);
static bool fu_rx_heartbeat     (RX_MSG_ARGS);
static bool fu_rx_gap           (RX_MSG_ARGS);
static bool fu_rx_info_ts       (RX_MSG_ARGS);
static bool fu_rx_info_src      (RX_MSG_ARGS);
static bool fu_rx_info_reply_ip4(RX_MSG_ARGS);
static bool fu_rx_dst           (RX_MSG_ARGS);
static bool fu_rx_reply         (RX_MSG_ARGS);
static bool fu_rx_nack_frag     (RX_MSG_ARGS);
static bool fu_rx_heartbeat_frag(RX_MSG_ARGS);
static bool fu_rx_data          (RX_MSG_ARGS);
static bool fu_rx_data_frag     (RX_MSG_ARGS);

static bool frudp_rx_data_pl(fu_receiver_state_t *rcvr, 
                             const fu_submsg_t *submsg,
                             const uint8_t *payload);

const char *fu_vendor(const fu_vid_t vid)
{
  switch (vid)
  {
    case 0x0101: return "RTI Connext";
    case 0x0102: return "PrismTech OpenSplice";
    case 0x0103: return "OCI OpenDDS";
    case 0x0104: return "MilSoft";
    case 0x0105: return "Gallium InterCOM";
    case 0x0106: return "TwinOaks CoreDX";
    case 0x0107: return "Lakota Technical Systems";
    case 0x0108: return "ICOUP Consulting";
    case 0x0109: return "ETRI";
    case 0x010a: return "RTI Connext Micro";
    case 0x010b: return "PrismTech Vortex Cafe";
    case 0x010c: return "PrismTech Vortex Gateway";
    case 0x010d: return "PrismTech Vortex Lite";
    case 0x010e: return "Technicolor Qeo";
    case 0x010f: return "eProsima";
    case 0x0120: return "PrismTech Vortex Cloud";
    case FREERTPS_VENDOR_ID: return "freertps";
    default:     return "unknown";
  }
}


bool fu_rx(const in_addr_t src_addr,
           const in_port_t src_port,
           const uint8_t *rx_data,
           const uint16_t rx_len)
{
  FREERTPS_INFO("freertps rx %d bytes\n", rx_len);
  const fu_msg_t *msg = (fu_msg_t *)rx_data;
  //const fu_header_t *header = (fu_header_t *)rx_data;
  if (msg->header.magic_word != 0x53505452) // todo: care about endianness someday
    return false; // it wasn't RTPS. no soup for you.
  FREERTPS_INFO("rx proto ver %d.%d\n", msg->header.pver.major, msg->header.pver.minor);
  if (msg->header.pver.major != 2)
    return false; // we aren't cool enough to be oldschool
  FREERTPS_INFO("rx vendor 0x%04x = %s\n", 
                (unsigned)ntohs(msg->header.vid), 
                fu_vendor(ntohs(msg->header.vid)));
  //fu_guidprefix_t *guidp = (fu_guidprefix_t *)(rx_data + 8);
  /*
  for (int i = 0; i < 12; i++)
    printf("guidprefix[%d] = %d = 0x%02x\n", i, (int)(*guidp)[i], (int)(*guidp)[i]);
  */
  // initialize the receiver state
  fu_receiver_state_t rcvr;
  rcvr.src_pver = msg->header.pver;
  rcvr.src_vid = msg->header.vid;
  memcpy(rcvr.src_guidprefix, msg->header.guidprefix, FU_GUIDPREFIX_LEN);
  rcvr.have_timestamp = false;
  // process all the submessages
  for (const uint8_t *submsg_start = msg->submsgs;
       submsg_start < rx_data + rx_len;)
  {
    const fu_submsg_t *submsg = (fu_submsg_t *)submsg_start;
    fu_rx_submsg(&rcvr, submsg);
    // todo: ensure alignment? if this isn't dword-aligned, we're hosed
    submsg_start += sizeof(fu_submsg_header_t) + submsg->header.len;
  }
}

static bool fu_rx_submsg(fu_receiver_state_t *rcvr, const fu_submsg_t *submsg)
{
  FREERTPS_INFO("rx submsg ID %d len %d\n", 
                submsg->header.id,
                submsg->header.len);
  // dispatch to message handlers
  switch (submsg->header.id)
  {
    case 0x01: return true; // pad submessage. ignore (?)
    case 0x06: return fu_rx_acknack(rcvr, submsg);
    case 0x07: return fu_rx_heartbeat(rcvr, submsg);
    case 0x08: return fu_rx_gap(rcvr, submsg);
    case 0x09: return fu_rx_info_ts(rcvr, submsg);
    case 0x0c: return fu_rx_info_src(rcvr, submsg);
    case 0x0d: return fu_rx_info_reply_ip4(rcvr, submsg);
    case 0x0e: return fu_rx_dst(rcvr, submsg);
    case 0x0f: return fu_rx_reply(rcvr, submsg);
    case 0x12: return fu_rx_nack_frag(rcvr, submsg);
    case 0x13: return fu_rx_heartbeat_frag(rcvr, submsg);
    case 0x15: return fu_rx_data(rcvr, submsg);
    case 0x16: return fu_rx_data_frag(rcvr, submsg);
    default: return false;
  }
  //FREERTPS_INFO("rx
  return true;
}

static bool fu_rx_acknack(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_heartbeat(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_gap(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_info_ts(RX_MSG_ARGS)
{
  const bool invalidate = submsg->header.flags & 0x02;
  if (invalidate)
  {
    rcvr->have_timestamp = false;
    rcvr->timestamp.seconds = -1;
    rcvr->timestamp.fraction = 0xffffffff;
  }
  else
  {
    rcvr->have_timestamp = true;
    // todo: care about alignment
    rcvr->timestamp = *((fu_time_t *)(submsg->contents));
    /*
    FREERTPS_INFO("info_ts rx timestamp %.6f\n", 
                  (double)(rcvr->timestamp.seconds) + 
                  ((double)(rcvr->timestamp.fraction)) / ULONG_MAX);
    */
  }
  return true;
}

static bool fu_rx_info_src(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_info_reply_ip4(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_dst(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_reply(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_nack_frag(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_heartbeat_frag(RX_MSG_ARGS)
{
  return true;
}

static bool fu_rx_data(RX_MSG_ARGS)
{
  fu_submsg_contents_data_t *contents = (fu_submsg_contents_data_t *)submsg->contents;
  FREERTPS_INFO("rx data flags = %d\n", 0x0f7 & submsg->header.flags);
  // todo: care about endianness
  const bool q = submsg->header.flags & 0x02;
  const bool d = submsg->header.flags & 0x04;
  const bool k = submsg->header.flags & 0x08;
  if (k)
  {
    FREERTPS_ERROR("ahhhh i don't know how to handle keyed data yet\n");
    return false;
  }
  uint8_t *inline_qos_start = (uint8_t *)(&contents->octets_to_inline_qos) + 
                              sizeof(contents->octets_to_inline_qos) + 
                              contents->octets_to_inline_qos;
  uint8_t *data_start = inline_qos_start;
  if (q)
  {
    // first parse out the QoS parameters
    fu_parameter_list_item_t *item = (fu_parameter_list_item_t *)inline_qos_start;
    while ((uint8_t *)item < submsg->contents + submsg->header.len)
    {
      FREERTPS_INFO("data inline QoS param 0x%x len %d\n", (unsigned)item->pid, item->len);
      const fu_parameterid_t pid = item->pid;
      const uint8_t *pval = item->value;
      // todo: process parameter value
      item = (fu_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
      if (pid == FU_PID_SENTINEL)
        break; // adios
    }
    data_start = (uint8_t *)item; // after a PID_SENTINEL, this is correct
  }
  const uint16_t scheme = ntohs(*((uint16_t *)data_start));
  uint8_t *data = data_start + 4;
  // spin through subscriptions and see if anyone is listening
  for (unsigned i = 0; i < g_frudp_subs_used; i++)
  {
    if (g_frudp_subs[i].writer_id.u == contents->writer_id.u &&
        g_frudp_subs[i].reader_id.u == contents->reader_id.u)
      g_frudp_subs[i].cb(rcvr, submsg, scheme, data);
  }
  //FREERTPS_ERROR("  ahh unknown data scheme: 0x%04x\n", (unsigned)scheme);
  return true;
}

static bool frudp_rx_data_pl(fu_receiver_state_t *rcvr, 
                             const fu_submsg_t *submsg,
                             const uint8_t *payload)
{
  FREERTPS_INFO("  rx data pl\n");
  return true;
}


static bool fu_rx_data_frag(RX_MSG_ARGS)
{
  return true;
}

bool frudp_subscribe(const frudp_entityid_t reader_id,
                     const frudp_entityid_t writer_id,
                     const frudp_rx_cb_t cb)
{
  if (g_frudp_subs_used >= FRUDP_MAX_SUBSCRIPTIONS)
    return false;
  frudp_subscription_t *sub = &g_frudp_subs[g_frudp_subs_used];
  sub->reader_id = reader_id;
  sub->writer_id = writer_id;
  sub->cb = cb;
  g_frudp_subs_used++;
  return true;
}

