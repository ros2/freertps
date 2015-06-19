#include "freertps/udp.h"
#include "freertps/spdp.h"
#include "freertps/discovery.h"
#include <limits.h>
#include <string.h>
#include <stdio.h>

const frudp_entity_id_t g_frudp_entity_id_unknown = { .u = 0 };
frudp_config_t g_frudp_config;

static frudp_subscription_t g_frudp_subs[FRUDP_MAX_SUBSCRIPTIONS];
static unsigned g_frudp_subs_used = 0;

static bool frudp_rx_submsg(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg);

#define RX_MSG_ARGS frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg

static bool frudp_rx_acknack       (RX_MSG_ARGS);
static bool frudp_rx_heartbeat     (RX_MSG_ARGS);
static bool frudp_rx_gap           (RX_MSG_ARGS);
static bool frudp_rx_info_ts       (RX_MSG_ARGS);
static bool frudp_rx_info_src      (RX_MSG_ARGS);
static bool frudp_rx_info_reply_ip4(RX_MSG_ARGS);
static bool frudp_rx_dst           (RX_MSG_ARGS);
static bool frudp_rx_reply         (RX_MSG_ARGS);
static bool frudp_rx_nack_frag     (RX_MSG_ARGS);
static bool frudp_rx_heartbeat_frag(RX_MSG_ARGS);
static bool frudp_rx_data          (RX_MSG_ARGS);
static bool frudp_rx_data_frag     (RX_MSG_ARGS);

static bool frudp_rx_data_pl(frudp_receiver_state_t *rcvr, 
                             const frudp_submsg_t *submsg,
                             const uint8_t *payload);

const char *frudp_vendor(const frudp_vid_t vid)
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


//#define RX_VERBOSE

bool frudp_rx(const in_addr_t src_addr, const in_port_t src_port,
              const in_addr_t dst_addr, const in_port_t dst_port,
              const uint8_t *rx_data  , const uint16_t rx_len)
{
#ifdef RX_VERBOSE
  FREERTPS_INFO("freertps rx %d bytes\n", rx_len);
#endif
  const frudp_msg_t *msg = (frudp_msg_t *)rx_data;
  //const frudp_header_t *header = (frudp_header_t *)rx_data;
  if (msg->header.magic_word != 0x53505452) // todo: care about endianness someday
    return false; // it wasn't RTPS. no soup for you.
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx proto ver %d.%d\n", msg->header.pver.major, msg->header.pver.minor);
#endif
  if (msg->header.pver.major != 2)
    return false; // we aren't cool enough to be oldschool
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx vendor 0x%04x = %s\n", 
                (unsigned)ntohs(msg->header.vid), 
                frudp_vendor(ntohs(msg->header.vid)));
#endif
  //frudp_guidprefix_t *guidp = (frudp_guidprefix_t *)(rx_data + 8);
  /*
  for (int i = 0; i < 12; i++)
    printf("guidprefix[%d] = %d = 0x%02x\n", i, (int)(*guidp)[i], (int)(*guidp)[i]);
  */
  // initialize the receiver state
  frudp_receiver_state_t rcvr;
  rcvr.src_pver = msg->header.pver;
  rcvr.src_vid = msg->header.vid;

  bool our_guid = true;
  for (int i = 0; i < 12; i++)
    if (msg->header.guid_prefix.prefix[i] != 
        g_frudp_config.guid_prefix.prefix[i])
      our_guid = false;
  if (our_guid)
    return true; // don't process our own messages

  memcpy(rcvr.src_guid_prefix.prefix, 
         msg->header.guid_prefix.prefix, 
         FRUDP_GUID_PREFIX_LEN);
  rcvr.have_timestamp = false;
  // process all the submessages
  for (const uint8_t *submsg_start = msg->submsgs;
       submsg_start < rx_data + rx_len;)
  {
    const frudp_submsg_t *submsg = (frudp_submsg_t *)submsg_start;
    frudp_rx_submsg(&rcvr, submsg);
    // todo: ensure alignment? if this isn't dword-aligned, we're hosed
    submsg_start += sizeof(frudp_submsg_header_t) + submsg->header.len;
  }
}

