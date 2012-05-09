// File: UnpackProgress.cpp

#include "stdafx.h"
#include "UnpackProgress.h"
#include "Gw2Dat.h"


namespace gw2du
{

UnpackProgress::UnpackProgress(Gw2Dat* pDatFile)
    : mDatFile(pDatFile), mLabel(NULL), wxFrame(NULL, wxID_ANY, wxT("Unpacking dat..."), wxDefaultPosition, wxSize(310, 85))
{
    wxPanel* panel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxSize(310, 85));
    mLabel = new wxStaticText(panel, wxID_ANY, wxEmptyString, wxPoint(8, 8));

}

}; // namespace gw2du
