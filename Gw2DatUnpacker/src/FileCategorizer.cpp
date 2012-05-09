// File: FileCategorizer.cpp

#include "stdafx.h"
#include "FileCategorizer.h"
#include <wx/filename.h>

namespace gw2du
{

wxFileName FileCategorizer::Categorize(const wxString& pOutPath, ANetFileType pFileType, uint pFileId, const byte* pData, uint pSize)
{
    wxFileName fileName(pOutPath, wxT(""));
    
    // Textures
    if (pFileType > ANFT_TextureStart && pFileType < ANFT_TextureEnd) {
        fileName.AppendDir(wxT("Textures"));

        switch (pFileType) {
        case ANFT_ATEX:
            fileName.AppendDir(wxT("Generic textures"));
            fileName.SetFullName(wxString::Format(wxT("%d.atex"), pFileId));
            break;
        case ANFT_ATTX:
            fileName.AppendDir(wxT("Terrain textures"));
            fileName.SetFullName(wxString::Format(wxT("%d.attx"), pFileId));
            break;
        case ANFT_ATEC:
            fileName.AppendDir(wxT("ATEC"));
            fileName.SetFullName(wxString::Format(wxT("%d.atec"), pFileId));
            break;
        case ANFT_ATEP:
            fileName.AppendDir(wxT("Map textures"));
            fileName.SetFullName(wxString::Format(wxT("%d.atep"), pFileId));
            break;
        case ANFT_ATEU:
            fileName.AppendDir(wxT("UI textures"));
            fileName.SetFullName(wxString::Format(wxT("%d.ateu"), pFileId));
            break;
        case ANFT_ATET:
            fileName.AppendDir(wxT("ATET"));
            fileName.SetFullName(wxString::Format(wxT("%d.atet"), pFileId));
            break;
        case ANFT_DDS:
            fileName.AppendDir(wxT("DDS"));
            fileName.SetFullName(wxString::Format(wxT("%d.dds"), pFileId));
            break;
        }

        if (pFileType != ANFT_DDS && pSize >= 12) {
            uint16 width  = *(const uint16*)(pData + 0x8);
            uint16 height = *(const uint16*)(pData + 0xa);
            fileName.AppendDir(wxString::Format(wxT("%dx%d"), width, height));
        } else if (pSize >= 20) {
            uint32 width  = *(const uint32*)(pData + 0x10);
            uint32 height = *(const uint32*)(pData + 0x0c);
            fileName.AppendDir(wxString::Format(wxT("%dx%d"), width, height));
        }
    }

    // Sounds
    else if (pFileType == ANFT_MP3 || pFileType == ANFT_OGG || pFileType == ANFT_Sound) {
        fileName.AppendDir(wxT("Sounds"));
        if (pFileType == ANFT_MP3) {
            fileName.SetFullName(wxString::Format(wxT("%d.mp3"), pFileId));
        } else if (pFileType == ANFT_OGG) {
            fileName.SetFullName(wxString::Format(wxT("%d.ogg"), pFileId));
        } else {
            fileName.SetFullName(wxString::Format(wxT("%d.asnd"), pFileId));
        }
    }

    else if (pFileType == ANFT_Binary || pFileType == ANFT_EXE || pFileType == ANFT_DLL) {
        fileName.AppendDir(wxT("Binaries"));
        
        if (pFileType == ANFT_EXE) {
            fileName.SetFullName(wxString::Format(wxT("%d.exe"), pFileId));
        } else if (pFileType == ANFT_DLL) {
            fileName.SetFullName(wxString::Format(wxT("%d.dll"), pFileId));
        } else {
            fileName.SetFullName(wxString::Format(wxT("%d"), pFileId));
        }
    }

    // Strings
    else if (pFileType == ANFT_StringFile) {
        fileName.AppendDir(wxT("Strings"));
        fileName.SetFullName(wxString::Format(wxT("%d.strs"), pFileId));
    }

    // Manifests
    else if (pFileType == ANFT_Manifest) {
        fileName.AppendDir(wxT("Manifests"));
        fileName.SetFullName(wxString::Format(wxT("%d.armf"), pFileId));
    }

    // Bank files
    else if (pFileType == ANFT_Bank) {
        fileName.AppendDir(wxT("Banks"));
        fileName.SetFullName(wxString::Format(wxT("%d.abnk"), pFileId));
    }

    // Model files
    else if (pFileType == ANFT_Model) {
        fileName.AppendDir(wxT("Models"));
        fileName.SetFullName(wxString::Format(wxT("%d.modl"), pFileId));
    }

    // Dependency table
    else if (pFileType == ANFT_DependencyTable) {
        fileName.AppendDir(wxT("Dependency tables"));
        fileName.SetFullName(wxString::Format(wxT("%d.deps"), pFileId));
    }

    // EULA
    else if (pFileType == ANFT_EULA) {
        fileName.AppendDir(wxT("EULA"));
        fileName.SetFullName(wxString::Format(wxT("%d.eula"), pFileId));
    }

    // Havok
    else if (pFileType == ANFT_HavokCloth) {
        fileName.AppendDir(wxT("Havok cloth"));
        fileName.SetFullName(wxString::Format(wxT("%d.hvkc"), pFileId));
    }

    // Maps
    else if (pFileType == ANFT_Map) {
        fileName.AppendDir(wxT("Maps"));
        fileName.SetFullName(wxString::Format(wxT("%d.mapc"), pFileId));
    }

    // Materials
    else if (pFileType == ANFT_Material) {
        fileName.AppendDir(wxT("Materials"));
        fileName.SetFullName(wxString::Format(wxT("%d.amat"), pFileId));
    }

    // Random PF files
    else if (pFileType == ANFT_PF) {
        fileName.AppendDir(wxT("Misc"));
        if (pSize >= 12) {
            wxString type = wxString((const char*)(pData + 8), 4);
            fileName.AppendDir(type);
            fileName.SetFullName(wxString::Format(wxT("%d.%s"), pFileId, type.Lower()));
        } else {
            fileName.SetFullName(wxString::Format(wxT("%d"), pFileId));
        }
    }

    // ABFF
    else if (pFileType == ANFT_ABFF) {
        fileName.AppendDir(wxT("Misc"));
        fileName.AppendDir(wxT("ABFF"));
        fileName.SetFullName(wxString::Format(wxT("%d.abff"), pFileId));
    }

    // unknown stuff
    else {
        fileName.AppendDir(wxT("Unknown"));
        fileName.AppendDir(wxString::Format(wxT("%x"), *(const uint32*)pData));
        fileName.SetFullName(wxString::Format(wxT("%d"), pFileId));
    }

    return fileName;
}

}; // namespace gw2du
