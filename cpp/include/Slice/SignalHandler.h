// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_SIGNAL_HANDLER_H
#define SLICE_SIGNAL_HANDLER_H

#include <IceUtil/Config.h>
#include <IceUtil/OutputUtil.h>
#include <string>

#ifndef SLICE_API
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API ICE_DECLSPEC_EXPORT
#   else
#       define SLICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace Slice
{

typedef void (*SignalHandlerCloseCallback)();

class SLICE_API SignalHandler
{
public:

    static void addFileForCleanup(const std::string&);

    static void setCloseCallback(SignalHandlerCloseCallback);

    static void clearCleanupFileList();

    static void removeFilesOnInterrupt(int signal);
};

}

#endif
