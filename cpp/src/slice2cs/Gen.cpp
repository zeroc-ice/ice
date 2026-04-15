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
    _fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        _fileBase = base.substr(pos + 1);
    }
    string file = _genMode == GenMode::IceRpc ? _fileBase + ".IceRpc.cs" : _fileBase + ".cs";

    if (!dir.empty())
    {
        file = dir + '/' + file;
    }

    _out.open(file.c_str());
    if (!_out)
    {
        ostringstream os;
        os << "cannot open '" << file << "': " << IceInternal::errorToString(errno);
        throw FileException(os.str());
    }
    FileTracker::instance()->addFile(file);

    printHeader(_fileBase + ".ice");
    if (_genMode == GenMode::IceRpc)
    {
        _out << sp;
        _out << nl << "using IceRpc.Ice;";
        _out << nl << "using IceRpc.Ice.Codec;";
    }
    else
    {
        _out << sp;
        _out << nl << "[assembly:Ice.Slice(\"" << _fileBase << ".ice\")]";
    }
}

Slice::Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << nl;
        _out.close();
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
        if (p->contains<InterfaceDef>())
        {
            // The proxy and skeleton code depends on this using directive.
            _out << nl << "using IceRpc.Ice.Operations;";
        }
        _out << sp;
        _out << nl << "[assembly:IceGeneratedCode(\"" << _fileBase << ".ice\")]";

        Slice::IceRpc::TypesVisitor typesVisitor(_out);
        p->visit(&typesVisitor);

        Slice::IceRpc::SkeletonVisitor skeletonVisitor(_out);
        p->visit(&skeletonVisitor);
    }
}

void
Slice::Gen::printHeader(const string& iceFile)
{
    _out << "// Copyright (c) ZeroC, Inc.";
    _out << sp;
    _out << nl << "// slice2cs version " << ICE_STRING_VERSION;

    if (!_enableAnalysis)
    {
        printGeneratedHeader(_out, iceFile);
    }

    _out << sp;
    _out << nl << "#nullable enable";
    _out << sp;

    if (_enableAnalysis)
    {
        // Disable some warnings when auto-generated is removed from the header. See printGeneratedHeader above.
        _out << nl << "#pragma warning disable SA1403 // File may only contain a single namespace";
        _out << nl << "#pragma warning disable SA1611 // The documentation for parameter x is missing";

        _out << nl << "#pragma warning disable CA1041 // Provide a message for the ObsoleteAttribute that marks ...";

        _out << nl << "#pragma warning disable CA1068 // Cancellation token as last parameter";
        _out << nl << "#pragma warning disable CA1725 // Change parameter name istr_ to istr in order to match ...";

        // Missing doc - only necessary for the tests.
        _out << nl << "#pragma warning disable SA1602";
        _out << nl << "#pragma warning disable SA1604";
        _out << nl << "#pragma warning disable SA1605";
    }

    _out << nl << "#pragma warning disable CS1591 // Missing XML Comment";
    _out << nl << "#pragma warning disable CS1573 // Parameter has no matching param tag in the XML comment";
    _out << nl << "#pragma warning disable CS0612 // Type or member is obsolete";
    _out << nl << "#pragma warning disable CS0618 // Type or member is obsolete";
    _out << nl << "#pragma warning disable CS0619 // Type or member is obsolete";
}
