/**
 * MeshCore DM key agreement: X25519 ECDH derived from the Ed25519 identities,
 * byte-compatible with orlp/ed25519 ed25519_key_exchange (what MeshCore uses).
 *
 *   shared = X25519( clamp(SHA512(our_seed)[:32]),  edwards->montgomery(peer_ed_pub) )
 *
 * The 32-byte result is the raw DM shared secret: AES-128 key = shared[0..15],
 * HMAC key = shared[0..31].
 */
#ifndef LZ_MC_X25519_H
#define LZ_MC_X25519_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void mc_ed25519_dh(uint8_t shared[32], const uint8_t peer_ed_pub[32], const uint8_t our_seed[32]);

#ifdef __cplusplus
}
#endif

#endif
