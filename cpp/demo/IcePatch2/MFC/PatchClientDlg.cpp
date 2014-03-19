// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "stdafx.h"
#include "PatchClient.h"
#include "PatchClientDlg.h"

#include <IcePatch2/ClientUtil.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

string
getBasename(const string& path)
{
    string::size_type pos = path.rfind('/');
    if(pos == string::npos)
    {
        pos = path.rfind('\\');
    }

    if(pos == string::npos)
    {
        return path;
    }
    else
    {
        return path.substr(pos + 1);
    }
}

class DialogPatcherFeedback : public IcePatch2::PatcherFeedback
{
public:

    DialogPatcherFeedback(CPatchDlg* dialog) :
        _dialog(dialog),
        _filesPatched(0)
    {
        assert(_dialog);
    }

    virtual bool
    noFileSummary(const string& /*reason*/)
    {
        return IDYES == AfxMessageBox(L"Cannot load file summary. Perform a thorough patch?", MB_YESNO|MB_ICONSTOP);
    }

    virtual bool
    checksumStart()
    {
        return _dialog->checksumStart();
    }

    virtual bool
    checksumProgress(const string& path)
    {
        return _dialog->checksumProgress(path);
    }

    virtual bool
    checksumEnd()
    {
        return _dialog->checksumEnd();
    }

    virtual bool
    fileListStart()
    {
        return _dialog->fileListStart();
    }

    virtual bool
    fileListProgress(Ice::Int percent)
    {
        return _dialog->fileListProgress(percent);
    }

    virtual bool
    fileListEnd()
    {
        return _dialog->fileListEnd();
    }

