// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;
using System.Globalization;

namespace Ice;

internal abstract class LoggerI : Logger
{
    public void print(string message)
    {
        lock (_globalMutex)
        {
            write(message);
        }
    }

    public virtual void trace(string category, string message)
    {
        string s = format("--", category, message);
        lock (_globalMutex)
        {
            write(s);
        }
    }

    public virtual void warning(string message)
    {
        string s = format("-!", "warning", message);
        lock (_globalMutex)
        {
            write(s);
        }
    }

    public virtual void error(string message)
    {
        string s = format("!!", "error", message);
        lock (_globalMutex)
        {
            write(s);
        }
    }

    public string getPrefix()
    {
        return _prefix;
    }

    public abstract Logger cloneWithPrefix(string prefix);

    protected abstract void write(string message);

    internal LoggerI(string prefix)
    {
        _prefix = prefix;

        if (prefix.Length > 0)
        {
            _formattedPrefix = prefix + ": ";
        }

        _date = "d";
        _time = "HH:mm:ss:fff";
    }

    private string format(string prefix, string category, string message)
    {
        System.Text.StringBuilder s = new System.Text.StringBuilder(prefix);
        s.Append(' ');
        s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
        s.Append(' ');
        s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
        s.Append(' ');
        s.Append(_formattedPrefix);
        s.Append(category);
        s.Append(": ");
        s.Append(message);
        s.Replace("\n", "\n   ");
        return s.ToString();
    }

    internal static object _globalMutex = new object();
    internal readonly string _prefix;
    internal readonly string? _formattedPrefix;
    internal string _date;
    internal string _time;
}

internal sealed class ConsoleLoggerI : LoggerI
{
    public override Logger cloneWithPrefix(string prefix)
    {
        return new ConsoleLoggerI(prefix);
    }

    protected override void write(string message)
    {
        System.Console.Error.WriteLine(message);
    }

    internal ConsoleLoggerI(string prefix)
        : base(prefix)
    {
        _date = "d";
        _time = "HH:mm:ss:fff";
    }
}

#pragma warning disable CA1001 // _writer is disposed by destroy.
internal sealed class FileLoggerI : LoggerI
#pragma warning restore CA1001
{
    public override Logger cloneWithPrefix(string prefix)
    {
        return new FileLoggerI(prefix, _file);
    }

    public void destroy()
    {
        _writer.Close();
        _writer.Dispose();
    }

    protected override void write(string message)
    {
        _writer.WriteLine(message);
        _writer.Flush();
    }

    internal FileLoggerI(string prefix, string file)
        : base(prefix)
    {
        _file = file;
        _writer = new StreamWriter(new FileStream(file, FileMode.Append, FileAccess.Write, FileShare.ReadWrite));
    }

    private readonly string _file;
    private readonly TextWriter _writer;
}

internal class ConsoleListener : TraceListener
{
    public override bool IsThreadSafe => true;

    public override void TraceEvent(
        TraceEventCache? cache,
        string source,
        TraceEventType type,
        int id,
        string? message)
    {
        System.Text.StringBuilder s;
        if (type == TraceEventType.Error)
        {
            s = new System.Text.StringBuilder("!!");
        }
        else if (type == TraceEventType.Warning)
        {
            s = new System.Text.StringBuilder("-!");
        }
        else
        {
            s = new System.Text.StringBuilder("--");
        }
        s.Append(' ');
        s.Append(System.DateTime.Now.ToString(_date, CultureInfo.CurrentCulture));
        s.Append(' ');
        s.Append(System.DateTime.Now.ToString(_time, CultureInfo.CurrentCulture));
        s.Append(' ');
        s.Append(message);
        WriteLine(s.ToString());
    }

    public override void Write(string? message)
    {
        System.Console.Error.Write(message);
    }

    public override void WriteLine(string? message)
    {
        System.Console.Error.WriteLine(message);
    }

    internal const string _date = "d";
    internal const string _time = "HH:mm:ss:fff";
}

internal sealed class TraceLoggerI : LoggerI
{
    public override void trace(string category, string message)
    {
        Trace.TraceInformation(format(category, message));
        Trace.Flush();
    }

    public override void warning(string message)
    {
        Trace.TraceWarning(format("warning", message));
        Trace.Flush();
    }

    public override void error(string message)
    {
        string s = format("error", message);
        {
            Trace.TraceError(s);
            Trace.Flush();
        }
    }

    public override Logger cloneWithPrefix(string prefix)
    {
        return new TraceLoggerI(prefix, _console);
    }

    protected override void write(string message)
    {
        Trace.WriteLine(message);
        Trace.Flush();
    }

    internal TraceLoggerI(string prefix, bool console)
        : base(prefix)
    {
        _console = console;
        if (console && !Trace.Listeners.Contains(_consoleListener))
        {
            Trace.Listeners.Add(_consoleListener);
        }
    }

    private string format(string category, string message)
    {
        System.Text.StringBuilder s = new System.Text.StringBuilder(_formattedPrefix);
        s.Append(category);
        s.Append(": ");
        s.Append(message);
        s.Replace("\n", "\n   ");
        return s.ToString();
    }

    private readonly bool _console;
    internal static ConsoleListener _consoleListener = new ConsoleListener();
}
