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
                public Task shutdownAsync(Current current)
                {
                    current.Adapter.Communicator.shutdown();
                    return Task.CompletedTask;
                }

                public Task<Test.MyClass.opNVResult>
                opNVAsync(Dictionary<int, int> i, Current current) => Task.FromResult(new Test.MyClass.opNVResult(i, i));

                public Task<Test.MyClass.opNRResult>
                opNRAsync(Dictionary<string, string> i, Current current) => Task.FromResult(new Test.MyClass.opNRResult(i, i));

                public Task<Test.MyClass.opNDVResult>
                opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Current current) => Task.FromResult(new Test.MyClass.opNDVResult(i, i));

                public Task<Test.MyClass.opNDRResult>
                opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Current current) => Task.FromResult(new Test.MyClass.opNDRResult(i, i));

                public Task<Test.MyClass.opNDAISResult>
                opNDAISAsync(Dictionary<string, int[]> i, Current current) => Task.FromResult(new Test.MyClass.opNDAISResult(i, i));

                public Task<Test.MyClass.opNDGISResult>
                opNDGISAsync(Dictionary<string, List<int>> i, Current current) => Task.FromResult(new Test.MyClass.opNDGISResult(i, i));

                public Task<Test.MyClass.opNDASSResult>
                opNDASSAsync(Dictionary<string, string[]> i, Current current) => Task.FromResult(new Test.MyClass.opNDASSResult(i, i));

                public Task<Test.MyClass.opNDGSSResult>
                opNDGSSAsync(Dictionary<string, List<string>> i, Current current) => Task.FromResult(new Test.MyClass.opNDGSSResult(i, i));
            }
        }
    }
}
