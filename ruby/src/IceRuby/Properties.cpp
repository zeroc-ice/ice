// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Properties.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IceRuby;

static VALUE _propertiesClass;

extern "C"
void
IceRuby_Properties_free(Ice::PropertiesPtr* p)
{
    assert(p);
    delete p;
}

extern "C"
VALUE
IceRuby_createProperties(int argc, VALUE* argv, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        Ice::StringSeq seq;
        if(argc >= 1 && !NIL_P(argv[0]) && !arrayToStringSeq(argv[0], seq))
        {
            throw RubyException(rb_eTypeError, "invalid array argument to Ice::createProperties");
        }

        Ice::PropertiesPtr defaults;
        if(argc == 2)
        {
            if(!NIL_P(argv[1]) && callRuby(rb_obj_is_instance_of, argv[1], _propertiesClass) == Qfalse)
            {
                throw RubyException(rb_eTypeError, "invalid properties argument to Ice::createProperties");
            }
            defaults = getProperties(argv[1]);
        }

        //
        // Insert the program name (stored in the Ruby global variable $0) as the first
        // element of the sequence.
        //
        volatile VALUE progName = callRuby(rb_gv_get, "$0");
        seq.insert(seq.begin(), getString(progName));

        Ice::PropertiesPtr obj;
        if(argc >= 1)
        {
            obj = Ice::createProperties(seq, defaults);
        }
        else
        {
            obj = Ice::createProperties();
        }

        //
        // Replace the contents of the given argument list with the filtered arguments.
        //
        if(argc > 0 && !NIL_P(argv[0]))
        {
            callRuby(rb_ary_clear, argv[0]);

            //
            // We start at index 1 in order to skip the element that we inserted earlier.
            //
            for(Ice::StringSeq::size_type i = 1; i < seq.size(); ++i)
            {
                volatile VALUE str = createString(seq[i]);
                callRuby(rb_ary_push, argv[0], str);
            }
        }

        return createProperties(obj);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getProperty(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        string v = p->getProperty(k);
        return createString(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getPropertyWithDefault(VALUE self, VALUE key, VALUE def)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        string d = getString(def);
        string v = p->getPropertyWithDefault(k, d);
        return createString(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getPropertyAsInt(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        Ice::Int v = p->getPropertyAsInt(k);
        return INT2FIX(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getPropertyAsIntWithDefault(VALUE self, VALUE key, VALUE def)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        Ice::Int d = getInteger(def);
        Ice::Int v = p->getPropertyAsIntWithDefault(k, d);
        return INT2FIX(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getPropertyAsList(VALUE self, VALUE key)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        Ice::StringSeq v = p->getPropertyAsList(k);
        return stringSeqToArray(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getPropertyAsListWithDefault(VALUE self, VALUE key, VALUE def)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        Ice::StringSeq d;
        if(!arrayToStringSeq(def, d))
        {
            throw RubyException(rb_eTypeError, "invalid array argument to Ice::getPropertyAsListWithDefault");
        }
        Ice::StringSeq v = p->getPropertyAsListWithDefault(k, d);
        return stringSeqToArray(v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getPropertiesForPrefix(VALUE self, VALUE prefix)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string pfx = getString(prefix);
        Ice::PropertyDict dict = p->getPropertiesForPrefix(pfx);
        volatile VALUE result = callRuby(rb_hash_new);
        for(Ice::PropertyDict::const_iterator q = dict.begin(); q != dict.end(); ++q)
        {
            volatile VALUE key = createString(q->first);
            volatile VALUE value = createString(q->second);
            callRuby(rb_hash_aset, result, key, value);
        }
        return result;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_setProperty(VALUE self, VALUE key, VALUE value)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string k = getString(key);
        string v = getString(value);
        p->setProperty(k, v);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_getCommandLineOptions(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        Ice::StringSeq options = p->getCommandLineOptions();
        return stringSeqToArray(options);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_parseCommandLineOptions(VALUE self, VALUE prefix, VALUE options)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string pfx = getString(prefix);
        Ice::StringSeq seq;
        if(!arrayToStringSeq(options, seq))
        {
            throw RubyException(rb_eTypeError, "invalid array argument to Ice::parseCommandLineOptions");
        }
        Ice::StringSeq filtered = p->parseCommandLineOptions(pfx, seq);
        return stringSeqToArray(filtered);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_parseIceCommandLineOptions(VALUE self, VALUE options)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        Ice::StringSeq seq;
        if(!arrayToStringSeq(options, seq))
        {
            throw RubyException(rb_eTypeError, "invalid array argument to Ice::parseIceCommandLineOptions");
        }
        Ice::StringSeq filtered = p->parseIceCommandLineOptions(seq);
        return stringSeqToArray(filtered);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_load(VALUE self, VALUE file)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        string f = getString(file);
        p->load(f);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_clone(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        Ice::PropertiesPtr props = p->clone();
        return createProperties(props);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C"
VALUE
IceRuby_Properties_to_s(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::PropertiesPtr p = getProperties(self);
        Ice::PropertyDict dict = p->getPropertiesForPrefix("");
        string str;
        for(Ice::PropertyDict::const_iterator q = dict.begin(); q != dict.end(); ++q)
        {
            if(q != dict.begin())
            {
                str.append("\n");
            }
            str.append(q->first + "=" + q->second);
        }
        return createString(str);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

void
IceRuby::initProperties(VALUE iceModule)
{
    rb_define_module_function(iceModule, "createProperties", CAST_METHOD(IceRuby_createProperties), -1);

    _propertiesClass = rb_define_class_under(iceModule, "PropertiesI", rb_cObject);
    rb_define_method(_propertiesClass, "getProperty", CAST_METHOD(IceRuby_Properties_getProperty), 1);
    rb_define_method(_propertiesClass, "getPropertyWithDefault",
                     CAST_METHOD(IceRuby_Properties_getPropertyWithDefault), 2);
    rb_define_method(_propertiesClass, "getPropertyAsInt", CAST_METHOD(IceRuby_Properties_getPropertyAsInt), 1);
    rb_define_method(_propertiesClass, "getPropertyAsIntWithDefault",
                     CAST_METHOD(IceRuby_Properties_getPropertyAsIntWithDefault), 2);
    rb_define_method(_propertiesClass, "getPropertyAsList", CAST_METHOD(IceRuby_Properties_getPropertyAsList), 1);
    rb_define_method(_propertiesClass, "getPropertyAsListWithDefault",
                     CAST_METHOD(IceRuby_Properties_getPropertyAsListWithDefault), 2);
    rb_define_method(_propertiesClass, "getPropertiesForPrefix",
                     CAST_METHOD(IceRuby_Properties_getPropertiesForPrefix), 1);
    rb_define_method(_propertiesClass, "setProperty", CAST_METHOD(IceRuby_Properties_setProperty), 2);
    rb_define_method(_propertiesClass, "getCommandLineOptions", CAST_METHOD(IceRuby_Properties_getCommandLineOptions),
                     0);
    rb_define_method(_propertiesClass, "parseCommandLineOptions",
                     CAST_METHOD(IceRuby_Properties_parseCommandLineOptions), 2);
    rb_define_method(_propertiesClass, "parseIceCommandLineOptions",
                     CAST_METHOD(IceRuby_Properties_parseIceCommandLineOptions), 1);
    rb_define_method(_propertiesClass, "load", CAST_METHOD(IceRuby_Properties_load), 1);
    rb_define_method(_propertiesClass, "clone", CAST_METHOD(IceRuby_Properties_clone), 0);
    rb_define_method(_propertiesClass, "to_s", CAST_METHOD(IceRuby_Properties_to_s), 0);
}

Ice::PropertiesPtr
IceRuby::getProperties(VALUE v)
{
    Ice::PropertiesPtr* p = reinterpret_cast<Ice::PropertiesPtr*>(DATA_PTR(v));
    assert(p);
    return *p;
}

VALUE
IceRuby::createProperties(const Ice::PropertiesPtr& p)
{
    return Data_Wrap_Struct(_propertiesClass, 0, IceRuby_Properties_free, new Ice::PropertiesPtr(p));
}
