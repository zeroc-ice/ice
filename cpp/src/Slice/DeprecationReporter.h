// Copyright (c) ZeroC, Inc.

#ifndef DEPRECATION_REPORTER_H
#define DEPRECATION_REPORTER_H

#include "Parser.h"

namespace Slice
{
    void emitDeprecationWarningsFor(const UnitPtr& unit);
}
#endif
