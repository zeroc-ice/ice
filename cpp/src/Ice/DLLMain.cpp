// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/EventLoggerI.h>

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);

extern "C"
{

BOOL WINAPI
Ice_DLL_Main(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
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
