// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Throughput.h>

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
    static const TCHAR windowClassName[] = L"Throughput Client";
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
    HWND mainWnd = CreateWindow(windowClassName, L"Throughput Client", WS_VISIBLE|WS_OVERLAPPED|WS_SYSMENU|WS_SIZEBOX,
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
	// Set a default value for Throughput.Proxy so that the demo will
	// run without a configuration file.
	//
	initData.properties->setProperty("Throughput.Proxy", "throughput:tcp -p 10000");

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

	const char* proxyProperty = "Throughput.Proxy";
	string proxy = initData.properties->getProperty(proxyProperty);

	ThroughputPrx throughput = ThroughputPrx::checkedCast(communicator->stringToProxy(proxy));
	if(!throughput)
	{
	    MessageBox(NULL, L"invalid proxy", L"Throughput Client", MB_ICONEXCLAMATION | MB_OK);
	    return EXIT_FAILURE;
	}
	ThroughputPrx throughputOneway = ThroughputPrx::uncheckedCast(throughput->ice_oneway());

        //
        // The amount by which we reduce buffer sizes for CE runs
        //
        const int reduce = 100;

	//
	// Initialize data structures
	//
	ByteSeq byteSeq(ByteSeqSize / reduce, 0);
	pair<const Ice::Byte*, const Ice::Byte*> byteArr;
	byteArr.first = &byteSeq[0];
	byteArr.second = byteArr.first + byteSeq.size();

	StringSeq stringSeq(StringSeqSize / reduce, "hello");

	StringDoubleSeq structSeq(StringDoubleSeqSize / reduce);
	int i;
	for(i = 0; i < StringDoubleSeqSize / reduce; ++i)
	{
	    structSeq[i].s = "hello";
	    structSeq[i].d = 3.14;
	}

	FixedSeq fixedSeq(FixedSeqSize / reduce);
	for(i = 0; i < FixedSeqSize / reduce; ++i)
	{
	    fixedSeq[i].i = 0;
	    fixedSeq[i].j = 0;
	    fixedSeq[i].d = 0;
	}

	const int repetitions = 1000;

	// Initial ping to setup the connection.
	throughput->ice_ping();

	::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, 
		      (LPARAM)L"Running throughput tests (this may take a while)\r\n");

	for(int type = 0; type < 4; ++type)
	{
	    wchar_t* data;
	    int seqSize;
	    if(type == 0)
	    {
	        data = L"byte";
		seqSize = ByteSeqSize / reduce;
	    }
	    else if(type == 1)
	    {
	        data = L"string";
		seqSize = StringSeqSize / reduce;
	    }
	    else if(type == 2)
	    {
	        data = L"variable-length struct";
		seqSize = StringDoubleSeqSize / reduce;
	    }
	    else if(type == 3)
	    {
	        data = L"fixed-length struct";
		seqSize = FixedSeqSize / reduce;
	    }
	    
	    for(int mode = 0; mode < 4; ++mode)
	    {
	    	wchar_t* action;
	    	wchar_t* qualifier = L"";
		if(mode == 0)
		{
		    action = L"sending";
		}
		else if(mode == 1)
		{
		    action = L"sending";
		    qualifier = L" as oneway";
		}
		else if(mode == 2)
		{
		    action = L"receiving";
		}
		else if(mode == 3)
		{
		    action = L"sending and receiving";
		}

	        wchar_t buf[1000];
	        wsprintf(buf, L"\r\n%s %d %s sequences of size %d%s\r\n", action, repetitions, data, seqSize,
			 qualifier);
	        ::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf);

	        IceUtil::Time tm = IceUtil::Time::now();
	        for(i = 0; i < repetitions; ++i)
	        {
                    switch(type)
                    {
                        case 0:
                        {
                            switch(mode)
                            {
                                case 0:
                                {
                                    throughput->sendByteSeq(byteArr);
                                    break;
                                }

                                case 1:
                                {
                                    throughputOneway->sendByteSeq(byteArr);
                                    break;
                                }

                                case 2:
                                {
                                    throughput->recvByteSeq();
                                    break;
                                }

                                case 3:
                                {
                                    throughput->echoByteSeq(byteSeq);
                                    break;
                                }
                            }
                            break;
                        }

                        case 1:
                        {
                            switch(mode)
                            {
                                case 0:
                                {
                                    throughput->sendStringSeq(stringSeq);
                                    break;
                                }

                                case 1:
                                {
                                    throughputOneway->sendStringSeq(stringSeq);
                                    break;
                                }

                                case 2:
                                {
                                    throughput->recvStringSeq();
                                    break;
                                }

                                case 3:
                                {
                                    throughput->echoStringSeq(stringSeq);
                                    break;
                                }
                            }
                            break;
                        }

                        case 2:
                        {
                            switch(mode)
                            {
                                case 0:
                                {
                                    throughput->sendStructSeq(structSeq);
                                    break;
                                }

                                case 1:
                                {
                                    throughputOneway->sendStructSeq(structSeq);
                                    break;
                                }

                                case 2:
                                {
                                    throughput->recvStructSeq();
                                    break;
                                }

                                case 3:
                                {
                                    throughput->echoStructSeq(structSeq);
                                    break;
                                }
                            }
                            break;
                        }

                        case 3:
                        {
                            switch(mode)
                            {
                                case 0:
                                {
                                    throughput->sendFixedSeq(fixedSeq);
                                    break;
                                }

                                case 1:
                                {
                                    throughputOneway->sendFixedSeq(fixedSeq);
                                    break;
                                }

                                case 2:
                                {
                                    throughput->recvFixedSeq();
                                    break;
                                }

                                case 3:
                                {
                                    throughput->echoFixedSeq(fixedSeq);
                                    break;
                                }
                            }
                            break;
                        }
		    }

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
	
		wsprintf(buf, L"\r\ntime for %d sequences: %.04fms\r\ntime per sequence: %.04fms\r\n",
		         repetitions, tm.toMilliSecondsDouble(), tm.toMilliSecondsDouble() / repetitions);
		::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf);

                int wireSize = 0;
                switch(type)
                {
                    case 0:
                    {
                        wireSize = 1;
                        break;
                    }
                    case 1:
                    {
                        wireSize = static_cast<int>(stringSeq[0].size());
                        break;
                    }
                    case 2:
                    {
                        wireSize = static_cast<int>(structSeq[0].s.size());
                        wireSize += 8; // Size of double on the wire.
                        break;
                    }
                    case 3:
                    {
                        wireSize = 16; // Size of two ints and a double on the wire.
                        break;
                    }
                }
                double mbit = repetitions * seqSize * wireSize * 8.0 / tm.toMicroSeconds();
                if(mode == 3)
                {
                    mbit *= 2;
                }
		wsprintf(buf, L"throughput: %.04f MBit/s\r\n", mbit);
		::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf);
	    }
	}
	
	::SendMessage(editHwnd, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)L"\r\nThroughput tests completed\r\n");

    }
    catch(const Ice::Exception& ex)
    {
	TCHAR wtext[1024];
	string err = ex.toString();
	mbstowcs(wtext, err.c_str(), err.size());
	MessageBox(NULL, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

	status = EXIT_FAILURE;
    }

    //
    // Run the message pump.
    //
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
	    TCHAR wtext[1024];
	    string err = ex.toString();
	    mbstowcs(wtext, err.c_str(), err.size());
	    MessageBox(NULL, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

	    status = EXIT_FAILURE;
	}
    }

    return status;
}
