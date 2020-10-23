//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include "JavaUtil.h"
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <IceUtil/FileUtil.h>

#include <sys/types.h>
#include <string.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

const std::array<std::string, 17> builtinSuffixTable =
{
    "Bool",
    "Byte",
    "Short",
    "UShort",
    "Int",
    "UInt",
    "VarInt",
    "VarUInt",
    "Long",
    "ULong",
    "VarLong",
    "VarULong",
    "Float",
    "Double",
    "String",
    "Proxy",
    "Value"
};

void
hashAdd(long& hashCode, const std::string& value)
{
    for(const auto& p : value)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ p;
    }
}

string
lookupKwd(const string& name)
{
    //
    // Keyword list. *Must* be kept in alphabetical order. Note that checkedCast and uncheckedCast
    // are not Java keywords, but are in this list to prevent illegal code being generated if
    // someone defines Slice operations with that name.
    //
    // NOTE: Any changes made to this list must also be made in BasicStream.java.
    //
    static const string keywordList[] =
    {
        "abstract", "assert", "boolean", "break", "byte", "case", "catch",
        "char", "checkedCast", "class", "clone", "const", "continue", "default", "do",
        "double", "else", "enum", "equals", "extends", "false", "final", "finalize",
        "finally", "float", "for", "getClass", "goto", "hashCode", "if",
        "implements", "import", "instanceof", "int", "interface", "long",
        "native", "new", "notify", "notifyAll", "null", "package", "private",
        "protected", "public", "return", "short", "static", "strictfp", "super", "switch",
        "synchronized", "this", "throw", "throws", "toString", "transient",
        "true", "try", "uncheckedCast", "void", "volatile", "wait", "while"
    };
    bool found =  binary_search(&keywordList[0],
                                &keywordList[sizeof(keywordList) / sizeof(*keywordList)],
                                name);
    return found ? "_" + name : name;
}

class MetadataVisitor : public ParserVisitor
{
public:

    bool visitUnitStart(const UnitPtr& p) override
    {
        static const string prefix = "java:";

        //
        // Validate global metadata in the top-level file and all included files.
        //
        StringList files = p->allFiles();

        for(StringList::iterator q = files.begin(); q != files.end(); ++q)
        {
            string file = *q;
            DefinitionContextPtr dc = p->findDefinitionContext(file);
            assert(dc);
            StringList globalMetadata = dc->getAllMetadata();
            for(StringList::const_iterator r = globalMetadata.begin(); r != globalMetadata.end();)
            {
                string s = *r++;
                if(s.find(prefix) == 0)
                {
                    static const string packagePrefix = "java:package:";
                    if(s.find(packagePrefix) == 0 && s.size() > packagePrefix.size())
                    {
                        continue;
                    }
                    else
                    {
                        dc->warning(InvalidMetadata, file, -1,  "ignoring invalid global metadata `" + s + "'");
                        globalMetadata.remove(s);
                        continue;
                    }
                };
            }
            dc->setMetadata(globalMetadata);
        }
        return true;
    }

    bool visitModuleStart(const ModulePtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
        return true;
    }

    void visitClassDecl(const ClassDeclPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
    }

    bool visitClassDefStart(const ClassDefPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
        return true;
    }

    void visitInterfaceDecl(const InterfaceDeclPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
    }

    bool visitInterfaceDefStart(const InterfaceDefPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
        return true;
    }

    bool visitExceptionStart(const ExceptionPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
        return true;
    }

    bool visitStructStart(const StructPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
        return true;
    }

    void visitOperation(const OperationPtr& p) override
    {
        TypePtr returnType = p->deprecatedReturnType();
        StringList metadata = getMetadata(p);

        UnitPtr unt = p->unit();
        string file = p->file();
        DefinitionContextPtr dc = unt->findDefinitionContext(p->file());

        if(!returnType)
        {
            for(StringList::const_iterator q = metadata.begin(); q != metadata.end();)
            {
                string s = *q++;
                if(s.find("java:type:", 0) == 0)
                {
                    dc->warning(InvalidMetadata, p->file(), p->line(), "ignoring invalid metadata `" + s +
                                "' for operation with void return type");
                    metadata.remove(s);
                    continue;
                }
            }
        }
        else
        {
            metadata = validateType(returnType, metadata, p->file(), p->line());
            metadata = validateGetSet(p, metadata, p->file(), p->line());
        }
        p->setMetadata(metadata);

        for(auto& q : p->allMembers())
        {
            metadata = getMetadata(q);
            metadata = validateType(q->type(), metadata, p->file(), q->line());
            metadata = validateGetSet(q->type(), metadata, p->file(), q->line());
            q->setMetadata(metadata);
        }
    }

    void visitDataMember(const MemberPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p->type(), metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
    }

    void visitSequence(const SequencePtr& p) override
    {
        static const string protobuf = "java:protobuf:";
        static const string serializable = "java:serializable:";
        static const string bytebuffer = "java:buffer";
        StringList metadata = getMetadata(p);
        StringList newMetadata;

        const string file =  p->file();
        const int line = p->line();
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc = unt->findDefinitionContext(file);

        for(StringList::const_iterator q = metadata.begin(); q != metadata.end(); )
        {
            string s = *q++;

            if(s.find(protobuf) == 0 || s.find(serializable) == 0)
            {
                //
                // Remove from list so validateType does not try to handle as well.
                //
                metadata.remove(s);
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
                if(!builtin || builtin->kind() != Builtin::KindByte)
                {
                    dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + s + "': " +
                                "this metadata can only be used with a byte sequence");
                    continue;
                }
                newMetadata.push_back(s);
            }
            else if(s.find(bytebuffer) == 0)
            {
                metadata.remove(s);

                BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
                if(!builtin ||
                   (builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindShort &&
                    builtin->kind() != Builtin::KindInt && builtin->kind() != Builtin::KindLong &&
                    builtin->kind() != Builtin::KindFloat && builtin->kind() != Builtin::KindDouble))
                {
                    dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + s + "': " +
                                "this metadata can not be used with this type");
                    continue;
                }
                newMetadata.push_back(s);
            }
        }

        metadata = validateType(p, metadata, file, line);
        metadata = validateGetSet(p, metadata, file, line);
        newMetadata.insert(newMetadata.begin(), metadata.begin(), metadata.end());
        p->setMetadata(newMetadata);
    }

    void visitDictionary(const DictionaryPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
    }

    void visitEnum(const EnumPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
    }

    void visitConst(const ConstPtr& p) override
    {
        StringList metadata = getMetadata(p);
        metadata = validateType(p, metadata, p->file(), p->line());
        metadata = validateGetSet(p, metadata, p->file(), p->line());
        p->setMetadata(metadata);
    }

