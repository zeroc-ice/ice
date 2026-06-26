// Copyright (c) ZeroC, Inc.

namespace Ice.dictMapping.AMD;

public sealed class MyInterfaceI : Test.AsyncMyInterfaceDisp_
{
    public override Task shutdownAsync(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        return Task.CompletedTask;
    }

    public override Task<Test.MyInterface_OpNVResult>
    opNVAsync(Dictionary<int, int> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNVResult(i, i));

    public override Task<Test.MyInterface_OpNRResult>
    opNRAsync(Dictionary<string, string> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNRResult(i, i));

    public override Task<Test.MyInterface_OpNDVResult>
    opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNDVResult(i, i));

    public override Task<Test.MyInterface_OpNDRResult>
    opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNDRResult(i, i));

    public override Task<Test.MyInterface_OpNDAISResult>
    opNDAISAsync(Dictionary<string, int[]> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNDAISResult(i, i));

    public override Task<Test.MyInterface_OpNDGISResult>
    opNDGISAsync(Dictionary<string, List<int>> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNDGISResult(i, i));

    public override Task<Test.MyInterface_OpNDASSResult>
    opNDASSAsync(Dictionary<string, string[]> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNDASSResult(i, i));

    public override Task<Test.MyInterface_OpNDGSSResult>
    opNDGSSAsync(Dictionary<string, List<string>> i, Ice.Current current) =>
        Task.FromResult(new Test.MyInterface_OpNDGSSResult(i, i));
}
