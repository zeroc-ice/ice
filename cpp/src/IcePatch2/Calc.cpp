// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePatch2/Util.h>

#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options] DIR\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-z, --bzip2          Compress files.\n"
        "-V, --verbose        Verbose mode.\n"
        ;
}

int
main(int argc, char* argv[])
{
    string dataDir;
    bool compress = false;
    bool verbose = false;

    int i;
    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            cout << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--bzip2") == 0)
        {
            compress = true;
        }
        else if(strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0)
        {
            verbose = true;
        }
        else if(argv[i][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            if(dataDir.empty())
            {
                dataDir = normalize(argv[i]);
            }
            else
            {
		cerr << argv[0] << ": too many arguments" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
        }
    }

    if(dataDir.empty())
    {
        cerr << argv[0] << ": no data directory specified" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    try
    {
	if(dataDir[0] != '/')
	{
#ifdef _WIN32
	    char cwd[_MAX_PATH];
	    if(_getcwd(cwd, _MAX_PATH) == NULL)
#else
	    char cwd[PATH_MAX];
	    if(getcwd(cwd, PATH_MAX) == NULL)
#endif
	    {
		throw "cannot get the current directory:\n" + lastError();
	    }
	    
	    dataDir = string(cwd) + '/' + dataDir;
	}

	FileInfoSeq infoSeq;
	getFileInfoSeq(dataDir, infoSeq, true, compress, verbose);
	saveFileInfoSeq(dataDir, infoSeq);
    }
    catch(const string& ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return EXIT_FAILURE;
    }
    catch(const char* ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
