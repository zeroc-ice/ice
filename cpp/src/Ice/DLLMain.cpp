// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/EventLoggerI.h>

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);

extern "C"
{

BOOL WINAPI
ice_DLL_Main(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
    if(!_CRT_INIT(hDLL, reason, reserved))
    {
        return FALSE;
    }

    if(reason == DLL_PROCESS_ATTACH)
    {
        Ice::EventLoggerI::setModuleHandle(hDLL);
    }

    return TRUE;
}

}
