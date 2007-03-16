// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloI : public Hello
{
public:

    HelloI(HWND wnd)
	: _wnd(wnd)
    {
    }

    virtual void
    sayHello(const Ice::Current&) const
    {
	//
	// Its not legal to write to windows controls in a thread
	// other than main, so we post a custom message to the main
	// thread which will cause the message to be written to the
	// edit control.
	//
	static wchar_t* hello = L"Hello World\r\n";
	::PostMessage(_wnd, WM_USER, (WPARAM)FALSE, (LPARAM)_wcsdup(hello));
    }

private:

    const HWND _wnd;
};

static HWND editHwnd;

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_USER:
    {
    	// tprint from a thread other than main. lParam holds a pointer to the text.
	::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)wParam, (LPARAM)lParam);
	free((wchar_t*)lParam);
    }
    break;

    case WM_CREATE:
    {
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	editHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
			       WS_CHILD | WS_VISIBLE | WS_VSCROLL /*| WS_HSCROLL*/ | ES_MULTILINE,
			       0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
			       hWnd, (HMENU)101, GetModuleHandle(NULL), NULL);
	assert(editHwnd != NULL);
    }
    break;

    case WM_SIZE:
    {
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	SetWindowPos(editHwnd, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
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
    wc.hIcon	         = LoadIcon(NULL, 0);
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
    HWND mainWnd = CreateWindow(windowClassName, L"Minimal Server", WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
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

    extern int    __argc;
    extern char **__argv; 

    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
	initData.properties = Ice::createProperties();
	//
	// Set a default value for Hello.Endpoints so that the demo
	// will run without a configuration file.
	//
	initData.properties->setProperty("Hello.Endpoints","tcp -p 10000");

	//
	// Now, load the configuration file if present. Under WinCE we
	// use "config.txt" since it can be edited with pocket word.
	//
	try
	{
	    initData.properties->load("config.txt");
	}
	catch(const Ice::FileException&)
	{
	}

	communicator = Ice::initialize(__argc, __argv, initData);

	Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("Hello");
	adapter->add(new HelloI(mainWnd), communicator->stringToIdentity("hello"));
	adapter->activate();

	//
	// Display a helpful message to the user.
	//
	::SendMessage(editHwnd, EM_REPLACESEL,
		      (WPARAM)FALSE, (LPARAM)L"Close the window to terminate the server.\r\n");

	//
	// Run the message pump.
	//
	MSG Msg;
	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
	    TranslateMessage(&Msg);
	    DispatchMessage(&Msg);
	}
    }
    catch(const Ice::Exception& ex)
    {
	TCHAR wtext[1024];
	string err = ex.toString();
	mbstowcs(wtext, err.c_str(), err.size());
	MessageBox(mainWnd, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    TCHAR wtext[1024];
	    string err = ex.toString();
	    mbstowcs(wtext, err.c_str(), err.size());
	    MessageBox(mainWnd, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

	    status = EXIT_FAILURE;
	}
    }
    return status;
}
