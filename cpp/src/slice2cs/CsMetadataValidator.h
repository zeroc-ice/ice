// Copyright (c) ZeroC, Inc.

#ifndef SLICE_CS_METADATA_VALIDATOR_H
#define SLICE_CS_METADATA_VALIDATOR_H

#include "../Slice/Parser.h"

namespace Slice
{

    // Validates the C# metadata using "cs:" prefix in the given unit.
    void validateCsMetadata(const UnitPtr& unit);

}

#endif
