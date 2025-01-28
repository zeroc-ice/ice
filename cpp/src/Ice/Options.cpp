// Copyright (c) ZeroC, Inc.

#include "Options.h"
#include "Ice/StringUtil.h"

#include <cassert>
#include <set>
#include <stdexcept>

using namespace std;

const char*
IceInternal::APIException::ice_id() const noexcept
{
    return "::IceInternal::APIException";
}

const char*
IceInternal::BadOptException::ice_id() const noexcept
{
    return "::IceInternal::BadOptException";
}

IceInternal::Options::Options() = default;

void
IceInternal::Options::checkArgs(const string& shortOpt, const string& longOpt, bool needArg, const string& dflt)
{
    if (shortOpt.empty() && longOpt.empty())
    {
        throw invalid_argument("short and long option cannot both be empty");
    }

    if (!shortOpt.empty())
    {
        if (shortOpt.size() != 1)
        {
            string err = "'";
            err += shortOpt;
            err += "': a short option cannot specify more than one option";
            throw invalid_argument(err);
        }
        if (shortOpt.find_first_of(" \t\n\r\f\v") != string::npos)
        {
            string err = "'";
            err += shortOpt;
            err += "': a short option cannot be whitespace";
            throw invalid_argument(err);
        }
        if (shortOpt[0] == '-')
        {
            string err = "'";
            err += shortOpt;
            err += "': a short option cannot be `-'";
            throw invalid_argument(err);
        }
    }

    if (!longOpt.empty())
    {
        if (longOpt.find_first_of(" \t\n\r\f\v") != string::npos)
        {
            string err = "'";
            err += longOpt;
            err += "': a long option cannot contain whitespace";
            throw invalid_argument(err);
        }
        if (longOpt[0] == '-')
        {
            string err = "'";
            err += longOpt;
            err += "': a long option must not contain a leading `-'";
            throw invalid_argument(err);
        }
    }

    if (!needArg && !dflt.empty())
    {
        throw invalid_argument("a default value can be specified only for options requiring an argument");
    }
}

void
IceInternal::Options::addOpt(
    const string& shortOpt,
    const string& longOpt,
    ArgType at,
    const string& dflt,
    RepeatType rt)
{
    if (parseCalled)
    {
        throw APIException(__FILE__, __LINE__, "cannot add options after parse() was called");
    }

    checkArgs(shortOpt, longOpt, at == NeedArg, dflt);

    addValidOpt(shortOpt, longOpt, at, dflt, rt);
}

//
// Split a command line into argv-style arguments, applying
// bash quoting rules. The return value is the arguments
// in the command line, with all shell escapes applied, and
// quotes removed.
//

