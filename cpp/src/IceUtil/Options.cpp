// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <iostream>

using namespace std;

IceUtil::Options::Options()
    : parseCalled(false)
{
}

void
IceUtil::Options::checkArgs(const string& shortOpt, const string& longOpt, bool needArg, const string& dflt)
{
    if(shortOpt.empty() && longOpt.empty())
    {
        throw APIError("short and long option cannot both be empty");
    }

    if(!shortOpt.empty())
    {
	if(shortOpt.size() != 1)
	{
	    string err = "`";
	    err += shortOpt;
	    err += "': a short option cannot specify more than one option";
	    throw APIError(err);
	}
	if(shortOpt.find_first_of(" \t\n\r\f\v") != string::npos)
	{
	    string err = "`";
	    err += shortOpt;
	    err += "': a short option cannot be whitespace";
	    throw APIError(err);
	}
	if(shortOpt[0] == '-')
	{
	    string err = "`";
	    err += shortOpt;
	    err += "': a short option cannot be `-'";
	    throw APIError(err);
	}
    }

    if(!longOpt.empty())
    {
	if(longOpt.find_first_of(" \t\n\r\f\v") != string::npos)
	{
	    string err = "`";
	    err += longOpt;
	    err += "': a long option cannot contain whitespace";
	    throw APIError(err);
	}
	if(longOpt[0] == '-')
	{
	    string err = "`";
	    err += longOpt;
	    err += "': a long option must not contain a leading `-'";
	    throw APIError(err);
	}
    }

    if(!needArg && !dflt.empty())
    {
	throw APIError("a default value can be specified only for options requiring an argument");
    }
}

void
IceUtil::Options::addOpt(const string& shortOpt, const string& longOpt, ArgType at, string dflt, RepeatType rt)
{
    IceUtil::RecMutex::Lock sync(_m);
    
    if(parseCalled)
    {
	throw APIError("cannot add options after parse() was called");
    }

    checkArgs(shortOpt, longOpt, at == NeedArg, dflt);

    addValidOpt(shortOpt, ShortOpt, at, dflt, rt);
    addValidOpt(longOpt, LongOpt, at, dflt, rt);
}