static bool frudp_rx_submsg(frudp_receiver_state_t *rcvr, const frudp_submsg_t *submsg)
{
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx submsg ID %d len %d\n", 
                submsg->header.id,
                submsg->header.len);
#endif
  // dispatch to message handlers
  switch (submsg->header.id)
  {
    case 0x01: return true; // pad submessage. ignore (?)
    case 0x06: return frudp_rx_acknack(rcvr, submsg);
    case 0x07: return frudp_rx_heartbeat(rcvr, submsg);
    case 0x08: return frudp_rx_gap(rcvr, submsg);
    case FRUDP_SUBMSG_ID_INFO_TS: return frudp_rx_info_ts(rcvr, submsg);
    case 0x0c: return frudp_rx_info_src(rcvr, submsg);
    case 0x0d: return frudp_rx_info_reply_ip4(rcvr, submsg);
    case 0x0e: return frudp_rx_dst(rcvr, submsg);
    case 0x0f: return frudp_rx_reply(rcvr, submsg);
    case 0x12: return frudp_rx_nack_frag(rcvr, submsg);
    case 0x13: return frudp_rx_heartbeat_frag(rcvr, submsg);
    case FRUDP_SUBMSG_ID_DATA:    return frudp_rx_data(rcvr, submsg);
    case 0x16: return frudp_rx_data_frag(rcvr, submsg);
    default: return false;
  }
  //FREERTPS_INFO("rx
  return true;
}

static bool frudp_rx_acknack(RX_MSG_ARGS)
{
  frudp_submsg_acknack_t *m = (frudp_submsg_acknack_t *)submsg->contents;
  printf("  ACKNACK   reader id = 0x%08x  writer id = 0x%08x\n",
         htonl(m->writer_id.u), 
         htonl(m->reader_id.u));
  return true;
}

static bool frudp_rx_heartbeat(RX_MSG_ARGS)
{
  // todo: care about endianness
  const bool f = submsg->header.flags & 0x02;
  const bool l = submsg->header.flags & 0x04;
  frudp_submsg_heartbeat_t *contents = 
                              (frudp_submsg_heartbeat_t *)submsg->contents;
  printf("  HEARTBEAT reader id = 0x%08x  writer id = 0x%08x\n",
         htonl(contents->writer_id.u), 
         htonl(contents->reader_id.u));
  return true;
}

static bool frudp_rx_gap(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_info_ts(RX_MSG_ARGS)
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
    rcvr->timestamp = *((fr_time_t *)(submsg->contents));
    /*
    FREERTPS_INFO("info_ts rx timestamp %.6f\n", 
                  (double)(rcvr->timestamp.seconds) + 
                  ((double)(rcvr->timestamp.fraction)) / ULONG_MAX);
    */
  }
  return true;
}

