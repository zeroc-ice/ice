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

                public Task<Test.MyClass.OpNVReturnValue>
                opNVAsync(Dictionary<int, int> i, Current current) => Task.FromResult(new Test.MyClass.OpNVReturnValue(i, i));

                public Task<Test.MyClass.OpNRReturnValue>
                opNRAsync(Dictionary<string, string> i, Current current) => Task.FromResult(new Test.MyClass.OpNRReturnValue(i, i));

                public Task<Test.MyClass.OpNDVReturnValue>
                opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Current current) => Task.FromResult(new Test.MyClass.OpNDVReturnValue(i, i));

                public Task<Test.MyClass.OpNDRReturnValue>
                opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Current current) => Task.FromResult(new Test.MyClass.OpNDRReturnValue(i, i));

                public Task<Test.MyClass.OpNDAISReturnValue>
                opNDAISAsync(Dictionary<string, int[]> i, Current current) => Task.FromResult(new Test.MyClass.OpNDAISReturnValue(i, i));

                public Task<Test.MyClass.OpNDGISReturnValue>
                opNDGISAsync(Dictionary<string, List<int>> i, Current current) => Task.FromResult(new Test.MyClass.OpNDGISReturnValue(i, i));

                public Task<Test.MyClass.OpNDASSReturnValue>
                opNDASSAsync(Dictionary<string, string[]> i, Current current) => Task.FromResult(new Test.MyClass.OpNDASSReturnValue(i, i));

                public Task<Test.MyClass.OpNDGSSReturnValue>
                opNDGSSAsync(Dictionary<string, List<string>> i, Current current) => Task.FromResult(new Test.MyClass.OpNDGSSReturnValue(i, i));
            }
        }
    }
}
