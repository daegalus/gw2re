// File: FileCategorizer.h

#pragma

#ifndef FILECATEGORIZER_H_INCLUDED
#define FILECATEGORIZER_H_INCLUDED

#include <wx/filename.h>
#include "ANetStructs.h"

namespace gw2du
{

class FileCategorizer
{
public:
    wxFileName Categorize(const wxString& pOutPath, ANetFileType pFileType, uint pFileId, const byte* pData, uint pSize);
}; // class FileCategorizer

}; // namespace gw2du

#endif // FILECATEGORIZER_H_INCLUDED
