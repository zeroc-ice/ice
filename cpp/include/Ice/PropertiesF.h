// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROPERTIES_F_H
#define ICE_PROPERTIES_F_H

#include <memory>

namespace Ice
{
    class Properties;

    /// A shared pointer to a Properties.
    using PropertiesPtr = std::shared_ptr<Properties>;

    class PropertiesAdmin;
    class PropertiesAdminPrx;
}

#endif