private:

    StringList getMetadata(const ContainedPtr& cont)
    {
        static const string prefix = "java:";

        StringList metadata = cont->getAllMetadata();
        StringList result;

        UnitPtr unt = cont->container()->unit();
        string file = cont->file();
        DefinitionContextPtr dc = unt->findDefinitionContext(file);
        assert(dc);

        for(StringList::const_iterator p = metadata.begin(); p != metadata.end(); ++p)
        {
            string s = *p;
            if(s.find(prefix) == 0)
            {
                string::size_type pos = s.find(':', prefix.size());
                if(pos == string::npos)
                {
                    if(s.size() > prefix.size())
                    {
                        string rest = s.substr(prefix.size());
                        if(rest == "getset")
                        {
                            result.push_back(s);
                            continue;
                        }
                        else if(rest == "buffer")
                        {
                            result.push_back(s);
                            continue;
                        }
                        else if(rest == "UserException")
                        {
                            result.push_back(s);
                            continue;
                        }
                    }
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "type")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "serializable")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "protobuf")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "serialVersionUID")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "implements")
                {
                    result.push_back(s);
                    continue;
                }
                else if(s.substr(prefix.size(), pos - prefix.size()) == "package")
                {
                    result.push_back(s);
                    continue;
                }

                dc->warning(InvalidMetadata, cont->file(), cont->line(), "ignoring invalid metadata `" + s + "'");
            }
            else
            {
                result.push_back(s);
                continue;
            }
        }

        return result;
    }

    StringList validateType(const SyntaxTreeBasePtr& p, const StringList& metadata, const string& file, int line)
    {
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc = unt->findDefinitionContext(file);
        assert(dc);
        StringList newMetadata;
        for(StringList::const_iterator i = metadata.begin(); i != metadata.end(); ++i)
        {
            //
            // Type metadata ("java:type:Foo") is only supported by sequences and dictionaries.
            //
            if(i->find("java:type:", 0) == 0 && (!SequencePtr::dynamicCast(p) && !DictionaryPtr::dynamicCast(p)))
            {
                string str;
                ContainedPtr cont = ContainedPtr::dynamicCast(p);
                if(cont)
                {
                    str = cont->kindOf();
                }
                else
                {
                    BuiltinPtr b = BuiltinPtr::dynamicCast(p);
                    assert(b);
                    str = b->typeId();
                }
                dc->warning(InvalidMetadata, file, line, "invalid metadata for " + str);
            }
            else if(i->find("java:buffer") == 0)
            {
                SequencePtr seq = SequencePtr::dynamicCast(p);
                if(seq)
                {
                    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                    if(builtin &&
                       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
                        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
                        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
                    {
                        newMetadata.push_back(*i);
                        continue;
                    }

                }

                dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + *i + "'");
            }
            else if(i->find("java:protobuf:") == 0 || i->find("java:serializable:") == 0)
            {
                //
                // Only valid in sequence definition which is checked in visitSequence
                //
                dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + *i + "'");
            }
            else if(i->find("java:implements:") == 0)
            {
                if(ClassDefPtr::dynamicCast(p) || StructPtr::dynamicCast(p))
                {
                    newMetadata.push_back(*i);
                }
                else
                {
                    dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + *i + "'");
                }
            }
            else if(i->find("java:package:") == 0)
            {
                ModulePtr m = ModulePtr::dynamicCast(p);
                if(m && UnitPtr::dynamicCast(m->container()))
                {
                    newMetadata.push_back(*i);
                }
                else
                {
                    dc->warning(InvalidMetadata, file, line, "ignoring invalid metadata `" + *i + "'");
                }
            }
            else
            {
                newMetadata.push_back(*i);
            }
        }
        return newMetadata;
    }

    StringList validateGetSet(const SyntaxTreeBasePtr& p, const StringList& metadata, const string& file, int line)
    {
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc= unt->findDefinitionContext(file);
        assert(dc);
        StringList newMetadata;
        for(StringList::const_iterator i = metadata.begin(); i != metadata.end(); ++i)
        {
            // The "getset" metadata can only be specified on a class, struct, exception or data member.
            if((*i) == "java:getset" &&
               (!ClassDefPtr::dynamicCast(p) && !StructPtr::dynamicCast(p) && !ExceptionPtr::dynamicCast(p) &&
                !MemberPtr::dynamicCast(p)))
            {
                string str;
                ContainedPtr cont = ContainedPtr::dynamicCast(p);
                if(cont)
                {
                    str = cont->kindOf();
                }
                else
                {
                    BuiltinPtr b = BuiltinPtr::dynamicCast(p);
                    assert(b);
                    str = b->typeId();
                }
                dc->warning(InvalidMetadata, file, line, "invalid metadata for " + str);
                continue;
            }
            newMetadata.push_back(*i);
        }
        return newMetadata;
    }
};

}

long
Slice::computeSerialVersionUUID(const ClassDefPtr& p)
{
    ostringstream os;
    os << "Name: " << p->scoped();

    os << " Bases: [";
    if (p->base())
    {
        os << p->base()->scoped();
    }
    os << "]";

    os << " Members: [";
    MemberList members = p->dataMembers();
    for (auto i = members.begin(); i != members.end();)
    {
        os << (*i)->name() << ":" << (*i)->type();
        i++;
        if(i != members.end())
        {
            os << ", ";
        }
    }
    os << "]";

    const string data = os.str();
    long hashCode = 5381;
    hashAdd(hashCode, data);
    return hashCode;
}

long
Slice::computeSerialVersionUUID(const StructPtr& p)
{
    ostringstream os;

    os << "Name: " << p->scoped();
    os << " Members: [";
    MemberList members = p->dataMembers();
    for (auto i = members.begin(); i != members.end();)
    {
        os << (*i)->name() << ":" << (*i)->type();
        i++;
        if(i != members.end())
        {
            os << ", ";
        }
    }
    os << "]";

    const string data = os.str();
    long hashCode = 5381;
    hashAdd(hashCode, data);
    return hashCode;
}

long
Slice::computeSerialVersionUUID(const ExceptionPtr& p)
{
    ostringstream os;

    os << "Name: " << p->scoped();
    os << " Members: [";
    MemberList members = p->dataMembers();
    for (auto i = members.begin(); i != members.end();)
    {
        os << (*i)->name() << ":" << (*i)->type();
        i++;
        if(i != members.end())
        {
            os << ", ";
        }
    }
    os << "]";

    const string data = os.str();
    long hashCode = 5381;
    hashAdd(hashCode, data);
    return hashCode;
}

Slice::JavaOutput::JavaOutput()
{
}

Slice::JavaOutput::JavaOutput(ostream& os) :
    Output(os)
{
}

Slice::JavaOutput::JavaOutput(const char* s) :
    Output(s)
{
}

