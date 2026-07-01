// Copyright (c) ZeroC, Inc.

#ifndef ICE_IAP_MATCH_H
#define ICE_IAP_MATCH_H

#include <string>
#include <vector>

namespace IceObjC
{
    // Returns true if an accessory with the given properties satisfies the endpoint's
    // manufacturer/modelNumber/name/protocol filters. Empty filter strings match any value. Extracted as a
    // free function so the matching logic can be unit-tested without the ExternalAccessory framework.
    bool iAPMatches(
        const std::string& manufacturer,
        const std::string& modelNumber,
        const std::string& name,
        const std::string& protocol,
        bool connected,
        const std::string& accessoryManufacturer,
        const std::string& accessoryModelNumber,
        const std::string& accessoryName,
        const std::vector<std::string>& accessoryProtocols);
}

#endif