    virtual bool
    patchStart(const string& path, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
    {
        return _dialog->patchStart(path, size, totalProgress, totalSize);
    }

    virtual bool
    patchProgress(Ice::Long progress, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
    {
        return _dialog->patchProgress(progress, size, totalProgress, totalSize);
    }

    virtual bool
    patchEnd()
    {
        ++_filesPatched;
        return _dialog->patchEnd();
    }

    virtual int
    filesPatched() const
    {
        return _filesPatched;
    }

private:

    CPatchDlg* const _dialog;
    int _filesPatched;
};

typedef IceUtil::Handle<DialogPatcherFeedback> DialogPatcherFeedbackPtr;

CPatchDlg::CPatchDlg(const Ice::CommunicatorPtr& communicator, CWnd* pParent /*=NULL*/) :
    CDialog(CPatchDlg::IDD, pParent), _communicator(communicator), _isCancel(false), _isPatch(false)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

bool
CPatchDlg::checksumStart()
{
    _status->SetWindowText(CString(L" Calculating checksums..."));

    _progress->SetRange(0, 0);
    _progress->SetPos(0);

    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::checksumProgress(const string& path)
{
    // TODO: indicate busy progress
 
    CString file;
    file.Format(L" %s", IceUtil::stringToWstring(getBasename(path)).c_str());
    _file->SetWindowText(file);

    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::checksumEnd()
{
    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::fileListStart()
{
    _status->SetWindowText(CString(L" Retrieving file list..."));

    _progress->SetRange(0, 100);
    _progress->SetPos(0);

    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::fileListProgress(Ice::Int pcnt)
{
    CString percent;
    percent.Format(L"%d%%", pcnt);
    _percent->SetWindowText(percent);

    _progress->SetPos(pcnt);

    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::fileListEnd()
{
    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::patchStart(const string& path, Ice::Long size, Ice::Long totalProgress, Ice::Long totalSize)
{
    if(!_isPatch)
    {
        _startTime = IceUtil::Time::now(IceUtil::Time::Monotonic);
        _status->SetWindowText(CString(L" Patching..."));
        _speed->SetWindowText(CString(L" 0.0 KB/s"));
        _isPatch = true;
    }

    CString file;
    file.Format(L" %s", IceUtil::stringToWstring(getBasename(path)).c_str());
    _file->SetWindowText(file);

    return patchProgress(0, size, totalProgress, totalSize);
}

bool
CPatchDlg::patchProgress(Ice::Long, Ice::Long, Ice::Long totalProgress, Ice::Long totalSize)
{
    IceUtil::Time elapsed = IceUtil::Time::now(IceUtil::Time::Monotonic) - _startTime;
    if(elapsed.toSeconds() > 0)
    {
        CString speed;
        speed.Format(L" %s/s", convertSize(totalProgress / elapsed.toSeconds()));
        _speed->SetWindowText(speed);
    }

    int pcnt = 100;
    if(totalSize > 0)
    {
        pcnt = static_cast<int>(totalProgress * 100 / totalSize);
    }
    CString percent;
    percent.Format(L"%d%%", pcnt);
    _percent->SetWindowText(percent);

    CString total;
    total.Format(L" %s / %s", convertSize(totalProgress), convertSize(totalSize));
    _total->SetWindowText(total);

    _progress->SetPos(pcnt);

    processMessages();
    return !_isCancel;
}

bool
CPatchDlg::patchEnd()
{
    processMessages();
    return !_isCancel;
}

void
CPatchDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPatchDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_SELECTDIR, OnSelectDir)
    ON_BN_CLICKED(IDC_STARTPATCH, OnStartPatch)
    ON_BN_CLICKED(IDC_CANCELPATCH, OnCancel)
END_MESSAGE_MAP()

BOOL
CPatchDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(_hIcon, TRUE);            // Set big icon
    SetIcon(_hIcon, FALSE);        // Set small icon

    //
    // Retrieve the controls.
    //
    _path = (CEdit*)GetDlgItem(IDC_PATH);
    _thorough = (CButton*)GetDlgItem(IDC_THOROUGH);
    _remove = (CButton*)GetDlgItem(IDC_ORPHAN);
    _select = (CButton*)GetDlgItem(IDC_SELECTDIR);
    _start = (CButton*)GetDlgItem(IDC_STARTPATCH);
    _cancel = (CButton*)GetDlgItem(IDC_CANCELPATCH);
    _file = (CStatic*)GetDlgItem(IDC_FILE);
    _total = (CStatic*)GetDlgItem(IDC_TOTAL);
    _speed = (CStatic*)GetDlgItem(IDC_SPEED);
    _status = (CStatic*)GetDlgItem(IDC_STATUSBAR);
    _percent = (CStatic*)GetDlgItem(IDC_PERCENT);
    _progress = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);

    //
    // Set the patch directory and thorough flag from properties.
    //
    Ice::PropertiesPtr properties = _communicator->getProperties();
    CString path = IceUtil::stringToWstring(properties->getPropertyWithDefault("IcePatch2Client.Directory", "")).c_str();
    _path->SetWindowText(path);

    CString thorough = IceUtil::stringToWstring(properties->getPropertyWithDefault("IcePatch2Client.Thorough", "0")).c_str();
    _thorough->SetCheck(thorough != "0");

    CString remove = IceUtil::stringToWstring(properties->getPropertyWithDefault("IcePatch2Client.Remove", "0")).c_str();
    _remove->SetCheck(remove != "0");

    //
    // Indicate ready status.
    //
    reset(L" Ready");

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CPatchDlg::OnPaint() 
{
    if(IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, _hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR
CPatchDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

void
CPatchDlg::OnSelectDir()
{
    BROWSEINFO info = { 0 };
    info.lpszTitle = _T("Select Patch Directory");
    LPITEMIDLIST pidl = SHBrowseForFolder(&info);
    if(pidl != 0)
    {
        //
        // Get the name of the selected folder.
        //
        TCHAR path[MAX_PATH];
        if(SHGetPathFromIDList(pidl, path))
        {
            _path->SetWindowText(path);
        }

        //
        // Free up memory used.
        //
        IMalloc * imalloc = 0;
        if(SUCCEEDED(SHGetMalloc(&imalloc)))
        {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
}

void
CPatchDlg::OnStartPatch()
{
    try
    {
        Ice::PropertiesPtr properties = _communicator->getProperties();

        //
        // Set the patch directory.
        // 
        CString path;
        _path->GetWindowText(path);
        if(path.IsEmpty())
        {
            AfxMessageBox(CString(L"Please select a patch directory."), MB_OK|MB_ICONEXCLAMATION);
            return;
        }
        properties->setProperty("IcePatch2Client.Directory", IceUtil::wstringToString(wstring(path)));

        //
        // Set the thorough patch flag.
        //
        string thorough = _thorough->GetCheck() == BST_CHECKED ? "1" : "0";
        properties->setProperty("IcePatch2Client.Thorough", thorough);

        //
        // Set the remove orphan flag.
        //
        string remove = _remove->GetCheck() == BST_CHECKED ? "1" : "0";
        properties->setProperty("IcePatch2Client.Remove", remove);

        DialogPatcherFeedbackPtr feedback = new DialogPatcherFeedback(this);
        IcePatch2::PatcherPtr patcher = new IcePatch2::Patcher(_communicator, feedback);

        //
        // Disable a few controls during the patch process.
        //
        _path->EnableWindow(false);
        _select->EnableWindow(false);
        _thorough->EnableWindow(false);
        _remove->EnableWindow(false);
        _start->EnableWindow(false);

        //
        // Patch
        //
        bool aborted = !patcher->prepare();
        if(!aborted)
        {
            aborted = !patcher->patch("");
        }
        if(!aborted)
        {
            patcher->finish();
        }

        //
        // Reset and indicate the completion status.
        //
        reset(aborted ? L" Aborted" : L" Completed");
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
    catch(const string& ex)
    {
        AfxMessageBox(CString(ex.c_str()), MB_OK|MB_ICONEXCLAMATION);
    }
}

void
CPatchDlg::OnCancel()
{
    _isCancel = true;
    _status->SetWindowText(CString(L" Canceled"));
    CDialog::OnCancel();
}

void
CPatchDlg::reset(const CString& status)
{
    _isPatch = false;

    _path->EnableWindow(true);
    _select->EnableWindow(true);
    _thorough->EnableWindow(true);
    _remove->EnableWindow(true);
    _start->EnableWindow(true);

    _status->SetWindowText(status);
    _file->SetWindowText(CString());
    _total->SetWindowText(CString());
    _speed->SetWindowText(CString());
    _percent->SetWindowText(CString());

    _progress->SetPos(0);
}

void 
CPatchDlg::processMessages()
{
    MSG msg;
    while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void
CPatchDlg::handleException(const IceUtil::Exception& e)
{
    try
    {
        e.ice_throw();
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ostr << ex;
        string s = ostr.str();
        AfxMessageBox(CString(s.c_str()), MB_OK|MB_ICONEXCLAMATION);
    }

    reset(L" Ready");
}

CString
CPatchDlg::convertSize(Ice::Long size) const
{
    CString units;
    double start = static_cast<double>(size);
    double final = start / gigabyte;
    if(final >= 1)
    {
        units = L"GB";
    }
    else
    {
        final = start / megabyte;
        if(final >= 1)
        {
            units = L"MB";
        }
        else
        {
            final = start / kilobyte;
            if(final >= 1)
            {
                units = L"KB";
            }
            else
            {
                final = start;
                units = L"B";
            }
        }
    }

    CString convert;
    convert.Format(L"%.1f %s", final, units);
    return convert;
}
