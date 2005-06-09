// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestCommon.h>

#include <stdarg.h>

#ifdef _WIN32_WCE

const TCHAR windowClassName[] = L"Test Driver";

#define IDC_MAIN_EDIT	101

class TestSuiteFailed
{
public:
};

static HWND hEdit;
void
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
	TCHAR wtext[1024];
	mbstowcs(wtext, start, (curr - start) + 1);
	::SendMessage(hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)wtext);
	if(nl)
	{
	    ::SendMessage(hEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)L"\r\n");
	}
	++curr;
	start = curr;
    }

    //
    // Process pending events.
    //
    MSG Msg;
    while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {
	TranslateMessage(&Msg);
	DispatchMessage(&Msg);
    }
}

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
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
    HWND hWnd;
    
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

    hWnd = CreateWindow(windowClassName, L"Test", WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT, 320, 200,
			NULL, NULL, hInstance, NULL);
    if(hWnd == NULL)
    {
	MessageBox(NULL, L"Window Creation Failed!", L"Error!",
		   MB_ICONEXCLAMATION | MB_OK);
	return 0;
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    try
    {
	extern int    __argc;
	extern char **__argv; 
	run(__argc, __argv);
    	tprintf("success!");
    }
    catch(const TestSuiteFailed& e)
    {
	tprintf("test failed\n");
    }

    MSG Msg;
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
	TranslateMessage(&Msg);
	DispatchMessage(&Msg);
    }
    return Msg.wParam;
}
#else
void
tprintf(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char buf[1024];
    vprintf(buf, sizeof(buf)-1, fmt, va);
    buf[sizeof(buf)-1] = '\0';
    va_end(va);
}

int
TestApplication::main(int ac, char*[] av)
{
    run(ac, av);
}
#endif

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

