// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <TestCommon.h>
#include <TestApplication.h>

#include <IceE/StaticMutex.h>
#include <IceE/Thread.h>
#include <IceE/Time.h>

#include <stdarg.h>

using namespace std;
using namespace Ice;

static IceUtil::StaticMutex globalMutex = ICE_STATIC_MUTEX_INITIALIZER;

class LoggerI : public Logger
{
public:

    virtual void
    print(const string& message)
    {
	IceUtil::StaticMutex::Lock sync(globalMutex);
	tprintf("%s\n", message.c_str());
    }
    
    virtual void
    trace(const string& category, const string& message)
    {
	IceUtil::StaticMutex::Lock sync(globalMutex);
	string s = "[ ";
	{
	    char buf[1024];
#ifdef _WIN32
	    sprintf(buf, "%ld", GetTickCount());
#else
	    sprintf(buf, "%lu", (long)IceUtil::Time::now().toMilliSeconds());
#endif
	    s += buf;
	}
	s += ' ';
	
	if(!category.empty())
	{
	    s += category + ": ";
	}
	s += message + " ]";
	
	string::size_type idx = 0;
	while((idx = s.find("\n", idx)) != string::npos)
	{
	    s.insert(idx + 1, "  ");
	    ++idx;
	}
    	tprintf("%s\n", s.c_str());
    }
    
    virtual void
    warning(const string& message)
    {
	IceUtil::StaticMutex::Lock sync(globalMutex);
	tprintf("warning: %s\n", message.c_str());
    }

    virtual void
    error(const string& message)
    {
	IceUtil::StaticMutex::Lock sync(globalMutex);
	tprintf("error: %s\n", message.c_str());
    }
};

static IceUtil::StaticMutex terminatedMutex = ICE_STATIC_MUTEX_INITIALIZER;
static bool appTerminated= false;

#ifdef _WIN32_WCE

const TCHAR windowClassName[] = L"Test Driver";

#define IDC_MAIN_EDIT	101

class TestSuiteFailed
{
public:
};

static FILE* _tprintfp = 0;
static HWND hEdit;
static HWND mainWnd;
static IceUtil::ThreadControl mainThread;

void
tprintf(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char buf[1024];
    _vsnprintf(buf, sizeof(buf)-1, fmt, va);
    buf[sizeof(buf)-1] = '\0';
    va_end(va);

    if(_tprintfp)
    {
	fwrite(buf, strlen(buf), 1, _tprintfp);
	fflush(_tprintfp);
	return;
    }

    char* start = buf;
    const char* end = start + strlen(start);
    char* curr = start;
    while(curr < end)
    {
	bool nl = false;
	while(curr < end && *curr != '\n')
	{
	    // Not designed to handle \r
	    assert(*curr != '\r');
	    ++curr;
	}
	if(*curr == '\n')
	{
	    nl = true;
	}
	*curr = '\0';
	static TCHAR nlStr[] = L"\r\n";

	//
	// If the thread is not the main thread we have to post a message
	// to the main thread to do the EM_REPLACESEL. Calling SendMessage
	// from a thread other than main is not permitted.
	//
	if(IceUtil::ThreadControl() != mainThread)
	{
	    wchar_t* wtext = new wchar_t[sizeof(wchar_t) * (curr - start)+1];
	    mbstowcs(wtext, start, (curr - start) + 1);
	    ::PostMessage(mainWnd, WM_USER, (WPARAM)FALSE, (LPARAM)wtext);
	    if(nl)
	    {
		wchar_t* wtext = new wchar_t[sizeof(nlStr)];
    	    	wcscpy(wtext, nlStr);
		::PostMessage(mainWnd, WM_USER, (WPARAM)FALSE, (LPARAM)wtext);
	    }
	}
    	else
	{
	    TCHAR wtext[1024];
	    mbstowcs(wtext, start, (curr - start) + 1);
	    ::SendMessage(hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)wtext);
	    if(nl)
	    {
		::SendMessage(hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)nlStr);
	    }
	}
	++curr;
	start = curr;
    }

    //
    // Process pending events.
    //
    if(IceUtil::ThreadControl() == mainThread)
    {
	MSG Msg;
	while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
	{
	    TranslateMessage(&Msg);
	    DispatchMessage(&Msg);
	}
    }
}

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_USER:
    {
    	// tprint from a thread other than main. lParam holds a pointer to the text.
	::SendMessage(hEdit, EM_REPLACESEL, (WPARAM)wParam, (LPARAM)lParam);
    	wchar_t* text = (wchar_t*)lParam;
    	delete[] text;
    }
    break;

    case WM_CREATE:
    {
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
			       WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE,
			       0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
			       hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
	assert(hEdit != NULL);
    }
    break;

    case WM_SIZE:
    {
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
    }
    break;

    case WM_CLOSE:
    {
	DestroyWindow(hWnd);
	break;
    }

    case WM_QUIT:
    case WM_DESTROY:
    {
	PostQuitMessage(0);
	IceUtil::StaticMutex::Lock sync(terminatedMutex);
	appTerminated = true;
	break;
    }

    default:
	return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int
