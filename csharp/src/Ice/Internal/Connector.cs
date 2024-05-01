// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

public interface Connector
{
    // Create a transceiver without blocking. The transceiver may not be fully connected
    // until its initialize method is called.
    Transceiver connect();

    short type();
}
