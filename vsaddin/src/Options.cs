// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Text;
using System.Collections.Generic;
using System.Diagnostics;
using System.Security.Permissions;
using System.Runtime.Serialization;

using ValidOpts = System.Collections.Generic.Dictionary<string, Ice.VisualStudio.Options.OptionDetails>;
using Opts = System.Collections.Generic.Dictionary<string, Ice.VisualStudio.Options.OptionValue>;
using ROpts = System.Collections.Generic.Dictionary<string, Ice.VisualStudio.Options.OptionValueVector>;
using Synonyms = System.Collections.Generic.Dictionary<string, string>;

namespace Ice.VisualStudio
{

[SerializableAttribute]
public class BadOptionException : Exception
{
    public BadOptionException()
    {
        this.reason = "";
    }

    public BadOptionException(string reason)
    {
        this.reason = reason;
    }

    public string reason
    {
        get
        {
            return _reason;
        }
        set
        {
            _reason = value;
        }
    }

    [SecurityPermission(SecurityAction.Demand, SerializationFormatter = true)]
    public override void GetObjectData(SerializationInfo info, StreamingContext context)
    {
        if(info == null)
        {
            throw new ArgumentNullException("info");
        }
        base.GetObjectData(info, context);
        info.AddValue("reason", reason);
    }

    private string _reason;
}

[SerializableAttribute]
public class APIException : Exception
{
    public APIException()
    {
        this.reason = "";
    }

    public APIException(string reason)
    {
        this.reason = reason;
    }

    public string reason
    {
        get
        {
            return _reason;
        }
        set
        {
            _reason = value;
        }
    }

    [SecurityPermission(SecurityAction.Demand, SerializationFormatter = true)]
    public override void GetObjectData(SerializationInfo info, StreamingContext context)
    {
        if(info == null)
        {
            throw new ArgumentNullException("info");
        }
        base.GetObjectData(info, context);
        info.AddValue("reason", reason);
    }

    private string _reason;
}

public class Options
{
    public enum LengthType { ShortOpt, LongOpt };
    public enum RepeatType { Repeat, NoRepeat };
    public enum ArgType { NeedArg, NoArg };
    private enum ParseState { Normal, DoubleQuote, SingleQuote };
    private bool parseCalled = false;

    public struct OptionDetails
    {
        public LengthType length;
        public ArgType arg;
        public RepeatType repeat;
        public bool hasDefault;
    }

    public struct OptionValue
    {
        public string val;
    }

    public class OptionValueVector
    {
        public List<string> vals = new List<string>();
    }

    ValidOpts _validOpts = new ValidOpts(); // Valid options and their details.
    Opts _opts = new Opts(); // Value of non-repeating options.
    ROpts _ropts = new ROpts(); // Value of repeating options.
    Synonyms _synonyms = new Synonyms(); // Map from short to long option and vice versa.

    public void addOpt(string shortOpt)
    {
        addOpt(shortOpt, "", ArgType.NoArg, "", RepeatType.NoRepeat);
    }

    public void addOpt(string shortOpt, string longOpt)
    {
        addOpt(shortOpt, longOpt, ArgType.NoArg, "", RepeatType.NoRepeat);
    }

    public void addOpt(string shortOpt, string longOpt, ArgType at)
    {
        addOpt(shortOpt, longOpt, at, "", RepeatType.NoRepeat);
    }

    public void addOpt(string shortOpt, string longOpt, ArgType at, string dflt)
    {
        addOpt(shortOpt, longOpt, at, dflt, RepeatType.NoRepeat);
    }

    public void addOpt(string shortOpt, string longOpt, ArgType at, string dflt, RepeatType rt)
    {
        checkArgs(shortOpt, longOpt, at == ArgType.NeedArg, dflt);
        addValidOpt(shortOpt, longOpt, at, dflt, rt);
    }

