// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

public interface PropertiesAdminUpdateCallback
{
    void updated(Dictionary<string, string> changes);
}

public interface NativePropertiesAdmin
{
    [Obsolete("This method is deprecated. Use addUpdateCallback(System.Action<Dictionary<string, string>> callback) instead.")]
    void addUpdateCallback(PropertiesAdminUpdateCallback callback);

    [Obsolete("This method is deprecated. Use removeUpdateCallback(System.Action<Dictionary<string, string>> callback) instead.")]
    void removeUpdateCallback(PropertiesAdminUpdateCallback callback);

    void addUpdateCallback(System.Action<Dictionary<string, string>> callback);

    void removeUpdateCallback(System.Action<Dictionary<string, string>> callback);
}
