//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.dictMapping
{
    public sealed class MyClass : Test.IMyClass
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public Test.IMyClass.OpNVReturnValue
        opNV(Dictionary<int, int> i, Current current) => new Test.IMyClass.OpNVReturnValue(i, i);

        public Test.IMyClass.OpNRReturnValue
        opNR(Dictionary<string, string> i, Current current) => new Test.IMyClass.OpNRReturnValue(i, i);

        public Test.IMyClass.OpNDVReturnValue
        opNDV(Dictionary<string, Dictionary<int, int>> i, Current current) => new Test.IMyClass.OpNDVReturnValue(i, i);

        public Test.IMyClass.OpNDRReturnValue
        opNDR(Dictionary<string, Dictionary<string, string>> i, Current current) => new Test.IMyClass.OpNDRReturnValue(i, i);

        public Test.IMyClass.OpNDAISReturnValue
        opNDAIS(Dictionary<string, int[]> i, Current current) => new Test.IMyClass.OpNDAISReturnValue(i, i);

        public Test.IMyClass.OpNDGISReturnValue
        opNDGIS(Dictionary<string, List<int>> i, Current current) => new Test.IMyClass.OpNDGISReturnValue(i, i);

        public Test.IMyClass.OpNDASSReturnValue
        opNDASS(Dictionary<string, string[]> i, Current current) => new Test.IMyClass.OpNDASSReturnValue(i, i);

        public Test.IMyClass.OpNDGSSReturnValue
        opNDGSS(Dictionary<string, List<string>> i, Current current) => new Test.IMyClass.OpNDGSSReturnValue(i, i);
    }
}
