// Copyright (c) ZeroC, Inc.

#include "Gen.h"
#include "../Ice/FileUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/Util.h"
#include "CsMetadataValidator.h"
#include "CsUtil.h"
#include "Ice/StringUtil.h"
#include "IceVisitors.h"

using namespace std;
using namespace Slice;
using namespace Slice::Csharp;
using namespace IceInternal;

Slice::Gen::Gen(const string& base, const string& dir, bool enableAnalysis) : _enableAnalysis(enableAnalysis)
{
    string fileBase = base;
    string::size_type pos = base.find_last_of("/\\");
    if (pos != string::npos)
    {
        fileBase = base.substr(pos + 1);
    }
    string file = fileBase + ".cs";
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
    printHeader();

    if (!_enableAnalysis)
    {
        printGeneratedHeader(_out, fileBase + ".ice");
    }

    _out << sp;
    _out << nl << "#nullable enable";
    _out << sp;
    _out << nl << "[assembly:Ice.Slice(\"" << fileBase << ".ice\")]";

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

Slice::Gen::~Gen()
{
    if (_out.isOpen())
    {
        _out << '\n';
        _out.close();
    }
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    Slice::validateCsMetadata(p);

    Ice::TypesVisitor typesVisitor(_out);
    p->visit(&typesVisitor);

    Ice::ResultVisitor resultVisitor(_out);
    p->visit(&resultVisitor);

    // Default skeleton.
    Ice::SkeletonVisitor skeletonVisitor(_out, false);
    p->visit(&skeletonVisitor);

    // Async skeleton.
    Ice::SkeletonVisitor asyncSkeletonVisitor(_out, true);
    p->visit(&asyncSkeletonVisitor);
}

void
Slice::Gen::printHeader()
{
    _out << "// Copyright (c) ZeroC, Inc.";
    _out << sp;
    _out << nl << "// slice2cs version " << ICE_STRING_VERSION;
}
