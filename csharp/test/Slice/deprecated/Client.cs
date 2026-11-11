// Copyright (c) ZeroC, Inc.

#nullable enable

using DeprecatedTest;
using System.Reflection;
using Test;

namespace Slice.deprecated;

// Make sure we can implement a deprecated interface without warning.
public class MyInterfaceI : MyInterfaceDisp_
{
    public override void opInt(int i, Ice.Current current)
    {
    }
}

public class Client : TestHelper
{
    private static void allTests()
    {
        Console.Out.Write("testing [Obsolete] attributes in generated code... ");
        Console.Out.Flush();

        // Deprecated definitions carry the attribute, with the deprecation message when there is one.
#pragma warning disable CS0612, CS0618 // typeof on an obsolete type warns
        testObsolete(typeof(MyClass));
        testObsolete(typeof(MyStruct));
        testObsolete(typeof(MyException));
        testObsolete(typeof(MyEnum));
        testObsolete(typeof(MyInterfacePrx));
        testObsolete(typeof(MyInterfacePrxHelper));
#pragma warning restore CS0612, CS0618
        testObsolete(member(typeof(MyIntConst), "value"));
        testObsolete(member(typeof(MyOtherClass), "i"));
        testObsolete(member(typeof(MyOtherException), "reason"));
        testObsolete(member(typeof(MyOtherEnum), "B"), "Use A instead.");
        testObsolete(member(typeof(MyOtherInterfacePrx), "opLong"), "Use opInt() instead.");
        testObsolete(member(typeof(MyOtherInterfacePrx), "opLongAsync"), "Use opInt() instead.");

        // The skeleton side never carries the attribute: implementing a deprecated interface or operation must
        // not warn.
        testNotObsolete(typeof(MyInterface));
        testNotObsolete(typeof(MyInterfaceDisp_));
        testNotObsolete(typeof(AsyncMyInterface));
        testNotObsolete(typeof(AsyncMyInterfaceDisp_));
        testNotObsolete(member(typeof(MyOtherInterface), "opLong"));
        testNotObsolete(member(typeof(MyOtherInterfaceDisp_), "opLong"));
        testNotObsolete(member(typeof(AsyncMyOtherInterface), "opLongAsync"));
        testNotObsolete(member(typeof(AsyncMyOtherInterfaceDisp_), "opLongAsync"));

        // Definitions that are not deprecated don't get the attribute.
        testNotObsolete(typeof(MyOtherClass));
        testNotObsolete(typeof(MyOtherException));
        testNotObsolete(typeof(MyOtherEnum));
        testNotObsolete(member(typeof(MyOtherEnum), "A"));
        testNotObsolete(typeof(MyOtherInterfacePrx));
        testNotObsolete(typeof(MyOtherInterfacePrxHelper));
        testNotObsolete(member(typeof(MyOtherInterfacePrx), "opInt"));
        testNotObsolete(member(typeof(MyOtherInterfacePrx), "opIntAsync"));

        Console.Out.WriteLine("ok");
    }

    // Returns the single public member of 'type' named 'name'.
    private static MemberInfo member(Type type, string name)
    {
        MemberInfo[] members = type.GetMember(name);
        test(members.Length == 1);
        return members[0];
    }

    private static void testObsolete(MemberInfo member, string? message = null)
    {
        ObsoleteAttribute? attribute = member.GetCustomAttribute<ObsoleteAttribute>();
        test(attribute is not null && attribute.Message == message);
    }

    private static void testNotObsolete(MemberInfo member) =>
        test(member.GetCustomAttribute<ObsoleteAttribute>() is null);

    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        allTests();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