vector<string>
IceUtil::Options::parse(const string& line)
{
    const string IFS = " \t\n"; // Internal Field Separator.

    //
    // Strip leading whitespace.
    //
    string::size_type start = line.find_first_not_of(IFS);
    if(start == string::npos)
    {
        return vector<string>();
    }
    string l(line, start);

    vector<string> vec;
    vec.push_back(""); // Push dummy argv[0].

    enum ParseState { Normal, DoubleQuote, SingleQuote, ANSIQuote };
    ParseState state = Normal;

    string arg;

    for(string::size_type i = 0; i < l.size(); ++i)
    {
	char c = l[i];
        switch(state)
	{
	    case Normal:
	    {
		switch(c)
		{
		    case '\\':
		    {
			//
			// Ignore a backslash at the end of the string,
			// and strip backslash-newline pairs. Anything
			// else following a backslash is taken literally.
			//
			if(i < l.size() - 1 && l[++i] != '\n')
			{
			    arg.push_back(l[i]);
			}
			break;
		    }
		    case '\'':
		    {
			state = SingleQuote;
			break;
		    }
		    case '"':
		    {
			state = DoubleQuote;
			break;
		    }
		    case '$':
		    {
			if(i < l.size() - 1 && l[i + 1] == '\'')
			{
			    state = ANSIQuote; // Bash uses $'<text>' to allow ANSI escape sequences within <text>.
			    ++i;
			}
			else
			{
			    arg.push_back(l[i]);
			}
			break;
		    }
		    default:
		    {
			if(IFS.find(l[i]) != string::npos)
			{
			    //
			    // We just saw the end of an argument.
			    // Push it onto the end of the argument
			    // vector and eat up any trailing consecutive
			    // whitespace characters.
			    //
			    vec.push_back(arg);
			    arg.clear();
			    while(++i < l.size() && IFS.find(l[i]) != string::npos)
			    {
				;
			    }
			    --i;
			}
			else
			{
			    arg.push_back(l[i]);
			}
			break;
		    }
		}
		break;
	    }
	    case DoubleQuote:
	    {
	        //
		// Within double quotes, only backslash retains its special
		// meaning, and only if followed by double quote, backslash,
		// or newline. If not followed by one of these characters,
		// both the backslash and the character are preserved.
		//
		if(c == '\\' && i < l.size() - 1)
		{
		    switch(c = l[++i])
		    {
			case '"':
			case '\\':
			case '\n':
			{
			    arg.push_back(c);
			    break;
			}
			default:
			{
			    arg.push_back('\\');
			    arg.push_back(c);
			    break;
			}
		    }
		}
		else
		{
		    arg.push_back(c);
		}
		break;
	    }
	    case SingleQuote:
	    {
		if(c == '\'')
		{
		   state = Normal; // End of single-quote mode.
		}
		else
		{
		    arg.push_back(c); // Everything else is taken literally.
		}
		break;
	    }
	    case ANSIQuote:
	    {
		switch(c)
		{
		    case '\\':
		    {
			switch(c = l[++i])
			{
			    //
			    // Single-letter escape sequences.
			    //
			    case 'a':
			    {
				arg.push_back('\a');
				break;
			    }
			    case 'b':
			    {
				arg.push_back('\b');
				break;
			    }
			    case 'f':
			    {
				arg.push_back('\f');
				break;
			    }
			    case 'n':
			    {
				arg.push_back('\n');
				break;
			    }
			    case 'r':
			    {
				arg.push_back('\r');
				break;
			    }
			    case 't':
			    {
				arg.push_back('\t');
				break;
			    }
			    case 'v':
			    {
				arg.push_back('\v');
				break;
			    }
			    case '\\':
			    {
				arg.push_back('\\');
				break;
			    }
			    case '\'':
			    {
				arg.push_back('\'');
				break;
			    }
			    case 'e': // Not ANSI-C, but used by bash.
			    {
				arg.push_back('\033');
				break;
			    }

			    //
			    // Process up to three octal digits.
			    //
			    case '0':
			    case '1':
			    case '2':
			    case '3':
			    {
				static string octalDigits = "01234567";
				unsigned short us = c - '0';
				for(string::size_type j = i;
				    j < i + 2 && j < l.size() && octalDigits.find_first_of(c = l[i]) != string::npos;
				    ++j)
				{
				    us = us * 8 + c - '0';
				    ++i;
				}
				arg.push_back(static_cast<char>(us));
				break;
			    }

			    //
			    // Process up to two hex digits.
			    //
			    case 'x':
			    {
				if(i == l.size() - 1)
				{
				    arg.push_back('x');
				    continue;
				}
				IceUtil::Int64 ull = 0;
				for(string::size_type j = i + 1; j < i + 3 && j < l.size() && isxdigit(c = l[j]); ++j)
				{
				    ull *= 16;
				    if(isdigit(c))
				    {
					ull += c - '0';
				    }
				    else if(islower(c))
				    {
					ull += c - 'a' + 10;
				    }
				    else
				    {
					ull += c - 'A' + 10;
				    }
				    ++i;
				}
				arg.push_back(static_cast<char>(ull));
				break;
			    }

			    //
			    // Process control-chars.
			    //
			    case 'c':
			    {
				if(isalpha(c) || c == '@' || (c >= '[' && c <= '_'))
				{
				    arg.push_back(static_cast<char>(toupper(c) - '@'));
				}
				else
				{
				    //
				    // Bash does not define what should happen if a \c
				    // is not followed by a recognized control character.
				    // We simply treat this case like other unrecognized
				    // escape sequences, that is, we preserver the escape
				    // sequence unchanged.
				    //
				    arg.push_back('\\');
				    arg.push_back('c');
				    arg.push_back(c);
				}
				break;
			    }

			    //
			    // If inside an ANSI-quoted string, a backslash isn't followed by
			    // one of the recognized characters, both the backslash and the
			    // character are preserved.
			    //
			    default:
			    {
				arg.push_back('\\');
				arg.push_back(c);
				break;
			    }
			}
			break;
		    }
		    case '\'':
		    {
			state = Normal;
			break;
		    }
		    default:
		    {
			arg.push_back(c);
			break;
		    }
		}
	        break;
	    }
	    default:
	    {
		assert(!"Impossible parse state");
		break;
	    }
	}
    }

    switch(state)
    {
	case Normal:
	{
	    break; // Everything worked fine.
	}
	case SingleQuote:
	{
	    throw BadQuote("missing closing single quote");
	    break;
	}
	case DoubleQuote:
	{
	    throw BadQuote("missing closing double quote");
	    break;
	}
	case ANSIQuote:
	{
	    throw BadQuote("unterminated $' quote");
	    break;
	}
	default:
	{
	    assert(!"Impossible parse state");
	    break;
	}
    }

    if(!arg.empty())
    {
	vec.push_back(arg);
    }

    //
    // Make an artificial argc/argv pair, and call the
    // normal parse() below to take care of the options.
    //
    int argc = vec.size();
    char **argv = new char*[argc + 1]; // + 1 for terminating null pointer.
    int j;
    for(j = 0; j < argc; ++j)
    {
	argv[j] = strdup(vec[j].c_str());
    }
    argv[j] = 0; // Required by ISO standard C++.
    try
    {
	vector<string> result = parse(argc, argv);
        for(j = 0; j < argc; ++j)
	{
	    free(argv[j]);
	}
	delete[] argv;
	return result;
    }
    catch(...)
    {
        for(j = 0; j < argc; ++j)
	{
	    free(argv[j]);
	}
	delete[] argv;
        throw;
    }
}

