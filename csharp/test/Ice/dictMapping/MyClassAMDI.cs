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
            current.Adapter.Communicator.shutdown();
            return Task.CompletedTask;
        }

        public Task<Test.IMyClass.OpNVReturnValue>
        opNVAsync(Dictionary<int, int> i, Current current) => Task.FromResult(new Test.IMyClass.OpNVReturnValue(i, i));

        public Task<Test.IMyClass.OpNRReturnValue>
        opNRAsync(Dictionary<string, string> i, Current current) => Task.FromResult(new Test.IMyClass.OpNRReturnValue(i, i));

        public Task<Test.IMyClass.OpNDVReturnValue>
        opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Current current) => Task.FromResult(new Test.IMyClass.OpNDVReturnValue(i, i));

        public Task<Test.IMyClass.OpNDRReturnValue>
        opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Current current) => Task.FromResult(new Test.IMyClass.OpNDRReturnValue(i, i));

        public Task<Test.IMyClass.OpNDAISReturnValue>
        opNDAISAsync(Dictionary<string, int[]> i, Current current) => Task.FromResult(new Test.IMyClass.OpNDAISReturnValue(i, i));

        public Task<Test.IMyClass.OpNDGISReturnValue>
        opNDGISAsync(Dictionary<string, List<int>> i, Current current) => Task.FromResult(new Test.IMyClass.OpNDGISReturnValue(i, i));

        public Task<Test.IMyClass.OpNDASSReturnValue>
        opNDASSAsync(Dictionary<string, string[]> i, Current current) => Task.FromResult(new Test.IMyClass.OpNDASSReturnValue(i, i));

        public Task<Test.IMyClass.OpNDGSSReturnValue>
        opNDGSSAsync(Dictionary<string, List<string>> i, Current current) => Task.FromResult(new Test.IMyClass.OpNDGSSReturnValue(i, i));
    }
}
