// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class HI : Test.HDisp_
{
    public HI(Ice.Communicator communicator) => _communicator = communicator;

    public override string callG(Ice.Current current) => "G";

    public override string callH(Ice.Current current) => "H";

    public override void shutdown(Ice.Current current) => _communicator.shutdown();

    private readonly Ice.Communicator _communicator;
}
