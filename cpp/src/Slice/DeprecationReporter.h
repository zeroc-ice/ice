// Copyright (c) ZeroC, Inc.

#ifndef DEPRECATION_REPORTER_H
#define DEPRECATION_REPORTER_H

#include "Parser.h"

namespace Slice
{
    /// Visits all Slice definitions in the provided unit, and emits a deprecation warning each time a deprecated Slice
    /// construct is referenced, unless the referencer is also marked deprecated.
    void emitDeprecationWarningsFor(const UnitPtr& unit);
}
#endif
