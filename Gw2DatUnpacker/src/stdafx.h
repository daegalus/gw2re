// File: stdafx.h

#pragma once

#ifndef STDAFX_H_INCLUDED
#define STDAFX_H_INCLUDED

// C/C++ runtime
#include <cassert>

// wxWidgets
#include <wx/wx.h>

namespace gw2du
{
    typedef wxByte          byte;
    typedef unsigned short  ushort;
    typedef unsigned int    uint;
    typedef unsigned long   ulong;

    typedef wxInt8          int8;
    typedef wxInt16         int16;
    typedef wxInt32         int32;
    typedef wxInt64         int64;

    typedef wxUint8         uint8;
    typedef wxUint16        uint16;
    typedef wxUint32        uint32;
    typedef wxUint64        uint64;

    template <typename T>
        void FreePointer(T*& pPointer)
    {
        ::free(pPointer);
        pPointer = NULL;
    }

    template <typename T>
        void DeletePointer(T*& pPointer)
    {
        delete pPointer;
        pPointer = NULL;
    }

    template <typename T>
        void DeleteArray(T*& pArray)
    {
        delete[] pArray;
        pArray = NULL;
    }

}; // namespace gw2du

#endif // STDAFX_H_INCLUDED
