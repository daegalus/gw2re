// File: Gw2Dat.cpp

#include "stdafx.h"
#include "Gw2Dat.h"
#include "gw2Inflate.h"

namespace gw2du
{

Gw2Dat::Gw2Dat()
    : mMftEntries(NULL), mFileIdEntries(NULL), mNumFileIdEntries(0)
{
    ::memset(&mDatHead, 0, sizeof(mDatHead));
    ::memset(&mMftHead, 0, sizeof(mMftHead));
}

Gw2Dat::Gw2Dat(const wxString& pFilename)
    : mMftEntries(NULL), mFileIdEntries(NULL), mNumFileIdEntries(0)
{
    this->Open(pFilename);
}

Gw2Dat::~Gw2Dat()
{
    this->Close();
}

bool Gw2Dat::Open(const wxString& pFilename)
{
    this->Close();

    while (true) {
        // Open file
        mFile.Open(pFilename);
        if (!mFile.IsOpened()) { break; }

        // Read header
        if (mFile.Length() < sizeof(mDatHead)) { break; }
        mFile.Read(&mDatHead, sizeof(mDatHead));

        // Read MFT Header
        if ((uint64)mFile.Length() < mDatHead.mMftOffset + mDatHead.mMftSize) { break; }
        mFile.Seek(mDatHead.mMftOffset, wxFromStart);
        mFile.Read(&mMftHead, sizeof(mMftHead));

        // Read all of the MFT
        if (mDatHead.mMftSize != mMftHead.mNumEntries * sizeof(ANetMftEntry)) { break; }
        mMftEntries = (ANetMftEntry*)::malloc(mDatHead.mMftSize);
        mFile.Seek(mDatHead.mMftOffset, wxFromStart);
        mFile.Read(mMftEntries, mDatHead.mMftSize);

        // Read the file id entry table
        if ((uint64)mFile.Length() < mMftEntries[2].mOffset + mMftEntries[2].mSize) { break; }
        mFileIdEntries = (ANetFileIdEntry*)::malloc(mMftEntries[2].mSize);
        mFile.Seek(mMftEntries[2].mOffset, wxFromStart);
        mFile.Read(mFileIdEntries, mMftEntries[2].mSize);
        mNumFileIdEntries = mMftEntries[2].mSize / sizeof(ANetFileIdEntry);

        // Success!
        return true;
    }

    this->Close();
    return false;
}

bool Gw2Dat::IsOpen() const
{
    return mFile.IsOpened();
}

void Gw2Dat::Close()
{
    mNumFileIdEntries = 0;
    ::memset(&mDatHead, 0, sizeof(mDatHead));
    ::memset(&mMftHead, 0, sizeof(mMftHead));
    FreePointer(mFileIdEntries);
    FreePointer(mMftEntries);
    mFile.Close();
}

int Gw2Dat::GetEntryNumFromFileId(uint pFileId) const
{
    if (!IsOpen()) { return -1; }

    for (int i = 0; i < (int)mNumFileIdEntries; i++) {
        if (mFileIdEntries[i].mFileId == pFileId) {
            return mFileIdEntries[i].mMftEntryIndex;
        }
    }

    return -1;
}

uint Gw2Dat::GetFileIdFromEntryNum(int pEntryNum) const
{
    if (!IsOpen()) { return 0; }

    for (uint i = 0; i < mNumFileIdEntries; i++) {
        if (mFileIdEntries[i].mMftEntryIndex == pEntryNum) {
            return mFileIdEntries[i].mFileId;
        }
    }

    return 0;
}


int Gw2Dat::GetNumEntries() const
{
    if (!IsOpen()) { return -1; }
    return mMftHead.mNumEntries;
}

ANetFileType Gw2Dat::ReadFile(int pEntryNum, byte*& pBufferPtr, uint& pSize)
{
    bool isOpen             = IsOpen();
    bool entryIsInRange     = mMftHead.mNumEntries > (uint)pEntryNum;
    bool fileIsLargeEnough  = (uint64)mFile.Length() > mMftEntries[pEntryNum].mOffset + mMftEntries[pEntryNum].mSize;

    if (!isOpen || !entryIsInRange || !fileIsLargeEnough) { 
        pBufferPtr = NULL;
        pSize      = 0;
        return ANFT_Unknown; 
    }

    uint inputSize = mMftEntries[pEntryNum].mSize;
    byte* input = new byte[inputSize];
    mFile.Seek(mMftEntries[pEntryNum].mOffset, wxFromStart);
    mFile.Read(input, inputSize);

    if (mMftEntries[pEntryNum].mCompressionFlag) {
        uint32 size;
        pBufferPtr = inflate(input, inputSize, size);
        DeleteArray(input);
        pSize = size;
    } else {
        pSize      = inputSize;
        pBufferPtr = input;
    }

    return IdentifyFileType(pBufferPtr, pSize);
}

ANetFileType Gw2Dat::IdentifyFileType(const byte* pBuffer, uint pSize) const
{
    if (pSize < 4) { return ANFT_Unknown; }

    // start with fourcc
    uint32 fourcc = *(uint32*)pBuffer;
    ANetFileType fileType = ANFT_Unknown;

    // abff files need offsetting
    if (fourcc == 0x66666261 && pSize >= 0x40) {
        pBuffer += 0x40;
        pSize   -= 0x40;
        fileType = ANFT_ABFF;

        if (pSize >= 4) {
            fourcc = *(uint32*)pBuffer;
        }
    }

    switch (fourcc) {
    case 0x58455441:
        fileType = ANFT_ATEX;
        break;
    case 0x58545441:
        fileType = ANFT_ATTX;
        break;
    case 0x43455441:
        fileType = ANFT_ATEC;
        break;
    case 0x50455441:
        fileType = ANFT_ATEP;
        break;
    case 0x55455441:
        fileType = ANFT_ATEU;
        break;
    case 0x54455441:
        fileType = ANFT_ATET;
        break;
    case 0x20534444:
        fileType = ANFT_DDS;
        break;
    case 0x73727473:
        fileType = ANFT_StringFile;
        break;
    case 0x646e7361:
        fileType = ANFT_Sound;
        break;
    case 0x504e4943:
        fileType = ANFT_Cinematic;
        break;
    }

    // Identify binary files
    if ((fourcc & 0xffff) == 0x5a4d) {
        fileType = ANFT_Binary;

        if (pSize >= 0x40) {
            uint32 peOffset = *(uint32*)(pBuffer + 0x3c);

            if (pSize >= (peOffset + 0x18)) {
                uint16 flags = *(uint16*)(pBuffer + peOffset + 0x16);
                fileType = (flags & 0x2000) ? ANFT_DLL : ANFT_EXE;
            }
        }
    }

    // Identify PF files
    if ((fourcc & 0xffff) == 0x4650) {
        fourcc   = *(uint32*)(pBuffer + 8);
        fileType = ANFT_PF;

        switch (fourcc) {
        case 0x464d5241:
            fileType = ANFT_Manifest;
            break;
        case 0x444e5341:
            fileType = ANFT_Sound;
            break;
        case 0x4b4e4241:
            fileType = ANFT_Bank;
            break;
        case 0x4c444f4d:
            fileType = ANFT_Model;
            break;
        case 0x53504544:
            fileType = ANFT_DependencyTable;
            break;
        case 0x616c7565:
            fileType = ANFT_EULA;
            break;
        case 0x436b7668:
            fileType = ANFT_HavokCloth;
            break;
        case 0x6370616d:
            fileType = ANFT_Map;
            break;
        case 0x54414d41:
            fileType = ANFT_Material;
            break;
        }
    }

    // Identify sounds
    if (fileType == ANFT_Sound || fileType == ANFT_Bank) {
        const uint16 PF   = 0x4650;
        const uint32 asnd = 0x646e7361;
        const uint32 ASND = 0x444e5341;
        const uint32 ABNK = 0x4b4e4241;
        const uint32 OggS = 0x5367674f;

        if (pSize >= 12) {
            if (*(uint32*)pBuffer == asnd && pSize >= 40) {
                fileType = (*(uint32*)(pBuffer + 36) == OggS) ? ANFT_OGG : ANFT_MP3;
            } else if (*(uint16*)pBuffer == PF && *(uint32*)(pBuffer + 8) == ASND && pSize >= 96) {
                fileType = (*(uint32*)(pBuffer + 92) == OggS) ? ANFT_OGG : ANFT_MP3;
            } else if (fileType == ANFT_Bank && pSize >= 592) {
                fileType = (*(uint32*)(pBuffer + 588) == OggS) ? ANFT_OGG : ANFT_MP3;
            }
        }
    }

    return fileType;
}

}; // namespace gw2du
