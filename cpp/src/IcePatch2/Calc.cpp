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

struct FileInfoPathEqual: public binary_function<const FileInfo&, const FileInfo&, bool>
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
	return lhs.path != rhs.path;
    }
};

struct FileInfoPathLess: public binary_function<const FileInfo&, const FileInfo&, bool>
{
    bool
    operator()(const FileInfo& lhs, const FileInfo& rhs)
    {
	return lhs.path < rhs.path;
    }
};

class CalcCB : public GetFileInfoSeqCB
{
public:

    virtual bool
    remove(const string& path)
    {
	cout << "removing: " << path << endl;
	return true;
    }

    virtual bool
    checksum(const string& path)
    {
	cout << "checksum: " << path << endl;
	return true;
    }

    virtual bool
    compress(const string& path)
    {
	cout << "compress: " << path << endl;
	return true;
    }
};

void
usage(const char* appName)
{
    cerr << "Usage: " << appName << " [options] DIR [FILES...]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-z, --compress       Always compress files.\n"
        "-Z, --no-compress    Never compress files.\n"
        "-V, --verbose        Verbose mode.\n"
        ;
}

int
main(int argc, char* argv[])
{
    string dataDir;
    StringSeq fileSeq;
    int compress = 1;
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
        else if(strcmp(argv[i], "-z") == 0 || strcmp(argv[i], "--compress") == 0)
        {
            compress = 2;
        }
        else if(strcmp(argv[i], "-Z") == 0 || strcmp(argv[i], "--no-compress") == 0)
        {
            compress = 0;
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
		fileSeq.push_back(normalize(argv[i]));
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
	{
#ifdef _WIN32
	    char cwd[_MAX_PATH];
	    if(_getcwd(cwd, _MAX_PATH) == NULL)
	    {
		throw "cannot get the current directory:\n" + lastError();
	    }
	    
	    if(dataDir[0] != '/' && !(dataDir.size() > 1 && isalpha(dataDir[0]) && dataDir[1] == ':'))
	    {
		dataDir = string(cwd) + '/' + dataDir;
	    }
	    
	    for(StringSeq::iterator p = fileSeq.begin(); p != fileSeq.end(); ++p)
	    {
		if((*p)[0] != '/' && !(p->size() > 1 && isalpha((*p)[0]) && (*p)[1] == ':'))
		{
		    *p = string(cwd) + '/' + *p;
		}
	    }
#else
	    char cwd[PATH_MAX];
	    if(getcwd(cwd, PATH_MAX) == NULL)
	    {
		throw "cannot get the current directory:\n" + lastError();
	    }
	    
	    if(dataDir[0] != '/')
	    {
		dataDir = string(cwd) + '/' + dataDir;
	    }
	    
	    for(StringSeq::iterator p = fileSeq.begin(); p != fileSeq.end(); ++p)
	    {
		if((*p)[0] != '/')
		{
		    *p = string(cwd) + '/' + *p;
		}
	    }
#endif
	}
	
	{
	    string dataDirWithSlash = dataDir + '/';

	    for(StringSeq::iterator p = fileSeq.begin(); p != fileSeq.end(); ++p)
	    {
		if(p->compare(0, dataDirWithSlash.size(), dataDirWithSlash) != 0)
		{
		    throw "`" + *p + "' is not a path in `" + dataDir + "'";
		}

		p->erase(0, dataDirWithSlash.size());
	    }
	}
	    
	FileInfoSeq infoSeq;
	    
	if(fileSeq.empty())
	{
	    CalcCB calcCB;
	    if(!getFileInfoSeq(dataDir, compress, verbose ? &calcCB : 0, infoSeq))
	    {
		return EXIT_FAILURE;
	    }
	}
	else
	{
	    loadFileInfoSeq(dataDir, infoSeq);

	    for(StringSeq::const_iterator p = fileSeq.begin(); p != fileSeq.end(); ++p)
	    {
		FileInfoSeq partialInfoSeq;

		CalcCB calcCB;
		if(!getFileInfoSeqSubDir(dataDir, *p, compress, verbose ? &calcCB : 0, partialInfoSeq))
		{
		    return EXIT_FAILURE;
		}

		FileInfoSeq newInfoSeq;
		newInfoSeq.reserve(infoSeq.size());

		set_difference(infoSeq.begin(),
			       infoSeq.end(),
			       partialInfoSeq.begin(),
			       partialInfoSeq.end(),
			       back_inserter(newInfoSeq),
			       FileInfoPathLess());

		infoSeq.swap(newInfoSeq);

		newInfoSeq.clear();
		newInfoSeq.reserve(infoSeq.size() + partialInfoSeq.size());

		set_union(infoSeq.begin(),
			  infoSeq.end(),
			  partialInfoSeq.begin(),
			  partialInfoSeq.end(),
			  back_inserter(newInfoSeq),
			  FileInfoPathLess());

		infoSeq.swap(newInfoSeq);
	    }
	}

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
