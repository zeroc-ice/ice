//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Options.h>
#include <IceUtil/StringUtil.h>
#include <Ice/Ice.h>
#include <Ice/ConsoleUtil.h>
#include <IcePatch2Lib/Util.h>
#include <IcePatch2/ClientUtil.h>

#include <iostream>

#ifdef _WIN32
#    include <conio.h>
#else
#    include <fcntl.h>
#    include <termios.h>
#    include <unistd.h>
#endif

using namespace std;
using namespace IceInternal;
using namespace IcePatch2;

class TextPatcherFeedback final : public IcePatch2::PatcherFeedback
{
public:
    TextPatcherFeedback() : _pressAnyKeyMessage(false)
    {
#ifndef _WIN32
        tcgetattr(0, &_savedTerm);
        _savedFlags = fcntl(0, F_GETFL);
        _block = true;
#endif
    }

    ~TextPatcherFeedback()
    {
#ifndef _WIN32
        tcsetattr(0, TCSANOW, &_savedTerm);
        fcntl(0, F_SETFL, _savedFlags);
#endif
    }

    bool noFileSummary(const string& reason) final
    {
        consoleOut << "Cannot load file summary:\n" << reason << endl;
        string answer;
        do
        {
            consoleOut << "Do a thorough patch? (yes/no)" << endl;
            cin >> answer;
            answer = IceUtilInternal::toLower(answer);
            if (answer == "no")
            {
                return false;
            }
        } while (answer != "yes");
        return true;
    }

    bool checksumStart() final
    {
        if (!_pressAnyKeyMessage)
        {
            consoleOut << "[Press any key to abort]" << endl;
            _pressAnyKeyMessage = true;
        }

        return !keyPressed();
    }

    bool checksumProgress(const string& path) final
    {
        consoleOut << "Calculating checksum for " << IcePatch2Internal::getBasename(path) << endl;
        return !keyPressed();
    }

    bool checksumEnd() final { return !keyPressed(); }

    bool fileListStart() final
    {
        if (!_pressAnyKeyMessage)
        {
            consoleOut << "[Press any key to abort]" << endl;
            _pressAnyKeyMessage = true;
        }

        _lastProgress = "0%";
        consoleOut << "Getting list of files to patch: " << _lastProgress << flush;
        return !keyPressed();
    }

    bool fileListProgress(int32_t percent) final
    {
        for (unsigned int i = 0; i < _lastProgress.size(); ++i)
        {
            consoleOut << '\b';
        }
        ostringstream s;
        s << percent << '%';
        _lastProgress = s.str();
        consoleOut << _lastProgress << flush;
        return !keyPressed();
    }

    bool fileListEnd() final
    {
        consoleOut << endl;
        return !keyPressed();
    }

    bool patchStart(const string& path, int64_t size, int64_t totalProgress, int64_t totalSize) final
    {
        if (!_pressAnyKeyMessage)
        {
            consoleOut << "[Press any key to abort]" << endl;
            _pressAnyKeyMessage = true;
        }

        ostringstream s;
        s << "0/" << size << " (" << totalProgress << '/' << totalSize << ')';
        _lastProgress = s.str();
        consoleOut << IcePatch2Internal::getBasename(path) << ' ' << _lastProgress << flush;
        return !keyPressed();
    }

    bool patchProgress(int64_t progress, int64_t size, int64_t totalProgress, int64_t totalSize) final
    {
        for (unsigned int i = 0; i < _lastProgress.size(); ++i)
        {
            consoleOut << '\b';
        }
        ostringstream s;
        s << progress << '/' << size << " (" << totalProgress << '/' << totalSize << ')';
        _lastProgress = s.str();
        consoleOut << _lastProgress << flush;
        return !keyPressed();
    }

    bool patchEnd() final
    {
        consoleOut << endl;
        return !keyPressed();
    }

private:
#ifdef _WIN32

    bool keyPressed()
    {
        bool pressed = false;
        while (_kbhit())
        {
            pressed = true;
            _getch();
        }
        if (pressed)
        {
            pressed = confirmAbort();
        }
        return pressed;
    }

#else

    bool keyPressed()
    {
        if (_block)
        {
            termios term;
            memcpy(&term, &_savedTerm, sizeof(termios));
            term.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON));
            term.c_cc[VTIME] = 0;
            term.c_cc[VMIN] = 1;
            tcsetattr(0, TCSANOW, &term);

            int flags = _savedFlags;
            flags |= O_NONBLOCK;
            fcntl(0, F_SETFL, flags);

