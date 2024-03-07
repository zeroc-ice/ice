//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
//

#include <Ice/ObjectF.h>
#include <map>

#ifndef ICE_FACET_MAP_H
#    define ICE_FACET_MAP_H

namespace Ice
{

    /**
     * A mapping from facet name to servant.
     */
    using FacetMap = ::std::map<::std::string, std::shared_ptr<Object>>;

}

#endif
