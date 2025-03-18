// Copyright (c) ZeroC, Inc.

#ifndef ICE_SLICED_DATA_F_H
#define ICE_SLICED_DATA_F_H

#include <memory>
#include <vector>

namespace Ice
{
    struct SliceInfo;

    /// A shared pointer to a SliceInfo.
    using SliceInfoPtr = std::shared_ptr<SliceInfo>;

    /// The slices of unknown types.
    using SliceInfoSeq = std::vector<SliceInfoPtr>;

    class SlicedData;

    /// A shared pointer to a SlicedData.
    using SlicedDataPtr = std::shared_ptr<SlicedData>;

    class UnknownSlicedValue;

    /// A shared pointer to an UnknownSlicedValue.
    using UnknownSlicedValuePtr = std::shared_ptr<UnknownSlicedValue>;
}

#endif
