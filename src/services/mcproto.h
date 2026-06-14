/**
 * Minimal MeshCore wire codec — enough to recognize MeshCore packets on the
 * air and decode ADVERT packets (which are signed but NOT encrypted, so they
 * carry a node's public key, type, and name in the clear).
 *
 * Constants + layout verified against meshcore-dev/MeshCore (src/Packet.cpp,
 * src/Mesh.h, src/helpers/AdvertDataHelpers.cpp), 2026-06:
 *   on-air: [header][transport_codes(4 if route 0/3)][path_len][path][payload]
 *   header: route(bits0-1) | payload_type(bits2-5) | version(bits6-7)
 *   ADVERT payload: pubkey[32] timestamp[4 LE] signature[64] app_data[<=32]
 *   app_data: flags[1] [latlon 8 if 0x10] [feat1 2 if 0x20] [feat2 2 if 0x40]
 *             [name rest if 0x80];  type = flags & 0x0F
 */
#ifndef LZ_MCPROTO_H
#define LZ_MCPROTO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MC_ROUTE_MASK             0x03
#define MC_TYPE_SHIFT             2
#define MC_TYPE_MASK              0x0F
#define MC_VER_SHIFT              6

#define MC_ROUTE_TRANSPORT_FLOOD  0x00
#define MC_ROUTE_FLOOD            0x01
#define MC_ROUTE_DIRECT           0x02
#define MC_ROUTE_TRANSPORT_DIRECT 0x03

#define MC_PAYLOAD_REQ            0x00
#define MC_PAYLOAD_RESPONSE       0x01
#define MC_PAYLOAD_TXT_MSG        0x02
#define MC_PAYLOAD_ACK            0x03
#define MC_PAYLOAD_ADVERT         0x04
#define MC_PAYLOAD_GRP_TXT        0x05
#define MC_PAYLOAD_GRP_DATA       0x06
#define MC_PAYLOAD_ANON_REQ       0x07
#define MC_PAYLOAD_PATH           0x08
#define MC_PAYLOAD_TRACE          0x09

#define MC_ADV_TYPE_CHAT          1
#define MC_ADV_TYPE_REPEATER      2
#define MC_ADV_TYPE_ROOM          3
#define MC_ADV_TYPE_SENSOR        4

typedef struct {
    uint8_t        route_type;
    uint8_t        payload_type;
    uint8_t        version;
    const uint8_t *payload;
    int            payload_len;
} mc_pkt_t;

typedef struct {
    uint8_t  pubkey[32];
    uint32_t timestamp;
    int      adv_type;          /* MC_ADV_TYPE_* */
    bool     has_name;
    char     name[32];
} mc_advert_t;

/* a decoded group/channel text message (GRP_TXT) */
typedef struct {
    uint32_t timestamp;
    uint8_t  flags;
    char     sender[32];        /* parsed from "sender: text"; empty if absent */
    char     text[176];
} mc_group_msg_t;

/* The default "Public" channel: 16-byte AES-128 secret (PSK izOH6cXN6mrJ5e26oRXNcg==)
 * and its packet channel-hash = SHA256(secret)[0]. */
extern const uint8_t MC_PUBLIC_SECRET[16];
#define MC_PUBLIC_CHANNEL_HASH 0x11

/* a decoded direct message (TXT_MSG). Unlike GRP_TXT, the plaintext carries no
 * sender name — the sender is identified by src_hash + the matched contact. */
typedef struct {
    uint32_t timestamp;
    uint8_t  txt_type;          /* 0 PLAIN, 1 CLI_DATA, 2 SIGNED_PLAIN */
    uint8_t  attempt;
    uint8_t  src_hash;          /* first byte of sender's pubkey */
    char     text[176];
} mc_dm_msg_t;

#define MC_TXT_TYPE_PLAIN        0
#define MC_TXT_TYPE_CLI_DATA     1
#define MC_TXT_TYPE_SIGNED_PLAIN 2

/* parse the on-air framing; false if the header/path are malformed */
bool mc_parse(const uint8_t *buf, int len, mc_pkt_t *out);
/* decode an ADVERT packet's identity + name; false if not an advert/too short */
bool mc_advert_decode(const mc_pkt_t *p, mc_advert_t *out);
/* the channel-hash byte a GRP_TXT/GRP_DATA payload is tagged with (payload[0]) */
uint8_t mc_group_channel_hash(const mc_pkt_t *p);
/* decode a GRP_TXT payload with a 16-byte channel secret: verify the MAC and
 * AES-128-ECB decrypt. false if not a group text, malformed, or MAC mismatch. */
bool mc_group_decode(const mc_pkt_t *p, const uint8_t secret16[16], mc_group_msg_t *out);
/* build a GRP_TXT on-air frame (FLOOD route, path_len 0). Returns length or -1. */
int  mc_group_encode(uint8_t *frame, int cap, const uint8_t secret16[16],
                     uint32_t timestamp, const char *sender, const char *text);

/* ---- direct messages (TXT_MSG). shared32 = X25519 ECDH secret (mc_ed25519_dh).
 * AES-128 key = shared32[0..15]; HMAC key = the full 32B (NOT zero-padded). ---- */
/* the 4-byte delivery checksum = SHA256( [ts:4][typebyte:1][text, no NUL] || sender_pub[32] )[:4] */
void mc_dm_ack4(uint8_t out4[4], uint32_t timestamp, uint8_t type_byte,
                const char *text, const uint8_t sender_pub[32]);
/* decode a TXT_MSG payload: verify MAC + AES-128-ECB decrypt. false if not a DM,
 * malformed, wrong payload version, or MAC mismatch (=> wrong peer/channel). */
bool mc_dm_decode(const mc_pkt_t *p, const uint8_t shared32[32], mc_dm_msg_t *out);
/* build a TXT_MSG frame (FLOOD route, path_len 0). Returns length or -1, and
 * writes the 4-byte expected-ACK so the sender can match the peer's reply. */
int  mc_dm_encode(uint8_t *frame, int cap, const uint8_t shared32[32],
                  uint8_t dest_hash, uint8_t src_hash, uint32_t timestamp,
                  uint8_t txt_type, uint8_t attempt, const char *text,
                  const uint8_t sender_pub[32], uint8_t out_ack4[4]);
/* human label for a payload type (for diagnostics) */
const char *mc_type_name(uint8_t payload_type);

#ifdef __cplusplus
}
#endif

#endif
