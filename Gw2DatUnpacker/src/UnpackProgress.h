// File: UnpackProgress.h

#pragma once

#ifndef UNPACKPROGRESS_H_INCLUDED
#define UNPACKPROGRESS_H_INCLUDED

#include <wx/frame.h>

namespace gw2du
{

class Gw2Dat;

class UnpackProgress : public wxFrame
{
    Gw2Dat*         mDatFile;
    int             mCurrentFile;
    wxStaticText*   mLabel;
public:
    UnpackProgress(Gw2Dat* pDatFile);
}; // class UnpackProgress

}; // namespace gw2du

#endif // UNPACKPROGRESS_H_INCLUDED