vector<string>
IceUtil::Options::parse(int argc, char* argv[])
{
    IceUtil::RecMutex::Lock sync(_m);

    if(parseCalled)
    {
	throw APIError("cannot call parse() more than once on the same Option instance");
    }
    parseCalled = true;

    set<string> seenNonRepeatableOpts; // To catch repeated non-repeatable options.

    vector<string> result;

    int i;
    for(i = 1; i < argc; ++i)
    {
	if(strcmp(argv[i], "-") == 0 || strcmp(argv[i], "--") == 0)
	{
	    ++i;
	    break; // "-" and "--" indicate end of options.
	}

	string opt;
	ValidOpts::iterator pos;
        bool argDone = false;

	if(strncmp(argv[i], "--", 2) == 0)
	{
	    //
	    // Long option. If the option has an argument, it can either be separated by '='
	    // or appear as a separate argument. For example, "--name value" is the same
	    // as "--name=value".
	    //
	    const char *p = argv[i] + 2;
	    while(*p != '=' && *p != '\0')
	    {
		++p;
	    }
	    if(*p == '=')
	    {
		opt.assign(argv[i] + 2, p - (argv[i] + 2));
	    }
	    else
	    {
		opt = argv[i] + 2;
	    }

	    pos = checkOpt(opt, LongOpt);

	    if(*p == '=')
	    {
	        if(pos->second.arg == NoArg)
		{
		    string err = "--";
		    err += opt;
		    err.push_back('=');
		    err += p + 1;
		    err += "': option does not take an argument";
		    throw BadOpt(err);
		}
		setOpt(opt, p + 1, pos->second.repeat);
		argDone = true;
	    }

	    if(pos->second.repeat == NoRepeat)
	    {
		set<string>::iterator seenPos = seenNonRepeatableOpts.find(opt);
		if(seenPos != seenNonRepeatableOpts.end())
		{
		    string err = "`--";
		    err += opt + ":' option cannot be repeated";
		    throw BadOpt(err);
		}
		seenNonRepeatableOpts.insert(seenPos, opt);
	    }
	}
	else if(*argv[i] == '-')
	{
	    //
	    // Short option.
	    //
	    const char *p = argv[i];
	    char c;
	    while((c = *++p) != '\0')
	    {
		opt.clear();
		opt.push_back(c);
		pos = checkOpt(opt, ShortOpt);
		if(pos->second.arg == NeedArg && *(p + 1) != '\0')
		{
		    string optArg;
		    while(*++p != '\0')
		    {
			optArg.push_back(*p);
		    }
		    --p;
		    setOpt(opt, optArg, pos->second.repeat);
		    argDone = true;
		}
	    }

	    if(pos->second.repeat == NoRepeat)
	    {
		set<string>::iterator seenPos = seenNonRepeatableOpts.find(opt);
		if(seenPos != seenNonRepeatableOpts.end())
		{
		    string err = "`-";
		    err += opt + ":' option cannot be repeated";
		    throw BadOpt(err);
		}
		seenNonRepeatableOpts.insert(seenPos, opt);
	    }
	}
	else
	{
	    //
	    // Not an option or option argument.
	    //
	    result.push_back(argv[i]);
	    argDone = true;
	}

	if(!argDone)
	{
	    if(pos->second.arg == NeedArg) // Need an argument that is separated by whitespace.
	    {
		if(i == argc - 1)
		{
		    string err = "`-";
		    if(opt.size() != 1)
		    {
			err += "-";
		    }
		    err += opt;
		    err += "' option requires an argument";
		    throw BadOpt(err);
		}
		setOpt(opt, argv[++i], pos->second.repeat);
	    }
	    else
	    {
		setOpt(opt, "1", pos->second.repeat);
	    }
	}
    }

    while(i < argc)
    {
        result.push_back(argv[i++]);
    }

    return result;
}

