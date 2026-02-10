// Copyright (c) ZeroC, Inc.

#include "../../../src/Ice/Endian.h"
#include "Ice/Ice.h"
#include "Ice/IconvStringConverter.h"
#include "Test.h"
#include "TestHelper.h"

#include <clocale>
#include <iostream>

#ifdef _MSC_VER
#    pragma warning(disable : 4127) // conditional expression is constant
#    pragma warning(disable : 4310) // cast truncates constant value
#endif

using namespace std;

static bool useLocale = false;
static bool useIconv = true;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);

    string narrowEncoding;
    string wideEncoding;

#ifdef _WIN32
    useIconv = false;
#else
    //
    // Switch to French locale
    // (we just used the codeset for as default internal code for
    // stringConverter below)
    //
    useLocale =
        (setlocale(LC_ALL, "fr_FR.ISO8859-15") != nullptr || setlocale(LC_ALL, "fr_FR.iso885915@euro") != nullptr);
#endif

    if (useIconv)
    {
        narrowEncoding = "ISO8859-15";

        if (sizeof(wchar_t) == 4)
        {
            if constexpr (endian::native == endian::big)
            {
                wideEncoding = "UTF-32BE";
            }
            else
            {
                wideEncoding = "UTF-32LE";
            }
        }
        else
        {
            if constexpr (endian::native == endian::big)
            {
                wideEncoding = "UTF-16BE";
            }
            else
            {
                wideEncoding = "UTF-16LE";
            }
        }
    }

    {
#ifdef _WIN32
        //
        // 28605 == ISO 8859-15 codepage
        //
        setProcessStringConverter(Ice::createWindowsStringConverter(28605));
#else
        if (useLocale)
        {
            setProcessStringConverter(Ice::createIconvStringConverter<char>(""));
        }
        else
        {
            setProcessStringConverter(Ice::createIconvStringConverter<char>(narrowEncoding));
        }

        setProcessWstringConverter(Ice::createIconvStringConverter<wchar_t>(wideEncoding));

#endif

        Ice::CommunicatorHolder ich = initialize(argc, argv, properties);
        const auto& communicator = ich.communicator();
        Test::MyObjectPrx proxy(communicator, "test:" + getTestEndpoint());

        char oe = char(0xBD); // A single character in ISO Latin 9
        string msg = string("tu me fends le c") + oe + "ur!";
        cout << "testing string converter";
        if (useLocale)
        {
            cout << " (using locale)";
        }
        if (useIconv)
        {
            cout << " (using iconv)";
        }
        cout << "... " << flush;
        wstring wmsg = proxy->widen(msg);
        test(proxy->narrow(wmsg) == msg);
        test(wmsg.size() == msg.size());

        // Test stringToIdentity and identityToString

        string identStr = "cat/" + msg;
        Ice::Identity ident = Ice::stringToIdentity(identStr);
        test(ident.name == msg);
        test(ident.category == "cat");
        test(identityToString(ident, Ice::ToStringMode::Unicode) == identStr);

        identStr = identityToString(ident, Ice::ToStringMode::ASCII);
        test(identStr == "cat/tu me fends le c\\u0153ur!");
        test(Ice::stringToIdentity(identStr) == ident);
        identStr = identityToString(ident, Ice::ToStringMode::Compat);
        test(identStr == "cat/tu me fends le c\\305\\223ur!");
        test(Ice::stringToIdentity(identStr) == ident);

        cout << "ok" << endl;

        //
        // Test writeConverted with various string sizes and UTF-8 expansion ratios.
        //
        // OutputStream::writeConverted uses 1-byte size encoding when the input string is <= 84 bytes
        // (254 / 3, where 3x is the worst-case narrow-to-UTF-8 expansion), and 5-byte size encoding otherwise.
        //
        // With ISO 8859-15:
        //   - ASCII chars (0x00-0x7F): 1 byte -> 1 UTF-8 byte (1x expansion)
        //   - Most Latin chars like 0xE9 (é, U+00E9): 1 byte -> 2 UTF-8 bytes (2x expansion)
        //   - Euro sign 0xA4 (€, U+20AC): 1 byte -> 3 UTF-8 bytes (3x expansion)
        //
        cout << "testing writeConverted with small and large strings... " << flush;

        // Small strings (vsize <= 84): uses 1-byte size encoding
        test(proxy->echoString("") == "");
        test(proxy->echoString(string(50, 'a')) == string(50, 'a'));
        test(proxy->echoString(string(50, char(0xE9))) == string(50, char(0xE9)));
        test(proxy->echoString(string(50, char(0xA4))) == string(50, char(0xA4)));
        test(proxy->echoString(string(84, char(0xE9))) == string(84, char(0xE9)));

        // Large strings (vsize > 84): uses 5-byte size encoding
        test(proxy->echoString(string(85, char(0xE9))) == string(85, char(0xE9)));
        test(proxy->echoString(string(200, 'a')) == string(200, 'a'));
        test(proxy->echoString(string(200, char(0xE9))) == string(200, char(0xE9)));
        test(proxy->echoString(string(200, char(0xA4))) == string(200, char(0xA4)));
        test(proxy->echoString(string(1000, char(0xE9))) == string(1000, char(0xE9)));

        // Mixed content: ASCII + 2x expansion + 3x expansion
        string mixed;
        mixed += string(100, 'x');       // 100 ASCII bytes -> 100 UTF-8 bytes
        mixed += string(50, char(0xE9)); // 50 Latin bytes -> 100 UTF-8 bytes
        mixed += string(50, char(0xA4)); // 50 euro bytes -> 150 UTF-8 bytes
        test(proxy->echoString(mixed) == mixed);

        cout << "ok" << endl;
        proxy->shutdown();
    }
}

DEFINE_TEST(Client);
