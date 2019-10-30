//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/StringUtil.h>
#include <Slice/JavaUtil.h>
#include <Slice/FileTracker.h>
#include <Slice/Util.h>
#include <Slice/MD5.h>
#include <IceUtil/Functional.h>
#include <IceUtil/FileUtil.h>

#include <sys/types.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#endif

#ifndef _WIN32
#include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;

namespace
{

void
hashAdd(long& hashCode, const std::string& value)
{
    for(std::string::const_iterator p = value.begin(); p != value.end(); ++p)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ *p;
    }
}

string
typeToBufferString(const TypePtr& type)
{
    static const char* builtinBufferTable[] =
    {
        "java.nio.ByteBuffer",
        "???",
        "java.nio.ShortBuffer",
        "java.nio.IntBuffer",
        "java.nio.LongBuffer",
        "java.nio.FloatBuffer",
        "java.nio.DoubleBuffer",
        "???",
        "???",
        "???",
        "???"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(!builtin)
    {
        return "???";
    }
    else
    {
        return builtinBufferTable[builtin->kind()];
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

//
// Split a scoped name into its components and return the components as a list of (unscoped) identifiers.
//
static StringList
splitScopedName(const string& scoped)
{
    assert(scoped[0] == ':');
    StringList ids;
    string::size_type next = 0;
    string::size_type pos;
    while((pos = scoped.find("::", next)) != string::npos)
    {
        pos += 2;
        if(pos != scoped.size())
        {
            string::size_type endpos = scoped.find("::", pos);
            if(endpos != string::npos)
            {
                ids.push_back(scoped.substr(pos, endpos - pos));
            }
        }
        next = pos;
    }
    if(next != scoped.size())
    {
        ids.push_back(scoped.substr(next));
    }
    else
    {
        ids.push_back("");
    }

    return ids;
}

class MetaDataVisitor : public ParserVisitor
{
public:

    virtual bool visitUnitStart(const UnitPtr& p)
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
            StringList globalMetaData = dc->getMetaData();
            for(StringList::const_iterator r = globalMetaData.begin(); r != globalMetaData.end();)
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
                        dc->warning(InvalidMetaData, file, "",  "ignoring invalid global metadata `" + s + "'");
                        globalMetaData.remove(s);
                        continue;
                    }
                };
            }
            dc->setMetaData(globalMetaData);
        }
        return true;
    }

    virtual bool visitModuleStart(const ModulePtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
        return true;
    }

    virtual void visitClassDecl(const ClassDeclPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
    }

    virtual bool visitClassDefStart(const ClassDefPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
        return true;
    }

    virtual bool visitExceptionStart(const ExceptionPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
        return true;
    }

    virtual bool visitStructStart(const StructPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
        return true;
    }

    virtual void visitOperation(const OperationPtr& p)
    {
        TypePtr returnType = p->returnType();
        StringList metaData = getMetaData(p);

        UnitPtr unt = p->unit();
        string file = p->file();
        DefinitionContextPtr dc = unt->findDefinitionContext(p->file());

        if(!returnType)
        {
            for(StringList::const_iterator q = metaData.begin(); q != metaData.end();)
            {
                string s = *q++;
                if(s.find("java:type:", 0) == 0)
                {
                    dc->warning(InvalidMetaData, p->file(), p->line(), "ignoring invalid metadata `" + s +
                                "' for operation with void return type");
                    metaData.remove(s);
                    continue;
                }
            }
        }
        else
        {
            metaData = validateType(returnType, metaData, p->file(), p->line());
            metaData = validateGetSet(p, metaData, p->file(), p->line());
        }
        p->setMetaData(metaData);

        ParamDeclList params = p->parameters();
        for(ParamDeclList::iterator q = params.begin(); q != params.end(); ++q)
        {
            metaData = getMetaData(*q);
            metaData = validateType((*q)->type(), metaData, p->file(), (*q)->line());
            metaData = validateGetSet((*q)->type(), metaData, p->file(), (*q)->line());
            (*q)->setMetaData(metaData);
        }
    }

    virtual void visitDataMember(const DataMemberPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p->type(), metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
    }

    virtual void visitSequence(const SequencePtr& p)
    {
        static const string protobuf = "java:protobuf:";
        static const string serializable = "java:serializable:";
        static const string bytebuffer = "java:buffer";
        StringList metaData = getMetaData(p);
        StringList newMetaData;

        const string file =  p->file();
        const string line = p->line();
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc = unt->findDefinitionContext(file);

        for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); )
        {
            string s = *q++;

            if(s.find(protobuf) == 0 || s.find(serializable) == 0)
            {
                //
                // Remove from list so validateType does not try to handle as well.
                //
                metaData.remove(s);
                BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
                if(!builtin || builtin->kind() != Builtin::KindByte)
                {
                    dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + s + "': " +
                                "this metadata can only be used with a byte sequence");
                    continue;
                }
                newMetaData.push_back(s);
            }
            else if(s.find(bytebuffer) == 0)
            {
                metaData.remove(s);

                BuiltinPtr builtin = BuiltinPtr::dynamicCast(p->type());
                if(!builtin ||
                   (builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindShort &&
                    builtin->kind() != Builtin::KindInt && builtin->kind() != Builtin::KindLong &&
                    builtin->kind() != Builtin::KindFloat && builtin->kind() != Builtin::KindDouble))
                {
                    dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + s + "': " +
                                "this metadata can not be used with this type");
                    continue;
                }
                newMetaData.push_back(s);
            }
        }

        metaData = validateType(p, metaData, file, line);
        metaData = validateGetSet(p, metaData, file, line);
        newMetaData.insert(newMetaData.begin(), metaData.begin(), metaData.end());
        p->setMetaData(newMetaData);
    }

    virtual void visitDictionary(const DictionaryPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
    }

    virtual void visitEnum(const EnumPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
    }

    virtual void visitConst(const ConstPtr& p)
    {
        StringList metaData = getMetaData(p);
        metaData = validateType(p, metaData, p->file(), p->line());
        metaData = validateGetSet(p, metaData, p->file(), p->line());
        p->setMetaData(metaData);
    }

