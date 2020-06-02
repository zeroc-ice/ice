//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.DictMapping.AMD
{
    public sealed class MyClass : IMyClass
    {
        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<(IReadOnlyDictionary<int, int>, IReadOnlyDictionary<int, int>)> opNVAsync(
            Dictionary<int, int> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, string>, IReadOnlyDictionary<string, string>)> opNRAsync(
            Dictionary<string, string> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, Dictionary<int, int>>, IReadOnlyDictionary<string, Dictionary<int, int>>)>
        opNDVAsync(Dictionary<string, Dictionary<int, int>> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, Dictionary<string, string>>, IReadOnlyDictionary<string, Dictionary<string, string>>)>
        opNDRAsync(Dictionary<string, Dictionary<string, string>> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, int[]>, IReadOnlyDictionary<string, int[]>)>
        opNDAISAsync(Dictionary<string, int[]> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, List<int>>, IReadOnlyDictionary<string, List<int>>)>
        opNDGISAsync(Dictionary<string, List<int>> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, string[]>, IReadOnlyDictionary<string, string[]>)>
        opNDASSAsync(Dictionary<string, string[]> i, Current current) => ToReturnValue(i);

        public ValueTask<(IReadOnlyDictionary<string, List<string>>, IReadOnlyDictionary<string, List<string>>)>
        opNDGSSAsync(Dictionary<string, List<string>> i, Current current) => ToReturnValue(i);

        private static ValueTask<(IReadOnlyDictionary<TKey, TValue>, IReadOnlyDictionary<TKey, TValue>)>
        ToReturnValue<TKey, TValue>(Dictionary<TKey, TValue> input) where TKey : notnull =>
            new ValueTask<(IReadOnlyDictionary<TKey, TValue>, IReadOnlyDictionary<TKey, TValue>)>((input, input));
    }
}
