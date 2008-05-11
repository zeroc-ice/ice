// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Latency.h>

using namespace std;
using namespace Demo;

static HWND editHwnd;
static bool wmDestroy = false;

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_CREATE:
    {
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	editHwnd = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", 
			       WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE,
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
	wmDestroy = true;
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
    static const TCHAR windowClassName[] = L"Latency Client";
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
    HWND mainWnd = CreateWindow(windowClassName, L"Latency Client", WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
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
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
	initData.properties = Ice::createProperties();

	//
	// Set a default value for Latency.Proxy so that the demo will
	// run without a configuration file.
	//
	initData.properties->setProperty("Latency.Proxy", "ping:tcp -p 10000");

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

	const char* proxyProperty = "Latency.Proxy";
	string proxy = initData.properties->getProperty(proxyProperty);

	PingPrx ping = PingPrx::checkedCast(communicator->stringToProxy(proxy));
	if(!ping)
	{
	    MessageBox(NULL, L"invalid proxy", L"Latency Client", MB_ICONEXCLAMATION | MB_OK);
	    return EXIT_FAILURE;
	}

	// Initial ping to setup the connection.
	ping->ice_ping();
	
	IceUtil::Time tm = IceUtil::Time::now();
	
	const int repetitions = 10000;
	wchar_t buf[1000];
	wsprintf(buf, L"pinging server %d times (this may take a while)\r\n", repetitions);
	::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf);
	for(int i = 0; i < repetitions; ++i)
	{
	    ping->ice_ping();
	    if((i % 100) == 0)
	    {
		::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)L".");
		//
		// Run the message pump just in case the user tries to close the app.
		//
		MSG Msg;
		while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
		{
		    TranslateMessage(&Msg);
		    DispatchMessage(&Msg);
		}
		if(wmDestroy)
		{
		    break;
		}
	    }   
	}
	
	tm = IceUtil::Time::now() - tm;
	
	wsprintf(buf, L"\r\ntime for %d pings: %fms\r\ntime per ping: %fms\r\n",
		 repetitions, tm.toMilliSecondsDouble(), tm.toMilliSecondsDouble() / repetitions);
	::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf);

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
	MessageBox(NULL, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

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
	    MessageBox(NULL, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

	    status = EXIT_FAILURE;
	}
    }

    return status;
}
