// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        dir = IceUtil::wstringToString(argv[1]);
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
            test(isLegalUTF8Sequence(reinterpret_cast<const Byte*>(line.data()), 
                                     reinterpret_cast<const Byte*>(line.data() + line.size())));
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
            test(isLegalUTF8Sequence(reinterpret_cast<const Byte*>(badUTF8[i].data()), 
                                     reinterpret_cast<const Byte*>(badUTF8[i].data() + badUTF8[i].size())) == false);

            try
            {
                wstring ws = IceUtil::stringToWstring(badUTF8[i]);
                test(false);
            }
            catch(const IceUtil::IllegalConversionException&)
            {}
        }     

	// TODO: need test for bad UTF-32 strings
#ifdef _WIN32
 
        wstring badWstring[] = { 
            wstring(1, wchar_t(0xD800)),
            wstring(2, wchar_t(0xDB7F)),
            L""
        };
        
        for(size_t i = 0; badWstring[i] != L""; ++i)
        {
            try
            {
                string s = IceUtil::wstringToString(badWstring[i]);
                test(false);
            }
            catch(const IceUtil::IllegalConversionException&)
            {}
        }
#endif

        cout << "ok" << endl;
       
    }
    return EXIT_SUCCESS;
}