    public List<string> parse(List<string> args)
    {
        if(parseCalled)
        {
            throw new APIException("cannot call parse() more than once on the same Option instance");
        }
        parseCalled = true;

        List<string> seenNonRepeatableOpts = new List<string>(); // To catch repeated non-repeatable options.

        List<string> result = new List<string>();

        int i;
        for(i = 0; i < args.Count; ++i)
        {
            if(args[i] == "-" || args[i] == "--")
            {
                ++i;
                break; // "-" and "--" indicate end of options.
            }

            string opt = null;
            OptionDetails pos = new OptionDetails();
            pos.repeat = RepeatType.NoRepeat;
            pos.length = LengthType.ShortOpt;
            pos.hasDefault = false;
            pos.arg = ArgType.NoArg;

            bool argDone = false;

            if(args[i].StartsWith("--", StringComparison.Ordinal))
            {
                //
                // Long option. If the option has an argument, it can either be separated by '='
                // or appear as a separate argument. For example, "--name value" is the same
                // as "--name=value".
                //
                int p = args[i].IndexOf('=', 2);
                if(p >= 0)
                {
                    opt = args[i].Substring(2, p - 2);
                }
                else
                {
                    opt = args[i].Substring(2);
                }

                pos = checkOpt(opt, LengthType.LongOpt);

                if(pos.repeat == RepeatType.NoRepeat)
                {
                    if(seenNonRepeatableOpts.Contains(opt))
                    {
                        string err = "`--";
                        err += opt + ":' option cannot be repeated";
                        throw new BadOptionException(err);
                    }
                    seenNonRepeatableOpts.Add(opt);
                    string synonym = getSynonym(opt);
                    if(!String.IsNullOrEmpty(synonym))
                    {
                        seenNonRepeatableOpts.Add(synonym);
                    }
                }

                if(p >= 0)
                {
                    if(pos.arg == ArgType.NoArg && p != args[i].Length - 1)
                    {
                        string err = "`";
                        err += args[i];
                        err += "': option does not take an argument";
                        throw new BadOptionException(err);
                    }
                    setOpt(opt, "", args[i].Substring(p + 1), pos.repeat);
                    argDone = true;
                }
            }
            else if(!String.IsNullOrEmpty(args[i]) && args[i][0] == '-')
            {
                //
                // Short option.
                //
                for(int p = 1; p < args[i].Length; ++p)
                {
                    opt = "";
                    opt += args[i][p];
                    pos = checkOpt(opt, LengthType.ShortOpt);

                    if(pos.repeat == RepeatType.NoRepeat)
                    {
                        if(seenNonRepeatableOpts.Contains(opt))
                        {
                            string err = "`-";
                            err += opt + ":' option cannot be repeated";
                            throw new BadOptionException(err);
                        }
                        seenNonRepeatableOpts.Add(opt);
                        string synonym = getSynonym(opt);
                        if(!String.IsNullOrEmpty(synonym))
                        {
                            seenNonRepeatableOpts.Add(synonym);
                        }
                    }

                    if(pos.arg == ArgType.NeedArg && p != args[i].Length - 1)
                    {
                        string optArg = args[i].Substring(p + 1);
                        setOpt(opt, "", optArg, pos.repeat);
                        argDone = true;
                        break;
                    }
                }
            }
            else
            {
                //
                // Not an option or option argument.
                //
                result.Add(args[i]);
                argDone = true;
            }

            if(!argDone)
            {
                if(pos.arg == ArgType.NeedArg) // Need an argument that is separated by whitespace.
                {
                    if(i == args.Count - 1)
                    {
                        string err = "`-";
                        if(opt.Length != 1)
                        {
                            err += "-";
                        }
                        err += opt;
                        err += "' option requires an argument";
                        throw new BadOptionException(err);
                    }
                    setOpt(opt, "", args[++i], pos.repeat);
                }
                else
                {
                    setOpt(opt, "", "1", pos.repeat);
                }
            }
        }

        _synonyms.Clear(); // Don't need the contents anymore.

        while(i < args.Count)
        {
            result.Add(args[i++]);
        }

        return result;
    }

