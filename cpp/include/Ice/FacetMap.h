//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_FACET_MAP_H
#define ICE_FACET_MAP_H

#include <Ice/ObjectF.h>
#include <map>

namespace Ice
{
    /**
     * A mapping from facet name to servant.
     */
    using FacetMap = ::std::map<::std::string, std::shared_ptr<Object>>;
}

#endif