private:

    StringList getMetaData(const ContainedPtr& cont)
    {
        static const string prefix = "java:";

        StringList metaData = cont->getMetaData();
        StringList result;

        UnitPtr unt = cont->container()->unit();
        string file = cont->file();
        DefinitionContextPtr dc = unt->findDefinitionContext(file);
        assert(dc);

        for(StringList::const_iterator p = metaData.begin(); p != metaData.end(); ++p)
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
                        else if(rest == "tie")
                        {
                            result.push_back(s);
                            continue;
                        }
                        else if(rest == "UserException")
                        {
                            result.push_back(s);
                            continue;
                        }
                        else if(rest == "optional")
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

                dc->warning(InvalidMetaData, cont->file(), cont->line(), "ignoring invalid metadata `" + s + "'");
            }
            else
            {
                result.push_back(s);
                continue;
            }
        }

        return result;
    }

    StringList validateType(const SyntaxTreeBasePtr& p, const StringList& metaData, const string& file,
                            const string& line)
    {
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc = unt->findDefinitionContext(file);
        assert(dc);
        StringList newMetaData;
        for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
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
                dc->warning(InvalidMetaData, file, line, "invalid metadata for " + str);
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
                        newMetaData.push_back(*i);
                        continue;
                    }

                }

                dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + *i + "'");
            }
            else if(i->find("java:protobuf:") == 0 || i->find("java:serializable:") == 0)
            {
                //
                // Only valid in sequence definition which is checked in visitSequence
                //
                dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + *i + "'");
            }
            else if(i->find("delegate") == 0)
            {
                ClassDefPtr cl = ClassDefPtr::dynamicCast(p);
                if(cl && cl->isDelegate())
                {
                    newMetaData.push_back(*i);
                }
                else
                {
                    dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + *i + "'");
                }
            }
            else if(i->find("java:implements:") == 0)
            {
                if(ClassDefPtr::dynamicCast(p) || StructPtr::dynamicCast(p))
                {
                    newMetaData.push_back(*i);
                }
                else
                {
                    dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + *i + "'");
                }
            }
            else if(i->find("java:package:") == 0)
            {
                ModulePtr m = ModulePtr::dynamicCast(p);
                if(m && UnitPtr::dynamicCast(m->container()))
                {
                    newMetaData.push_back(*i);
                }
                else
                {
                    dc->warning(InvalidMetaData, file, line, "ignoring invalid metadata `" + *i + "'");
                }
            }
            else
            {
                newMetaData.push_back(*i);
            }
        }
        return newMetaData;
    }

    StringList validateGetSet(const SyntaxTreeBasePtr& p, const StringList& metaData, const string& file,
                              const string& line)
    {
        const UnitPtr unt = p->unit();
        const DefinitionContextPtr dc= unt->findDefinitionContext(file);
        assert(dc);
        StringList newMetaData;
        for(StringList::const_iterator i = metaData.begin(); i != metaData.end(); ++i)
        {
            //
            // The "getset" metadata can only be specified on a class, struct, exception or data member.
            //
            if((*i) == "java:getset" &&
               (!ClassDefPtr::dynamicCast(p) && !StructPtr::dynamicCast(p) && !ExceptionPtr::dynamicCast(p) &&
                !DataMemberPtr::dynamicCast(p)))
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
                dc->warning(InvalidMetaData, file, line, "invalid metadata for " + str);
                continue;
            }
            newMetaData.push_back(*i);
        }
        return newMetaData;
    }
};

}

