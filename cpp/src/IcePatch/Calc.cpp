// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <Ice/Ice.h>
#include <IcePatch/Util.h>
#include <set>
#ifdef _WIN32
#   include <direct.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch;

class CalcApp : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);

private:

    void update(const string&);
    Long compare(const string&, const string&);

    bool _totals;
};

void
CalcApp::usage()
{
    cerr << "Usage: " << appName() << " [options] DIR [OLD_DIR OLD_DIR ...]\n";
    cerr <<     
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n"
        "-t                   Show totals.\n"
        ;
}

int
CalcApp::run(int argc, char* argv[])
{
    string dataDir;
    vector<string> oldDataDirs;
    _totals;

    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("t");
    
    vector<string> args;
    try
    {
    	args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        cerr << e.reason << endl;
	usage();
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage();
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }
    _totals = opts.isSet("t");


    if(args.empty())
    {
        cerr << argv[0] << ": no data directory specified" << endl;
        usage();
        return EXIT_FAILURE;
    }
    dataDir = args[0];

    for(vector<string>::size_type i = 1; i < args.size(); ++i)
    {
	oldDataDirs.push_back(args[i]);
    }

    if(dataDir == ".")
    {
#ifdef _WIN32
        char cwd[_MAX_PATH];
        _getcwd(cwd, _MAX_PATH);
#else
        char cwd[PATH_MAX];
        getcwd(cwd, PATH_MAX);
#endif
        dataDir = cwd;
    }

    //
    // Update the data directory (create .bz2 and .md5 files).
    //
    cout << "Updating data directory...";
    if(_totals)
    {
        cout << endl;
    }
    else
    {
        cout << flush;
    }

    try
    {
        update(dataDir);
    }
    catch(const FileAccessException& ex)
    {
        cerr << endl << "exception during update:\n" << ex << ":\n" << ex.reason << endl;
        return EXIT_FAILURE;
    }
    catch(const Exception& ex)
    {
        cerr << endl << "exception during update:\n" << ex << endl;
        return EXIT_FAILURE;
    }

    if(!_totals)
    {
        cout << " done." << endl;
    }

    if(!oldDataDirs.empty())
    {
        for(vector<string>::iterator p = oldDataDirs.begin(); p != oldDataDirs.end(); ++p)
        {
            cout << "Comparing with " << *p << "...";
            if(_totals)
            {
                cout << endl;
            }
            else
            {
                cout << flush;
            }

            try
            {
                compare(*p, dataDir);
            }
            catch(const FileAccessException& ex)
            {
                cerr << endl << "exception during comparison:\n" << ex << ":\n" << ex.reason << endl;
                return EXIT_FAILURE;
            }
            catch(const Exception& ex)
            {
                cerr << endl << "exception during comparison:\n" << ex << endl;
                return EXIT_FAILURE;
            }

            if(!_totals)
            {
                cout << " done." << endl;
            }
        }
    }

    return EXIT_SUCCESS;
}

