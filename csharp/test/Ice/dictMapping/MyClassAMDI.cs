//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice
{
    namespace dictMapping
    {
        namespace AMD
        {
            public sealed class MyClassI : Test.MyClass
            {
                public Task shutdownAsync(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                    return Task.CompletedTask;
                }

                public Task<(Dictionary<int, int> returnValue, Dictionary<int, int> o)>
                opNVAsync(Dictionary<int, int> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, string> returnValue, Dictionary<string, string> o)>
                opNRAsync(Dictionary<string, string> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, Dictionary<int, int>> returnValue, Dictionary<string, Dictionary<int, int>> o)>
                opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, Dictionary<string, string>> returnValue, Dictionary<string, Dictionary<string, string>> o)>
                opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, int[]> returnValue, Dictionary<string, int[]> o)>
                opNDAISAsync(Dictionary<string, int[]> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, List<int>> returnValue, Dictionary<string, List<int>> o)>
                opNDGISAsync(Dictionary<string, List<int>> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, string[]> returnValue, Dictionary<string, string[]> o)>
                opNDASSAsync(Dictionary<string, string[]> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Dictionary<string, List<string>> returnValue, Dictionary<string, List<string>> o)>
                opNDGSSAsync(Dictionary<string, List<string>> i, Ice.Current current)
                {
                    return Task.FromResult((i, i));
                }
            }
        }
    }
}
