// Copyright (c) ZeroC, Inc.

#ifndef DOC_COMMENT_PARSER_H
#define DOC_COMMENT_PARSER_H

#include "Parser.h"

namespace Slice
{
    /// Parses all doc-comments within the provided unit (in-place).
    void parseAllDocCommentsWithin(const UnitPtr& unit);
}
#endif
