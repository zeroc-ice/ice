//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_FACET_MAP_H
#define ICE_FACET_MAP_H

#include "ObjectF.h"
#include <map>

namespace Ice
{
    /**
     * A mapping from facet name to servant.
     */
    using FacetMap = std::map<std::string, ObjectPtr>;
}

#endif
