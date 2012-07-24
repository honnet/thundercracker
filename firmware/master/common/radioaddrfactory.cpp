/*
 * Thundercracker Firmware -- Confidential, not for redistribution.
 * Copyright <c> 2012 Sifteo, Inc. All rights reserved.
 */

#include <protocol.h>
#include "radioaddrfactory.h"


const uint8_t RadioAddrFactory::gf84[0x100] = {
    // Lookup table for Galois Field multiplication by our chosen polynomial, 0x84.
    0x00, 0x84, 0x13, 0x97, 0x26, 0xa2, 0x35, 0xb1, 0x4c, 0xc8, 0x5f, 0xdb, 0x6a, 0xee, 0x79, 0xfd,
    0x98, 0x1c, 0x8b, 0x0f, 0xbe, 0x3a, 0xad, 0x29, 0xd4, 0x50, 0xc7, 0x43, 0xf2, 0x76, 0xe1, 0x65,
    0x2b, 0xaf, 0x38, 0xbc, 0x0d, 0x89, 0x1e, 0x9a, 0x67, 0xe3, 0x74, 0xf0, 0x41, 0xc5, 0x52, 0xd6,
    0xb3, 0x37, 0xa0, 0x24, 0x95, 0x11, 0x86, 0x02, 0xff, 0x7b, 0xec, 0x68, 0xd9, 0x5d, 0xca, 0x4e,
    0x56, 0xd2, 0x45, 0xc1, 0x70, 0xf4, 0x63, 0xe7, 0x1a, 0x9e, 0x09, 0x8d, 0x3c, 0xb8, 0x2f, 0xab,
    0xce, 0x4a, 0xdd, 0x59, 0xe8, 0x6c, 0xfb, 0x7f, 0x82, 0x06, 0x91, 0x15, 0xa4, 0x20, 0xb7, 0x33,
    0x7d, 0xf9, 0x6e, 0xea, 0x5b, 0xdf, 0x48, 0xcc, 0x31, 0xb5, 0x22, 0xa6, 0x17, 0x93, 0x04, 0x80,
    0xe5, 0x61, 0xf6, 0x72, 0xc3, 0x47, 0xd0, 0x54, 0xa9, 0x2d, 0xba, 0x3e, 0x8f, 0x0b, 0x9c, 0x18,
    0xac, 0x28, 0xbf, 0x3b, 0x8a, 0x0e, 0x99, 0x1d, 0xe0, 0x64, 0xf3, 0x77, 0xc6, 0x42, 0xd5, 0x51,
    0x34, 0xb0, 0x27, 0xa3, 0x12, 0x96, 0x01, 0x85, 0x78, 0xfc, 0x6b, 0xef, 0x5e, 0xda, 0x4d, 0xc9,
    0x87, 0x03, 0x94, 0x10, 0xa1, 0x25, 0xb2, 0x36, 0xcb, 0x4f, 0xd8, 0x5c, 0xed, 0x69, 0xfe, 0x7a,
    0x1f, 0x9b, 0x0c, 0x88, 0x39, 0xbd, 0x2a, 0xae, 0x53, 0xd7, 0x40, 0xc4, 0x75, 0xf1, 0x66, 0xe2,
    0xfa, 0x7e, 0xe9, 0x6d, 0xdc, 0x58, 0xcf, 0x4b, 0xb6, 0x32, 0xa5, 0x21, 0x90, 0x14, 0x83, 0x07,
    0x62, 0xe6, 0x71, 0xf5, 0x44, 0xc0, 0x57, 0xd3, 0x2e, 0xaa, 0x3d, 0xb9, 0x08, 0x8c, 0x1b, 0x9f,
    0xd1, 0x55, 0xc2, 0x46, 0xf7, 0x73, 0xe4, 0x60, 0x9d, 0x19, 0x8e, 0x0a, 0xbb, 0x3f, 0xa8, 0x2c,
    0x49, 0xcd, 0x5a, 0xde, 0x6f, 0xeb, 0x7c, 0xf8, 0x05, 0x81, 0x16, 0x92, 0x23, 0xa7, 0x30, 0xb4
};


void RadioAddrFactory::random(RadioAddress &addr, _SYSPseudoRandomState &prng)
{
    addr.channel = PRNG::valueBounded(&prng, MAX_RF_CHANNEL);

    for (unsigned i = 0; i < arraysize(addr.id); ++i) {
        unsigned value = PRNG::valueBounded(&prng, 255 - 4) + 1;
        if (value >= 0x55) value++;
        if (value >= 0xAA) value++;
        ASSERT(allowedByte(value));
        addr.id[i] = value;
    }
}


void RadioAddrFactory::fromHardwareID(RadioAddress &addr, uint64_t hwid)
{
    uint8_t reg;

    // Feed first two bytes into CRC, to collect entropy
    reg =      0xB4 ^ hwid; hwid >>= 8;
    reg = gf84[reg] ^ hwid; hwid >>= 8;

    for (unsigned i = 0; i < 5; ++i) {
        // Each of the next five bytes clock out an address byte
        reg = gf84[reg] ^ hwid; hwid >>= 8;

        // Explicitly avoid disallowed values
        while (!allowedByte(reg))
            reg = ~gf84[reg];

        addr.id[i] = reg;
    }

    // Pick a legal channel, using the last entropy byte
    reg = gf84[reg] ^ hwid; hwid >>= 8;

    while (1) {
        addr.channel = reg & 0x7F;
        if (addr.channel <= MAX_RF_CHANNEL)
            break;
        reg = ~gf84[reg];
    }
}

void RadioAddrFactory::channelToggle(RadioAddress &addr)
{
    if ((addr.channel += MAX_RF_CHANNEL / 2) > MAX_RF_CHANNEL)
        addr.channel -= MAX_RF_CHANNEL + 1;
}