IceInternal::Options::StringVector
IceInternal::Options::split(const string& line)
{
    const string IFS = " \t\n"; // Internal Field Separator.

    //
    // Strip leading and trailing whitespace.
    //
    string::size_type start = line.find_first_not_of(IFS);
    if (start == string::npos)
    {
        return {};
    }
    string::size_type end = line.find_last_not_of(IFS);
    assert(end != string::npos);

    string l(line, start, end - start + 1);

    StringVector vec;

    enum ParseState
    {
        Normal,
        DoubleQuote,
        SingleQuote,
        ANSIQuote
    };
    ParseState state = Normal;

    string arg;

    for (string::size_type i = 0; i < l.size(); ++i)
    {
        char c = l[i];
        switch (state)
        {
            case Normal:
            {
                switch (c)
                {
                    case '\\':
                    {
                        //
                        // Ignore a backslash at the end of the string,
                        // and strip backslash-newline pairs. If a
                        // backslash is followed by a space, single quote,
                        // double quote, or dollar sign, we drop the backslash
                        // and write the space, single quote, double quote,
                        // or dollar sign. This is necessary to allow quotes
                        // to be escaped. Dropping the backslash preceding a
                        // space deviates from bash quoting rules, but is
                        // necessary so we don't drop backslashes from Windows
                        // path names.)
                        //
                        if (i < l.size() - 1 && l[++i] != '\n')
                        {
                            switch (l[i])
                            {
                                case ' ':
                                case '$':
                                case '\'':
                                case '"':
                                {
                                    arg.push_back(l[i]);
                                    break;
                                }
                                default:
                                {
                                    arg.push_back('\\');
                                    arg.push_back(l[i]);
                                    break;
                                }
                            }
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
                        if (i < l.size() - 1 && l[i + 1] == '\'')
                        {
                            state = ANSIQuote; // Bash uses $'<text>' to allow ANSI escape sequences within <text>.
                            ++i;
                        }
                        else
                        {
                            arg.push_back('$');
                        }
                        break;
                    }
                    default:
                    {
                        if (IFS.find(l[i]) != string::npos)
                        {
                            vec.push_back(arg);
                            arg.clear();

                            //
                            // Move to start of next argument.
                            //
                            while (++i < l.size() && IFS.find(l[i]) != string::npos)
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
                if (c == '\\' && i < l.size() - 1)
                {
                    switch (c = l[++i])
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
                else if (c == '"') // End of double-quote mode.
                {
                    state = Normal;
                }
                else
                {
                    arg.push_back(c); // Everything else is taken literally.
                }
                break;
            }
            case SingleQuote:
            {
                if (c == '\'') // End of single-quote mode.
                {
                    state = Normal;
                }
                else
                {
                    arg.push_back(c); // Everything else is taken literally.
                }
                break;
            }
            case ANSIQuote:
            {
                switch (c)
                {
                    case '\\':
                    {
                        if (i == l.size() - 1)
                        {
                            break;
                        }
                        switch (c = l[++i])
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
                            case '4':
                            case '5':
                            case '6':
                            case '7':
                            {
                                static const string octalDigits = "01234567";
                                unsigned short us = 0;
                                string::size_type j;
                                for (j = i;
                                     j < i + 3 && j < l.size() && octalDigits.find_first_of(c = l[j]) != string::npos;
                                     ++j)
                                {
                                    us = us * 8 + static_cast<unsigned short>(c - '0');
                                }
                                i = j - 1;
                                arg.push_back(static_cast<char>(us));
                                break;
                            }

                            //
                            // Process up to two hex digits.
                            //
                            case 'x':
                            {
                                if (i < l.size() - 1 && !isxdigit(static_cast<unsigned char>(l[i + 1])))
                                {
                                    arg.push_back('\\');
                                    arg.push_back('x');
                                    break;
                                }

                                int64_t ull = 0;
                                string::size_type j;
                                for (j = i + 1;
                                     j < i + 3 && j < l.size() && isxdigit(static_cast<unsigned char>(c = l[j]));
                                     ++j)
                                {
                                    ull *= 16;
                                    if (isdigit(static_cast<unsigned char>(c)))
                                    {
                                        ull += c - '0';
                                    }
                                    else if (islower(static_cast<unsigned char>(c)))
                                    {
                                        ull += c - 'a' + 10;
                                    }
                                    else
                                    {
                                        ull += c - 'A' + 10;
                                    }
                                }
                                i = j - 1;
                                arg.push_back(static_cast<char>(ull));
                                break;
                            }

                            //
                            // Process control-chars.
                            //
                            case 'c':
                            {
                                c = l[++i];
                                if (IceInternal::isAlpha(c) || c == '@' || (c >= '[' && c <= '_'))
                                {
                                    arg.push_back(static_cast<char>(toupper(static_cast<unsigned char>(c)) - '@'));
                                }
                                else
                                {
                                    //
                                    // Bash does not define what should happen if a \c
                                    // is not followed by a recognized control character.
                                    // We simply treat this case like other unrecognized
                                    // escape sequences, that is, we preserve the escape
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
                    case '\'': // End of ANSI-quote mode.
                    {
                        state = Normal;
                        break;
                    }
                    default:
                    {
                        arg.push_back(c); // Everything else is taken literally.
                        break;
                    }
                }
                break;
            }
            default:
            {
                assert(false); // Impossible parse state
                break;
            }
        }
    }

    switch (state)
    {
        case Normal:
        {
            vec.push_back(arg);
            break;
        }
        case SingleQuote:
        {
            throw BadOptException(__FILE__, __LINE__, "missing closing single quote");
            break;
        }
        case DoubleQuote:
        {
            throw BadOptException(__FILE__, __LINE__, "missing closing double quote");
            break;
        }
        case ANSIQuote:
        {
            throw BadOptException(__FILE__, __LINE__, "unterminated $' quote");
            break;
        }
        default:
        {
            assert(false); // Impossible parse state
            break;
        }
    }

    return vec;
}

//
// Parse a vector of arguments and return the non-option
// arguments as the return value. Throw BadOptException if any of the
// options are invalid.
// Note that args[0] is ignored because that is the name
// of the executable.
//

IceInternal::Options::StringVector
IceInternal::Options::parse(const StringVector& args)
{
    if (parseCalled)
    {
        throw APIException(__FILE__, __LINE__, "cannot call parse() more than once on the same Option instance");
    }
    parseCalled = true;

    set<string> seenNonRepeatableOpts; // To catch repeated non-repeatable options.

    StringVector result;

    string::size_type i;
    for (i = 1; i < args.size(); ++i)
    {
        if (args[i] == "-" || args[i] == "--")
        {
            ++i;
            break; // "-" and "--" indicate end of options.
        }

        string opt;
        ValidOpts::iterator pos;
        bool argDone = false;

        if (args[i].compare(0, 2, "--") == 0)
        {
            //
            // Long option. If the option has an argument, it can either be separated by '='
            // or appear as a separate argument. For example, "--name value" is the same
            // as "--name=value".
            //
            string::size_type p = args[i].find('=', 2);
            if (p != string::npos)
            {
                opt = args[i].substr(2, p - 2);
            }
            else
            {
                opt = args[i].substr(2);
            }

            pos = checkOpt(opt, LongOpt);

            if (pos->second->repeat == NoRepeat)
            {
                auto seenPos = seenNonRepeatableOpts.find(opt);
                if (seenPos != seenNonRepeatableOpts.end())
                {
                    string err = "`--";
                    err += opt + ":' option cannot be repeated";
                    throw BadOptException(__FILE__, __LINE__, err);
                }
                seenNonRepeatableOpts.insert(seenPos, opt);
                string synonym = getSynonym(opt);
                if (!synonym.empty())
                {
                    seenNonRepeatableOpts.insert(synonym);
                }
            }

            if (p != string::npos)
            {
                if (pos->second->arg == NoArg)
                {
                    string err = "'";
                    err += opt;
                    err += "': option does not take an argument";
                    throw BadOptException(__FILE__, __LINE__, err);
                }
                else if (pos->second->arg == NeedArg && p == args[i].size() - 1)
                {
                    string err = "'";
                    err += opt;
                    err += "': option requires an argument";
                    throw BadOptException(__FILE__, __LINE__, err);
                }
                setOpt(opt, "", args[i].substr(p + 1), pos->second->repeat);
                argDone = true;
            }
        }
        else if (!args[i].empty() && args[i][0] == '-')
        {
            //
            // int16_t option.
            //
            for (string::size_type p = 1; p < args[i].size(); ++p)
            {
                opt.clear();
                opt.push_back(args[i][p]);
                pos = checkOpt(opt, ShortOpt);

                if (pos->second->repeat == NoRepeat)
                {
                    auto seenPos = seenNonRepeatableOpts.find(opt);
                    if (seenPos != seenNonRepeatableOpts.end())
                    {
                        string err = "`-";
                        err += opt + ":' option cannot be repeated";
                        throw BadOptException(__FILE__, __LINE__, err);
                    }
                    seenNonRepeatableOpts.insert(seenPos, opt);
                    string synonym = getSynonym(opt);
                    if (!synonym.empty())
                    {
                        seenNonRepeatableOpts.insert(synonym);
                    }
                }

                if (pos->second->arg == NeedArg)
                {
                    if (p != args[i].size() - 1)
                    {
                        string optArg = args[i].substr(p + 1);
                        setOpt(opt, "", optArg, pos->second->repeat);
                        argDone = true;
                        break;
                    }
                }
                else
                {
                    setOpt(opt, "", "1", pos->second->repeat);
                    argDone = true;
                }
            }
        }
        else
        {
            //
            // Not an option or option argument.
            //
            result.push_back(args[i]);
            argDone = true;
        }

        if (!argDone)
        {
            if (pos->second->arg == NeedArg) // Need an argument that is separated by whitespace.
            {
                if (i == args.size() - 1)
                {
                    string err = "`-";
                    if (opt.size() != 1)
                    {
                        err += "-";
                    }
                    err += opt;
                    err += "' option requires an argument";
                    throw BadOptException(__FILE__, __LINE__, err);
                }
                setOpt(opt, "", args[++i], pos->second->repeat);
            }
            else
            {
                setOpt(opt, "", "1", pos->second->repeat);
            }
        }
    }

    _synonyms.clear(); // Don't need the contents anymore.

    while (i < args.size())
    {
        result.push_back(args[i++]);
    }

    return result;
}

//
// Parse a normal argc/argv pair and return the non-option
// arguments as the return value.
//

IceInternal::Options::StringVector
IceInternal::Options::parse(int argc, const char* const argv[])
{
    StringVector vec;
    for (int i = 0; i < argc; ++i)
    {
        vec.emplace_back(argv[i]);
    }
    return parse(vec);
}

bool
IceInternal::Options::isSet(const string& opt) const
{
    if (!parseCalled)
    {
        throw APIException(__FILE__, __LINE__, "cannot lookup options before calling parse()");
    }

    auto pos = checkOptIsValid(opt);
    return pos->second->repeat == NoRepeat ? _opts.find(opt) != _opts.end() : _ropts.find(opt) != _ropts.end();
}

string
IceInternal::Options::optArg(const string& opt) const
{
    if (!parseCalled)
    {
        throw APIException(__FILE__, __LINE__, "cannot lookup options before calling parse()");
    }

    auto pos = checkOptHasArg(opt);

    if (pos->second->repeat == Repeat)
    {
        string err = "`-";
        if (pos->second->length == LongOpt)
        {
            err.push_back('-');
        }
        err += opt;
        err += "': is a repeating option -- use argVec() to get its arguments";
        throw invalid_argument(err);
    }

    auto p = _opts.find(opt);
    if (p == _opts.end())
    {
        return "";
    }
    return p->second->val;
}

IceInternal::Options::StringVector
IceInternal::Options::argVec(const string& opt) const
{
    if (!parseCalled)
    {
        throw APIException(__FILE__, __LINE__, "cannot lookup options before calling parse()");
    }

    auto pos = checkOptHasArg(opt);

    if (pos->second->repeat == NoRepeat)
    {
        string err = "`-";
        if (pos->second->length == LongOpt)
        {
            err.push_back('-');
        }
        err += opt + "': is a non-repeating option -- use optArg() to get its argument";
        throw invalid_argument(err);
    }

    auto p = _ropts.find(opt);
    return p == _ropts.end() ? StringVector() : p->second->vals;
}

void
IceInternal::Options::addValidOpt(
    const string& shortOpt,
    const string& longOpt,
    ArgType at,
    const string& dflt,
    RepeatType rt)
{
    if (!shortOpt.empty() && _validOpts.find(shortOpt) != _validOpts.end())
    {
        string err = "'";
        err += shortOpt;
        err += "': duplicate option";
        throw invalid_argument(err);
    }
    if (!longOpt.empty() && _validOpts.find(longOpt) != _validOpts.end())
    {
        string err = "'";
        err += longOpt;
        err += "': duplicate option";
        throw invalid_argument(err);
    }

    ODPtr odp = make_shared<OptionDetails>();
    odp->arg = at;
    odp->repeat = rt;
    odp->hasDefault = !dflt.empty();

    if (!shortOpt.empty())
    {
        odp->length = ShortOpt;
        _validOpts[shortOpt] = odp;
    }
    if (!longOpt.empty())
    {
        odp->length = LongOpt;
        _validOpts[longOpt] = odp;
    }

    updateSynonyms(shortOpt, longOpt);

    if (at == NeedArg && !dflt.empty())
    {
        setOpt(shortOpt, longOpt, dflt, rt);
    }
}

IceInternal::Options::ValidOpts::iterator
IceInternal::Options::checkOpt(const string& opt, LengthType lt)
{
    auto pos = _validOpts.find(opt);
    if (pos == _validOpts.end())
    {
        string err = "invalid option: `-";
        if (lt == LongOpt)
        {
            err.push_back('-');
        }
        err += opt;
        err.push_back('\'');
        throw BadOptException(__FILE__, __LINE__, err);
    }
    return pos;
}

void
IceInternal::Options::setOpt(const string& opt1, const string& opt2, const string& val, RepeatType rt)
{
    //
    // opt1 and opt2 (short and long opt) can't both be empty.
    //
    assert(!(opt1.empty() && opt2.empty()));

    if (rt == NoRepeat)
    {
        setNonRepeatingOpt(opt1, val);
        setNonRepeatingOpt(opt2, val);
    }
    else
    {
        setRepeatingOpt(opt1, val);
        setRepeatingOpt(opt2, val);
    }
}

void
IceInternal::Options::setNonRepeatingOpt(const string& opt, const string& val)
{
    if (opt.empty())
    {
        return;
    }

    //
    // The option must not have been set before or, if it was set, it must have
    // been because of a default value.
    //
    assert(_opts.find(opt) == _opts.end() || _validOpts.find(opt)->second->hasDefault);

    OValPtr ovp = make_shared<OptionValue>();
    ovp->val = val;
    _opts[opt] = ovp;

    const string synonym = getSynonym(opt);
    if (!synonym.empty())
    {
        _opts[synonym] = ovp;
    }
}

void
IceInternal::Options::setRepeatingOpt(const string& opt, const string& val)
{
    if (opt.empty())
    {
        return;
    }

    auto vpos = _validOpts.find(opt);
    assert(vpos != _validOpts.end());

    auto pos = _ropts.find(opt);
    const string synonym = getSynonym(opt);
    auto spos = _ropts.find(synonym);

    if (pos != _ropts.end())
    {
        assert(_validOpts.find(opt) != _validOpts.end());
        assert(vpos->second->repeat == Repeat);

        _ropts[opt] = pos->second;
        if (vpos->second->hasDefault && pos->second->vals.size() == 1)
        {
            pos->second->vals[0] = val;
            vpos->second->hasDefault = false;
        }
        else
        {
            pos->second->vals.push_back(val);
        }
    }
    else if (spos != _ropts.end())
    {
        assert(_validOpts.find(synonym) != _validOpts.end());
        assert(_validOpts.find(synonym)->second->repeat == Repeat);

        _ropts[synonym] = spos->second;
        if (vpos->second->hasDefault && spos->second->vals.size() == 1)
        {
            spos->second->vals[0] = val;
            vpos->second->hasDefault = false;
        }
        else
        {
            spos->second->vals.push_back(val);
        }
    }
    else
    {
        OVecPtr ovp = make_shared<OptionValueVector>();
        ovp->vals.push_back(val);
        _ropts[opt] = ovp;
        if (!synonym.empty())
        {
            _ropts[synonym] = ovp;
        }
    }
}

IceInternal::Options::ValidOpts::const_iterator
IceInternal::Options::checkOptIsValid(const string& opt) const
{
    auto pos = _validOpts.find(opt);
    if (pos == _validOpts.end())
    {
        string err = "'";
        err += opt;
        err += "': invalid option";
        throw invalid_argument(err);
    }
    return pos;
}

IceInternal::Options::ValidOpts::const_iterator
IceInternal::Options::checkOptHasArg(const string& opt) const
{
    auto pos = checkOptIsValid(opt);
    if (pos->second->arg == NoArg)
    {
        string err = "`-";
        if (pos->second->length == LongOpt)
        {
            err.push_back('-');
        }
        err += opt;
        err += "': option does not take arguments";
        throw invalid_argument(err);
    }
    return pos;
}

void
IceInternal::Options::updateSynonyms(const std::string& shortOpt, const std::string& longOpt)
{
    if (!shortOpt.empty() && !longOpt.empty())
    {
        _synonyms[shortOpt] = longOpt;
        _synonyms[longOpt] = shortOpt;
    }
}

string
IceInternal::Options::getSynonym(const std::string& optName) const
{
    auto pos = _synonyms.find(optName);
    return pos != _synonyms.end() ? pos->second : string("");
}
