/* -*- mode: C; c-basic-offset: 4; intent-tabs-mode: nil -*-
 *
 * This file is part of the public interface to the Sifteo SDK.
 * Copyright <c> 2012 Sifteo, Inc. All rights reserved.
 */

#ifndef _SIFTEO_ABI_ASSET_H
#define _SIFTEO_ABI_ASSET_H

#include <sifteo/abi/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Asset loading
 */

#define _SYS_ASSETLOAD_BUF_SIZE  48   // Makes _SYSAssetLoaderCube come to 64 bytes

struct _SYSAssetGroupHeader {
    uint8_t reserved;           /// OUT    Reserved, must be zero
    uint8_t ordinal;            /// OUT    Small integer, unique within an ELF
    uint16_t numTiles;          /// OUT    Uncompressed size, in tiles
    uint32_t dataSize;          /// OUT    Size of compressed data, in bytes
    uint64_t hash;              /// OUT    Hash of this asset group's data
    // Followed by compressed data
};

struct _SYSAssetGroupCube {
    uint16_t baseAddr;          /// IN     Installed base address, in tiles
};

struct _SYSAssetGroup {
    uint32_t pHdr;              /// OUT    Read-only data for this asset group
    // Followed by a _SYSAssetGroupCube array
};

struct _SYSAssetLoaderCube {
    uint32_t pAssetGroup;   /// IN    Address for _SYSAssetGroup in RAM
    uint32_t progress;      /// IN    Number of compressed bytes read from flash
    uint32_t dataSize;      /// IN    Local copy of asset group's dataSize
    uint16_t reserved;      /// -
    uint8_t head;           /// -     Index of the next sample to read
    uint8_t tail;           /// -     Index of the next empty slot to write into
    uint8_t buf[_SYS_ASSETLOAD_BUF_SIZE];
};

struct _SYSAssetLoader {
    _SYSCubeIDVector cubeVec;   /// OUT   Which _SYSAssetLoaderCube structs are valid?
    _SYSCubeIDVector complete;  /// OUT   Which cubes have fully completed their loading?
    // Followed by a _SYSAssetLoaderCube array
};

enum _SYSAssetImageFormat {
    _SYS_AIF_PINNED = 0,        /// All tiles are linear. "data" is index of the first tile
    _SYS_AIF_FLAT,              /// "data" points to a flat array of 16-bit tile indices
    _SYS_AIF_DUB_I8,            /// Dictionary Uniform Block codec, 8-bit index
    _SYS_AIF_DUB_I16,           /// Dictionary Uniform Block codec, 16-bit index
};

struct _SYSAssetImage {
    uint32_t pAssetGroup;       /// Address for _SYSAssetGroup in RAM, 0 for pre-relocated
    uint16_t width;             /// Width of the asset image, in tiles
    uint16_t height;            /// Height of the asset image, in tiles
    uint16_t frames;            /// Number of "frames" in this image
    uint8_t  format;            /// _SYSAssetImageFormat
    uint8_t  reserved;          /// Reserved, must be zero
    uint32_t pData;             /// Format-specific data or data pointer
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