bool
IceUtil::Options::isSet(const string& opt) const
{
    IceUtil::RecMutex::Lock sync(_m);

    if(!parseCalled)
    {
	throw APIError("cannot lookup options before calling parse()");
    }

    ValidOpts::const_iterator pos = checkOptIsValid(opt);
    return pos->second.repeat == NoRepeat ? _opts.find(opt) != _opts.end() : _ropts.find(opt) != _ropts.end();
}

string
IceUtil::Options::optArg(const string& opt) const
{
    IceUtil::RecMutex::Lock sync(_m);

    if(!parseCalled)
    {
	throw APIError("cannot lookup options before calling parse()");
    }

    ValidOpts::const_iterator pos = checkOptHasArg(opt);

    if(pos->second.repeat == Repeat)
    {
	string err = "`-";
	if(pos->second.length == LongOpt)
	{
	    err.push_back('-');
	}
	err += opt;
	err += "': is a repeating option -- use argVec() to get its arguments";
	throw APIError(err);
    }

    map<string, string>::const_iterator p = _opts.find(opt);
    return p == _opts.end() ? "" : p->second;
}

vector<string>
IceUtil::Options::argVec(const string& opt) const
{
    IceUtil::RecMutex::Lock sync(_m);

    if(!parseCalled)
    {
	throw APIError("cannot lookup options before calling parse()");
    }

    ValidOpts::const_iterator pos = checkOptHasArg(opt);

    if(pos->second.repeat == NoRepeat)
    {
	string err = "`-";
	if(pos->second.length == LongOpt)
	{
	    err.push_back('-');
	}
	err += opt + "': is a non-repeating option -- use optArg() to get its argument";
	throw APIError(err);
    }

    map<string, vector<string> >::const_iterator p = _ropts.find(opt);
    return p == _ropts.end() ? vector<string>() : p->second;
}

void
IceUtil::Options::addValidOpt(const string& opt, LengthType lt, ArgType at, const string& dflt, RepeatType rt)
{
    if(opt.empty())
    {
        return;
    }

    ValidOpts::iterator pos = _validOpts.find(opt);
    if(pos != _validOpts.end())
    {
	string err = "`";
	err += opt;
	err += "': duplicate option";
	throw APIError(err);
    }

    OptionDetails od;
    od.length = lt;
    od.arg = at;
    od.repeat = rt;

    _validOpts.insert(pos, ValidOpts::value_type(opt, od));

    if(at == NeedArg && !dflt.empty())
    {
	setOpt(opt, dflt, rt);
    }
}

IceUtil::Options::ValidOpts::iterator
IceUtil::Options::checkOpt(const string& opt, LengthType lt)
{
    ValidOpts::iterator pos = _validOpts.find(opt);
    if(pos == _validOpts.end())
    {
	string err = "invalid option: `-";
	if(lt == LongOpt)
	{
	    err.push_back('-');
	}
	err += opt;
	err.push_back('\'');
	throw BadOpt(err);
    }
    return pos;
}

void
IceUtil::Options::setOpt(const string& opt, const string& val, RepeatType rt)
{
    if(rt == Repeat)
    {
	ROpts::iterator pos = _ropts.find(opt);
	if(pos != _ropts.end())
	{
	    pos->second.push_back(val);
	}
	else
	{
	    vector<string> vec;
	    vec.push_back(val);
	    _ropts.insert(pos, ROpts::value_type(opt, vec));
	}
    }
    else
    {
	_opts[opt] = val;
    }
}

IceUtil::Options::ValidOpts::const_iterator
IceUtil::Options::checkOptIsValid(const string& opt) const
{
    ValidOpts::const_iterator pos = _validOpts.find(opt);
    if(pos == _validOpts.end())
    {
	string err = "`";
	err += opt;
	err += "': invalid option";
	throw APIError(err);
    }
    return pos;
}

IceUtil::Options::ValidOpts::const_iterator
IceUtil::Options::checkOptHasArg(const string& opt) const
{
    ValidOpts::const_iterator pos = checkOptIsValid(opt);
    if(pos->second.arg == NoArg)
    {
	string err = "`-";
	if(pos->second.length == LongOpt)
	{
	    err.push_back('-');
	}
	err += opt;
	err += "': option does not take arguments";
	throw APIError(err);
    }
    return pos;
}
