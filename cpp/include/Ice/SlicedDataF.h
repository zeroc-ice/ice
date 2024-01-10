//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SLICED_DATA_F_H
#define ICE_SLICED_DATA_F_H

#include <Ice/SharedPtr.h>

namespace Ice
{

struct SliceInfo;
class SlicedData;
class UnknownSlicedValue;

#ifdef ICE_CPP11_MAPPING
/// \cond INTERNAL
using SliceInfoPtr = ::std::shared_ptr<SliceInfo>;
using SlicedDataPtr = ::std::shared_ptr<SlicedData>;
using UnknownSlicedValuePtr = ::std::shared_ptr<UnknownSlicedValue>;
/// \endcond
#else
using SliceInfoPtr = SharedPtr<SliceInfo>;
using SlicedDataPtr = SharedPtr<SlicedData>;
using UnknownSlicedValuePtr = SharedPtr<UnknownSlicedValue>;
#endif

/** The slices of unknown types. */
typedef ::std::vector<SliceInfoPtr> SliceInfoSeq;

}

#endif