void
Slice::JavaOutput::openClass(const string& cls, const string& prefix, const string& sliceFile)
{
    string package;
    string file;
    string path = prefix;

    string::size_type pos = cls.rfind('.');
    if(pos != string::npos)
    {
        package = cls.substr(0, pos);
        file = cls.substr(pos + 1);
        string dir = package;

        //
        // Create package directories if necessary.
        //
        pos = 0;
        string::size_type start = 0;
        do
        {
            if(!path.empty())
            {
                path += "/";
            }
            pos = dir.find('.', start);
            if(pos != string::npos)
            {
                path += dir.substr(start, pos - start);
                start = pos + 1;
            }
            else
            {
                path += dir.substr(start);
            }

            IceUtilInternal::structstat st;
            if(!IceUtilInternal::stat(path, &st))
            {
                if(!(st.st_mode & S_IFDIR))
                {
                    ostringstream os;
                    os << "failed to create package directory `" << path
                       << "': file already exists and is not a directory";
                    throw FileException(__FILE__, __LINE__, os.str());
                }
                continue;
            }

            if(IceUtilInternal::mkdir(path, 0777) != 0)
            {
                ostringstream os;
                os << "cannot create directory `" << path << "': " << IceUtilInternal::errorToString(errno);
                throw FileException(__FILE__, __LINE__, os.str());
            }
            FileTracker::instance()->addDirectory(path);
        }
        while(pos != string::npos);
    }
    else
    {
        file = cls;
    }
    file += ".java";

    //
    // Open class file.
    //
    if(!path.empty())
    {
        path += "/";
    }
    path += file;

    open(path.c_str());
    if(isOpen())
    {
        FileTracker::instance()->addFile(path);
        printHeader();
        printGeneratedHeader(*this, sliceFile);
        if(!package.empty())
        {
            separator();
            print("package ");
            print(package.c_str());
            print(";");
        }
    }
    else
    {
        ostringstream os;
        os << "cannot open file `" << path << "': " << IceUtilInternal::errorToString(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
}

void
Slice::JavaOutput::printHeader()
{
    static const char* header =
"//\n"
"// Copyright (c) ZeroC, Inc. All rights reserved.\n"
"//\n"
        ;

    print(header);
    print("//\n");
    print("// Ice version ");
    print(ICE_STRING_VERSION);
    print("\n");
    print("//\n");
}

const string Slice::JavaGenerator::_getSetMetadata = "java:getset";

Slice::JavaGenerator::JavaGenerator(const string& dir) :
    _dir(dir),
    _out(0)
{
}

Slice::JavaGenerator::~JavaGenerator()
{
    // If open throws an exception other generators could be left open
    // during the stack unwind.
    if(_out != 0)
    {
        close();
    }
    assert(_out == 0);
}

void
Slice::JavaGenerator::open(const string& absolute, const string& file)
{
    assert(_out == 0);

    JavaOutput* out = createOutput();
    try
    {
        out->openClass(absolute, _dir, file);
    }
    catch(const FileException&)
    {
        delete out;
        throw;
    }
    _out = out;
}

void
Slice::JavaGenerator::close()
{
    assert(_out != 0);
    *_out << nl;
    delete _out;
    _out = 0;
}

Output&
Slice::JavaGenerator::output() const
{
    assert(_out != 0);
    return *_out;
}

//
// If the passed name is a scoped name, return the identical scoped name,
// but with all components that are Java keywords replaced by
// their "_"-prefixed version; otherwise, if the passed name is
// not scoped, but a Java keyword, return the "_"-prefixed name;
// otherwise, return the name unchanged.
//
string
Slice::JavaGenerator::fixKwd(const string& name) const
{
    if(name.empty())
    {
        return name;
    }
    if(name[0] != ':')
    {
        return lookupKwd(name);
    }
    auto ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), lookupKwd);
    stringstream result;
    for(const auto& id : ids)
    {
        result << "::" << id;
    }
    return result.str();
}

string
Slice::JavaGenerator::convertScopedName(const string& scoped, const string& prefix, const string& suffix) const
{
    string result;
    string::size_type start = 0;
    string fscoped = fixKwd(scoped);

    //
    // Skip leading "::"
    //
    if(fscoped[start] == ':')
    {
        assert(fscoped[start + 1] == ':');
        start += 2;
    }

    //
    // Convert all occurrences of "::" to "."
    //
    string::size_type pos;
    do
    {
        pos = fscoped.find(':', start);
        string fix;
        if(pos == string::npos)
        {
            string s = fscoped.substr(start);
            if(!s.empty())
            {
                fix = prefix + fixKwd(s) + suffix;
            }
        }
        else
        {
            assert(fscoped[pos + 1] == ':');
            fix = fixKwd(fscoped.substr(start, pos - start));
            start = pos + 2;
        }

        if(!result.empty() && !fix.empty())
        {
            result += ".";
        }
        result += fix;
    }
    while(pos != string::npos);

    return result;
}

string
Slice::JavaGenerator::addAnnotation(const string& str, const string& annotation) const
{
    // Find the last scope delimeter in the type (ignoring generic type parameters).
    auto genericsStartIndex = str.find_first_of('<');
    auto index = str.find_last_of('.', genericsStartIndex);
    if(index == string::npos)
    {
        return annotation + " " + str;
    }
    else
    {
        return (str.substr(0, index + 1) + annotation + " " + str.substr(index + 1));
    }
}

string
Slice::JavaGenerator::getPackagePrefix(const ContainedPtr& cont) const
{
    //
    // Traverse to the top-level module.
    //
    ModulePtr m;
    ContainedPtr p = cont;
    while(true)
    {
        if(ModulePtr::dynamicCast(p))
        {
            m = ModulePtr::dynamicCast(p);
        }

        ContainerPtr c = p->container();
        p = ContainedPtr::dynamicCast(c); // This cast fails for Unit.
        if(!p)
        {
            break;
        }
    }

    assert(m);

    //
    // The java:package metadata can be defined as global metadata or applied to a top-level module.
    // We check for the metadata at the top-level module first and then fall back to the global scope.
    //
    static const string prefix = "java:package:";

    string q;
    if(!m->findMetadata(prefix, q))
    {
        UnitPtr ut = cont->unit();
        string file = cont->file();
        assert(!file.empty());

        DefinitionContextPtr dc = ut->findDefinitionContext(file);
        assert(dc);
        q = dc->findMetadata(prefix);
    }

    if(!q.empty())
    {
        q = q.substr(prefix.size());
    }

    return q;
}

string
Slice::JavaGenerator::getPackage(const ContainedPtr& cont) const
{
    string scope = convertScopedName(cont->scope());
    string prefix = getPackagePrefix(cont);
    if(!prefix.empty())
    {
        if(!scope.empty())
        {
            return prefix + "." + scope;
        }
        else
        {
            return prefix;
        }
    }

    return scope;
}

string
Slice::JavaGenerator::getUnqualified(const std::string& type, const std::string& package) const
{
    if(type.find(".") != string::npos && type.find(package) == 0 && type.find(".", package.size() + 1) == string::npos)
    {
        return type.substr(package.size() + 1);
    }
    return type;
}

