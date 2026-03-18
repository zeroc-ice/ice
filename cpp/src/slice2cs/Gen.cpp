// Copyright (c) ZeroC, Inc.

#include "Gen.h"
#include "../Ice/FileUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "CsMetadataValidator.h"
#include "CsUtil.h"
#include "Ice/StringUtil.h"
#include "IceRpcVisitors.h"
#include "IceVisitors.h"

using namespace std;
using namespace Slice;
using namespace Slice::Csharp;
using namespace IceInternal;

Slice::Gen::Gen(const string& base, const string& dir, GenMode genMode, bool enableAnalysis)
    : _genMode(genMode),
      _enableAnalysis(enableAnalysis)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".cs";
    string iceRpcFile = fileBase + ".IceRpc.cs";

    if (!dir.empty())
    {
        file = dir + '/' + file;
        iceRpcFile = dir + '/' + iceRpcFile;
    }

    _out.open(file.c_str());
    if (!_out)
    {
        ostringstream os;
        os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
        throw FileException(os.str());
    }
    FileTracker::instance()->addFile(file);

    if (_genMode == GenMode::IceRpc)
    {
        _iceRpcOut.open(iceRpcFile.c_str());
        if (!_iceRpcOut)
        {
            ostringstream os;
            os << "cannot open '" << iceRpcFile << "': " << IceInternal::errorToString(errno);
            throw FileException(os.str());
        }
        FileTracker::instance()->addFile(iceRpcFile);
    }

    printHeader(_out, fileBase + ".ice", _enableAnalysis);
    if (_genMode == GenMode::IceRpc)
    {
        printHeader(_iceRpcOut, fileBase + ".ice", _enableAnalysis);

        _out << sp;
        _out << nl << "using IceRpc.Ice;";
        _out << nl << "using IceRpc.Ice.Codec;";
        _out << sp;
        _out << nl << "[assembly:Ice(\"" << fileBase << ".ice\")]";

        _iceRpcOut << sp;
        _iceRpcOut << nl << "using IceRpc.Ice;";
        _iceRpcOut << nl << "using IceRpc.Ice.Codec;";
        _iceRpcOut << nl << "using IceRpc.Ice.Operations;";
    }
    else
    {
        _out << sp;
        _out << nl << "[assembly:Ice.Slice(\"" << fileBase << ".ice\")]";
    }
}

Slice::Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << nl;
        _out.close();
    }
    if (_iceRpcOut.isOpen())
    {
        _iceRpcOut << nl;
        _iceRpcOut.close();
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    Slice::validateCsMetadata(p);

    if (_genMode == GenMode::Ice)
    {
        Slice::Ice::TypesVisitor typesVisitor(_out);
        p->visit(&typesVisitor);

        Slice::Ice::ResultVisitor resultVisitor(_out);
        p->visit(&resultVisitor);

        // Default skeleton.
        Slice::Ice::SkeletonVisitor skeletonVisitor(_out, false);
        p->visit(&skeletonVisitor);

        // Async skeleton.
        Slice::Ice::SkeletonVisitor asyncSkeletonVisitor(_out, true);
        p->visit(&asyncSkeletonVisitor);
    }
    else
    {
        Slice::IceRpc::TypesVisitor typesVisitor(_out);
        p->visit(&typesVisitor);

        Slice::IceRpc::ProxyVisitor proxyVisitor(_iceRpcOut);
        p->visit(&proxyVisitor);

        Slice::IceRpc::SkeletonVisitor skeletonVisitor(_iceRpcOut);
        p->visit(&skeletonVisitor);
    }
}

void
Slice::Gen::printHeader(IceInternal::Output& out, const string& iceFile, bool enableAnalysis)
{
    out << "// Copyright (c) ZeroC, Inc.";
    out << sp;
    out << nl << "// slice2cs version " << ICE_STRING_VERSION;

    if (!enableAnalysis)
    {
        printGeneratedHeader(out, iceFile);
    }

    out << sp;
    out << nl << "#nullable enable";
    out << sp;

    if (enableAnalysis)
    {
        // Disable some warnings when auto-generated is removed from the header. See printGeneratedHeader above.
        out << nl << "#pragma warning disable SA1403 // File may only contain a single namespace";
        out << nl << "#pragma warning disable SA1611 // The documentation for parameter x is missing";

        out << nl << "#pragma warning disable CA1041 // Provide a message for the ObsoleteAttribute that marks ...";

        out << nl << "#pragma warning disable CA1068 // Cancellation token as last parameter";
        out << nl << "#pragma warning disable CA1725 // Change parameter name istr_ to istr in order to match ...";

        // Missing doc - only necessary for the tests.
        out << nl << "#pragma warning disable SA1602";
        out << nl << "#pragma warning disable SA1604";
        out << nl << "#pragma warning disable SA1605";
    }

    out << nl << "#pragma warning disable CS1591 // Missing XML Comment";
    out << nl << "#pragma warning disable CS1573 // Parameter has no matching param tag in the XML comment";
    out << nl << "#pragma warning disable CS0612 // Type or member is obsolete";
    out << nl << "#pragma warning disable CS0618 // Type or member is obsolete";
    out << nl << "#pragma warning disable CS0619 // Type or member is obsolete";
}
