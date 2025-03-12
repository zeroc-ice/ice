// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace ZeroC.Ice.Slice.Tools.Common;

public class StreamReader
{
    public string Output { get; private set; } = "";

    public string Error { get; private set; } = "";

    public void WriteOutput(object sendingProcess, DataReceivedEventArgs args)
    {
        if (args.Data is not null)
        {
            Output += args.Data;
        }
    }

    public void WriteError(object sendingProcess, DataReceivedEventArgs args)
    {
        if (args.Data is not null)
        {
            Error += args.Data;
        }
    }
}
