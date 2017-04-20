// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <TestCommon.h>

#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#endif
#include <fstream>

// Uncomment to include performance testing
//#define TEST_PERF

using namespace IceUtil;
using namespace std;

//
// Note that each file starts with a BOM; stringToWstring and wstringToString
// converts these BOMs back and forth.
//

//COMPILERFIX: MINGW doesn't support wmain for console applications.
#if defined(_WIN32) && !defined(__MINGW32__)

int
wmain(int argc, wchar_t* argv[])

#else

int
main(int argc, char* argv[])

#endif
{
    string dir = "";

    if(argc > 1)
    {
#ifdef _WIN32

#   ifdef __MINGW32__
        dir = argv[1];
#   else
        dir = wstringToString(argv[1]);
#   endif
        dir += "\\";
#else
        dir = argv[1];
        dir += "/";
#endif
    }

    ostringstream os;
    os << "utf" << sizeof(wchar_t) * 8;
#ifdef ICE_LITTLE_ENDIAN
    os << "le";
#else
    os << "be";
#endif
    string wstringEncoding = os.str();
    string wcoeurFile = string("coeur.") + wstringEncoding;

    {
        cout << "testing UTF-8 to wstring (" << wstringEncoding << ") conversion... ";
        ifstream is((dir + "coeur.utf8").c_str());
        test(is.good());
        ifstream bis((dir + wcoeurFile).c_str(), ios_base::binary);
        test(bis.good());

        int lineNumber = 0;

        do
        {
            string line;
            getline(is, line, '\n');
            lineNumber++;
            wstring wline = stringToWstring(line);

            for(size_t i = 0; i < wline.length(); ++i)
            {
                wchar_t wc = wline[i];
                const char* buffer = reinterpret_cast<char*>(&wc);
                for(size_t j = 0; j < sizeof(wchar_t); ++j)
                {
                    test(bis.good());
                    char c;
                    bis.get(c);
                    if(buffer[j] != c)
                    {
                        cerr << "Error at line " << lineNumber << " column " << i << endl;
                        cerr << "buffer[j] == " << hex << (int)static_cast<unsigned char>(buffer[j]) << endl;
                        cerr << "c == " << hex << (int)static_cast<unsigned char>(c) << endl;
                    }
                    test(buffer[j] == c);
                }
            }
            //
            // Skip newline character (Unix-style newline)
            //
            if(is.good())
            {
                for(size_t j = 0; j < sizeof(wchar_t); ++j)
                {
                    test(bis.good());
                    char c;
                    bis.get(c);
                }
            }
            else
            {
                char c;
                bis.get(c);
                test(bis.eof());
            }
        } while(is.good());

        cout << "ok" << endl;
    }

    {
        cout << "testing wstring (" << wstringEncoding << ") to UTF-8 conversion... ";

        ifstream bis((dir + wcoeurFile).c_str(), ios_base::binary);
        test(bis.good());

        wstring ws;
        char c;

        do
        {
            wchar_t wc;
            char* buffer = reinterpret_cast<char*>(&wc);

            for(size_t j = 0; j < sizeof(wchar_t); ++j)
            {
                if(!bis.good())
                {
                    break;
                }
                bis.get(c);
                buffer[j] = c;
            }

            if(bis.good())
            {
                ws.push_back(wc);
            }
        } while(bis.good());

        string s = wstringToString(ws);

        ifstream nbis((dir + "coeur.utf8").c_str(), ios_base::binary);
        test(nbis.good());

        for(size_t i = 0; i < s.size(); ++i)
        {
            test(nbis.good());
            nbis.get(c);
            char ci = s[i];

            if(c != ci)
            {
                cerr << "i == " << i << endl;
                cerr << "ci == " << hex << (int)static_cast<unsigned char>(ci) << endl;
                cerr << "c == " << hex << (int)static_cast<unsigned char>(c) << endl;
            }
            test(c == s[i]);
        }
        test(!nbis.eof());
        nbis.get(c);
        test(nbis.eof());

        cout << "ok" << endl;
    }

    {
        cout << "testing wstring with surrogates... ";

        //
        // Euro sign (U+20AC) is encoded with 1 UTF-16 code unit, and 3 UTF-8 code units
        // U+10437 is a Deseret character, encoded with 2 UTF-16 code units, and 4 UTF-8 code units
        //
        wstring ws = L"\u20ac\u20ac\U00010437";

        if(sizeof(wchar_t) == 2)
        {
            test(ws.length() == 4);
        }
        else
        {
            test(sizeof(wchar_t) == 4);
            test(ws.length() == 3);
        }

        //
        // The Unicode string converter implementation allocates an initial buffer
        // of size max(2 * (sourceEnd - sourceStart), 4).
        // With UTF-16 encoding, that's 8 and the first 2 euros will use the first 6
        // bytes of the initial buffer.

        string ns = wstringToString(ws);

        const string good = "\xE2\x82\xAC\xE2\x82\xAC\xF0\x90\x90\xB7";
        test(ns == good);
        test(ws == stringToWstring(ns));

        cout << "ok" << endl;

        cout << "testing IceUtilInternal::toUTF16, toUTF32 and fromUTF32... ";

        vector<Byte> u8 = vector<Byte>(reinterpret_cast<const Byte*>(ns.data()),
                                       reinterpret_cast<const Byte*>(ns.data() + ns.length()));

        vector<unsigned short> u16 = IceUtilInternal::toUTF16(u8);
        test(u16.size() == 4);
        test(u16[0] == 0x20ac);
        test(u16[1] == 0x20ac);
        test(u16[2] == 0xd801);
        test(u16[3] == 0xdc37);

        vector<unsigned int> u32 = IceUtilInternal::toUTF32(u8);
        test(u32.size() == 3);
        test(u32[0] == 0x20ac);
        test(u32[1] == 0x20ac);
        test(u32[2] == 0x10437);

        vector<Byte> nu8 = IceUtilInternal::fromUTF32(u32);
        test(nu8 == u8);

        cout << "ok" << endl;
    }

#ifdef TEST_PERF
    {
        // The only performance-critical code is the UnicodeWstringConverter
        // that is used whenever we marshal/unmarshal wstrings.

        const long iterations = 5000000;
        const wstring ws = L"abcdefghijklmnopqrstuvwxyz+\u20ac\u20ac\U00010437";
        const string ns = wstringToString(ws);
        test(stringToWstring(ns) == ws);

        cout << "testing performance with " << iterations << " iterations... ";

        IceUtil::Time toU8 = IceUtil::Time::now(IceUtil::Time::Monotonic);
        for(long i = 0; i < iterations; ++i)
        {
            test(wstringToString(ws) == ns);
        }
        IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
        toU8 = now - toU8;

        IceUtil::Time fromU8 = now;
        for(long i = 0; i < iterations; ++i)
        {
            test(stringToWstring(ns) == ws);
        }
        fromU8 = IceUtil::Time::now(IceUtil::Time::Monotonic) - fromU8;

        cout << "toUTF8 = " << toU8 * 1000 << " ms; fromUTF8 = "
             << fromU8 * 1000 << " ms ok" << endl;
    }

#endif


    {
        cout << "testing error handling... ";

        // From http://stackoverflow.com/questions/1301402/example-invalid-utf8-string

        string badUTF8[] = {
            "\xc3\x28",
            "\xa0\xa1",
            "\xe2\x28\xa1",
            "\xe2\x82\x28",
            "\xf0\x28\x8c\xbc",
            "\xf0\x90\x28\xbc",
            "\xf0\x28\x8c\x28",
            "\xf8\xa1\xa1\xa1\xa1",
            "\xfc\xa1\xa1\xa1\xa1\xa1",
            ""
        };

        for(size_t i = 0; badUTF8[i] != ""; ++i)
        {
            try
            {
                wstring ws = stringToWstring(badUTF8[i]);
                wcerr << L"Unexpected: " << ws << endl;
                test(false);
            }
            catch(const IllegalConversionException&)
            {}
        }

        // TODO: need test for bad UTF-32 strings
#ifdef _WIN32

        // Note: for an unknown reason, the conversion works without
        // the extra letter (x below) when using codecvt_utf8_utf16.
        wstring badWstring[] = {
#   ifdef ICE_HAS_CODECVT_UTF8
            wstring(1, wchar_t(0xD800)) + L"x",
#   else
            wstring(1, wchar_t(0xD800)),
#   endif
            wstring(2, wchar_t(0xDB7F)),
            L""
        };

        for(size_t i = 0; badWstring[i] != L""; ++i)
        {
            try
            {
                string s = wstringToString(badWstring[i]);
                test(false);
            }
            catch(const IllegalConversionException&)
            {}
        }
#endif

        cout << "ok" << endl;

    }
    return EXIT_SUCCESS;
}
