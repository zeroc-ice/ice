// Copyright (c) ZeroC, Inc.

namespace Ice.dictMapping;

public sealed class MyClassI : Test.MyClassDisp_
{
    public override void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public override Dictionary<int, int> opNV(Dictionary<int, int> i, out Dictionary<int, int> o,
                                              Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, string> opNR(Dictionary<string, string> i, out Dictionary<string, string> o,
                                                    Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, Dictionary<int, int>> opNDV(Dictionary<string, Dictionary<int, int>> i,
                                                                   out Dictionary<string, Dictionary<int, int>> o,
                                                                   Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, Dictionary<string, string>> opNDR(Dictionary<
                                                               string, Dictionary<string, string>> i,
                                                               out Dictionary<string, Dictionary<string, string>> o,
                                                               Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, int[]> opNDAIS(Dictionary<string, int[]> i,
                                                      out Dictionary<string, int[]> o,
                                                      Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, List<int>> opNDGIS(Dictionary<string, List<int>> i,
                                                          out Dictionary<string, List<int>> o,
                                                          Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, string[]> opNDASS(Dictionary<string, string[]> i,
                                                         out Dictionary<string, string[]> o,
                                                         Ice.Current current)
    {
        o = i;
        return i;
    }

    public override Dictionary<string, List<string>> opNDGSS(Dictionary<string, List<string>> i,
                                                             out Dictionary<string, List<string>> o,
                                                             Ice.Current current)
    {
        o = i;
        return i;
    }
}
