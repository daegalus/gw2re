// File: Gw2Dat.h

#pragma once

#ifndef GW2DAT_H_INCLUDED
#define GW2DAT_H_INCLUDED

#include <wx/file.h>
#include "ANetStructs.h"

namespace gw2du
{

class Gw2Dat
{
    wxFile              mFile;
    ANetDatHeader       mDatHead;
    ANetMftHeader       mMftHead;
    ANetMftEntry*       mMftEntries;
    ANetFileIdEntry*    mFileIdEntries;
    uint                mNumFileIdEntries;
    
public:
    Gw2Dat();
    Gw2Dat(const wxString& pFilename);
    ~Gw2Dat();

    bool Open(const wxString& pFilename);
    bool IsOpen() const;
    void Close();

    int GetEntryNumFromFileId(uint pFileId) const;
    uint GetFileIdFromEntryNum(int pEntryNum) const;
    int GetNumEntries() const;

    ANetFileType ReadFile(int pEntryNum, byte*& pBufferPtr, uint& pSize);
    ANetFileType IdentifyFileType(const byte* pBuffer, uint pSize) const;

}; // class Gw2Dat

}; // namespace gw2du

#endif // GW2DAT_H_INCLUDED
