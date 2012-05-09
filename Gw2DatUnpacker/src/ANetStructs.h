// File: ANetStructs.h

#pragma once

#ifndef ANETSTRUCTS_H_INCLUDED
#define ANETSTRUCTS_H_INCLUDED

#include "stdafx.h"

namespace gw2du
{

enum ANetFileType
{
    ANFT_Unknown,

    // Texture types
    ANFT_TextureStart,
    ANFT_ATEX,
    ANFT_ATTX,
    ANFT_ATEC,
    ANFT_ATEP,
    ANFT_ATEU,
    ANFT_ATET,
    ANFT_DDS,
    ANFT_TextureEnd,

    // Sound
    ANFT_Sound,
    ANFT_MP3,
    ANFT_OGG,

    // PF
    ANFT_PF,
    ANFT_Manifest,
    ANFT_Bank,
    ANFT_Model,
    ANFT_DependencyTable,
    ANFT_EULA,
    ANFT_HavokCloth,
    ANFT_Map,
    ANFT_Material,
    ANFT_Cinematic,

    // Binary
    ANFT_Binary,
    ANFT_DLL,
    ANFT_EXE,

    // Misc
    ANFT_ABFF,
    ANFT_StringFile,
};

#pragma pack(push, 1)

struct ANetDatHeader
{
    byte mIdentifier[4];            // 0x97 0x41 0x4e 0x1a
    uint32 mHeaderSize;             // 0x28 (40)
    uint32 mUnknownField1;
    uint32 mChunkSize;              // 0x200 (512)
    uint32 mCRC;
    uint32 mUnknownField2;
    uint64 mMftOffset;
    uint32 mMftSize;
    uint32 mFlags;
};

struct ANetMftHeader
{
    byte mIdentifier[4];            // 'Mft' 0x1a
    uint64 mUnknownField1;           
    uint32 mNumEntries;             // Including this header as entry 0
    uint64 mUnknownField2;
};

struct ANetMftEntry
{
    uint64 mOffset;                 // Location in the dat that the file is stored
    uint32 mSize;                   // Uncompressed size of the file
    uint16 mCompressionFlag;        // 0x08 = compressed, 0x00 = uncompressed
    uint8 mFlagB;
    uint8 mFlagC;
    uint32 mCounter;
    uint32 mCRC;
};

struct ANetFileIdEntry
{
    uint32 mFileId;                 // File id
    uint32 mMftEntryIndex;          // Index of the file in the mft
};

struct ANetPfHeader
{
    byte mIdentifier[2];            // always 'PF'
    uint16 mUnknownField1;          // seems to always be 0x0001 for models
    uint16 mUnknownField2;          // must always be 0 according to the exe
    uint16 mPkFileVersion;          // format version of this file

    union {
        byte mType[4];              // type of data contained in this PF file
        uint32 mTypeInteger;        // same as above, but as integer for easy comparison
    };
};

struct ANetPfChunkHeader
{
    union {
        byte mChunkType[4];
        uint32 mChunkTypeInteger;
    };
    uint32 mChunkDataSize;          // excluding this field and the identifier, but INCLUDING chunk version... ಠ_ಠ
    uint16 mChunkVersion;
    uint16 mUnknownField1;          // seems to always be 0x0010
};

#pragma pack(pop)

}; // namespace gw2mw

#endif // ANETSTRUCTS_H_INCLUDED
