// **********************************************************************
//
// Copyright (c) 2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePatch2/Util.h>
#include <IcePatch2/ClientUtil.h>

#ifdef _WIN32
#   include <conio.h>
#else
#   include <fcntl.h>
#   include <termios.h>
#endif

using namespace std;
using namespace Ice;
using namespace IcePatch2;

namespace IcePatch2
{

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);

private:

    void usage(const std::string&);
};

};

class TextPatcherFeedback : public PatcherFeedback
{
public:

    TextPatcherFeedback()
    {
#ifndef _WIN32
	tcgetattr(0, &_savedTerm);
	termios term;
	memcpy(&term, &_savedTerm, sizeof(termios));
	term.c_lflag &= ~(ECHO | ICANON);
	term.c_cc[VTIME] = 0;
	term.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &term);

	_savedFlags = fcntl(0, F_GETFL);
	int flags = _savedFlags;
	flags |= O_NONBLOCK;
	fcntl(0, F_SETFL, flags);
#endif
    }

    virtual ~TextPatcherFeedback()
    {
#ifndef _WIN32
	tcsetattr(0, TCSANOW, &_savedTerm);
	fcntl(0, F_SETFL, _savedFlags);
#endif
    }

    virtual bool
    noFileSummary(const string& reason)
    {
	cout << "Cannot load file summary:\n" << reason << endl;
	string answer;
	do
	{
	    cout << "Do a thorough patch? (yes/no)" << endl;
	    cin >> answer;
	    transform(answer.begin(), answer.end(), answer.begin(), ::tolower);
	    if(answer == "no")
	    {
		return false;
	    }
	}
	while(answer != "yes");
	cout << "Calculating checksums -- please wait, this might take awhile..." << endl;
	return true;
    }

    virtual bool
    fileListStart()
    {
	cout << "[Press any key to interrupt]" << endl;
	_lastProgress = "0%";
	cout << "Getting list of files to patch: " << _lastProgress << flush;
	return !keyPressed();
    }

    virtual bool
    fileListProgress(Int percent)
    {
	for(unsigned int i = 0; i < _lastProgress.size(); ++i)
	{
	    cout << '\b';
	}
	ostringstream s;
	s << percent << '%';
	_lastProgress = s.str();
	cout << _lastProgress << flush;
	return !keyPressed();
    }

    virtual bool
    fileListEnd()
    {
	cout << endl;
	return !keyPressed();
    }

    virtual bool
    patchStart(const string& path, Long size, Long totalProgress, Long totalSize)
    {
	ostringstream s;
	s << "0/" << size << " (" << totalProgress << '/' << totalSize << ')';
	_lastProgress = s.str();
	cout << getBasename(path) << ' ' << _lastProgress << flush;
	return !keyPressed();
    }

    virtual bool
    patchProgress(Long progress, Long size, Long totalProgress, Long totalSize)
    {
	for(unsigned int i = 0; i < _lastProgress.size(); ++i)
	{
	    cout << '\b';
	}
	ostringstream s;
	s << progress << '/' << size << " (" << totalProgress << '/' << totalSize << ')';
	_lastProgress = s.str();
	cout << _lastProgress << flush;
	return !keyPressed();
    }

    virtual bool
    patchEnd()
    {
	cout << endl;
	return !keyPressed();
    }

private:

    bool
    keyPressed()
    {
	bool pressed = false;
#ifdef _WIN32
	while(_kbhit())
	{
	    pressed = true;
	    _getch();
	}
#else
	char c;
	while(read(0, &c, 1) > 0)
	{
	    pressed = true;
	}
#endif
	return pressed;
    }

    string _lastProgress;

#ifndef _WIN32
    termios _savedTerm;
    int _savedFlags;
#endif
};

int
IcePatch2::Client::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();

    for(int i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	{
	    usage(argv[0]);
	    return EXIT_FAILURE;
	}
	else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	{
	    cout << ICE_STRING_VERSION << endl;
	    return EXIT_FAILURE;
	}
	else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--thorough") == 0)
	{
	    properties->setProperty("IcePatch2.Thorough", "1");
	}
	else if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dry") == 0)
	{
	    properties->setProperty("IcePatch2.DryRun", "1");
	}
        else if(argv[i][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            if(properties->getProperty("IcePatch2.Directory").empty())
            {
                properties->setProperty("IcePatch2.Directory", argv[i]);
            }
            else
            {
		cerr << argv[0] << ": too many arguments" << endl;
		usage(argv[0]);
		return EXIT_FAILURE;
            }
        }
    }

    bool patchComplete = false;

    try
    {
	PatcherFeedbackPtr feedback = new TextPatcherFeedback;
	PatcherPtr patcher = new Patcher(communicator(), feedback);
	patcher->patch();
    }
    catch(const string& ex)
    {
        cerr << argv[0] << ": " << ex << endl;
        return EXIT_FAILURE;
    }

    if(patchComplete)
    {
	return EXIT_SUCCESS;
    }
    else
    {
	cout << "\n[Interrupted]" << endl;
	return EXIT_FAILURE;
    }
}

void
IcePatch2::Client::usage(const string& appName)
{
    string options =
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	"-t, --thorough       Recalculate all checksums.\n"
	"-d, --dry            Don't update, do a dry run only.";

    cerr << "Usage: " << appName << " [options] [DIR]" << endl;
    cerr << options << endl;
}

int
main(int argc, char* argv[])
{
    Client app;
    return app.main(argc, argv);
}
