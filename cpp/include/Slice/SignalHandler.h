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

typedef void (*SignalHandlerCallback)();

class SLICE_API SignalHandler
{
public:

    SignalHandler();
    ~SignalHandler();

    static void setCallback(SignalHandlerCallback);

    static void addFile(const std::string&);
};

}

#endif
