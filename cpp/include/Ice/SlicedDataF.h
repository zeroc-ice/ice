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
class SlicedData;
class UnknownSlicedValue;

#ifdef ICE_CPP11_MAPPING
typedef ::std::shared_ptr<SliceInfo> SliceInfoPtr;
typedef ::std::shared_ptr<SlicedData> SlicedDataPtr;
typedef ::std::shared_ptr<UnknownSlicedValue> UnknownSlicedValuePtr;
#else
ICE_API IceUtil::Shared* upCast(SliceInfo*);
typedef IceInternal::Handle<SliceInfo> SliceInfoPtr;

ICE_API IceUtil::Shared* upCast(SlicedData*);
typedef IceInternal::Handle<SlicedData> SlicedDataPtr;

ICE_API IceUtil::Shared* upCast(UnknownSlicedValue*);
typedef IceInternal::Handle<UnknownSlicedValue> UnknownSlicedValuePtr;
#endif

typedef ::std::vector<SliceInfoPtr> SliceInfoSeq;

}

#endif
