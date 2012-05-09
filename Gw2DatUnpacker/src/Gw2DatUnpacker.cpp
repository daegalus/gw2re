// File: Gw2DatUnpacker.cpp

#include "stdafx.h"
#include <vld.h>
#include <wx/filename.h>

#include "Gw2DatUnpacker.h"
#include "FileCategorizer.h"

namespace gw2du
{

IMPLEMENT_APP(Gw2DatUnpacker);

Gw2DatUnpacker::Gw2DatUnpacker()
    : mProgress(NULL), mCurrentEntry(0)
{
}

bool Gw2DatUnpacker::OnInit()
{
    this->ParseArguments();
    this->PromptDatPath();
    this->PromptOutPath();

    // Make sure the paths are valid
    if (mDatPath.Length() == 0) {
        return false;
    }
    if (mOutPath.Length() == 0) {
        return false;
    }

    // Open the dat
    mDatFile.Open(mDatPath);
    if (!mDatFile.IsOpen()) {
        OnError(wxT("Failed to open the dat file specified."));
        return false;
    }

    // Does the dat contain any files at all?
    if (mDatFile.GetNumEntries() <= 16) {
        return false;
    }

    // Start processing!
    mCurrentEntry = 15;
    this->Connect(wxEVT_IDLE, wxIdleEventHandler(Gw2DatUnpacker::UpdateProgress));
    mProgress = new wxProgressDialog(wxT("Unpacking..."), wxT("Preparing to unpack..."), mDatFile.GetNumEntries() - 16, NULL, wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME | wxPD_REMAINING_TIME);
    mProgress->Show();

    return true;
}

void Gw2DatUnpacker::UpdateProgress(wxIdleEvent& pEvent)
{
    while (true) {
        // Done?
        bool isDone = (++mCurrentEntry == mDatFile.GetNumEntries());
        if (mOutPath.Length() == 0 || isDone) {
            break;
        }

        // Read file
        byte* buffer;
        uint size;
        ANetFileType fileType = mDatFile.ReadFile(mCurrentEntry, buffer, size);
        uint fileId = mDatFile.GetFileIdFromEntryNum(mCurrentEntry);

        // Skip if no contents
        if (!buffer || !size) {
            FreePointer(buffer);
            pEvent.RequestMore();
            return;
        }

        // Categorize
        FileCategorizer categorizer;
        wxFileName outPath = categorizer.Categorize(mOutPath, fileType, fileId, buffer, size);

        // Create directory if it does not exist
        if (!outPath.DirExists()) {
            if (!outPath.Mkdir(511, wxPATH_MKDIR_FULL)) {
                OnError(wxString::Format(wxT("Failed to create path: %s"), outPath.GetPath()));
                FreePointer(buffer);
                break;
            }
        }

        // Open file for writing
        wxFile outFile(outPath.GetFullPath(), wxFile::write);
        if (!outFile.IsOpened()) {
            OnError(wxString::Format(wxT("Failed to write to file: %s"), outPath.GetFullPath()));
            FreePointer(buffer);
            break;
        }

        // Write data and clear buffer
        outFile.Write(buffer, size);
        outFile.Close();
        FreePointer(buffer);

        // Update progress and return
        if (!mProgress->Update(mCurrentEntry - 16, wxString::Format(wxT("(%d/%d): %s"), mCurrentEntry - 16, mDatFile.GetNumEntries() - 16, outPath.GetFullName()))) {
            break;
        }
        pEvent.RequestMore();
        return;
    }

    this->Disconnect(wxEVT_IDLE, wxIdleEventHandler(Gw2DatUnpacker::UpdateProgress));
    DeletePointer(mProgress);
}

void Gw2DatUnpacker::PromptDatPath()
{
    // Argument?
    if (mDatPath.Length() > 0) {
        return;
    }

    // Prompt for file
    wxFileDialog dialog(NULL, wxT("Open Gw2.dat"), wxT(""), wxT("Gw2.dat"), wxT("DAT files (*.dat)|*.dat"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dialog.ShowModal() == wxID_OK) {
        mDatPath = dialog.GetPath();
    }
}

void Gw2DatUnpacker::PromptOutPath()
{
    // Argument?
    if (mOutPath.Length() > 0) {
        return;
    }

    // Prompt for path
    wxDirDialog dialog(NULL, wxT("Select output folder"));
    if (dialog.ShowModal() == wxID_OK) {
        mOutPath = dialog.GetPath();
    }
}

void Gw2DatUnpacker::ParseArguments()
{
    wxString lastArg;
    for (int i = 1; i < this->argc; i++) {
        if (lastArg == wxT("-i")) {
            mDatPath = this->argv[i];
        } else if (lastArg == wxT("-o")) {
            mOutPath = this->argv[i];
        }
        lastArg = this->argv[i];
    }
}

void Gw2DatUnpacker::OnError(const wxString& pErrorMsg)
{
    wxMessageBox(pErrorMsg, wxT("Error"), wxOK | wxCENTER | wxICON_ERROR);
}

}; // namespace gw2du