long
Slice::computeSerialVersionUUID(const ClassDefPtr& p)
{
    ostringstream os;

    ClassList bases = p->bases();
    os << "Name: " << p->scoped();

    os << " Bases: [";
    for(ClassList::const_iterator i = bases.begin(); i != bases.end();)
    {
        os << (*i)->scoped();
        i++;
        if(i != bases.end())
        {
            os << ", ";
        }
    }
    os << "]";

    os << " Members: [";
    DataMemberList members = p->dataMembers();
    for(DataMemberList::const_iterator i = members.begin(); i != members.end();)
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
    DataMemberList members = p->dataMembers();
    for(DataMemberList::const_iterator i = members.begin(); i != members.end();)
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
    DataMemberList members = p->dataMembers();
    for(DataMemberList::const_iterator i = members.begin(); i != members.end();)
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

const string Slice::JavaGenerator::_getSetMetaData = "java:getset";

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
    StringList ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), ptr_fun(lookupKwd));
    stringstream result;
    for(StringList::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
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
    if(!m->findMetaData(prefix, q))
    {
        UnitPtr ut = cont->unit();
        string file = cont->file();
        assert(!file.empty());

        DefinitionContextPtr dc = ut->findDefinitionContext(file);
        assert(dc);
        q = dc->findMetaData(prefix);
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

    if(b && b->kind() == Builtin::KindObject)
    {
        return getUnqualified("com.zeroc.Ice.Object", package) + ".ice_staticId()";
    }
    else if(b && b->kind() == Builtin::KindValue)
    {
        return getUnqualified("com.zeroc.Ice.Value", package) + ".ice_staticId()";
    }
    else
    {
        return getUnqualified(cl, package) + ".ice_staticId()";
    }
}

bool
Slice::JavaGenerator::useOptionalMapping(const OperationPtr& p)
{
    //
    // The "java:optional" metadata can be applied to an operation or its
    // interface to force the mapping to use the Optional types.
    //
    // Without the tag, parameters use the normal (non-optional) mapping.
    //
    static const string tag = "java:optional";

    ClassDefPtr cl = ClassDefPtr::dynamicCast(p->container());
    assert(cl);

    return p->hasMetaData(tag) || cl->hasMetaData(tag);
}

string
Slice::JavaGenerator::getOptionalFormat(const TypePtr& type)
{
    const string prefix = "com.zeroc.Ice.OptionalFormat.";

    BuiltinPtr bp = BuiltinPtr::dynamicCast(type);
    if(bp)
    {
        switch(bp->kind())
        {
        case Builtin::KindByte:
        case Builtin::KindBool:
        {
            return prefix + "F1";
        }
        case Builtin::KindShort:
        {
            return prefix + "F2";
        }
        case Builtin::KindInt:
        case Builtin::KindFloat:
        {
            return prefix + "F4";
        }
        case Builtin::KindLong:
        case Builtin::KindDouble:
        {
            return prefix + "F8";
        }
        case Builtin::KindString:
        {
            return prefix + "VSize";
        }
        case Builtin::KindObject:
        {
            return prefix + "Class";
        }
        case Builtin::KindObjectProxy:
        {
            return prefix + "FSize";
        }
        case Builtin::KindLocalObject:
        {
            assert(false);
            break;
        }
        case Builtin::KindValue:
        {
            return prefix + "Class";
        }
        }
    }

    if(EnumPtr::dynamicCast(type))
    {
        return prefix + "Size";
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        return seq->type()->isVariableLength() ? prefix + "FSize" : prefix + "VSize";
    }

    DictionaryPtr d = DictionaryPtr::dynamicCast(type);
    if(d)
    {
        return (d->keyType()->isVariableLength() || d->valueType()->isVariableLength()) ?
            prefix + "FSize" : prefix + "VSize";
    }

    StructPtr st = StructPtr::dynamicCast(type);
    if(st)
    {
        return st->isVariableLength() ? prefix + "FSize" : prefix + "VSize";
    }

    if(ProxyPtr::dynamicCast(type))
    {
        return prefix + "FSize";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
    assert(cl);
    return prefix + "Class";
}

string
Slice::JavaGenerator::typeToString(const TypePtr& type,
                                   TypeMode mode,
                                   const string& package,
                                   const StringList& metaData,
                                   bool formal,
                                   bool optional) const
{
    static const char* builtinTable[] =
    {
        "byte",
        "boolean",
        "short",
        "int",
        "long",
        "float",
        "double",
        "String",
        "com.zeroc.Ice.Object",
        "com.zeroc.Ice.ObjectPrx",
        "java.lang.Object",
        "com.zeroc.Ice.Value"
    };

    static const char* builtinOptionalTable[] =
    {
        "java.util.Optional<java.lang.Byte>",
        "java.util.Optional<java.lang.Boolean>",
        "java.util.Optional<java.lang.Short>",
        "java.util.OptionalInt",
        "java.util.OptionalLong",
        "java.util.Optional<java.lang.Float>",
        "java.util.OptionalDouble",
        "???",
        "???",
        "???",
        "???",
        "???"
    };

    if(!type)
    {
        assert(mode == TypeModeReturn);
        return "void";
    }

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin)
    {
        if(optional)
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
                    return getUnqualified(builtinOptionalTable[builtin->kind()], package);
                }
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                case Builtin::KindValue:
                {
                    break;
                }
            }
        }
        else
        {
            if(builtin->kind() == Builtin::KindObject)
            {
                return getUnqualified(builtinTable[Builtin::KindValue], package);
            }
            else
            {
                return getUnqualified(builtinTable[builtin->kind()], package);
            }
        }
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);

    if(optional)
    {
        return "java.util.Optional<" + typeToObjectString(type, mode, package, metaData, formal) + ">";
    }

    if(cl)
    {
        if(cl->isInterface())
        {
            return getUnqualified("com.zeroc.Ice.Value", package);
        }
        else
        {
            return getUnqualified(cl, package);
        }
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(type);
    if(proxy)
    {
        ClassDefPtr def = proxy->_class()->definition();
        if(!def || def->isAbstract())
        {
            return getUnqualified(proxy->_class(), package, "", "Prx");
        }
        else
        {
            return getUnqualified("com.zeroc.Ice.ObjectPrx", package);
        }
    }

    DictionaryPtr dict = DictionaryPtr::dynamicCast(type);
    if(dict)
    {
        string instanceType, formalType;
        getDictionaryTypes(dict, package, metaData, instanceType, formalType);
        return formal ? formalType : instanceType;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        string instanceType, formalType;
        getSequenceTypes(seq, package, metaData, instanceType, formalType);
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
Slice::JavaGenerator::typeToObjectString(const TypePtr& type,
                                         TypeMode mode,
                                         const string& package,
                                         const StringList& metaData,
                                         bool formal) const
{
    static const char* builtinTable[] =
    {
        "java.lang.Byte",
        "java.lang.Boolean",
        "java.lang.Short",
        "java.lang.Integer",
        "java.lang.Long",
        "java.lang.Float",
        "java.lang.Double",
        "java.lang.String",
        "com.zeroc.Ice.Value",
        "com.zeroc.Ice.ObjectPrx",
        "java.lang.Object",
        "com.zeroc.Ice.Value"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
    if(builtin && mode != TypeModeOut)
    {
        return builtinTable[builtin->kind()];
    }

    return typeToString(type, mode, package, metaData, formal, false);
}

void
Slice::JavaGenerator::writeMarshalUnmarshalCode(Output& out,
                                                const string& package,
                                                const TypePtr& type,
                                                OptionalMode mode,
                                                bool optionalMapping,
                                                int tag,
                                                const string& param,
                                                bool marshal,
                                                int& iter,
                                                const string& customStream,
                                                const StringList& metaData,
                                                const string& patchParams)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    const bool optionalParam = mode == OptionalInParam || mode == OptionalOutParam || mode == OptionalReturnParam;
    string typeS = typeToString(type, TypeModeIn, package, metaData);

    assert(!marshal || mode != OptionalMember); // Only support OptionalMember for un-marshaling

    static const char* builtinTable[] =
    {
        "Byte",
        "Bool",
        "Short",
        "Int",
        "Long",
        "Float",
        "Double",
        "String",
        "???",
        "???",
        "???"
    };

    const BuiltinPtr builtin = BuiltinPtr::dynamicCast(type);
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
            case Builtin::KindString:
            {
                string s = builtinTable[builtin->kind()];
                if(marshal)
                {
                    if(optionalParam)
                    {
                        out << nl << stream << ".write" << s << "(" << tag << ", " << param << ");";
                    }
                    else
                    {
                        out << nl << stream << ".write" << s << "(" << param << ");";
                    }
                }
                else
                {
                    if(optionalParam)
                    {
                        out << nl << param << " = " << stream << ".read" << s << "(" << tag << ");";
                    }
                    else
                    {
                        out << nl << param << " = " << stream << ".read" << s << "();";
                    }
                }
                break;
            }
            case Builtin::KindObject:
            case Builtin::KindValue:
            {
                if(marshal)
                {
                    if(optionalParam)
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
                    if(optionalParam)
                    {
                        out << nl << stream << ".readValue(" << tag << ", " << patchParams << ");";
                    }
                    else
                    {
                        out << nl << stream << ".readValue(" << patchParams << ");";
                    }
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if(marshal)
                {
                    if(optionalParam)
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
                    if(optionalParam)
                    {
                        out << nl << param << " = " << stream << ".readProxy(" << tag << ");";
                    }
                    else if(mode == OptionalMember)
                    {
                        out << nl << stream << ".skip(4);";
                        out << nl << param << " = " << stream << ".readProxy();";
                    }
                    else
                    {
                        out << nl << param << " = " << stream << ".readProxy();";
                    }
                }
                break;
            }
            case Builtin::KindLocalObject:
            {
                assert(false);
                break;
            }
        }
        return;
    }

    ProxyPtr prx = ProxyPtr::dynamicCast(type);
    if(prx)
    {
        if(marshal)
        {
            if(optionalParam)
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
            if(optionalParam)
            {
                out << nl << param << " = " << stream << ".readProxy(" << tag << ", " << typeS << "::uncheckedCast);";
            }
            else if(mode == OptionalMember)
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
            if(optionalParam)
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
            if(optionalParam)
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
        if(optionalParam || mode == OptionalMember)
        {
            string instanceType, formalType, origInstanceType, origFormalType;
            getDictionaryTypes(dict, "", metaData, instanceType, formalType);
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
                else if(mode != OptionalMember)
                {
                    out << nl << param << " = " << helper << ".read" << spar << stream << tag << epar << ";";
                    return;
                }
            }

            TypePtr keyType = dict->keyType();
            TypePtr valueType = dict->valueType();
            if(marshal)
            {
                if(optionalParam)
                {
                    out << nl;
                    if(optionalMapping)
                    {
                        out << "if(" << param << " != null && " << param << ".isPresent() && " << stream
                            << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                    }
                    else
                    {
                        out << "if(" << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                    }
                    out << sb;
                }

                if(keyType->isVariableLength() || valueType->isVariableLength())
                {
                    string d = optionalParam && optionalMapping ? param + ".get()" : param;
                    out << nl << "int pos = " <<  stream << ".startSize();";
                    writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, customStream, metaData);
                    out << nl << stream << ".endSize(pos);";
                }
                else
                {
                    const size_t sz = keyType->minWireSize() + valueType->minWireSize();
                    string d = optionalParam && optionalMapping ? param + ".get()" : param;
                    out << nl << "final int optSize = " << d << " == null ? 0 : " << d << ".size();";
                    out << nl << stream
                        << ".writeSize(optSize > 254 ? optSize * " << sz << " + 5 : optSize * " << sz << " + 1);";
                    writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, customStream, metaData);
                }

                if(optionalParam)
                {
                    out << eb;
                }
            }
            else
            {
                string d = optionalParam ? "optDict" : param;
                if(optionalParam)
                {
                    out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << "))";
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
                writeDictionaryMarshalUnmarshalCode(out, package, dict, d, marshal, iter, true, customStream, metaData);
                if(optionalParam)
                {
                    out << nl << param << " = java.util.Optional.of(" << d << ");";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << param << " = java.util.Optional.empty();";
                    out << eb;
                }
            }
        }
        else
        {
            writeDictionaryMarshalUnmarshalCode(out, package, dict, param, marshal, iter, true, customStream, metaData);
        }
        return;
    }

    SequencePtr seq = SequencePtr::dynamicCast(type);
    if(seq)
    {
        if(optionalParam || mode == OptionalMember)
        {
            string ignored;
            TypePtr elemType = seq->type();
            BuiltinPtr eltBltin = BuiltinPtr::dynamicCast(elemType);
            if(!hasTypeMetaData(seq, metaData) && eltBltin && eltBltin->kind() < Builtin::KindObject)
            {
                string bs = builtinTable[eltBltin->kind()];
                if(marshal)
                {
                    out << nl << stream << ".write" << bs << "Seq(" << tag << ", " << param << ");";
                    return;
                }
                else if(mode != OptionalMember)
                {
                    out << nl << param << " = " << stream << ".read" << bs << "Seq(" << tag << ");";
                    return;
                }
            }
            else if(findMetaData("java:serializable", seq->getMetaData(), ignored))
            {
                if(marshal)
                {
                    out << nl << stream << ".writeSerializable" << spar << tag << param << epar << ";";
                    return;
                }
                else if(mode != OptionalMember)
                {
                    out << nl << param << " = " << stream << ".readSerializable" << spar << tag << typeS + ".class"
                        << epar << ";";
                    return;
                }
            }
            else if(!hasTypeMetaData(seq, metaData) ||
                    findMetaData("java:type", seq->getMetaData(), ignored) ||
                    findMetaData("java:type", metaData, ignored))
            {
                string instanceType, formalType, origInstanceType, origFormalType;
                getSequenceTypes(seq, "", metaData, instanceType, formalType);
                getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
                if(formalType == origFormalType && (marshal || instanceType == origInstanceType))
                {
                    string helper = getUnqualified(seq, package, "", "Helper");
                    if(marshal)
                    {
                        out << nl << helper << ".write" << spar << stream << tag << param << epar << ";";
                        return;
                    }
                    else if(mode != OptionalMember)
                    {
                        out << nl << param << " = " << helper << ".read" << spar << stream << tag << epar << ";";
                        return;
                    }
                }
            }

            if(marshal)
            {
                if(optionalParam)
                {
                    out << nl;
                    if(optionalMapping)
                    {
                        out << "if(" << param << " != null && " << param << ".isPresent() && " << stream
                            << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                    }
                    else
                    {
                        out << "if(" << stream << ".writeOptional(" << tag << ", " << getOptionalFormat(type) << "))";
                    }
                    out << sb;
                }

                if(elemType->isVariableLength())
                {
                    string s = optionalParam && optionalMapping ? param + ".get()" : param;
                    out << nl << "int pos = " <<  stream << ".startSize();";
                    writeSequenceMarshalUnmarshalCode(out, package, seq, s, true, iter, true, customStream, metaData);
                    out << nl << stream << ".endSize(pos);";
                }
                else
                {
                    const size_t sz = elemType->minWireSize();
                    string s = optionalParam && optionalMapping ? param + ".get()" : param;
                    if(sz > 1)
                    {
                        string ignored2;
                        out << nl << "final int optSize = " << s << " == null ? 0 : ";
                        if(findMetaData("java:buffer", seq->getMetaData(), ignored2) ||
                           findMetaData("java:buffer", metaData, ignored2))
                        {
                            out << s << ".remaining() / " << sz << ";";
                        }
                        else if(hasTypeMetaData(seq, metaData))
                        {
                            out << s << ".size();";
                        }
                        else
                        {
                            out << s << ".length;";
                        }
                        out << nl << stream << ".writeSize(optSize > 254 ? optSize * " << sz
                            << " + 5 : optSize * " << sz << " + 1);";
                    }
                    writeSequenceMarshalUnmarshalCode(out, package, seq, s, true, iter, true, customStream, metaData);
                }

                if(optionalParam)
                {
                    out << eb;
                }
            }
            else
            {
                const size_t sz = elemType->minWireSize();
                string s = optionalParam ? "optSeq" : param;
                if(optionalParam)
                {
                    out << nl << "if(" << stream << ".readOptional(" << tag << ", " << getOptionalFormat(type) << "))";
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
                writeSequenceMarshalUnmarshalCode(out, package, seq, s, false, iter, true, customStream, metaData);
                if(optionalParam)
                {
                    out << nl << param << " = java.util.Optional.of(" << s << ");";
                    out << eb;
                    out << nl << "else";
                    out << sb;
                    out << nl << param << " = java.util.Optional.empty();";
                    out << eb;
                }
            }
        }
        else
        {
            writeSequenceMarshalUnmarshalCode(out, package, seq, param, marshal, iter, true, customStream, metaData);
        }
        return;
    }

    ConstructedPtr constructed = ConstructedPtr::dynamicCast(type);
    StructPtr st = StructPtr::dynamicCast(type);
    assert(constructed);
    if(marshal)
    {
        if(optionalParam)
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
        if(optionalParam)
        {
            out << nl << param << " = " << typeS << ".ice_read(" << stream << ", " << tag << ");";
        }
        else if(mode == OptionalMember && st)
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
                                                          const StringList& metaData)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    string v = param;

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
    getDictionaryTypes(dict, "", metaData, instanceType, formalType);
    getDictionaryTypes(dict, "", StringList(), origInstanceType, origFormalType);
    if(useHelper && formalType == origFormalType && (marshal || instanceType == origInstanceType))
    {
        //
        // If we can use the helper, it's easy.
        //
        string helper = getUnqualified(dict, package, "", "Helper");
        if(marshal)
        {
            out << nl << helper << ".write" << spar << stream << v << epar << ";";
        }
        else
        {
            out << nl << v << " = " << helper << ".read" << spar << stream << epar << ";";
        }
        return;
    }

    TypePtr key = dict->keyType();
    TypePtr value = dict->valueType();

    string keyS = typeToString(key, TypeModeIn, package);
    string valueS = typeToString(value, TypeModeIn, package);

    ostringstream o;
    o << iter;
    string iterS = o.str();
    iter++;

    if(marshal)
    {
        out << nl << "if(" << v << " == null)";
        out << sb;
        out << nl << "ostr.writeSize(0);";
        out << eb;
        out << nl << "else";
        out << sb;
        out << nl << "ostr.writeSize(" << v << ".size());";
        string keyObjectS = typeToObjectString(key, TypeModeIn, package);
        string valueObjectS = typeToObjectString(value, TypeModeIn, package);
        out << nl;
        out << "for(java.util.Map.Entry<" << keyObjectS << ", " << valueObjectS << "> e : " << v << ".entrySet())";
        out << sb;
        for(int i = 0; i < 2; i++)
        {
            string arg;
            TypePtr type;
            if(i == 0)
            {
                arg = "e.getKey()";
                type = key;
            }
            else
            {
                arg = "e.getValue()";
                type = value;
            }
            writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, arg, true, iter, customStream);
        }
        out << eb;
        out << eb;
    }
    else
    {
        out << nl << v << " = new " << instanceType << "();";
        out << nl << "int sz" << iterS << " = " << stream << ".readSize();";
        out << nl << "for(int i" << iterS << " = 0; i" << iterS << " < sz" << iterS << "; i" << iterS << "++)";
        out << sb;

        BuiltinPtr b = BuiltinPtr::dynamicCast(value);
        if(ClassDeclPtr::dynamicCast(value) || (b && b->usesClasses()))
        {
            out << nl << "final " << keyS << " key;";
            writeMarshalUnmarshalCode(out, package, key, OptionalNone, false, 0, "key", false, iter, customStream);

            valueS = typeToObjectString(value, TypeModeIn, package);
            ostringstream patchParams;
            patchParams << "value -> " << v << ".put(key, value), " << valueS << ".class";
            writeMarshalUnmarshalCode(out, package, value, OptionalNone, false, 0, "value", false, iter, customStream,
                                      StringList(), patchParams.str());
        }
        else
        {
            out << nl << keyS << " key;";
            writeMarshalUnmarshalCode(out, package, key, OptionalNone, false, 0, "key", false, iter, customStream);

            out << nl << valueS << " value;";
            writeMarshalUnmarshalCode(out, package, value, OptionalNone, false, 0, "value", false, iter, customStream);

            BuiltinPtr builtin = BuiltinPtr::dynamicCast(value);
            if(!(builtin && builtin->usesClasses()) && !ClassDeclPtr::dynamicCast(value))
            {
                out << nl << "" << v << ".put(key, value);";
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
                                                        const StringList& metaData)
{
    string stream = customStream;
    if(stream.empty())
    {
        stream = marshal ? "ostr" : "istr";
    }

    string typeS = typeToString(seq, TypeModeIn, package);
    string v = param;

    //
    // If the sequence is a byte sequence, check if there's the serializable or protobuf metadata to
    // get rid of these two easy cases first.
    //
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
    if(builtin && builtin->kind() == Builtin::KindByte)
    {
        string meta;
        static const string protobuf = "java:protobuf:";
        static const string serializable = "java:serializable:";
        if(seq->findMetaData(serializable, meta))
        {
            if(marshal)
            {
                out << nl << stream << ".writeSerializable(" << v << ");";
            }
            else
            {
                out << nl << v << " = " << stream << ".readSerializable(" << typeS << ".class);";
            }
            return;
        }
        else if(seq->findMetaData(protobuf, meta))
        {
            if(marshal)
            {
                out << nl << "if(!" << v << ".isInitialized())";
                out << sb;
                out << nl << "throw new com.zeroc.Ice.MarshalException(\"type not fully initialized\");";
                out << eb;
                out << nl << stream << ".writeByteSeq(" << v << ".toByteArray());";
            }
            else
            {
                string type = typeToString(seq, TypeModeIn, package);
                out << nl << "try";
                out << sb;
                out << nl << v << " = " << typeS << ".parseFrom(" << stream << ".readByteSeq());";
                out << eb;
                out << nl << "catch(com.google.protobuf.InvalidProtocolBufferException ex)";
                out << sb;
                out << nl << "throw new com.zeroc.Ice.MarshalException(ex);";
                out << eb;
            }
            return;
        }
    }

    static const char* builtinTable[] =
    {
        "Byte",
        "Bool",
        "Short",
        "Int",
        "Long",
        "Float",
        "Double",
        "String"
    };

    if(builtin &&
       (builtin->kind() == Builtin::KindByte || builtin->kind() == Builtin::KindShort ||
        builtin->kind() == Builtin::KindInt || builtin->kind() == Builtin::KindLong ||
        builtin->kind() == Builtin::KindFloat || builtin->kind() == Builtin::KindDouble))
    {
        string meta;
        static const string bytebuffer = "java:buffer";
        if(seq->findMetaData(bytebuffer, meta) || findMetaData(bytebuffer, metaData, meta))
        {
            if(marshal)
            {
                out << nl << stream << ".write" << builtinTable[builtin->kind()] << "Buffer(" << v << ");";
            }
            else
            {
                out << nl << v << " = " << stream << ".read" << builtinTable[builtin->kind()] << "Buffer();";
            }
            return;
        }
    }

    if(!hasTypeMetaData(seq, metaData) && builtin && builtin->kind() <= Builtin::KindString)
    {
        if(marshal)
        {
            out << nl << stream << ".write" << builtinTable[builtin->kind()] << "Seq(" << v << ");";
        }
        else
        {
            out << nl << v << " = " << stream << ".read" << builtinTable[builtin->kind()] << "Seq();";
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
    bool customType = getSequenceTypes(seq, "", metaData, instanceType, formalType);
    getSequenceTypes(seq, "", StringList(), origInstanceType, origFormalType);
    if(useHelper && formalType == origFormalType && (marshal || instanceType == origInstanceType))
    {
        //
        // If we can use the helper, it's easy.
        //
        string helper = getUnqualified(seq, package, "", "Helper");
        if(marshal)
        {
            out << nl << helper << ".write" << spar << stream << v << epar << ";";
        }
        else
        {
            out << nl << v << " = " << helper << ".read" << spar << stream << epar << ";";
        }
        return;
    }

    //
    // Determine sequence depth.
    //
    int depth = 0;
    TypePtr origContent = seq->type();
    SequencePtr s = SequencePtr::dynamicCast(origContent);
    while(s)
    {
        //
        // Stop if the inner sequence type has a custom, serializable or protobuf type.
        //
        if(hasTypeMetaData(s))
        {
            break;
        }
        depth++;
        origContent = s->type();
        s = SequencePtr::dynamicCast(origContent);
    }
    string origContentS = typeToString(origContent, TypeModeIn, package);

    TypePtr type = seq->type();

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
            out << nl << "if(" << v << " == null)";
            out << sb;
            out << nl << stream << ".writeSize(0);";
            out << eb;
            out << nl << "else";
            out << sb;
            out << nl << stream << ".writeSize(" << v << ".size());";
            string ctypeS = typeToString(type, TypeModeIn, package);
            out << nl << "for(" << ctypeS << " elem : " << v << ')';
            out << sb;
            writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, "elem", true, iter, customStream);
            out << eb;
            out << eb; // else
        }
        else
        {
            bool isObject = false;
            ClassDeclPtr cl = ClassDeclPtr::dynamicCast(type);
            if((b && b->usesClasses()) || cl)
            {
                isObject = true;
            }
            out << nl << v << " = new " << instanceType << "();";
            out << nl << "final int len" << iter << " = " << stream << ".readAndCheckSeqSize(" << type->minWireSize()
                << ");";
            out << nl << "for(int i" << iter << " = 0; i" << iter << " < len" << iter << "; i" << iter << "++)";
            out << sb;
            if(isObject)
            {
                //
                // Add a null value to the list as a placeholder for the element.
                //
                out << nl << v << ".add(null);";
                ostringstream patchParams;
                out << nl << "final int fi" << iter << " = i" << iter << ";";
                patchParams << "value -> " << v << ".set(fi" << iter << ", value), " << origContentS << ".class";

                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, "elem", false, iter,
                                          customStream, StringList(), patchParams.str());
            }
            else
            {
                out << nl << cont << " elem;";
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, "elem", false, iter, customStream);
                out << nl << v << ".add(elem);";
            }
            out << eb;
            iter++;
        }
    }
    else
    {
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b && b->kind() != Builtin::KindObject &&
                b->kind() != Builtin::KindValue &&
                b->kind() != Builtin::KindObjectProxy)
        {
            switch(b->kind())
            {
                case Builtin::KindByte:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeByteSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readByteSeq();";
                    }
                    break;
                }
                case Builtin::KindBool:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeBoolSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readBoolSeq();";
                    }
                    break;
                }
                case Builtin::KindShort:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeShortSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readShortSeq();";
                    }
                    break;
                }
                case Builtin::KindInt:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeIntSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readIntSeq();";
                    }
                    break;
                }
                case Builtin::KindLong:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeLongSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readLongSeq();";
                    }
                    break;
                }
                case Builtin::KindFloat:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeFloatSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readFloatSeq();";
                    }
                    break;
                }
                case Builtin::KindDouble:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeDoubleSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readDoubleSeq();";
                    }
                    break;
                }
                case Builtin::KindString:
                {
                    if(marshal)
                    {
                        out << nl << stream << ".writeStringSeq(" << v << ");";
                    }
                    else
                    {
                        out << nl << v << " = " << stream << ".readStringSeq();";
                    }
                    break;
                }
                case Builtin::KindValue:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    assert(false);
                    break;
                }
            }
        }
        else
        {
            if(marshal)
            {
                out << nl << "if(" << v << " == null)";
                out << sb;
                out << nl << stream << ".writeSize(0);";
                out << eb;
                out << nl << "else";
                out << sb;
                out << nl << stream << ".writeSize(" << v << ".length);";
                out << nl << "for(int i" << iter << " = 0; i" << iter << " < " << v << ".length; i" << iter
                    << "++)";
                out << sb;
                ostringstream o;
                o << v << "[i" << iter << "]";
                iter++;
                writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, o.str(), true, iter, customStream);
                out << eb;
                out << eb;
            }
            else
            {
                bool isObject = false;
                ClassDeclPtr cl = ClassDeclPtr::dynamicCast(origContent);
                if((b && b->usesClasses()) || cl)
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
                    out << nl << v << " = (" << origContentS << "[]";
                    int d = depth;
                    while(d--)
                    {
                        out << "[]";
                    }
                    out << ")new " << nonGenericType << "[len" << iter << "]";
                }
                else
                {
                    out << nl << v << " = new " << origContentS << "[len" << iter << "]";
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
                o << v << "[i" << iter << "]";
                if(isObject)
                {
                    ostringstream patchParams;
                    out << nl << "final int fi" << iter << " = i" << iter << ";";
                    patchParams << "value -> " << v << "[fi" << iter << "] = value, " << origContentS << ".class";
                    writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, o.str(), false, iter,
                                              customStream, StringList(), patchParams.str());
                }
                else
                {
                    writeMarshalUnmarshalCode(out, package, type, OptionalNone, false, 0, o.str(), false, iter,
                                              customStream);
                }
                out << eb;
                iter++;
            }
        }
    }
}

