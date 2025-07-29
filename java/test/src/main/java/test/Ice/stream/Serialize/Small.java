// Copyright (c) ZeroC, Inc.

package test.Ice.stream.Serialize;

import java.io.Serializable;

// Fewer than 254 bytes when serialized.
public class Small implements Serializable {
    public int i;
}
