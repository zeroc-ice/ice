// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <TestCommon.h>
#include <fstream>

using namespace IceUtil;
using namespace std;

//
// Note that each file starts with a BOM; stringToWstring and wstringToString
// converts these BOMs back and forth.
//
int
main(int argc, char** argv)
{
    string dir = "";

    if(argc > 1)
    {
        dir = argv[1];
#ifdef _WIN32
        dir += "\\";
#else
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
        cout << "testing UTF-8 to wstring (" << wstringEncoding << ") conversion...";
        
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
        cout << "wstring (" << wstringEncoding << ") to UTF-8 conversion...";

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

    return EXIT_SUCCESS;
}
