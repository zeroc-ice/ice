// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Unicode.h>
#include <IceUtil/FileUtil.h>
#include <TestCommon.h>

#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#endif

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
        IceUtilInternal::ifstream is((dir + "coeur.utf8"));
        test(is.good());
        IceUtilInternal::ifstream bis((dir + wcoeurFile), ios_base::binary);
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
        cout << "wstring (" << wstringEncoding << ") to UTF-8 conversion... ";

        IceUtilInternal::ifstream bis((dir + wcoeurFile), ios_base::binary);
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
        
        IceUtilInternal::ifstream nbis((dir + "coeur.utf8"), ios_base::binary);
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

#ifndef __MINGW32__
    {
        cout << "testing UTF-8 filename... ";
        IceUtilInternal::ifstream fn(dir + "filename.txt");
        string filename;
        getline(fn, filename);
        fn.close();
        
        string filepath = dir + filename;
        
        {
            IceUtilInternal::ofstream os(filepath);
            test(os.is_open());
            os << "dummy";
            os.close();
        }

        IceUtilInternal::isAbsolutePath(filepath);
        IceUtilInternal::structstat st;
        test(IceUtilInternal::stat(filepath, &st) == 0);

        test(IceUtilInternal::mkdir(filepath + ".directory", 0777) == 0);
        test(IceUtilInternal::directoryExists(filepath + ".directory"));
        test(IceUtilInternal::rmdir(filepath + ".directory") == 0);

        int fd = IceUtilInternal::open(filepath, O_RDONLY);
        test(fd > 0);
#if defined(_MSC_VER)
        test(_close(fd) == 0);
#   else
        test(::close(fd) == 0);
#   endif

        FILE* f = IceUtilInternal::fopen(filepath, "r");
        test(f != 0);
        test(::fclose(f) == 0);
        
        IceUtilInternal::ifstream is(filepath);
        string str;
        getline(is, str);
        test(str == "dummy");
        is.close();

        IceUtilInternal::ifstream is2;
        is2.open(filepath);
        getline(is2, str);
        test(str == "dummy");
        is2.close();

        IceUtilInternal::ofstream os(filepath + ".out");
        os << "dummy" << endl;
        os.close();

        IceUtilInternal::ofstream os2;
        os2.open(filepath + ".out", ios_base::app);
        os2 << "dummy2" << endl;
        os2.close();

        IceUtilInternal::ifstream is3;
        is3.open(filepath + ".out");
        getline(is3, str);
        test(str == "dummy");
        getline(is3, str);
        test(str == "dummy2");
        is3.close();

        test(IceUtilInternal::unlink(filepath + ".out") == 0);
        
        IceUtilInternal::unlink(filepath);
        
        cout << "ok" << endl;
    }
#endif
    return EXIT_SUCCESS;
}
