/* MeshCore wire codec — see mcproto.h for the layout references. */
#include "mcproto.h"
#include <string.h>

bool mc_parse(const uint8_t *buf, int len, mc_pkt_t *o)
{
    if(len < 2) return false;
    uint8_t h = buf[0];
    o->route_type   = h & MC_ROUTE_MASK;
    o->payload_type = (h >> MC_TYPE_SHIFT) & MC_TYPE_MASK;
    o->version      = (h >> MC_VER_SHIFT) & MC_ROUTE_MASK;

    int i = 1;
    bool transport = (o->route_type == MC_ROUTE_TRANSPORT_FLOOD ||
                      o->route_type == MC_ROUTE_TRANSPORT_DIRECT);
    if(transport) { if(i + 4 > len) return false; i += 4; }  /* 2x uint16 codes */

    if(i >= len) return false;
    uint8_t path_len = buf[i++];
    uint8_t hash_count = path_len & 63;
    uint8_t hash_size  = (path_len >> 6) + 1;
    if(hash_size == 4) return false;                          /* reserved */
    int bl = hash_count * hash_size;
    if(bl > 64 || i + bl > len) return false;                 /* MAX_PATH_SIZE */
    i += bl;

    if(i > len) return false;
    o->payload = buf + i;
    o->payload_len = len - i;
    return true;
}

bool mc_advert_decode(const mc_pkt_t *p, mc_advert_t *o)
{
    if(p->payload_type != MC_PAYLOAD_ADVERT) return false;
    if(p->payload_len < 32 + 4 + 64) return false;            /* pubkey+ts+sig */

    memset(o, 0, sizeof *o);
    memcpy(o->pubkey, p->payload, 32);
    memcpy(&o->timestamp, p->payload + 32, 4);                /* LE */

    const uint8_t *app = p->payload + 100;
    int app_len = p->payload_len - 100;
    if(app_len < 1) return true;                              /* valid, no app data */

    uint8_t flags = app[0];
    o->adv_type = flags & 0x0F;
    int i = 1;
    if(flags & 0x10) i += 8;                                  /* lat/lon */
    if(flags & 0x20) i += 2;                                  /* feat1 */
    if(flags & 0x40) i += 2;                                  /* feat2 */
    if((flags & 0x80) && app_len > i) {                       /* name = remainder */
        int nlen = app_len - i;
        if(nlen > (int)sizeof o->name - 1) nlen = sizeof o->name - 1;
        memcpy(o->name, app + i, nlen);
        o->name[nlen] = 0;
        o->has_name = nlen > 0;
    }
    return true;
}

const char *mc_type_name(uint8_t t)
{
    switch(t) {
        case MC_PAYLOAD_REQ:      return "REQ";
        case MC_PAYLOAD_RESPONSE: return "RESP";
        case MC_PAYLOAD_TXT_MSG:  return "TXT";
        case MC_PAYLOAD_ACK:      return "ACK";
        case MC_PAYLOAD_ADVERT:   return "ADVERT";
        case MC_PAYLOAD_GRP_TXT:  return "GRP_TXT";
        case MC_PAYLOAD_GRP_DATA: return "GRP_DATA";
        case MC_PAYLOAD_ANON_REQ: return "ANON_REQ";
        case MC_PAYLOAD_PATH:     return "PATH";
        case MC_PAYLOAD_TRACE:    return "TRACE";
        default:                  return "?";
    }
}
