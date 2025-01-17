// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

#include <iomanip>

using namespace std;
using namespace Test;

template<class T>
void
testPrint(const T& value, const string& expected, std::optional<int> floatPrecision = std::nullopt)
{
    ostringstream os;
    if (floatPrecision)
    {
        os << setprecision(*floatPrecision);
    }
    os << value;
    string result = os.str();
    if (result != expected)
    {
        cout << "expected: " << expected << endl;
        cout << "got:      " << result << endl;
        test(false);
    }
}

void
testSimpleStruct()
{
    SimpleStruct simpleStruct{true, 199, 201, 150000, -100000000, 3.14f, 152853.5047l, "hello", FlagColor::Blue};
    testPrint(
        simpleStruct,
        "Test::SimpleStruct{myBool = true, myByte = 199, myShort = 201, myInt = 150000, myLong = -100000000, myFloat = "
        "3.14, myDouble = 152854, myString = hello, myEnum = 2}");

    // Increasing the floating-point precision to 9 digits changes the output (C++ default is 6).
    testPrint(
        simpleStruct,
        "Test::SimpleStruct{myBool = true, myByte = 199, myShort = 201, myInt = 150000, myLong = -100000000, myFloat = "
        "3.1400001, myDouble = 152853.505, myString = hello, myEnum = 2}",
        9);
}

void
testByteBoolStruct()
{
    vector<byte> byteSeq{byte{100}, byte{150}, byte{255}};
    vector<bool> boolSeq{true, false, true};
    ByteBoolStruct byteBoolStruct{byteSeq, boolSeq};

    testPrint(byteBoolStruct, "Test::ByteBoolStruct{myByteSeq = [100, 150, 255], myBoolSeq = [true, false, true]}");
}

void
testCustomStruct()
{
    CustomStruct customStruct{{"name", "cat"}, {3, 4}, {5, 6}};
    testPrint(
        customStruct,
        "Test::CustomStruct{myIdentity = cat/name, myProtocolVersion = 3.4, myEncodingVersion = 5.6}");
}

void
testDictionaryStruct()
{
    DictionaryStruct dictionaryStruct{{{"key1", {1, 2, 3, 4}}, {"key2", {5, 6, 7, 8, 9}}}};
    testPrint(dictionaryStruct, "Test::DictionaryStruct{myDict = [{key1 : [1, 2, 3, 4]}, {key2 : [5, 6, 7, 8, 9]}]}");
}

void
testClass()
{
    auto monty = make_shared<Employee>("Monty", 61, "President", nullptr, vector<EmployeePtr>{});
    auto tommy = make_shared<Employee>("Tommy", std::nullopt, "VP", monty, vector<EmployeePtr>{});
    monty->directReports.push_back(tommy); // creates a circular reference

    testPrint(
        monty,
        "Test::Employee{name = Monty, age = 61, title = President, manager = nullptr, directReports = "
        "[Test::Employee{name = Tommy, age = std::nullopt, title = VP, manager = Test::Employee{...already "
        "printed...}, directReports = []}]}");

    testPrint(
        tommy,
        "Test::Employee{name = Tommy, age = std::nullopt, title = VP, manager = Test::Employee{name = Monty, age = 61, "
        "title = President, manager = nullptr, directReports = [Test::Employee{...already printed...}]}, directReports "
        "= []}");
}

void
allTests(Test::TestHelper*)
{
    testSimpleStruct();
    testByteBoolStruct();
    testCustomStruct();
    testDictionaryStruct();
    testClass();
}