TestApplication::main(HINSTANCE hInstance)
{
    WNDCLASS wc;
    
    wc.style	         = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc	 = (WNDPROC)WndProc;
    wc.cbClsExtra	 = 0;
    wc.cbWndExtra	 = 0;
    wc.hInstance	 = hInstance;
    wc.hIcon	         = LoadIcon(NULL, 0);
    wc.hCursor	         = 0;
    wc.hbrBackground	 = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName      = NULL;
    wc.lpszClassName     = windowClassName;

    if(!RegisterClass(&wc))
    {
	MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
	return 0;
    }

    wchar_t wName[1024] = L"Test";
    if(_name.size() > 0)
    {
	int len = _name.size();
	if(len > 1023)
	{
	    len = 1023;
	}
        mbstowcs(wName, _name.c_str(), len);
	wName[len] = L'\0';
    }
    
    RECT rect;
    GetClientRect(GetDesktopWindow(), &rect);
    int width = rect.right - rect.left;
    if(width > 320)
    {
	width = 320;
    }
    int height = rect.bottom - rect.top;
    if(height > 200)
    {
	height = 200;
    }
    mainWnd = CreateWindow(windowClassName, wName, WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT, width, height,
			NULL, NULL, hInstance, NULL);
    if(mainWnd == NULL)
    {
	MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
	return 0;
    }

    ShowWindow(mainWnd, SW_SHOW);
    UpdateWindow(mainWnd);
    int status = EXIT_SUCCESS;

    try
    {
	extern int    __argc;
	extern char **__argv; 
	status = run(__argc, __argv);
    }
    catch(const TestSuiteFailed&)
    {
	tprintf("test failed\n");
    }
    catch(const Exception& ex)
    {
	tprintf("%s\n", ex.toString().c_str());
	status = EXIT_FAILURE;
    }
    catch(const std::exception& ex)
    {
	tprintf("std::exception: %s\n", ex.what());
	status = EXIT_FAILURE;
    }
    catch(const string& msg)
    {
	tprintf("std::string: %s\n", msg.c_str());
	status = EXIT_FAILURE;
    }
    catch(const char* msg)
    {
	tprintf("const char*: %s\n", msg);
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	tprintf("unknown exception\n");
	status = EXIT_FAILURE;
    }

    MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
	TranslateMessage(&Msg);
	DispatchMessage(&Msg);
    }

    if(_communicator)
    {
	try
	{
	    _communicator->destroy();
	}
	catch(const Exception& ex)
	{
    	    tprintf("communicator::destroy() failed: %s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
	_communicator = 0;
    }

    return status;
}

void
TestApplication::loadConfig(const PropertiesPtr& properties)
{
    //
    // COMPILERBUG: For some unknown reason the simple approach
    // doesn't work under WinCE if you compile with optimization.  It
    // looks like a compiler bug to me.
    //
    string config = "config";
    WIN32_FIND_DATA data;
    HANDLE h = FindFirstFile(L"config", &data);
    if(h == INVALID_HANDLE_VALUE)
    {
	config = "config.txt";
	HANDLE h = FindFirstFile(L"config.txt", &data);
	if(h == INVALID_HANDLE_VALUE)
	{
	    return;
	}
    }
    FindClose(h);

    try
    {
	properties->load(config);
    }
    catch(const FileException&)
    {
    }
}
#else

static IceUtil::StaticMutex tprintMutex = ICE_STATIC_MUTEX_INITIALIZER;

void
tprintf(const char* fmt, ...)
{
    IceUtil::StaticMutex::Lock sync(tprintMutex);

    va_list va;
    va_start(va, fmt);
    char buf[1024];
    vprintf(fmt, va);
    buf[sizeof(buf)-1] = '\0';
    va_end(va);
    fflush(stdout);
}

int
TestApplication::main(int ac, char* av[])
{
    int status;
    try
    {
	status = run(ac, av);
    }
    catch(const Exception& ex)
    {
	tprintf("%s\n", ex.toString().c_str());
	status = EXIT_FAILURE;
    }
    catch(const std::exception& ex)
    {
	tprintf("std::exception: %s\n", ex.what());
	status = EXIT_FAILURE;
    }
    catch(const string& msg)
    {
	tprintf("std::string: %s\n", msg.c_str());
	status = EXIT_FAILURE;
    }
    catch(const char* msg)
    {
	tprintf("const char*: %s\n", msg);
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	tprintf("unknown exception\n");
	status = EXIT_FAILURE;
    }
    if(_communicator)
    {
	try
	{
	    _communicator->destroy();
	}
	catch(const Exception& ex)
	{
    	    tprintf("communicator::destroy() failed: %s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
	_communicator = 0;
    }

    return status;
}

void
TestApplication::loadConfig(const PropertiesPtr& properties)
{
    try
    {
	properties->load("config");
    }
    catch(const FileException&)
    {
	try
	{
	    properties->load("config.txt");
	}
	catch(const FileException&)
	{
	}
    }
}

#endif

TestApplication::TestApplication(const std::string& name)
    : _name(name)
{
}

LoggerPtr
TestApplication::getLogger()
{
    return new LoggerI();
}

void
TestApplication::setCommunicator(const CommunicatorPtr& communicator)
{
    _communicator = communicator;
#ifdef _WIN32_WCE
    if(communicator->getProperties()->getPropertyWithDefault("LogToFile", "0") != "0")
    {
	_tprintfp = fopen(("log-" + _name + ".txt").c_str(), "w");
    }
#endif

}

CommunicatorPtr
TestApplication::communicator()
{
    return _communicator;
}

bool
TestApplication::terminated() const
{
    IceUtil::StaticMutex::Lock sync(terminatedMutex);
    return appTerminated;
}

void
testFailed(const char* expr, const char* file, unsigned int line)
{
    tprintf("failed!\n");
    tprintf("%s:%u: assertion `%s' failed\n", file, line, expr);
#ifdef _WIN32_WCE
    throw TestSuiteFailed();
#else
    abort();
#endif
}
