// Copyright (c) ZeroC, Inc.

#include "Ice/Service.h"

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE, DWORD, LPVOID);

extern "C"
{
    BOOL WINAPI ice_DLL_Main(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
    {
        //
        // During ATTACH, we must call _CRT_INIT first.
        //
        if (reason == DLL_PROCESS_ATTACH || reason == DLL_THREAD_ATTACH)
        {
            if (!_CRT_INIT(hDLL, reason, reserved))
            {
                return FALSE;
            }
        }

        if (reason == DLL_PROCESS_ATTACH)
        {
            Ice::Service::setModuleHandle(hDLL);
        }

        //
        // During DETACH, we must call _CRT_INIT last.
        //
        if (reason == DLL_PROCESS_DETACH || reason == DLL_THREAD_DETACH)
        {
            if (!_CRT_INIT(hDLL, reason, reserved))
            {
                return FALSE;
            }
        }

        return TRUE;
    }
}