    public bool isSet(string opt)
    {
        if(!parseCalled)
        {
            throw new APIException("cannot lookup options before calling parse()");
        }

        OptionDetails pos = checkOptIsValid(opt);
        return pos.repeat == RepeatType.NoRepeat ? _opts.ContainsKey(opt) : _ropts.ContainsKey(opt);
    }

    public string optArg(string opt)
    {
        if(!parseCalled)
        {
            throw new APIException("cannot lookup options before calling parse()");
        }

        OptionDetails pos = checkOptHasArg(opt);

        if(pos.repeat == RepeatType.Repeat)
        {
            string err = "`-";
            if(pos.length ==  LengthType.LongOpt)
            {
                err += '-';
            }
            err += opt;
            err += "': is a repeating option -- use argVec() to get its arguments";
            throw new BadOptionException(err);
        }

        if(!_opts.ContainsKey(opt))
        {
            return "";
        }
        return _opts[opt].val;
    }
    
    public List<string> argVec(string opt)
    {
        if(!parseCalled)
        {
            throw new APIException("cannot lookup options before calling parse()");
        }

        OptionDetails pos = checkOptHasArg(opt);

        if(pos.repeat == RepeatType.NoRepeat)
        {
            string err = "`-";
            if(pos.length == LengthType.LongOpt)
            {
                err += '-';
            }
            err += opt + "': is a non-repeating option -- use optArg() to get its argument";
            throw new BadOptionException(err);
        }

        if(!_ropts.ContainsKey(opt))
        {
            return new List<string>();
        }
        return _ropts[opt].vals;
    }

    private void
    addValidOpt(string shortOpt, string longOpt, ArgType at, string dflt, RepeatType rt)
    {
        if(shortOpt.Length > 0 && _validOpts.ContainsKey(shortOpt))
        {
            string err = "`";
            err += shortOpt;
            err += "': duplicate option";
            throw new BadOptionException(err);
        }
        if(longOpt.Length > 0 && _validOpts.ContainsKey(longOpt))
        {
            string err = "`";
            err += longOpt;
            err += "': duplicate option";
            throw new BadOptionException(err);
        }

        OptionDetails odp = new OptionDetails();
        odp.arg = at;
        odp.repeat = rt;
        odp.hasDefault = dflt.Length > 0;

        if(shortOpt.Length > 0)
        {
            odp.length = LengthType.ShortOpt;
            _validOpts[shortOpt] = odp;
        }
        if(longOpt.Length > 0)
        {
            odp.length = LengthType.LongOpt;
            _validOpts[longOpt] = odp;
        }

        updateSynonyms(shortOpt, longOpt);

        if(at == ArgType.NeedArg && dflt.Length > 0)
        {
            setOpt(shortOpt, longOpt, dflt, rt);
        }
    }

    void
    updateSynonyms(string shortOpt, string longOpt)
    {
        if(shortOpt.Length > 0 && longOpt.Length > 0)
        {
            _synonyms[shortOpt] = longOpt;
            _synonyms[longOpt] = shortOpt;
        }
    }

    string
    getSynonym(string optName)
    {
        return _synonyms.ContainsKey(optName) ? _synonyms[optName] : "";
    }

