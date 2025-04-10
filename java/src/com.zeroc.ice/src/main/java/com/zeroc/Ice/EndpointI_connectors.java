// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.List;

/**
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public interface EndpointI_connectors {
    void connectors(List<Connector> connectors);

    void exception(LocalException ex);
}
