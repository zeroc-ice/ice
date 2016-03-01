// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ResponseHandler.h>

using namespace std;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(ResponseHandler* obj) { return obj; }

ResponseHandler::~ResponseHandler()
{
}