    private static void
    checkArgs(string shortOpt, string longOpt, bool needArg, string dflt)
    {
        if(shortOpt.Length == 0 && longOpt.Length == 0)
        {
            throw new BadOptionException("short and long option cannot both be empty");
        }

        if(shortOpt.Length > 0)
        {
            if(shortOpt.Length != 1)
            {
                string err = "`";
                err += shortOpt;
                err += "': a short option cannot specify more than one option";
                throw new BadOptionException(err);
            }
            if(" \t\n\r\f\v".IndexOf(shortOpt, StringComparison.Ordinal) >= 0)
            {
                string err = "`";
                err += shortOpt;
                err += "': a short option cannot be whitespace";
                throw new BadOptionException(err);
            }
            if(!String.IsNullOrEmpty(shortOpt) && shortOpt[0].Equals('-'))
            {
                string err = "`";
                err += shortOpt;
                err += "': a short option cannot be `-'";
                throw new BadOptionException(err);
            }
        }

        if(longOpt.Length > 0)
        {
            if(" \t\n\r\f\v".IndexOf(longOpt, StringComparison.Ordinal) >= 0)
            {
                string err = "`";
                err += shortOpt;
                err += "': a long option cannot be whitespace";
                throw new BadOptionException(err);
            }
            if(!String.IsNullOrEmpty(shortOpt) && shortOpt[0].Equals('-'))
            {
                string err = "`";
                err += shortOpt;
                err += "': a long option cannot be `-'";
                throw new BadOptionException(err);
            }
        }

        if(!needArg && dflt.Length > 0)
        {
            throw new BadOptionException("a default value can be specified only for options requiring an argument");
        }
    }