void
CalcApp::update(const string& dir)
{
    StringSeq contents = readDirectory(dir);
    StringSeq filteredContents;
    StringSeq::const_iterator p;

    for(p = contents.begin(); p != contents.end(); ++p)
    {
        FileInfo info = getFileInfo(*p, true);
        if(info.type == FileTypeDirectory)
        {
            //
            // Depth-first traversal.
            //
            update(*p);
            filteredContents.push_back(*p);
        }
        else
        {
            if(ignoreSuffix(*p))
            {
                //
                // Check for orphans (i.e., bz2 or md5 files for which the original file no longer exists).
                //
                string orphan = removeSuffix(*p);
                FileInfo infoOrphan = getFileInfo(orphan, false);
                if(infoOrphan.type == FileTypeNotExist)
                {
                    removeRecursive(*p, 0);
                }
            }
            else
            {
                //
                // Create .bz2 file if necessary.
                //
                FileInfo infoBZ2 = getFileInfo(*p + ".bz2", false);
                if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
                {
                    createBZ2(*p, 0);
                }

                //
                // Create .md5 file if necessary.
                //
                FileInfo infoMD5 = getFileInfo(*p + ".md5", false);
                if(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
                {
                    createMD5(*p, 0);
                }

                filteredContents.push_back(*p);
            }
        }
    }

    //
    // Create .md5 for directory if necessary.
    //
    FileInfo info = getFileInfo(dir, true);
    FileInfo infoMD5 = getFileInfo(dir + ".md5", false);
    if(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
    {
        createMD5(dir, 0);
    }

    //
    // Compute totals if necessary.
    //
    FileInfo infoTot = getFileInfo(dir + ".tot", false);
    if(infoTot.type != FileTypeRegular || infoTot.time <= info.time)
    {
        Long total = 0;
        ByteSeq md5; // Empty
        for(p = filteredContents.begin(); p != filteredContents.end(); ++p)
        {
            FileInfo info = getFileInfo(*p, true);
            if(info.type == FileTypeDirectory)
            {
                TotalMap m = getTotalMap(communicator(), *p);
                TotalMap::iterator q = m.find(md5);
                assert(q != m.end());
                total += q->second;
            }
            else
            {
                FileInfo infoBZ2 = getFileInfo(*p + ".bz2", true);
                assert(infoBZ2.type == FileTypeRegular);
                total += infoBZ2.size;
            }
        }

        TotalMap totals = getTotalMap(communicator(), dir);
        TotalMap::iterator q = totals.find(md5);
        if(q != totals.end())
        {
            q->second = total;
        }
        else
        {
            totals.insert(TotalMap::value_type(md5, total));
        }
        if(_totals)
        {
            cout << " " << dir << " (" << total << " bytes)" << endl;
        }
        putTotalMap(communicator(), dir, totals);
    }
    else if(_totals)
    {
        TotalMap totals = getTotalMap(communicator(), dir);
        ByteSeq md5; // Empty
        TotalMap::iterator q = totals.find(md5);
        assert(q != totals.end());
        cout << " " << dir << " (" << q->second << " bytes)" << endl;
    }
}

#ifdef _WIN32

typedef set<string, CICompare> EntrySet;

#else

typedef set<string> EntrySet;

#endif

Long
CalcApp::compare(const string& oldDir, const string& newDir)
{
    Long total = 0;

    ByteSeq oldMD5 = getMD5(oldDir);
    ByteSeq newMD5 = getMD5(newDir);
    if(oldMD5 != newMD5)
    {
        StringSeq oldContents = readDirectory(oldDir, ".");
        StringSeq newContents = readDirectory(newDir, ".");
        StringSeq::const_iterator p;

        EntrySet newEntries;
        copy(newContents.begin(), newContents.end(), inserter(newEntries, newEntries.begin()));

        for(p = oldContents.begin(); p != oldContents.end(); ++p)
        {
            if(ignoreSuffix(*p))
            {
                continue;
            }

            string oldEntry = oldDir + "/" + *p;
            string newEntry = newDir + "/" + *p;
            FileInfo oldInfo = getFileInfo(oldEntry, true);
            FileInfo newInfo = getFileInfo(newEntry, false);

            if(newInfo.type == FileTypeNotExist)
            {
                continue;
            }

            if(oldInfo.type == FileTypeDirectory)
            {
                if(newInfo.type == FileTypeDirectory)
                {
                    total += compare(oldEntry, newEntry);
                }
                else
                {
                    FileInfo infoBZ2 = getFileInfo(newEntry + ".bz2", true);
                    total += infoBZ2.size;
                }
            }
            else
            {
                if(newInfo.type == FileTypeDirectory)
                {
                    TotalMap totals = getTotalMap(communicator(), newEntry);
                    ByteSeq empty;
                    TotalMap::const_iterator q = totals.find(empty);
                    if(q != totals.end())
                    {
                        total += q->second;
                    }
                }
                else
                {
                    ByteSeq oldEntryMD5 = getMD5(oldEntry);
                    ByteSeq newEntryMD5 = getMD5(newEntry);
                    if(oldEntryMD5 != newEntryMD5)
                    {
                        FileInfo newEntryBZ2 = getFileInfo(newEntry + ".bz2", true);
                        total += newEntryBZ2.size;
                    }
                }
            }

            newEntries.erase(*p);
        }

        for(EntrySet::iterator q = newEntries.begin(); q != newEntries.end(); ++q)
        {
            if(ignoreSuffix(*q))
            {
                continue;
            }

            string entry = newDir + "/" + *q;
            FileInfo info = getFileInfo(entry, true);
            if(info.type == FileTypeDirectory)
            {
                TotalMap totals = getTotalMap(communicator(), entry);
                ByteSeq empty;
                TotalMap::const_iterator r = totals.find(empty);
                if(r != totals.end())
                {
                    total += r->second;
                }
            }
            else
            {
                assert(info.type == FileTypeRegular);
                FileInfo infoBZ2 = getFileInfo(entry + ".bz2", true);
                total += infoBZ2.size;
            }
        }
    }

    //
    // An MD5 full of zeros indicates an old directory that is empty.
    // We ignore this case because the difference is simply the size of
    // the current directory (i.e., an empty MD5).
    //
    ByteSeq zeroMD5(16, 0);
    if(oldMD5 != zeroMD5)
    {
        TotalMap totals = getTotalMap(communicator(), newDir);
        TotalMap::iterator q = totals.find(oldMD5);
        if(q != totals.end())
        {
            q->second = total;
        }
        else
        {
            totals.insert(TotalMap::value_type(oldMD5, total));
        }
        putTotalMap(communicator(), newDir, totals);
    }

    if(_totals)
    {
        cout << " " << newDir << " (" << total << " bytes)" << endl;
    }

    return total;
}

int
main(int argc, char* argv[])
{
    CalcApp app;
    return app.main(argc, argv);
}
