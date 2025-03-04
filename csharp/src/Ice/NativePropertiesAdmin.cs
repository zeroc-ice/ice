// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

public interface NativePropertiesAdmin
{
    void addUpdateCallback(System.Action<Dictionary<string, string>> callback);

    void removeUpdateCallback(System.Action<Dictionary<string, string>> callback);
}
