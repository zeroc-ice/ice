//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ASYNC_RESULT_F_H
#define ICE_ASYNC_RESULT_F_H

#ifndef ICE_CPP11_MAPPING

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

class AsyncResult;
ICE_API IceUtil::Shared* upCast(::Ice::AsyncResult*);
typedef IceInternal::Handle<AsyncResult> AsyncResultPtr;

}

#endif
#endif