static bool frudp_rx_info_src(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_info_reply_ip4(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_dst(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_reply(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_nack_frag(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_heartbeat_frag(RX_MSG_ARGS)
{
  return true;
}

static bool frudp_rx_data(RX_MSG_ARGS)
{
  frudp_submsg_contents_data_t *contents = (frudp_submsg_contents_data_t *)submsg->contents;
#ifdef RX_VERBOSE
  FREERTPS_INFO("rx data flags = %d\n", 0x0f7 & submsg->header.flags);
#endif
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
    frudp_parameter_list_item_t *item = (frudp_parameter_list_item_t *)inline_qos_start;
    while ((uint8_t *)item < submsg->contents + submsg->header.len)
    {
#ifdef RX_VERBOSE
      FREERTPS_INFO("data inline QoS param 0x%x len %d\n", (unsigned)item->pid, item->len);
#endif
      const frudp_parameterid_t pid = item->pid;
      const uint8_t *pval = item->value;
      // todo: process parameter value
      item = (frudp_parameter_list_item_t *)(((uint8_t *)item) + 4 + item->len);
      if (pid == FRUDP_PID_SENTINEL)
        break; // adios
    }
    data_start = (uint8_t *)item; // after a PID_SENTINEL, this is correct
  }
  const uint16_t scheme = ntohs(*((uint16_t *)data_start));
  //printf("rx scheme = 0x%04x\n", scheme);
  uint8_t *data = data_start + 4;
  // spin through subscriptions and see if anyone is listening
  printf("  DATA reader id = 0x%08x  writer id = 0x%08x\n",
         htonl(contents->writer_id.u), 
         htonl(contents->reader_id.u));
  for (unsigned i = 0; i < g_frudp_subs_used; i++)
  {
    if (g_frudp_subs[i].writer_id.u == contents->writer_id.u &&
        g_frudp_subs[i].reader_id.u == contents->reader_id.u)
      g_frudp_subs[i].cb(rcvr, submsg, scheme, data);
  }
  //FREERTPS_ERROR("  ahh unknown data scheme: 0x%04x\n", (unsigned)scheme);
  return true;
}

static bool frudp_rx_data_pl(frudp_receiver_state_t *rcvr, 
                             const frudp_submsg_t *submsg,
                             const uint8_t *payload)
{
  FREERTPS_INFO("  rx data pl\n");
  return true;
}


static bool frudp_rx_data_frag(RX_MSG_ARGS)
{
  return true;
}

bool frudp_subscribe(const frudp_entity_id_t reader_id,
                     const frudp_entity_id_t writer_id,
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

bool frudp_guid_prefix_identical(frudp_guid_prefix_t * const a,
                                 frudp_guid_prefix_t * const b)
{
  for (int i = 0; i < FRUDP_GUID_PREFIX_LEN; i++)
    if (a->prefix[i] != b->prefix[i])
      return false;
  return true;
}

bool frudp_generic_init()
{
  FREERTPS_INFO("frudp_generic_init()\n");
  frudp_add_mcast_rx(htonl(FRUDP_DEFAULT_MCAST_GROUP), 
                     frudp_mcast_builtin_port());
  frudp_add_mcast_rx(htonl(FRUDP_DEFAULT_MCAST_GROUP), 
                     frudp_mcast_user_port());
  frudp_add_ucast_rx(frudp_ucast_user_port());
  return true;
}

uint16_t frudp_mcast_builtin_port()
{
  return FRUDP_PORT_PB + 
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D0;
}

uint16_t frudp_ucast_builtin_port()
{
  return FRUDP_PORT_PB + 
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D1 +
         FRUDP_PORT_PG * g_frudp_config.participant_id;
}

uint16_t frudp_mcast_user_port()
{
  return FRUDP_PORT_PB + 
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D2;
}

uint16_t frudp_ucast_user_port()
{
  return FRUDP_PORT_PB + 
         FRUDP_PORT_DG * FRUDP_DOMAIN_ID +
         FRUDP_PORT_D3 +
         FRUDP_PORT_PG * g_frudp_config.participant_id;
}

const char *frudp_ip4_ntoa(const uint32_t addr)
{
  static uint8_t ntoa_buf[20]; 
  snprintf(ntoa_buf, sizeof(ntoa_buf), "%d.%d.%d.%d",
           (addr      ) & 0xff,
           (addr >>  8) & 0xff,
           (addr >> 16) & 0xff,
           (addr >> 24) & 0xff);
  return ntoa_buf;
}

bool frudp_parse_string(char *buf, uint32_t buf_len, frudp_rtps_string_t *s)
{
  int wpos = 0;
  for (; wpos < s->len && wpos < buf_len-1; wpos++)
    buf[wpos] = s->data[wpos];
  buf[wpos] = 0;
  if (wpos < buf_len - 1)
    return true;
  else
    return false; // couldn't fit entire string in buffer
}

frudp_msg_t *frudp_init_msg(uint8_t *buf)
{
  frudp_msg_t *msg = (frudp_msg_t *)buf; 
  msg->header.magic_word = 0x53505452;
  msg->header.pver.major = 2;
  msg->header.pver.minor = 1;
  msg->header.vid = FREERTPS_VENDOR_ID;
  memcpy(msg->header.guid_prefix.prefix, 
         g_frudp_config.guid_prefix.prefix, 
         FRUDP_GUID_PREFIX_LEN);
  g_frudp_discovery_tx_buf_wpos = 0;
  return msg;
}

