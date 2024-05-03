// Copyright (c) ZeroC, Inc.

namespace Ice;

public interface MarshaledResult
{
    OutputStream getOutputStream(Current current);
};
