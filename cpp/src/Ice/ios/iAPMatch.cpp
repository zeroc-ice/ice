// Copyright (c) ZeroC, Inc.

#include "iAPMatch.h"

#include <algorithm>

using namespace std;

bool
IceObjC::iAPMatches(
    const string& manufacturer,
    const string& modelNumber,
    const string& name,
    const string& protocol,
    bool connected,
    const string& accessoryManufacturer,
    const string& accessoryModelNumber,
    const string& accessoryName,
    const vector<string>& accessoryProtocols)
{
    if (!connected)
    {
        return false;
    }
    if (!manufacturer.empty() && manufacturer != accessoryManufacturer)
    {
        return false;
    }
    if (!modelNumber.empty() && modelNumber != accessoryModelNumber)
    {
        return false;
    }
    if (!name.empty() && name != accessoryName)
    {
        return false;
    }
    // Unlike the manufacturer/modelNumber/name filters, the protocol is required: the accessory must
    // advertise it. An empty protocol is never a wildcard and matches nothing.
    if (protocol.empty())
    {
        return false;
    }
    return find(accessoryProtocols.begin(), accessoryProtocols.end(), protocol) != accessoryProtocols.end();
}
