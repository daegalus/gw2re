// File: Gw2DatUnpacker.h

#pragma once

#ifndef GW2DATUNPACKER_H_INCLUDED
#define GW2DATUNPACKER_H_INCLUDED

#include <wx/progdlg.h>
#include "Gw2Dat.h"

namespace gw2du
{

class Gw2DatUnpacker : public wxApp
{
    Gw2Dat              mDatFile;
    wxProgressDialog*   mProgress;
    int                 mCurrentEntry;

    wxString            mDatPath;
    wxString            mOutPath;
public:
    Gw2DatUnpacker();
    virtual bool OnInit();
    void UpdateProgress(wxIdleEvent& pEvent);
private:
    void ParseArguments();
    void PromptDatPath();
    void PromptOutPath();
    void OnError(const wxString& pErrorMsg);
}; // class Gw2DatUnpacker

}; // namespace gw2du

#endif // GW2DATUNPACKER_H_INCLUDED
