// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <iomanip>

using namespace std;
using namespace Test;

void
Test::Neighbor::ice_print(ostream& os) const
{
    os << name;
    if (age)
    {
        os << " (" << *age << ")";
    }
    os << " @ " << address;
}

void
Test::MyCustomPrintException::ice_print(ostream& os) const
{
    os << "Custom print: " << message;
}

ostream&
Test::operator<<(ostream& os, Fruit value)
{
    os << (value == Fruit::Apple ? "yum" : "no thanks");
    return os;
}

template<class T>
void
testPrint(const T& value, const string& expected, const std::function<void(ostream&)>& manip = nullptr)
{
    ostringstream os;
    if (manip)
    {
        manip(os);
    }
    os << value;
    string result = os.str();
    if (result != expected)
    {
        cout << "\nexpected: " << expected << endl;
        cout << "got:      " << result << endl;
        test(false);
    }
}

void
testEnum()
{
    cout << "testing enum... " << flush;
    testPrint(FlagColor::Red, "Red");
    testPrint(FlagColor::OldGloryBlue, "OldGloryBlue");
    testPrint(static_cast<FlagColor>(10), "10");
    cout << "ok" << endl;
}

void
testCustomPrintEnum()
{
    cout << "testing enum with custom print... " << flush;
    testPrint(Fruit::Apple, "yum");
    cout << "ok" << endl;
}

void
testSimpleStruct()
{
    cout << "testing basic types... " << flush;
    SimpleStruct simpleStruct{true, 199, 201, 150000, -10000000, 3.14f, 152853.5047L, "hello", FlagColor::OldGloryBlue};
    testPrint(
        simpleStruct,
        "Test::SimpleStruct{myBool = true, myByte = 199, myShort = 201, myInt = 150000, myLong = -10000000, myFloat = "
        "3.14, myDouble = 152854, myString = hello, myEnum = OldGloryBlue}");

    // Increasing the floating-point precision to 9 digits changes the output (C++ default is 6).
    testPrint(
        simpleStruct,
        "Test::SimpleStruct{myBool = true, myByte = 199, myShort = 201, myInt = 150000, myLong = -10000000, myFloat = "
        "3.1400001, myDouble = 152853.505, myString = hello, myEnum = OldGloryBlue}",
        [](ostream& os) { os << setprecision(9); });
    cout << "ok" << endl;
}

void
testByteBoolStruct()
{
    cout << "testing sequence<bool> and sequence<byte>... " << flush;
    vector<byte> byteSeq{byte{100}, byte{150}, byte{255}};
    vector<bool> boolSeq{true, false, true};
    ByteBoolStruct byteBoolStruct{byteSeq, boolSeq};

    testPrint(byteBoolStruct, "Test::ByteBoolStruct{myByteSeq = [100, 150, 255], myBoolSeq = [true, false, true]}");

    testPrint(
        byteBoolStruct,
        "Test::ByteBoolStruct{myByteSeq = [0x64, 0x96, 0xff], myBoolSeq = [true, false, true]}",
        [](ostream& os) { os << hex << showbase; });
    cout << "ok" << endl;
}

void
testCustomStruct()
{
    cout << "testing struct with custom print... " << flush;
    CustomStruct customStruct{{"name", "cat"}, {3, 4}, {5, 6}};
    testPrint(
        customStruct,
        "Test::CustomStruct{myIdentity = cat/name, myProtocolVersion = 3.4, myEncodingVersion = 5.6}");
    cout << "ok" << endl;
}

void
testDictionaryStruct()
{
    cout << "testing dictionary... " << flush;
    DictionaryStruct dictionaryStruct{{{"key1", {1, 2, 3, 4}}, {"key2", {5, 6, 7, 8, 9}}}};
    testPrint(dictionaryStruct, "Test::DictionaryStruct{myDict = [{key1 : [1, 2, 3, 4]}, {key2 : [5, 6, 7, 8, 9]}]}");
    cout << "ok" << endl;
}

void
testClass()
{
    cout << "testing class... " << flush;
    auto monty = make_shared<Employee>("Monty", 61, "President", nullptr, vector<EmployeePtr>{});
    auto tommy = make_shared<Employee>("Tommy", std::nullopt, "VP", monty, vector<EmployeePtr>{});
    monty->directReports.push_back(tommy); // creates a circular reference

    testPrint(
        monty,
        "Test::Employee{name = Monty, age = 61, title = President, manager = nullptr, directReports = "
        "[Test::Employee{name = Tommy, age = nullopt, title = VP, manager = Test::Employee{...already "
        "printed...}, directReports = []}]}");

    testPrint(
        tommy,
        "Test::Employee{name = Tommy, age = nullopt, title = VP, manager = Test::Employee{name = Monty, age = 61, "
        "title = President, manager = nullptr, directReports = [Test::Employee{...already printed...}]}, directReports "
        "= []}");
    cout << "ok" << endl;
}

void
testException()
{
    cout << "testing exception... " << flush;
    MyDerivedException myDerivedException{"hello", 42, 84};
    testPrint(myDerivedException, "Test::MyDerivedException{message = hello, errorCode = 42, extraCode = 84}");
    cout << "ok" << endl;
}

void
testClassCustomPrint()
{
    cout << "testing class with custom print... " << flush;
    auto neighbor = make_shared<Neighbor>("Alice", 30, "123 Main St");
    testPrint(neighbor, "Alice (30) @ 123 Main St");

    // This works too
    testPrint(*neighbor, "Alice (30) @ 123 Main St");
    cout << "ok" << endl;
}

void
testExceptionCustomPrint()
{
    cout << "testing exception with custom print... " << flush;
    MyCustomPrintException exception{"hello"};
    testPrint(exception, "Custom print: hello");
    cout << "ok" << endl;
}

void
testProxy(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing proxy... " << flush;
    Ice::ObjectPrx object{communicator, "obj:tcp -h localhost -p 4061"};
    PrinterPrx printer{communicator, "printer:tcp -h localhost -p 10000"};
    ProxyStruct proxyStruct{object, nullopt, printer};
    testPrint(
        proxyStruct,
        "Test::ProxyStruct{object = obj:tcp -h localhost -p 4061, nullPrinter = , printer = printer:tcp -h localhost "
        "-p 10000}");

    cout << "ok" << endl;
}

void
testRemappedIdentifier()
{
    cout << "testing remapped identifier... " << flush;
    MyStruct myStruct{"hello"};
    testPrint(myStruct, "Test::MyStruct{myString = hello}"); // uses the C++ identifiers, not the Slice identifiers
    cout << "ok" << endl;
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    testEnum();
    testCustomPrintEnum();
    testSimpleStruct();
    testByteBoolStruct();
    testCustomStruct();
    testDictionaryStruct();
    testClass();
    testException();
    testClassCustomPrint();
    testExceptionCustomPrint();
    testProxy(communicator);
    testRemappedIdentifier();
}