    //
    // Split a command line into argv-style arguments, applying
    // bash quoting rules. The return value is the arguments
    // in the command line, with all shell escapes applied, and
    // quotes removed.
    //
    static public List<string> split(string line)
    {
        const string IFS = " \t\n"; // Internal Field Separator.
        if(String.IsNullOrEmpty(line))
        {
            return new List<string>();
        }
        String l = line.Trim(' ').Trim('\t').Trim('\n');

        if(String.IsNullOrEmpty(l))
        {
            return new List<string>();
        }

        List<string> vec = new List<string>();

        
        ParseState state = ParseState.Normal;

        string arg = null;

        for(int i = 0; i < l.Length; ++i)
        {
            char c = l[i];
            switch(state)
            {
                case ParseState.Normal:
                {
                    switch(c)
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
                            if(i < l.Length - 1 && l[++i] != '\n')
                            {
                                switch(l[i])
                                {
                                    case ' ':
                                    case '$':
                                    case '\'':
                                    case '"':
                                    {
                                        arg += l[i];
                                        break;
                                    }
                                    default:
                                    {
                                        arg += '\\';
                                        arg += l[i];
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                        case '\'':
                        {
                            state = ParseState.SingleQuote;
                            break;
                        }
                        case '"':
                        {
                            state = ParseState.DoubleQuote;
                            break;
                        }
                        default:
                        {
                            if(IFS.IndexOf(l[i]) >= 0)
                            {
                                vec.Add(arg);
                                arg = "";

                                //
                                // Move to start of next argument.
                                //
                                while(++i < l.Length && IFS.IndexOf(l[i]) >= 0)
                                {
                                    ;
                                }
                                --i;
                            }
                            else
                            {
                                arg += l[i];
                            }
                            break;
                        }
                    }
                    break;
                }
                case ParseState.DoubleQuote:
                {
                    //
                    // Within double quotes, only backslash retains its special
                    // meaning, and only if followed by double quote, backslash,
                    // or newline. If not followed by one of these characters,
                    // both the backslash and the character are preserved.
                    //
                    if(c.Equals('\\') && i < l.Length - 1)
                    {
                        switch(c = l[++i])
                        {
                            case '"':
                            case '\\':
                            case '\n':
                            {
                                arg += c;
                                break;
                            }
                            default:
                            {
                                arg += '\\';
                                arg += c;
                                break;
                            }
                        }
                    }
                    else if(c.Equals('"')) // End of double-quote mode.
                    {
                        state = ParseState.Normal;
                    }
                    else
                    {
                        arg += c; // Everything else is taken literally.
                    }
                    break;
                }
                case ParseState.SingleQuote:
                {
                    if(c.Equals('\'')) // End of single-quote mode.
                    {
                        state = ParseState.Normal;
                    }
                    else
                    {
                        arg += c; // Everything else is taken literally.
                    }
                    break;
                }
                default:
                {
                    // Impossible parse state
                    Debug.Assert(false);
                    break;
                }
            }
        }

        switch(state)
        {
            case ParseState.Normal:
            {
                vec.Add(arg);
                break;
            }
            case ParseState.SingleQuote:
            {
                throw new BadOptionException("missing closing single quote");
            }
            case ParseState.DoubleQuote:
            {
                throw new BadOptionException("missing closing double quote");
            }
            default:
            {
                // Impossible parse state
                Debug.Assert(false);
                break;
            }
        }
        return vec;
    }

    void setOpt(string opt1, string opt2, string val, RepeatType rt)
    {
        //
        // opt1 and opt2 (short and long opt) can't both be empty.
        //
        Debug.Assert(!(String.IsNullOrEmpty(opt1) && String.IsNullOrEmpty(opt2)));

        if(rt == RepeatType.NoRepeat)
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

    void setNonRepeatingOpt(string opt, string val)
    {
        if(String.IsNullOrEmpty(opt))
        {
            return;
        }

        //
        // The option must not have been set before or, if it was set, it must have
        // been because of a default value.
        //
        Debug.Assert(!_opts.ContainsKey(opt) || _validOpts[opt].hasDefault);

        OptionValue ovp = new OptionValue();
        ovp.val = val;
        _opts[opt] = ovp;

        string synonym = getSynonym(opt);
        if(!String.IsNullOrEmpty(synonym))
        {
            _opts[synonym] = ovp;
        }
    }

    void setRepeatingOpt(string opt, string val)
    {
        if(String.IsNullOrEmpty(opt))
        {
            return;
        }

        Debug.Assert(_validOpts.ContainsKey(opt));
        OptionDetails vpos = _validOpts[opt];

        string synonym = getSynonym(opt);


        if(_ropts.ContainsKey(opt))
        {
            Debug.Assert(_validOpts.ContainsKey(opt));
            Debug.Assert(_ropts.ContainsKey(opt));
            Debug.Assert(vpos.repeat == RepeatType.Repeat);

            OptionValueVector pos = _ropts[opt];
            if(vpos.hasDefault && pos.vals.Count == 1)
            {
                pos.vals[0] = val;
                vpos.hasDefault = false;
            }
            else
            {
                pos.vals.Add(val);
            }
        }
        else if(_ropts.ContainsKey(synonym))
        {
            Debug.Assert(_validOpts.ContainsKey(synonym));
            Debug.Assert(_validOpts[synonym].repeat == RepeatType.Repeat);

            OptionValueVector spos = _ropts[synonym];

            if(vpos.hasDefault && spos.vals.Count == 1)
            {
                spos.vals[0] = val;
                vpos.hasDefault = false;
            }
            else
            {
                spos.vals.Add(val);
            }
        }
        else
        {
            OptionValueVector ovp = new OptionValueVector();
            ovp.vals.Add(val);
            _ropts[opt] = ovp;
            if(!String.IsNullOrEmpty(synonym))
            {
                _ropts[synonym] = ovp;
            }
        }
    }

    OptionDetails checkOpt(string opt, LengthType lt)
    {
        if(!_validOpts.ContainsKey(opt))
        {
            string err = "invalid option: `-";
            if(lt == LengthType.LongOpt)
            {
                err += '-';
            }
            err += opt;
            err += '\'';
            throw new BadOptionException(err);
        }
        return _validOpts[opt];
    }

    OptionDetails checkOptIsValid(string opt)
    {
        if(!_validOpts.ContainsKey(opt))
        {
            string err = "`";
            err += opt;
            err += "': invalid option";
            throw new BadOptionException(err);
        }
        return _validOpts[opt];
    }

    OptionDetails checkOptHasArg(string opt)
    {
        OptionDetails pos = checkOptIsValid(opt);
        if(pos.arg == ArgType.NoArg)
        {
            string err = "`-";
            if(pos.length == LengthType.LongOpt)
            {
                err += '-';
            }
            err += opt;
            err += "': option does not take arguments";
            throw new BadOptionException(err);
        }
        return pos;
    }
}

}