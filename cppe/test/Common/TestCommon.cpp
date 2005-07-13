// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestCommon.h>
#include <TestApplication.h>
#include <IceE/StaticMutex.h>
#include <IceE/Thread.h>
#ifndef _WIN32
# include <IceE/Time.h>
#endif


#include <stdarg.h>

using namespace Ice;
using namespace std;

class LoggerI : public Ice::Logger
{
public:

    virtual void
    print(const string& message)
    {
	tprintf("%s\n", message.c_str());
    }
    
    virtual void
    trace(const string& category, const string& message)
    {
	string s = "[ ";
#ifdef _WIN32
	char buf[1024];
	sprintf(buf, "%ld", GetTickCount());
	s += buf;
#else
 	s += IceUtil::Time::now().toMilliSeconds();
#endif
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
	tprintf("warning: %s\n", message.c_str());
    }

    virtual void
    error(const string& message)
    {
	tprintf("error: %s\n", message.c_str());
    }
};


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
	//HFONT hfDefault;
	
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
			       WS_CHILD | WS_VISIBLE | WS_VSCROLL /*| WS_HSCROLL*/ | ES_MULTILINE,
			       0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
			       /*0,0,100,100,*/
			       hWnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL);
	assert(hEdit != NULL);
    }
    break;

    case WM_SIZE:
    {
	HWND hEdit;
	RECT rcClient;
	
	GetClientRect(hWnd, &rcClient);
	
	hEdit = GetDlgItem(hWnd, IDC_MAIN_EDIT);
	SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
    }
    break;

    case WM_CLOSE:
	DestroyWindow(hWnd);
	break;

    case WM_DESTROY:
	PostQuitMessage(0);
	break;

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
    wc.hIcon	         = LoadIcon(NULL, 0/*IDI_APPLICATION*/);
    wc.hCursor	         = 0;
    wc.hbrBackground	 = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName      = NULL;
    wc.lpszClassName     = windowClassName;

    if(!RegisterClass(&wc))
    {
	MessageBox(NULL, L"Window Registration Failed!", L"Error!",
		   MB_ICONEXCLAMATION | MB_OK);
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
    mainWnd = CreateWindow(windowClassName, wName, WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT, 320, 200,
			NULL, NULL, hInstance, NULL);
    if(mainWnd == NULL)
    {
	MessageBox(NULL, L"Window Creation Failed!", L"Error!",
		   MB_ICONEXCLAMATION | MB_OK);
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
    catch(const std::string& msg)
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
#else

static IceUtil::StaticMutex tprintMutex = ICEE_STATIC_MUTEX_INITIALIZER;

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
    catch(const std::string& msg)
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
#endif

TestApplication::TestApplication(const std::string& name)
    : _name(name)
{

}

void
TestApplication::setCommunicator(const Ice::CommunicatorPtr& communicator)
{
    _communicator = communicator;
    _communicator->setLogger(new LoggerI);

    //_tprintfp = fopen(("log-" + _name + ".txt").c_str(), "w");
}

Ice::CommunicatorPtr
TestApplication::communicator()
{
    return _communicator;
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