bool
Slice::JavaGenerator::findMetaData(const string& prefix, const StringList& metaData, string& value)
{
    for(StringList::const_iterator q = metaData.begin(); q != metaData.end(); ++q)
    {
        if(q->find(prefix) == 0)
        {
            value = *q;
            return true;
        }
    }

    return false;
}

bool
Slice::JavaGenerator::getTypeMetaData(const StringList& metaData, string& instanceType, string& formalType)
{
    //
    // Extract the instance type and an optional formal type.
    // The correct syntax is "java:type:instance-type[:formal-type]".
    //
    static const string prefix = "java:type:";
    string directive;
    if(findMetaData(prefix, metaData, directive))
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
Slice::JavaGenerator::hasTypeMetaData(const TypePtr& type, const StringList& localMetaData)
{
    ContainedPtr cont = ContainedPtr::dynamicCast(type);
    if(cont)
    {
        static const string prefix = "java:type:";
        string directive;

        if(findMetaData(prefix, localMetaData, directive))
        {
            return true;
        }

        StringList metaData = cont->getMetaData();

        if(findMetaData(prefix, metaData, directive))
        {
            return true;
        }

        if(findMetaData("java:protobuf:", metaData, directive) ||
           findMetaData("java:serializable:", metaData, directive))
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

        if(findMetaData("java:buffer", metaData, directive) ||
           findMetaData("java:buffer", localMetaData, directive))
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
                                         const StringList& metaData,
                                         string& instanceType,
                                         string& formalType) const
{
    //
    // Get the types of the key and value.
    //
    string keyTypeStr = typeToObjectString(dict->keyType(), TypeModeIn, package, StringList(), true);
    string valueTypeStr = typeToObjectString(dict->valueType(), TypeModeIn, package, StringList(), true);

    //
    // Collect metadata for a custom type.
    //
    if(getTypeMetaData(metaData, instanceType, formalType) ||
       getTypeMetaData(dict->getMetaData(), instanceType, formalType))
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
                                       const StringList& metaData,
                                       string& instanceType,
                                       string& formalType) const
{
    BuiltinPtr builtin = BuiltinPtr::dynamicCast(seq->type());
    if(builtin)
    {
        if(builtin->kind() == Builtin::KindByte)
        {
            string prefix = "java:serializable:";
            string meta;
            if(seq->findMetaData(prefix, meta))
            {
                instanceType = formalType = meta.substr(prefix.size());
                return true;
            }
            prefix = "java:protobuf:";
            if(seq->findMetaData(prefix, meta))
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
            if(seq->findMetaData(prefix, meta) || findMetaData(prefix, metaData, ignored))
            {
                instanceType = formalType = typeToBufferString(seq->type());
                return true;
            }
        }
    }

    //
    // Collect metadata for a custom type.
    //
    if(getTypeMetaData(metaData, instanceType, formalType) ||
       getTypeMetaData(seq->getMetaData(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if(formalType.empty())
        {
            formalType = "java.util.List<" + typeToObjectString(seq->type(), TypeModeIn, package, StringList(), true) +
                ">";
        }
        return true;
    }

    //
    // The default mapping is a native array.
    //
    instanceType = formalType = typeToString(seq->type(), TypeModeIn, package, metaData, true, false) + "[]";
    return false;
}

JavaOutput*
Slice::JavaGenerator::createOutput()
{
    return new JavaOutput;
}

void
Slice::JavaGenerator::validateMetaData(const UnitPtr& u)
{
    MetaDataVisitor visitor;
    u->visit(&visitor, true);
}
