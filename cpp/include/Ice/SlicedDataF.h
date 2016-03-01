// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SLICED_DATA_F_H
#define ICE_SLICED_DATA_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace Ice
{

struct SliceInfo;
ICE_API IceUtil::Shared* upCast(SliceInfo*);
typedef IceInternal::Handle<SliceInfo> SliceInfoPtr;
typedef ::std::vector<SliceInfoPtr> SliceInfoSeq;

class SlicedData;
ICE_API IceUtil::Shared* upCast(SlicedData*);
typedef IceInternal::Handle<SlicedData> SlicedDataPtr;

class UnknownSlicedObject;
ICE_API IceUtil::Shared* upCast(UnknownSlicedObject*);
typedef IceInternal::Handle<UnknownSlicedObject> UnknownSlicedObjectPtr;

}

#endif