            _block = false;
        }

        bool pressed = false;
        char c;
        while (read(0, &c, 1) > 0)
        {
            pressed = true;
        }
        if (pressed)
        {
            pressed = confirmAbort();
        }
        return pressed;
    }

    termios _savedTerm;
    int _savedFlags;
    bool _block;

#endif

    bool confirmAbort()
    {
#ifndef _WIN32
        if (!_block)
        {
            tcsetattr(0, TCSANOW, &_savedTerm);
            fcntl(0, F_SETFL, _savedFlags);
            _block = true;
        }
#endif
        string answer;
        do
        {
            consoleOut << "\nConfirm abort? (Y/N)" << endl;
            cin >> answer;
            answer = IceUtilInternal::toLower(answer);
            if (answer == "n")
            {
                return false;
            }
        } while (answer != "y");
        return true;
    }

    string _lastProgress;
    bool _pressAnyKeyMessage;
};

int run(const Ice::StringSeq&);

Ice::CommunicatorPtr communicator;

void
destroyCommunicator(int)
{
    communicator->destroy();
}

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
#else
main(int argc, char* argv[])
#endif
{
    int status = 0;

    try
    {
        Ice::CtrlCHandler ctrlCHandler;
        Ice::CommunicatorHolder ich(argc, argv);
        communicator = ich.communicator();

        ctrlCHandler.setCallback(&destroyCommunicator);

        status = run(Ice::argsToStringSeq(argc, argv));
    }
    catch (const std::exception& ex)
    {
        consoleErr << ex.what() << endl;
        status = 1;
    }

    return status;
}

void
usage(const string& appName)
{
    string options = "Options:\n"
                     "-h, --help           Show this message.\n"
                     "-v, --version        Display the Ice version.\n"
                     "-t, --thorough       Recalculate all checksums.";

    consoleErr << "Usage: " << appName << " [options] [DIR]" << endl;
    consoleErr << options << endl;
}

int
run(const Ice::StringSeq& args)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");
    opts.addOpt("t", "thorough");

    vector<string> props;
    try
    {
        props = opts.parse(args);
    }
    catch (const IceUtilInternal::BadOptException& e)
    {
        consoleErr << e.reason << endl;
        usage(args[0]);
        return 1;
    }

    if (opts.isSet("help"))
    {
        usage(args[0]);
        return 0;
    }
    if (opts.isSet("version"))
    {
        consoleOut << ICE_STRING_VERSION << endl;
        return 0;
    }
    if (opts.isSet("thorough"))
    {
        properties->setProperty("IcePatch2Client.Thorough", "1");
    }

    if (props.size() > 1)
    {
        consoleErr << args[0] << ": too many arguments" << endl;
        usage(args[0]);
        return 1;
    }
    if (props.size() == 1)
    {
        properties->setProperty("IcePatch2Client.Directory", IcePatch2Internal::simplify(props[0]));
    }

    bool aborted = false;

    try
    {
        IcePatch2::PatcherFeedbackPtr feedback = make_shared<TextPatcherFeedback>();

        const string dataDir = properties->getPropertyWithDefault("IcePatch2Client.Directory", ".");
        const bool thorough = properties->getPropertyAsIntWithDefault("IcePatch2Client.Thorough", 0) > 0;
        const int32_t chunkSize = properties->getPropertyAsIntWithDefault("IcePatch2Client.ChunkSize", 100);
        const int32_t remove = properties->getPropertyAsIntWithDefault("IcePatch2Client.Remove", 1);

        const string clientProxyPropertyName = "IcePatch2Client.Proxy";
        string clientProxyPropertyValue = properties->getProperty(clientProxyPropertyName);
        if (clientProxyPropertyValue.empty())
        {
            throw runtime_error("property `" + clientProxyPropertyName + "' is not set");
        }

        FileServerPrx server(communicator, clientProxyPropertyValue);

        PatcherPtr patcher = PatcherFactory::create(server, feedback, dataDir, thorough, chunkSize, remove);

        aborted = !patcher->prepare();

        if (!aborted)
        {
            aborted = !patcher->patch("");
        }

        if (!aborted)
        {
            patcher->finish();
        }
    }
    catch (const exception& ex)
    {
        consoleErr << args[0] << ": " << ex.what() << endl;
        return 1;
    }

    if (aborted)
    {
        consoleOut << "\n[Aborted]" << endl;
        return 1;
    }
    else
    {
        return 0;
    }
}