string
Slice::JavaGenerator::getUnqualified(const ContainedPtr& cont,
                                  const string& package,
                                  const string& prefix,
                                  const string& suffix) const
{
    string name = cont->name();
    if(prefix == "" && suffix == "")
    {
        name = fixKwd(name);
    }
    string contPkg = getPackage(cont);
    if(contPkg == package)
    {
        return prefix + name + suffix;
    }
    else if(!contPkg.empty())
    {
        return contPkg + "." + prefix + name + suffix;
    }
    else
    {
        return prefix + name + suffix;
    }
}

string
Slice::JavaGenerator::getStaticId(const TypePtr& type, const string& package) const
{
    BuiltinPtr b = BuiltinPtr::dynamicCast(type);
    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);

    assert((b && b->usesClasses()) || cl);

    if(b && b->kind() == Builtin::KindAnyClass)
    {
        return getUnqualified("com.zeroc.Ice.Value", package) + ".ice_staticId()";
    }
    else
    {
        return getUnqualified(cl, package) + ".ice_staticId()";
    }
}

string
Slice::JavaGenerator::getTagFormat(const TypePtr& type)
{
    return "com.zeroc.Ice.TagFormat." + type->getTagFormat();
}

string
Slice::JavaGenerator::typeToString(const TypePtr& constType,
                                   TypeMode mode,
                                   const string& package,
                                   const StringList& metadata,
                                   bool formal) const
{
    TypePtr type = unwrapIfOptional(constType);

    static const std::array<std::string, 17> builtinTable =
    {
        "boolean",
        "byte",
        "short",
        "???",
        "int",
        "???",
        "int",
        "???",
        "long",
        "???",
        "long",
        "???",
        "float",
        "double",
        "String",
        "com.zeroc.Ice.ObjectPrx",
        "com.zeroc.Ice.Value"
    };

    if(!type)
    {
        assert(mode == TypeModeReturn);
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return getUnqualified(builtinTable[builtin->kind()], package);
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        return getUnqualified(cl, package);
    }

    InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(type);
    if(interface)
    {
        return getUnqualified(interface, package, "", "Prx");
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        string instanceType, formalType;
        getDictionaryTypes(dict, package, metadata, instanceType, formalType);
        return formal ? formalType : instanceType;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string instanceType, formalType;
        getSequenceTypes(seq, package, metadata, instanceType, formalType);
        return formal ? formalType : instanceType;
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(type);
    if(contained)
    {
        if(mode == TypeModeOut)
        {
            return getUnqualified(contained, package, "", "Holder");
        }
        else
        {
            return getUnqualified(contained, package);
        }
    }

    return "???";
}

string
Slice::JavaGenerator::typeToObjectString(const TypePtr& constType,
                                         TypeMode mode,
                                         const string& package,
                                         const StringList& metadata,
                                         bool formal) const
{
    TypePtr type = unwrapIfOptional(constType);

    static const std::array<std::string, 17> builtinTable =
    {
        "java.lang.Boolean",
        "java.lang.Byte",
        "java.lang.Short",
        "???",
        "java.lang.Integer",
        "???",
        "java.lang.Integer",
        "???",
        "java.lang.Long",
        "???",
        "java.lang.Long",
        "???",
        "java.lang.Float",
        "java.lang.Double",
        "java.lang.String",
        "com.zeroc.Ice.ObjectPrx",
        "com.zeroc.Ice.Value"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin && mode != TypeModeOut)
    {
        return builtinTable[builtin->kind()];
    }

    return typeToString(type, mode, package, metadata, formal);
}

string
Slice::JavaGenerator::typeToAnnotatedString(const TypePtr& constType,
                                            TypeMode mode,
                                            const string& package,
                                            const StringList& metadata,
                                            bool tagged,
                                            bool object) const
{
    TypePtr type = unwrapIfOptional(constType);

    if(tagged)
    {
        return addAnnotation(typeToObjectString(type, mode, package, metadata, true), "@Nullable");
    }
    else if(object)
    {
        return typeToObjectString(type, mode, package, metadata, true);
    }
    else
    {
        string typeString = typeToString(type, mode, package, metadata, true);

        // TODO: Drop this logic once we stop using parameterless constructors.
        if(mode == TypeModeMember)
        {
            BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
            if(builtin)
            {
                switch(builtin->kind())
                {
                    case Builtin::KindByte:
                    case Builtin::KindBool:
                    case Builtin::KindShort:
                    case Builtin::KindInt:
                    case Builtin::KindLong:
                    case Builtin::KindFloat:
                    case Builtin::KindDouble:
                    {
                        return typeString;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            return addAnnotation(typeString, "@MonotonicNonNull");
        }
        else
        {
            return typeString;
        }
    }
}

string
typeToBufferString(const TypePtr& type)
{
    static const std::array<std::string, 17> builtinBufferTable =
    {
        "???",
        "java.nio.ByteBuffer",
        "java.nio.ShortBuffer",
        "???",
        "java.nio.IntBuffer",
        "???",
        "java.nio.IntBuffer",
        "???",
        "java.nio.LongBuffer",
        "???",
        "java.nio.LongBuffer",
        "???",
        "java.nio.FloatBuffer",
        "java.nio.DoubleBuffer",
        "???",
        "???",
        "???"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        return builtinBufferTable[builtin->kind()];
    }
    else
    {
        return "???";
    }
}

void
Slice::JavaGenerator::writeMarshalUnmarshalCode(Output& out,
                                                const string& package,
                                                const TypePtr& constType,
                                                TagMode mode,
                                                int tag,
                                                const string& param,
                                                bool marshal,
                                                int& iter,
                                                const string& customStream,
                                                const StringList& metadata,
                                                const string& patchParams)
{
    // TODO: for now, we handle marshaling of Optional<T> like T
    TypePtr type = unwrapIfOptional(constType);
    assert(!OptionalPtr::dynamicCast(type));

    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    const bool isTaggedParam = mode == TaggedInParam || mode == TaggedOutParam || mode == TaggedReturnParam;

    assert(!marshal || mode != TaggedMember); // Only support TaggedMember for un-marshaling

    const BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(marshal)
        {
            out << nl << stream << ".write" << builtinSuffixTable[builtin->kind()] << "(";
            if(isTaggedParam)
            {
                out << tag << ", ";
            }
            out << param << ");";
        }
        else
        {
            if(builtin->usesClasses())
            {
                assert(!patchParams.empty());
                out << nl << stream << ".readValue(";
                if(isTaggedParam)
                {
                    out << tag << ", ";
                }
                out << patchParams << ");";
            }
            else if(builtin->kind() == Builtin::KindObject)
            {
                if(isTaggedParam)
                {
                    out << nl << param << " = " << stream << ".readProxy(" << tag << ");";
                }
                else
                {
                    if(mode == TaggedMember)
                    {
                        out << nl << stream << ".skip(4);";
                    }
                    out << nl << param << " = " << stream << ".readProxy();";
                }
            }
            else
            {
                out << nl << param << " = " << stream << ".read" << builtinSuffixTable[builtin->kind()] << "(";
                if(isTaggedParam)
                {
                    out << tag;
                }
                out << ");";
            }
        }
        return;
    }

    string typeS = typeToString(type, TypeModeIn, package, metadata);

    InterfaceDeclPtr interface = InterfaceDeclPtr::dynamicCast(type);
    if(interface)
    {
        if(marshal)
        {
            if(isTaggedParam)
            {
                out << nl << stream << ".writeProxy(" << tag << ", " << param << ");";
            }
            else
            {
                out << nl << stream << ".writeProxy(" << param << ");";
            }
        }
        else
        {
            if(isTaggedParam)
            {
                out << nl << param << " = " << stream << ".readProxy(" << tag << ", " << typeS << "::uncheckedCast);";
            }
            else if(mode == TaggedMember)
            {
                out << nl << stream << ".skip(4);";
                out << nl << param << " = " << typeS << ".uncheckedCast(" << stream << ".readProxy());";
            }
            else
            {
                out << nl << param << " = " << typeS << ".uncheckedCast(" << stream << ".readProxy());";
            }
        }
        return;
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    if(cl)
    {
        if(marshal)
        {
            if(isTaggedParam)
            {
                out << nl << stream << ".writeValue(" << tag << ", " << param << ");";
            }
            else
            {
                out << nl << stream << ".writeValue(" << param << ");";
            }
        }
        else
        {
            assert(!patchParams.empty());
            if(isTaggedParam)
            {
                out << nl << stream << ".readValue(" << tag << ", " << patchParams << ");";
            }
            else
            {
                out << nl << stream << ".readValue(" << patchParams << ");";
            }
        }
        return;
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        if(isTaggedParam || mode == TaggedMember)
        {
            string instanceType, formalType, origInstanceType, origFormalType;
            getDictionaryTypes(dict, "", metadata, instanceType, formalType);
            getDictionaryTypes(dict, "", StringList(), origInstanceType, origFormalType);
            if(formalType == origFormalType && (marshal || instanceType == origInstanceType))
            {
                //
                // If we can use the helper, it's easy.
                //
                string helper = getUnqualified(dict, package, "", "Helper");
                if(marshal)
                {
                    out << nl << helper << ".write" << spar << stream << tag << param << epar << ";";
                    return;
                }
                else if(mode != TaggedMember)
                {
                    out << nl << param << " = " << helper << ".read" << spar << stream << tag << epar << ";";
                    return;
                }
            }

            TypePtr keyType = dict->keyType();
            TypePtr valueType = dict->valueType();
            if(marshal)
            {
                if(isTaggedParam)
                {
                    out << nl;
                    out << "if(" << param << " != null && " << stream << ".writeTag(" << tag << ", "
                        << getTagFormat(type) << "))";
                    out << sb;
                }

                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    out << nl << "int pos = " <<  stream << ".startSize();";
                    writeDictionaryMarshalUnmarshalCode(out, package, dict, param, marshal, iter, true, customStream, metadata);
                    out << nl << stream << ".endSize(pos);";
                }
                else
                {
                    const size_t sz = keyType->minWireSize() + valueType->minWireSize();
                    out << nl << "final int taggedSize = " << param << " == null ? 0 : " << param << ".size();";
                    out << nl << stream
                        << ".writeSize(taggedSize > 254 ? taggedSize * " << sz << " + 5 : taggedSize * " << sz << " + 1);";
                    writeDictionaryMarshalUnmarshalCode(out, package, dict, param, marshal, iter, true, customStream, metadata);
                }

                if(isTaggedParam)
                {
                    out << eb;
                }
            }
            else
            {
                string d = isTaggedParam ? "taggedDict" : param;
                if(isTaggedParam)
                {
                    out << nl << "if(" << stream << ".readTag(" << tag << ", " << getTagFormat(type) << "))";
                    out << sb;
                    out << nl << typeS << ' ' << d << ';';
                }
                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else
                {
                    out << nl << stream << ".skipSize();";
                }
                writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, customStream, metadata);
                if(isTaggedParam)
                {
                    out << nl << param << " = " << d << ";";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << param << " = null;";
                    out << eb;
                }
            }
        }
        else
        {
            writeDictionaryMarshalUnmarshalCode(out, package, dict, param, marshal, iter, true, customStream, metadata);
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(isTaggedParam || mode == TaggedMember)
        {
            string ignored;
            TypePtr elemType = seq->type();
            BuiltinPtr eltBltin = BuiltinPtr::dynamicCast(elemType);
            if(!hasTypeMetadata(seq, metadata) && eltBltin && eltBltin->kind() < Builtin::KindObject)
            {
                string bs = builtinSuffixTable[eltBltin->kind()];
                if(marshal)
                {
                    out << nl << stream << ".write" << bs << "Seq(" << tag << ", " << param << ");";
                    return;
                }
                else if(mode != TaggedMember)
                {
                    out << nl << param << " = " << stream << ".read" << bs << "Seq(" << tag << ");";
                    return;
                }
            }
            else if(findMetadata("java:serializable", seq->getAllMetadata(), ignored))
            {
                if(marshal)
                {
                    out << nl << stream << ".writeSerializable" << spar << tag << param << epar << ";";
                    return;
                }
                else if(mode != TaggedMember)
                {
                    out << nl << param << " = " << stream << ".readSerializable" << spar << tag << typeS + ".class"
                        << epar << ";";
                    return;
                }
            }
            else if(!hasTypeMetadata(seq, metadata) ||
                    findMetadata("java:type", seq->getAllMetadata(), ignored) ||
                    findMetadata("java:type", metadata, ignored))
            {
                string instanceType, formalType, origInstanceType, origFormalType;
                getSequenceTypes(seq, "", metadata, instanceType, formalType);
                getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
                if(formalType == origFormalType && (marshal || instanceType == origInstanceType))
                {
                    string helper = getUnqualified(seq, package, "", "Helper");
                    if(marshal)
                    {
                        out << nl << helper << ".write" << spar << stream << tag << param << epar << ";";
                        return;
                    }
                    else if(mode != TaggedMember)
                    {
                        out << nl << param << " = " << helper << ".read" << spar << stream << tag << epar << ";";
                        return;
                    }
                }
            }

            if(marshal)
            {
                if(isTaggedParam)
                {
                    out << nl;
                    out << "if(" << param << " != null && " << stream << ".writeTag(" << tag << ", "
                        << getTagFormat(type) << "))";
                    out << sb;
                }

                if(elemType->isVariableLength())
                {
                    out << nl << "int pos = " <<  stream << ".startSize();";
                    writeSequenceMarshalUnmarshalCode(out, package, seq, param, true, iter, true, customStream, metadata);
                    out << nl << stream << ".endSize(pos);";
                }
                else
                {
                    const size_t sz = elemType->minWireSize();
                    if(sz > 1)
                    {
                        string ignored2;
                        out << nl << "final int taggedSize = " << param << " == null ? 0 : ";
                        if(findMetadata("java:buffer", seq->getAllMetadata(), ignored2) ||
                           findMetadata("java:buffer", metadata, ignored2))
                        {
                            out << param << ".remaining() / " << sz << ";";
                        }
                        else if(hasTypeMetadata(seq, metadata))
                        {
                            out << param << ".size();";
                        }
                        else
                        {
                            out << param << ".length;";
                        }
                        out << nl << stream << ".writeSize(taggedSize > 254 ? taggedSize * " << sz
                            << " + 5 : taggedSize * " << sz << " + 1);";
                    }
                    writeSequenceMarshalUnmarshalCode(out, package, seq, param, true, iter, true, customStream, metadata);
                }

                if(isTaggedParam)
                {
                    out << eb;
                }
            }
            else
            {
                const size_t sz = elemType->minWireSize();
                string s = isTaggedParam ? "taggedSeq" : param;
                if(isTaggedParam)
                {
                    out << nl << "if(" << stream << ".readTag(" << tag << ", " << getTagFormat(type) << "))";
                    out << sb;
                    out << nl << typeS << ' ' << s << ';';
                }
                if(elemType->isVariableLength())
                {
                    out << nl << stream << ".skip(4);";
                }
                else if(sz > 1)
                {
                    out << nl << stream << ".skipSize();";
                }
                writeSequenceMarshalUnmarshalCode(out, package, seq, s, false, iter, true, customStream, metadata);
                if(isTaggedParam)
                {
                    out << nl << param << " = " << s << ";";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << param << " = null;";
                    out << eb;
                }
            }
        }
        else
        {
            writeSequenceMarshalUnmarshalCode(out, package, seq, param, marshal, iter, true, customStream, metadata);
        }
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    StructPtr st = StructPtr::dynamicCast(type);
    assert(constructed);
    if(marshal)
    {
        if(isTaggedParam)
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << tag << ", " << param << ");";
        }
        else
        {
            out << nl << typeS << ".ice_write(" << stream << ", " << param << ");";
        }
    }
    else
    {
        if(isTaggedParam)
        {
            out << nl << param << " = " << typeS << ".ice_read(" << stream << ", " << tag << ");";
        }
        else if(mode == TaggedMember && st)
        {
            out << nl << stream << (st->isVariableLength() ? ".skip(4);" : ".skipSize();");
            out << nl << param << " = " << typeS << ".ice_read(" << stream << ");";
        }
        else
        {
            out << nl << param << " = " << typeS << ".ice_read(" << stream << ");";
        }
    }
}

void
Slice::JavaGenerator::writeDictionaryMarshalUnmarshalCode(Output& out,
                                                          const string& package,
                                                          const DictionaryPtr& dict,
                                                          const string& param,
                                                          bool marshal,
                                                          int& iter,
                                                          bool useHelper,
                                                          const string& customStream,
                                                          const StringList& metadata)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    //
    // We have to determine whether it's possible to use the
    // type's generated helper class for this marshal/unmarshal
    // task. Since the user may have specified a custom type in
    // metadata, it's possible that the helper class is not
    // compatible and therefore we'll need to generate the code
    // in-line instead.
    //
    // Specifically, there may be "local" metadata (i.e., from
    // a data member or parameter definition) that overrides the
    // original type. We'll compare the mapped types with and
    // without local metadata to determine whether we can use
    // the helper.
    //
    string instanceType, formalType, origInstanceType, origFormalType;
    getDictionaryTypes(dict, "", metadata, instanceType, formalType);
    getDictionaryTypes(dict, "", StringList(), origInstanceType, origFormalType);
    if(useHelper && formalType == origFormalType && (marshal || instanceType == origInstanceType))
    {
        //
        // If we can use the helper, it's easy.
        //
        string helper = getUnqualified(dict, package, "", "Helper");
        if(marshal)
        {
            out << nl << helper << ".write" << spar << stream << param << epar << ";";
        }
        else
        {
            out << nl << param << " = " << helper << ".read" << spar << stream << epar << ";";
        }
        return;
    }

    TypePtr key = dict->keyType();
    TypePtr value = unwrapIfOptional(dict->valueType());

    string keyS = typeToString(key, TypeModeIn, package);
    string valueS = typeToString(value, TypeModeIn, package);

    ostringstream o;
    o << iter;
    string iterS = o.str();
    iter++;

    if(marshal)
    {
        out << nl << "if(" << param << " == null)";
        out << sb;
        out << nl << "ostr.writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "ostr.writeSize(" << param << ".size());";
        string keyObjectS = typeToObjectString(key, TypeModeIn, package);
        string valueObjectS = typeToObjectString(value, TypeModeIn, package);
        out << nl;
        out << "for(java.util.Map.Entry<" << keyObjectS << ", " << valueObjectS << "> e : " << param << ".entrySet())";
        out << sb;
        writeMarshalUnmarshalCode(out, package, key, NotTagged, 0, "e.getKey()", true, iter, customStream);
        writeMarshalUnmarshalCode(out, package, value, NotTagged, 0, "e.getValue()", true, iter,
                                  customStream);
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << param << " = new " << instanceType << "();";
        out << nl << "int sz" << iterS << " = " << stream << ".readSize();";
        out << nl << "for(int i" << iterS << " = 0; i" << iterS << " < sz" << iterS << "; i" << iterS << "++)";
        out << sb;

        BuiltinPtr b = BuiltinPtr::dynamicCast(value);
        if(ClassDeclPtr::dynamicCast(value) || (b && b->usesClasses()))
        {
            out << nl << "final " << keyS << " key;";
            writeMarshalUnmarshalCode(out, package, key, NotTagged, 0, "key", false, iter, customStream);

            valueS = typeToObjectString(value, TypeModeIn, package);
            ostringstream patchParams;
            patchParams << "value -> " << param << ".put(key, value), " << valueS << ".class";
            writeMarshalUnmarshalCode(out, package, value, NotTagged, 0, "value", false, iter, customStream,
                                      StringList(), patchParams.str());
        }
        else
        {
            out << nl << keyS << " key;";
            writeMarshalUnmarshalCode(out, package, key, NotTagged, 0, "key", false, iter, customStream);

            out << nl << valueS << " value;";
            writeMarshalUnmarshalCode(out, package, value, NotTagged, 0, "value", false, iter, customStream);

            BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
            if(!(builtin && builtin->usesClasses()) && !ClassDeclPtr::dynamicCast(value))
            {
                out << nl << param << ".put(key, value);";
            }
        }
        out << eb;
    }
}

void
Slice::JavaGenerator::writeSequenceMarshalUnmarshalCode(Output& out,
                                                        const string& package,
                                                        const SequencePtr& seq,
                                                        const string& param,
                                                        bool marshal,
                                                        int& iter,
                                                        bool useHelper,
                                                        const string& customStream,
                                                        const StringList& metadata)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    string typeS = typeToObjectString(seq, TypeModeIn, package);

    TypePtr elementType = unwrapIfOptional(seq->type());

    //
    // If the sequence is a byte sequence, check if there's the serializable or protobuf metadata to
    // get rid of these two easy cases first.
    //
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(elementType);
    if(builtin && builtin->kind() == Builtin::KindByte)
    {
        string meta;
        static const string protobuf = "java:protobuf:";
        static const string serializable = "java:serializable:";
        if(seq->findMetadata(serializable, meta))
        {
            if(marshal)
            {
                out << nl << stream << ".writeSerializable(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".readSerializable(" << typeS << ".class);";
            }
            return;
        }
        else if(seq->findMetadata(protobuf, meta))
        {
            if(marshal)
            {
                out << nl << "if(!" << param << ".isInitialized())";
                out << sb;
                out << nl << "throw new com.zeroc.Ice.MarshalException(\"type not fully initialized\");";
                out << eb;
                out << nl << stream << ".writeByteSeq(" << param << ".toByteArray());";
            }
            else
            {
                out << nl << "try";
                out << sb;
                out << nl << param << " = " << typeS << ".parseFrom(" << stream << ".readByteSeq());";
                out << eb;
                out << nl << "catch(com.google.protobuf.InvalidProtocolBufferException ex)";
                out << sb;
                out << nl << "throw new com.zeroc.Ice.MarshalException(ex);";
                out << eb;
            }
            return;
        }
    }

    if(builtin &&
       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
    {
        string meta;
        static const string bytebuffer = "java:buffer";
        if(seq->findMetadata(bytebuffer, meta) || findMetadata(bytebuffer, metadata, meta))
        {
            if(marshal)
            {
                out << nl << stream << ".write" << builtinSuffixTable[builtin->kind()] << "Buffer(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".read" << builtinSuffixTable[builtin->kind()] << "Buffer();";
            }
            return;
        }
    }

    if(!hasTypeMetadata(seq, metadata) && builtin && builtin->kind() <= Builtin::KindString)
    {
        if(marshal)
        {
            out << nl << stream << ".write" << builtinSuffixTable[builtin->kind()] << "Seq(" << param << ");";
        }
        else
        {
            out << nl << param << " = " << stream << ".read" << builtinSuffixTable[builtin->kind()] << "Seq();";
        }
        return;
    }

    //
    // We have to determine whether it's possible to use the
    // type's generated helper class for this marshal/unmarshal
    // task. Since the user may have specified a custom type in
    // metadata, it's possible that the helper class is not
    // compatible and therefore we'll need to generate the code
    // in-line instead.
    //
    // Specifically, there may be "local" metadata (i.e., from
    // a data member or parameter definition) that overrides the
    // original type. We'll compare the mapped types with and
    // without local metadata to determine whether we can use
    // the helper.
    //
    string instanceType, formalType, origInstanceType, origFormalType;
    bool customType = getSequenceTypes(seq, "", metadata, instanceType, formalType);
    getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
    if(useHelper && formalType == origFormalType && (marshal || instanceType == origInstanceType))
    {
        //
        // If we can use the helper, it's easy.
        //
        string helper = getUnqualified(seq, package, "", "Helper");
        if(marshal)
        {
            out << nl << helper << ".write" << spar << stream << param << epar << ";";
        }
        else
        {
            out << nl << param << " = " << helper << ".read" << spar << stream << epar << ";";
        }
        return;
    }

    //
    // Determine sequence depth.
    //
    int depth = 0;
    TypePtr origContent = elementType;
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while(s)
    {
        //
        // Stop if the inner sequence type has a custom, serializable or protobuf type.
        //
        if(hasTypeMetadata(s))
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    TypePtr type = elementType;

    if(customType)
    {
        //
        // Marshal/unmarshal a custom sequence type.
        //
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        typeS = getUnqualified(seq, package);
        ostringstream o;
        o << origContentS;
        int d = depth;
        while(d--)
        {
            o << "[]";
        }
        string cont = o.str();
        if(marshal)
        {
            out << nl << "if(" << param << " == null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << param << ".size());";
            string ctypeS = typeToString(type, TypeModeIn, package);
            out << nl << "for(" << ctypeS << " elem : " << param << ')';
            out << sb;
            writeMarshalUnmarshalCode(out, package, type, NotTagged, 0, "elem", true, iter, customStream);
            out << eb;
            out << eb; // else
        }
        else
        {
            bool isObject = false;
            ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
            if((builtin && builtin->usesClasses()) || cl)
            {
                isObject = true;
            }
            out << nl << param << " = new " << instanceType << "();";
            out << nl << "final int len" << iter << " = " << stream << ".readAndCheckSeqSize(" << type->minWireSize()
                << ");";
            out << nl << "for(int i" << iter << " = 0; i" << iter << " < len" << iter << "; i" << iter << "++)";
            out << sb;
            if(isObject)
            {
                //
                // Add a null value to the list as a placeholder for the element.
                //
                out << nl << param << ".add(null);";
                ostringstream patchParams;
                out << nl << "final int fi" << iter << " = i" << iter << ";";
                patchParams << "value -> " << param << ".set(fi" << iter << ", value), " << origContentS << ".class";

                writeMarshalUnmarshalCode(out, package, type, NotTagged, 0, "elem", false, iter,
                                          customStream, StringList(), patchParams.str());
            }
            else
            {
                out << nl << cont << " elem;";
                writeMarshalUnmarshalCode(out, package, type, NotTagged, 0, "elem", false, iter, customStream);
                out << nl << param << ".add(elem);";
            }
            out << eb;
            iter++;
        }
    }
    else
    {
        if(builtin &&
        (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
            builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
            builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble ||
            builtin->kind() == Builtin::KindString))
        {
            if(marshal)
            {
                out << nl << stream << ".write" << builtinSuffixTable[builtin->kind()] << "Seq(" << param << ");";
            }
            else
            {
                out << nl << param << " = " << stream << ".read" << builtinSuffixTable[builtin->kind()] << "Seq();";
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << "if(" << param << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << param << ".length);";
                out << nl << "for(int i" << iter << " = 0; i" << iter << " < " << param << ".length; i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << param << "[i" << iter << "]";
                iter++;
                writeMarshalUnmarshalCode(out, package, type, NotTagged, 0, o.str(), true, iter, customStream);
                out << eb;
                out << eb;
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(origContent);
                if((builtin && builtin->usesClasses()) || cl)
                {
                    isObject = true;
                }
                out << nl << "final int len" << iter << " = " << stream << ".readAndCheckSeqSize("
                    << type->minWireSize() << ");";
                //
                // We cannot allocate an array of a generic type, such as
                //
                // arr = new Map<String, String>[sz];
                //
                // Attempting to compile this code results in a "generic array creation" error
                // message. This problem can occur when the sequence's element type is a
                // dictionary, or when the element type is a nested sequence that uses a custom
                // mapping.
                //
                // The solution is to rewrite the code as follows:
                //
                // arr = (Map<String, String>[])new Map[sz];
                //
                // Unfortunately, this produces an unchecked warning during compilation.
                //
                // A simple test is to look for a "<" character in the content type, which
                // indicates the use of a generic type.
                //
                string::size_type pos = origContentS.find('<');
                if(pos != string::npos)
                {
                    string nonGenericType = origContentS.substr(0, pos);
                    out << nl << param << " = (" << origContentS << "[]";
                    int d = depth;
                    while(d--)
                    {
                        out << "[]";
                    }
                    out << ")new " << nonGenericType << "[len" << iter << "]";
                }
                else
                {
                    out << nl << param << " = new " << origContentS << "[len" << iter << "]";
                }
                int d = depth;
                while(d--)
                {
                    out << "[]";
                }
                out << ';';
                out << nl << "for(int i" << iter << " = 0; i" << iter << " < len" << iter << "; i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << param << "[i" << iter << "]";
                if(isObject)
                {
                    ostringstream patchParams;
                    out << nl << "final int fi" << iter << " = i" << iter << ";";
                    patchParams << "value -> " << param << "[fi" << iter << "] = value, " << origContentS << ".class";
                    writeMarshalUnmarshalCode(out, package, type, NotTagged, 0, o.str(), false, iter,
                                              customStream, StringList(), patchParams.str());
                }
                else
                {
                    writeMarshalUnmarshalCode(out, package, type, NotTagged, 0, o.str(), false, iter,
                                              customStream);
                }
                out << eb;
                iter++;
            }
        }
    }
}

bool
Slice::JavaGenerator::findMetadata(const string& prefix, const StringList& metadata, string& value)
{
    for(const auto& q : metadata)
    {
        if(q.find(prefix) == 0)
        {
            value = q;
            return true;
        }
    }

    return false;
}

bool
Slice::JavaGenerator::getTypeMetadata(const StringList& metadata, string& instanceType, string& formalType)
{
    //
    // Extract the instance type and an optional formal type.
    // The correct syntax is "java:type:instance-type[:formal-type]".
    //
    static const string prefix = "java:type:";
    string directive;
    if(findMetadata(prefix, metadata, directive))
    {
        string::size_type pos = directive.find(':', prefix.size());
        if(pos != string::npos)
        {
            instanceType = directive.substr(prefix.size(), pos - prefix.size());
            formalType = directive.substr(pos + 1);
        }
        else
        {
            instanceType = directive.substr(prefix.size());
            formalType.clear();
        }
        return true;
    }

    return false;
}

bool
Slice::JavaGenerator::hasTypeMetadata(const TypePtr& type, const StringList& localMetadata)
{
    ContainedPtr cont = ContainedPtr::dynamicCast(type);
    if(cont)
    {
        static const string prefix = "java:type:";
        string directive;

        if(findMetadata(prefix, localMetadata, directive))
        {
            return true;
        }

        StringList metadata = cont->getAllMetadata();

        if(findMetadata(prefix, metadata, directive))
        {
            return true;
        }

        if(findMetadata("java:protobuf:", metadata, directive) ||
           findMetadata("java:serializable:", metadata, directive))
        {
            SequencePtr seq = SequencePtr::dynamicCast(cont);
            if(seq)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin && builtin->kind() == Builtin::KindByte)
                {
                    return true;
                }
            }
        }

        if(findMetadata("java:buffer", metadata, directive) ||
           findMetadata("java:buffer", localMetadata, directive))
        {
            SequencePtr seq = SequencePtr::dynamicCast(cont);
            if(seq)
            {
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
                if(builtin &&
                   (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
                    builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
                    builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool
Slice::JavaGenerator::getDictionaryTypes(const DictionaryPtr& dict,
                                         const string& package,
                                         const StringList& metadata,
                                         string& instanceType,
                                         string& formalType) const
{
    //
    // Get the types of the key and value.
    //
    string keyTypeStr = typeToObjectString(dict->keyType(), TypeModeIn, package);
    string valueTypeStr = typeToObjectString(unwrapIfOptional(dict->valueType()), TypeModeIn, package);

    //
    // Collect metadata for a custom type.
    //
    if(getTypeMetadata(metadata, instanceType, formalType) ||
       getTypeMetadata(dict->getAllMetadata(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if(formalType.empty())
        {
            formalType = "java.util.Map<" + keyTypeStr + ", " + valueTypeStr + ">";
        }
        return true;
    }

    //
    // Return a default type for the platform.
    //
    instanceType = "java.util.HashMap<" + keyTypeStr + ", " + valueTypeStr + ">";
    formalType = "java.util.Map<" + keyTypeStr + ", " + valueTypeStr + ">";
    return false;
}

bool
Slice::JavaGenerator::getSequenceTypes(const SequencePtr& seq,
                                       const string& package,
                                       const StringList& metadata,
                                       string& instanceType,
                                       string& formalType) const
{
    // TODO: not quite correct
    TypePtr elementType = unwrapIfOptional(seq->type());

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(elementType);
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindByte)
        {
            string prefix = "java:serializable:";
            string meta;
            if(seq->findMetadata(prefix, meta))
            {
                instanceType = formalType = meta.substr(prefix.size());
                return true;
            }
            prefix = "java:protobuf:";
            if(seq->findMetadata(prefix, meta))
            {
                instanceType = formalType = meta.substr(prefix.size());
                return true;
            }
        }

        if((builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
            builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
            builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
        {
            string prefix = "java:buffer";
            string meta;
            string ignored;
            if(seq->findMetadata(prefix, meta) || findMetadata(prefix, metadata, ignored))
            {
                instanceType = formalType = typeToBufferString(elementType);
                return true;
            }
        }
    }

    //
    // Collect metadata for a custom type.
    //
    if(getTypeMetadata(metadata, instanceType, formalType) ||
       getTypeMetadata(seq->getAllMetadata(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if(formalType.empty())
        {
            formalType = "java.util.List<" + typeToObjectString(elementType, TypeModeIn, package) + ">";
        }
        return true;
    }

    //
    // The default mapping is a native array.
    //
    instanceType = formalType = typeToString(elementType, TypeModeIn, package, metadata) + "[]";
    return false;
}

JavaOutput*
Slice::JavaGenerator::createOutput()
{
    return new JavaOutput;
}

void
Slice::JavaGenerator::validateMetadata(const UnitPtr& u)
{
    MetadataVisitor visitor;
    u->visit(&visitor, true);
}
