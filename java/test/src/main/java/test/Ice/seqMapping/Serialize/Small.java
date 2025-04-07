// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping.Serialize;

import java.io.Serializable;

public class Small implements Serializable // Fewer than 254 bytes when serialized.
{
    public int i;
}
