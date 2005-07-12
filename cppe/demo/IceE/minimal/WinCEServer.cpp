// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;

#define IDC_MAIN_EDIT	101

static HWND hEdit;
static HWND mainWnd;
static IceUtil::ThreadControl mainThread;

static void
tprintf(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char buf[1024];
    _vsnprintf(buf, sizeof(buf)-1, fmt, va);
    buf[sizeof(buf)-1] = '\0';
    va_end(va);

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

class HelloI : public ::Demo::Hello
{
public:

    virtual void
    sayHello(const Ice::Current&) const
    {
	tprintf("Hello World!\n");
    }
};


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

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    static const TCHAR windowClassName[] = L"Minimal Server";
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

    mainWnd = CreateWindow(windowClassName, L"Minimal Server", WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
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

    extern int    __argc;
    extern char **__argv; 

    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties();
	properties->setProperty("Hello.Endpoints","tcp -p 10000");
	communicator = Ice::initializeWithProperties(__argc, __argv, properties);
	Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Hello");
	Ice::ObjectPtr object = new HelloI;
	adapter->add(object, Ice::stringToIdentity("hello"));
	adapter->activate();
    }
    catch(const Ice::Exception& ex)
    {
	tprintf("%s\n", ex.toString().c_str());
	status = EXIT_FAILURE;
    }
    tprintf("Close the window to terminate the server.\n");

    MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
	TranslateMessage(&Msg);
	DispatchMessage(&Msg);
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    tprintf("%s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
    }
    return status;
}
