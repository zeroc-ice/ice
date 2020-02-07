//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice.dictMapping.AMD
{
    public sealed class MyClass : Test.IMyClass
    {
        public Task shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return Task.CompletedTask;
        }

        public Task<(Dictionary<int, int>, Dictionary<int, int>)>
        opNVAsync(Dictionary<int, int> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, string>, Dictionary<string, string>)>
        opNRAsync(Dictionary<string, string> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, Dictionary<int, int>>, Dictionary<string, Dictionary<int, int>>)>
        opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, Dictionary<string, string>>, Dictionary<string, Dictionary<string, string>>)>
        opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, int[]>, Dictionary<string, int[]>)>
        opNDAISAsync(Dictionary<string, int[]> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, List<int>>, Dictionary<string, List<int>>)>
        opNDGISAsync(Dictionary<string, List<int>> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, string[]>, Dictionary<string, string[]>)>
        opNDASSAsync(Dictionary<string, string[]> i, Current current) => Task.FromResult((i, i));

        public Task<(Dictionary<string, List<string>>, Dictionary<string, List<string>>)>
        opNDGSSAsync(Dictionary<string, List<string>> i, Current current) => Task.FromResult((i, i));
    }
}
