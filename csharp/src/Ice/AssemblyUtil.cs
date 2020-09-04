//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Runtime.InteropServices;

namespace ZeroC.Ice
{
    public static class AssemblyUtil
    {
        public static readonly bool IsLinux = RuntimeInformation.IsOSPlatform(OSPlatform.Linux);
        public static readonly bool IsMacOS = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
        public static readonly bool IsMono = RuntimeInformation.FrameworkDescription.Contains("Mono");
        public static readonly bool IsWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);
    }
}
