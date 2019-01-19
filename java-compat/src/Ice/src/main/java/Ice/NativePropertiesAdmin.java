//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

public interface NativePropertiesAdmin
{
    void addUpdateCallback(PropertiesAdminUpdateCallback callback);
    void removeUpdateCallback(PropertiesAdminUpdateCallback callback);
}
