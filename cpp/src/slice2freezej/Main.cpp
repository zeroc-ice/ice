// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Slice/Preprocessor.h>
#include <Slice/JavaUtil.h>

using namespace std;
using namespace Slice;
using namespace IceUtil;

struct Dict
{
    string name;
    string key;
    string value;
};

class FreezeGenerator : public JavaGenerator
{
public:
    FreezeGenerator(const string&, bool, const string&);

    bool generate(UnitPtr&, const Dict&);

private:
    string _prog;
    bool _binary;
};

FreezeGenerator::FreezeGenerator(const string& prog, bool binary, const string& dir)
    : JavaGenerator(dir, string()),
      _prog(prog),
      _binary(binary)
{
}

bool
FreezeGenerator::generate(UnitPtr& u, const Dict& dict)
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
        "Ice.Object",
        "Ice.ObjectPrx",
        "Ice.LocalObject"
    };

    string name;
    string::size_type pos = dict.name.rfind('.');
    if(pos == string::npos)
    {
        name = dict.name;
    }
    else
    {
        name = dict.name.substr(pos + 1);
    }

    TypeList keyTypes = u->lookupType(dict.key, false);
    if(keyTypes.empty())
    {
        cerr << _prog << ": `" << dict.key << "' is not a valid type" << endl;
        return false;
    }
    TypePtr keyType = keyTypes.front();
    
    TypeList valueTypes = u->lookupType(dict.value, false);
    if(valueTypes.empty())
    {
        cerr << _prog << ": `" << dict.value << "' is not a valid type" << endl;
        return false;
    }
    TypePtr valueType = valueTypes.front();

    if(!open(dict.name))
    {
        cerr << _prog << ": unable to open class " << dict.name << endl;
        return false;
    }

    Output& out = output();

    out << sp << nl << "public class " << name << " extends Freeze.Map";
    out << sb;

    //
    // Constructors
    //
    out << sp << nl << "public" << nl << name << "(Ice.Communicator communicator, String envName, String dbName, boolean createDb)";
    out << sb;
    out << nl << "super(communicator, envName, dbName, createDb);";
    out << eb;

    out << sp << nl << "public" << nl << name << "(Ice.Communicator communicator, com.sleepycat.db.DbEnv dbEnv, String dbName, boolean createDb)";
    out << sb;
    out << nl << "super(communicator, dbEnv, dbName, createDb);";
    out << eb;

    //
    // encode/decode
    //
    for(int i = 0; i < 2; i++)
    {
        string keyValue;
        TypePtr type;

        if(i == 0)
        {
            keyValue = "Key";
            type = keyType;
        }
        else
        {
            keyValue = "Value";
            type = valueType;
        }

        string typeS, valS;
        BuiltinPtr b = BuiltinPtr::dynamicCast(type);
        if(b)
        {
            typeS = builtinTable[b->kind()];
            switch(b->kind())
            {
                case Builtin::KindByte:
                {
                    valS = "((java.lang.Byte)o).byteValue()";
                    break;
                }
                case Builtin::KindBool:
                {
                    valS = "((java.lang.Boolean)o).booleanValue()";
                    break;
                }
                case Builtin::KindShort:
                {
                    valS = "((java.lang.Short)o).shortValue()";
                    break;
                }
                case Builtin::KindInt:
                {
                    valS = "((java.lang.Integer)o).intValue()";
                    break;
                }
                case Builtin::KindLong:
                {
                    valS = "((java.lang.Long)o).longValue()";
                    break;
                }
                case Builtin::KindFloat:
                {
                    valS = "((java.lang.Float)o).floatValue()";
                    break;
                }
                case Builtin::KindDouble:
                {
                    valS = "((java.lang.Double)o).doubleValue()";
                    break;
                }
                case Builtin::KindString:
                case Builtin::KindObject:
                case Builtin::KindObjectProxy:
                case Builtin::KindLocalObject:
                {
                    valS = "((" + typeS + ")o)";
                    break;
                }
            }
        }
        else
        {
            typeS = typeToString(type, TypeModeIn);
            valS = "((" + typeS + ")o)";
        }

        int iter;

        //
        // encode
        //
        out << sp << nl << "public static byte[]" << nl << "encode" << keyValue
            << "Impl(Object o, Ice.Communicator communicator)";
        out << sb;
        out << nl << "assert(o instanceof " << typeS << ");";
        if(_binary)
        {
            out << nl << "IceInternal.BasicStream __os = "
                << "new IceInternal.BasicStream(Ice.Util.getInstance(communicator));";
            out << nl << "try";
            out << sb;
            iter = 0;
            writeMarshalUnmarshalCode(out, "", type, valS, true, iter, false);
            out << nl << "java.nio.ByteBuffer __buf = __os.prepareWrite();";
            out << nl << "byte[] __r = new byte[__buf.limit()];";
            out << nl << "__buf.get(__r);";
            out << nl << "return __r;";
            out << eb;
            out << nl << "finally";
            out << sb;
            out << nl << "__os.destroy();";
            out << eb;
        }
        else
        {
            out << nl << "try";
            out << sb;
            out << nl << "java.io.StringWriter sw = new java.io.StringWriter();";
            out << nl << "java.io.PrintWriter pw = new java.io.PrintWriter(sw);";
            out << nl << "Ice.Stream __os = new IceXML.StreamI(communicator, pw);";
            string quotedKeyValue = "\"" + keyValue + "\"";
            iter = 0;
            writeGenericMarshalUnmarshalCode(out, "", type, quotedKeyValue, valS, true, iter, false);
            out << nl << "return sw.toString().getBytes(\"UTF8\");";
            out << eb;
            out << nl << "catch(java.io.UnsupportedEncodingException ex)";
            out << sb;
            out << nl << "Ice.SyscallException e = new Ice.SyscallException();";
            out << nl << "e.initCause(ex);";
            out << nl << "throw e;";
            out << eb;
        }
        out << eb;

	out << sp << nl << "public byte[]" << nl << "encode" << keyValue
            << "(Object o, Ice.Communicator communicator)";
        out << sb;
        out << nl << "return encode" << keyValue << "Impl(o, communicator);";
	out << eb;

        //
        // decode
        //
        out << sp << nl << "public static Object" << nl << "decode" << keyValue
            << "Impl(byte[] b, Ice.Communicator communicator)";
        out << sb;
        if(_binary)
        {
            out << nl << "IceInternal.BasicStream __is = "
                << "new IceInternal.BasicStream(Ice.Util.getInstance(communicator));";
            out << nl << "try";
            out << sb;
            out << nl << "__is.resize(b.length, true);";
            out << nl << "java.nio.ByteBuffer __buf = __is.prepareRead();";
            out << nl << "__buf.position(0);";
            out << nl << "__buf.put(b);";
            out << nl << "__buf.position(0);";
            iter = 0;
            out << nl << typeS << " __r;";
            if(b)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "__r = new java.lang.Byte(__is.readByte());";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "__r = new java.lang.Boolean(__is.readBool());";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "__r = new java.lang.Short(__is.readShort());";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "__r = new java.lang.Integer(__is.readInt());";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "__r = new java.lang.Long(__is.readLong());";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "__r = new java.lang.Float(__is.readFloat());";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "__r = new java.lang.Double(__is.readDouble());";
                        break;
                    }
                    case Builtin::KindString:
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        writeMarshalUnmarshalCode(out, "", type, "__r", false, iter, false);
                        break;
                    }
                }
            }
            else
            {
                writeMarshalUnmarshalCode(out, "", type, "__r", false, iter, false);
            }
            out << nl << "return __r;";
            out << eb;
            out << nl << "finally";
            out << sb;
            out << nl << "__is.destroy();";
            out << eb;
        }
        else
        {
            out << nl << "try";
            out << sb;
            out << nl << "final String data = _header + new String(b, \"UTF8\") + _footer;";
            out << nl << "Ice.Stream __is = new IceXML.StreamI(communicator, new java.io.StringReader(data));";
            string quotedKeyValue = "\"" + keyValue + "\"";
            iter = 0;
            out << nl << typeS << " __r;";
            if(b)
            {
                switch(b->kind())
                {
                    case Builtin::KindByte:
                    {
                        out << nl << "__r = new java.lang.Byte(__is.readByte(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindBool:
                    {
                        out << nl << "__r = new java.lang.Boolean(__is.readBool(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindShort:
                    {
                        out << nl << "__r = new java.lang.Short(__is.readShort(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindInt:
                    {
                        out << nl << "__r = new java.lang.Integer(__is.readInt(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindLong:
                    {
                        out << nl << "__r = new java.lang.Long(__is.readLong(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindFloat:
                    {
                        out << nl << "__r = new java.lang.Float(__is.readFloat(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindDouble:
                    {
                        out << nl << "__r = new java.lang.Double(__is.readDouble(" << quotedKeyValue << "));";
                        break;
                    }
                    case Builtin::KindString:
                    case Builtin::KindObject:
                    case Builtin::KindObjectProxy:
                    case Builtin::KindLocalObject:
                    {
                        writeGenericMarshalUnmarshalCode(out, "", type, quotedKeyValue, "__r", false, iter, false);
                        break;
                    }
                }
            }
            else
            {
                writeGenericMarshalUnmarshalCode(out, "", type, quotedKeyValue, "__r", false, iter, false);
            }
            out << nl << "return __r;";
            out << eb;
            out << nl << "catch(java.io.UnsupportedEncodingException ex)";
            out << sb;
            out << nl << "Ice.SyscallException e = new Ice.SyscallException();";
            out << nl << "e.initCause(ex);";
            out << nl << "throw e;";
            out << eb;
        }
        out << eb;

	out << sp << nl << "public Object" << nl << "decode" << keyValue
            << "(byte[] b, Ice.Communicator communicator)";
        out << sb;
        out << nl << "return decode" << keyValue << "Impl(b, communicator);";
	out << eb;
    }

    if(!_binary)
    {
        out << sp << nl << "private static final String _header = ";
        out.useCurrentPosAsIndent();
        out << "\"<ice:data xmlns=\\\"http://www.noorg.org/schemas\\\"\" +"
            << nl << "\" xmlns:ice=\\\"http://www.zeroc.com/schemas\\\"\" +"
            << nl << "\" xmlns:xsi=\\\"http://www.w3.org/2003/XMLSchema-instance\\\"\" +"
            << nl << "\" xsi:schemaLocation=\\\"http://www.noorg.org/schemas " << name << ".xsd\\\">\";";
        out.restoreIndent();
        out << nl << "private static final String _footer = \"</ice:data>\";";
    }

    out << eb;

    close();

    return true;
}

void
usage(const char* n)
{
    cerr << "Usage: " << n << " [options] [slice-files...]\n";
    cerr <<
        "Options:\n"
        "-h, --help            Show this message.\n"
        "-v, --version         Display the Ice version.\n"
        "-DNAME                Define NAME as 1.\n"
        "-DNAME=DEF            Define NAME as DEF.\n"
        "-UNAME                Remove any definition for NAME.\n"
        "-IDIR                 Put DIR in the include file search path.\n"
        "--include-dir DIR     Use DIR as the header include directory.\n"
        "--dict NAME,KEY,VALUE Create a Freeze dictionary with the name NAME,\n"
        "                      using KEY as key, and VALUE as value. This\n"
        "                      option may be specified multiple times for\n"
        "                      different names. NAME may be a scoped name.\n"
	"--binary              Use the binary encoding.\n"
        "--output-dir DIR      Create files in the directory DIR.\n"
	"--depend              Generate Makefile dependencies.\n"
        "-d, --debug           Print debug messages.\n"
        "--ice                 Permit `Ice' prefix (for building Ice source code only)\n"
        ;
    // Note: --case-sensitive is intentionally not shown here!
}

int
main(int argc, char* argv[])
{
    string cppArgs;
    vector<string> includePaths;
    string include;
    bool binary = false;
    string output;
    bool depend = false;
    bool debug = false;
    bool ice = false;
    bool caseSensitive = false;
    vector<Dict> dicts;

    int idx = 1;
    while(idx < argc)
    {
        if(strncmp(argv[idx], "-I", 2) == 0)
        {
            cppArgs += ' ';
            cppArgs += argv[idx];

            string path = argv[idx] + 2;
            if(path.length())
            {
                includePaths.push_back(path);
            }

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strncmp(argv[idx], "-D", 2) == 0 || strncmp(argv[idx], "-U", 2) == 0)
        {
            cppArgs += ' ';
            cppArgs += argv[idx];

            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
        else if(strcmp(argv[idx], "--dict") == 0)
        {
            if(idx + 1 >= argc || argv[idx + 1][0] == '-')
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            string s = argv[idx + 1];
            s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
            
            Dict dict;

            string::size_type pos;
            pos = s.find(',');
            if(pos != string::npos)
            {
                dict.name = s.substr(0, pos);
                s.erase(0, pos + 1);
            }
            pos = s.find(',');
            if(pos != string::npos)
            {
                dict.key = s.substr(0, pos);
                s.erase(0, pos + 1);
            }
            dict.value = s;

            if(dict.name.empty())
            {
                cerr << argv[0] << ": " << argv[idx] << ": no name specified" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            if(dict.key.empty())
            {
                cerr << argv[0] << ": " << argv[idx] << ": no key specified" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            if(dict.value.empty())
            {
                cerr << argv[0] << ": " << argv[idx] << ": no value specified" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }

            dicts.push_back(dict);

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "-h") == 0 || strcmp(argv[idx], "--help") == 0)
        {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(strcmp(argv[idx], "-v") == 0 || strcmp(argv[idx], "--version") == 0)
        {
            cout << ICE_STRING_VERSION << endl;
            return EXIT_SUCCESS;
        }
	else if(strcmp(argv[idx], "--depend") == 0)
	{
	    depend = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
        else if(strcmp(argv[idx], "-d") == 0 || strcmp(argv[idx], "--debug") == 0)
        {
            debug = true;
            for(int i = idx ; i + 1 < argc ; ++i)
            {
                argv[i] = argv[i + 1];
            }
            --argc;
        }
	else if(strcmp(argv[idx], "--ice") == 0)
	{
	    ice = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
	else if(strcmp(argv[idx], "--case-sensitive") == 0)
	{
	    caseSensitive = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
        else if(strcmp(argv[idx], "--include-dir") == 0)
        {
            if(idx + 1 >= argc)
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            
            include = argv[idx + 1];
            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
	else if(strcmp(argv[idx], "--binary") == 0)
	{
	    binary = true;
	    for(int i = idx ; i + 1 < argc ; ++i)
	    {
		argv[i] = argv[i + 1];
	    }
	    --argc;
	}
        else if(strcmp(argv[idx], "--output-dir") == 0)
        {
            if(idx + 1 >= argc)
            {
                cerr << argv[0] << ": argument expected for `" << argv[idx] << "'" << endl;
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            
            output = argv[idx + 1];
            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(argv[idx][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[idx] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        else
        {
            ++idx;
        }
    }

    if(dicts.empty())
    {
        cerr << argv[0] << ": no Freeze types specified" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    UnitPtr u = Unit::createUnit(true, false, ice, caseSensitive);

    int status = EXIT_SUCCESS;

    for(idx = 1 ; idx < argc ; ++idx)
    {
	if(depend)
	{
	    Preprocessor icecpp(argv[0], argv[idx], cppArgs);
	    icecpp.printMakefileDependencies();
	}
	else
	{
	    Preprocessor icecpp(argv[0], argv[idx], cppArgs);
	    FILE* cppHandle = icecpp.preprocess(false);

	    if(cppHandle == 0)
	    {
		u->destroy();
		return EXIT_FAILURE;
	    }
	    
	    status = u->parse(cppHandle, debug);

	    if(!icecpp.close())
	    {
		u->destroy();
		return EXIT_FAILURE;
	    }	    
	}
    }

    if(depend)
    {
	return EXIT_SUCCESS;
    }

    if(status == EXIT_SUCCESS)
    {
        u->mergeModules();
        u->sort();

        FreezeGenerator gen(argv[0], binary, output);

        for(vector<Dict>::const_iterator p = dicts.begin(); p != dicts.end(); ++p)
        {
            try
            {
                if(!gen.generate(u, *p))
                {
                    u->destroy();
                    return EXIT_FAILURE;
                }
            }
            catch(...)
            {
                cerr << argv[0] << ": unknown exception" << endl;
                u->destroy();
                return EXIT_FAILURE;
            }
        }
    }
    
    u->destroy();

    return status;
}
