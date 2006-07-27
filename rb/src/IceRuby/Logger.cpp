// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Logger.h>
#include <Util.h>

using namespace std;
using namespace IceRuby;

//
// LoggerWrapper is a C++ wrapper around a Ruby logger implementation, which
// is necessary when the application supplies its own logger to initialize().
// It's possible that the Ice run time might invoke on a logger from a thread
// other than Ruby's main thread, so we make all invocations from a new Ruby
// thread.
//
// The Ruby class LoggerI does the opposite; it delegates to a C++ object,
// which is necessary when the application calls getLogger() in order to
// invoke directly on the logger object.
//

namespace
{
class LoggerWrapperThread : public RubyThread
{
public:

    LoggerWrapperThread(VALUE logger, const string& method, const vector<string>& args) :
	_logger(logger), _method(method), _args(args)
    {
    }

    virtual void run()
    {
	int numArgs = static_cast<int>(_args.size());
	VALUE* args = 0;
	if(numArgs > 0)
	{
	    args = new VALUE[numArgs];
	    for(vector<string>::size_type i = 0; i < _args.size(); ++i)
	    {
		args[i] = createString(_args[i]);
	    }
	}
	try
	{
	    callRuby(rb_funcall2, _logger, rb_intern(_method.c_str()), numArgs, args);
	}
	catch(...)
	{
	    delete[] args;
	    throw;
	}
	delete[] args;
    }

private:

    VALUE _logger;
    string _method;
    vector<string> _args;
};
}

IceRuby::LoggerWrapper::LoggerWrapper(VALUE logger) :
    _logger(logger)
{
}

void
IceRuby::LoggerWrapper::print(const string& message)
{
    vector<string> args;
    args.push_back(message);
    RubyThreadPtr t = new LoggerWrapperThread(_logger, "print", args);
    t->start(true);
}

void
IceRuby::LoggerWrapper::trace(const string& category, const string& message)
{
    vector<string> args;
    args.push_back(category);
    args.push_back(message);
    RubyThreadPtr t = new LoggerWrapperThread(_logger, "trace", args);
    t->start(true);
}

void
IceRuby::LoggerWrapper::warning(const string& message)
{
    vector<string> args;
    args.push_back(message);
    RubyThreadPtr t = new LoggerWrapperThread(_logger, "warning", args);
    t->start(true);
}

void
IceRuby::LoggerWrapper::error(const string& message)
{
    vector<string> args;
    args.push_back(message);
    RubyThreadPtr t = new LoggerWrapperThread(_logger, "error", args);
    t->start(true);
}

VALUE
IceRuby::LoggerWrapper::getObject()
{
    return _logger;
}

void
IceRuby::LoggerWrapper::mark()
{
    rb_gc_mark(_logger);
}

static VALUE _loggerClass;

extern "C"
void
IceRuby_Logger_free(Ice::LoggerPtr* p)
{
    assert(p);
    delete p;
}

VALUE
IceRuby::createLogger(const Ice::LoggerPtr& p)
{
    return Data_Wrap_Struct(_loggerClass, 0, IceRuby_Logger_free, new Ice::LoggerPtr(p));
}

extern "C"
VALUE
IceRuby_Logger_print(VALUE self, VALUE message)
{
    ICE_RUBY_TRY
    {
	Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
	assert(p);

	string msg = getString(message);
	(*p)->print(msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Logger_trace(VALUE self, VALUE category, VALUE message)
{
    ICE_RUBY_TRY
    {
	Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
	assert(p);

	string cat = getString(category);
	string msg = getString(message);
	(*p)->trace(cat, msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Logger_warning(VALUE self, VALUE message)
{
    ICE_RUBY_TRY
    {
	Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
	assert(p);

	string msg = getString(message);
	(*p)->warning(msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Logger_error(VALUE self, VALUE message)
{
    ICE_RUBY_TRY
    {
	Ice::LoggerPtr* p = reinterpret_cast<Ice::LoggerPtr*>(DATA_PTR(self));
	assert(p);

	string msg = getString(message);
	(*p)->error(msg);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

bool
IceRuby::initLogger(VALUE iceModule)
{
    //
    // Logger.
    //
    _loggerClass = rb_define_class_under(iceModule, "LoggerI", rb_cObject);

    //
    // Instance methods.
    //
    rb_define_method(_loggerClass, "print", CAST_METHOD(IceRuby_Logger_print), 1);
    rb_define_method(_loggerClass, "trace", CAST_METHOD(IceRuby_Logger_trace), 2);
    rb_define_method(_loggerClass, "warning", CAST_METHOD(IceRuby_Logger_warning), 1);
    rb_define_method(_loggerClass, "error", CAST_METHOD(IceRuby_Logger_error), 1);

    return true;
}
