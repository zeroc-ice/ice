// Copyright (c) ZeroC, Inc.

#include "Communicator.h"
#include "DefaultSliceLoader.h"
#include "IceDiscovery/IceDiscovery.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"
#include "ImplicitContext.h"
#include "Logger.h"
#include "Properties.h"
#include "Proxy.h"
#include "RubySliceLoader.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IceRuby;

namespace
{
    VALUE _communicatorClass;

    using CommunicatorMap = map<Ice::CommunicatorPtr, VALUE>;
    CommunicatorMap _communicatorMap;

    map<Ice::CommunicatorPtr, Ice::SliceLoaderPtr> _sliceLoaderMap;
}

extern "C" void
IceRuby_Communicator_free(void* p)
{
    delete static_cast<Ice::CommunicatorPtr*>(p);
}

static const rb_data_type_t IceRuby_CommunicatorType = {
    .wrap_struct_name = "Ice::Communicator",
    .function =
        {
            .dfree = IceRuby_Communicator_free,
        },
    .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

extern "C" VALUE
IceRuby_initialize(int argc, VALUE* argv, VALUE /*self*/)
{
    // This is the implementation of the Ice.initialize module method. There is no Ruby wrapper.

    ICE_RUBY_TRY
    {
        // The argument options are:
        //
        // Ice::initialize()
        // Ice::initialize(args)
        // Ice::initialize(initData)
        //
        // An implicit block is optional.

        if (argc > 1)
        {
            throw RubyException(rb_eArgError, "invalid number of arguments to Ice::initialize");
        }

        volatile VALUE initDataCls = callRuby(rb_path2class, "Ice::InitializationData");
        volatile VALUE args = Qnil, rbInitData = Qnil;

        if (argc == 1)
        {
            if (isArray(argv[0]))
            {
                args = argv[0];
            }
            else if (callRuby(rb_obj_is_instance_of, argv[0], initDataCls) == Qtrue)
            {
                rbInitData = argv[0];
            }
            else
            {
                throw RubyException(rb_eTypeError, "initialize expects an argument list or an InitializationData");
            }
        }

        Ice::InitializationData initData;
        Ice::SliceLoaderPtr sliceLoader = DefaultSliceLoader::instance();

        if (!NIL_P(rbInitData))
        {
            VALUE properties = callRuby(rb_iv_get, rbInitData, "@properties");
            if (!NIL_P(properties))
            {
                initData.properties = getProperties(properties);
            }

            VALUE initDataSliceLoader = callRuby(rb_iv_get, rbInitData, "@sliceLoader");
            if (!NIL_P(initDataSliceLoader))
            {
                auto compositeSliceLoader = make_shared<Ice::CompositeSliceLoader>();
                compositeSliceLoader->add(make_shared<RubySliceLoader>(initDataSliceLoader));
                compositeSliceLoader->add(std::move(sliceLoader));
                sliceLoader = std::move(compositeSliceLoader);
            }
        }
        else if (!NIL_P(args))
        {
            Ice::StringSeq seq;
            if (!arrayToStringSeq(args, seq))
            {
                throw RubyException(rb_eTypeError, "invalid array argument to Ice::initialize");
            }

            initData.properties = Ice::createProperties(seq);

            // Replace the contents of the given argument list with the filtered arguments.
            callRuby(rb_ary_clear, args);

            for (const auto& arg : seq)
            {
                volatile VALUE str = createString(arg);
                callRuby(rb_ary_push, args, str);
            }
        }

        if (!initData.properties)
        {
            initData.properties = Ice::createProperties();
        }

        // Insert the program name (stored in the Ruby global variable $0) unless already set.
        if (initData.properties->getProperty("Ice.ProgramName").empty())
        {
            volatile VALUE progName = callRuby(rb_gv_get, "$0");
            initData.properties->setProperty("Ice.ProgramName", getString(progName));
        }

        // Always accept class cycles during the unmarshaling of Ruby objects by the C++ code.
        initData.properties->setProperty("Ice.AcceptClassCycles", "1");

        // Add IceDiscovery/IceLocatorDiscovery if these plug-ins are configured via Ice.Plugin.name.
        if (!initData.properties->getIceProperty("Ice.Plugin.IceDiscovery").empty())
        {
            initData.pluginFactories.push_back(IceDiscovery::discoveryPluginFactory());
        }

        if (!initData.properties->getIceProperty("Ice.Plugin.IceLocatorDiscovery").empty())
        {
            initData.pluginFactories.push_back(IceLocatorDiscovery::locatorDiscoveryPluginFactory());
        }

        Ice::CommunicatorPtr communicator = Ice::initialize(initData);

        VALUE result = TypedData_Wrap_Struct(
            _communicatorClass,
            &IceRuby_CommunicatorType,
            new Ice::CommunicatorPtr(communicator));

        CommunicatorMap::iterator p = _communicatorMap.find(communicator);
        if (p != _communicatorMap.end())
        {
            _communicatorMap.erase(p);
        }
        _communicatorMap.insert(CommunicatorMap::value_type(communicator, reinterpret_cast<const VALUE&>(result)));

        _sliceLoaderMap[communicator] = sliceLoader;

        //
        // If an implicit block was provided, yield to the block and pass it the communicator instance.
        // We destroy the communicator after the block is finished, and return the result of the block.
        //
        if (rb_block_given_p())
        {
            Ice::CommunicatorHolder communicatorHolder{communicator};
            VALUE proc = callRuby(rb_block_proc);
            int arity = rb_proc_arity(proc);
            if (arity == 1)
            {
                return callRuby(rb_yield, result);
            }
            else
            {
                throw RubyException(rb_eArgError, "block must accept a single argument (the communicator)");
            }
        }
        else
        {
            return result;
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_stringToIdentity(VALUE /*self*/, VALUE str)
{
    ICE_RUBY_TRY
    {
        string s = getString(str);
        Ice::Identity ident = Ice::stringToIdentity(s);
        return createIdentity(ident);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_identityToString(int argc, VALUE* argv, VALUE /*self*/)
{
    ICE_RUBY_TRY
    {
        if (argc < 1 || argc > 2)
        {
            throw RubyException(rb_eArgError, "wrong number of arguments");
        }

        Ice::Identity ident = getIdentity(argv[0]);

        Ice::ToStringMode toStringMode = Ice::ToStringMode::Unicode;
        if (argc == 2)
        {
            volatile VALUE modeValue = callRuby(rb_funcall, argv[1], rb_intern("to_i"), 0);
            assert(TYPE(modeValue) == T_FIXNUM);
            toStringMode = static_cast<Ice::ToStringMode>(FIX2LONG(modeValue));
        }

        string str = Ice::identityToString(ident, toStringMode);
        return createString(str);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_destroy(VALUE self)
{
    Ice::CommunicatorPtr p = getCommunicator(self);

    ICE_RUBY_TRY { p->destroy(); }
    ICE_RUBY_CATCH

    _sliceLoaderMap.erase(p);

    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_shutdown(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        p->shutdown();
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_isShutdown(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        return p->isShutdown() ? Qtrue : Qfalse;
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_waitForShutdown(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        p->waitForShutdown();
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_stringToProxy(VALUE self, VALUE str)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        string s = getString(str);
        optional<Ice::ObjectPrx> proxy = p->stringToProxy(s);
        if (proxy)
        {
            return createProxy(proxy.value());
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_proxyToString(VALUE self, VALUE proxy)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        optional<Ice::ObjectPrx> prx;
        if (!NIL_P(proxy))
        {
            if (!checkProxy(proxy))
            {
                throw RubyException(rb_eTypeError, "argument must be a proxy");
            }
            prx = getProxy(proxy);
        }
        string str = p->proxyToString(prx);
        return createString(str);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_propertyToProxy(VALUE self, VALUE str)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        string s = getString(str);
        optional<Ice::ObjectPrx> proxy = p->propertyToProxy(s);
        if (proxy)
        {
            return createProxy(proxy.value());
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_proxyToProperty(VALUE self, VALUE obj, VALUE str)
{
    ICE_RUBY_TRY
    {
        if (!checkProxy(obj))
        {
            throw RubyException(rb_eTypeError, "argument must be a proxy");
        }
        Ice::CommunicatorPtr p = getCommunicator(self);
        optional<Ice::ObjectPrx> o = getProxy(obj);
        string s = getString(str);
        Ice::PropertyDict dict = p->proxyToProperty(o, s);
        volatile VALUE result = callRuby(rb_hash_new);
        for (Ice::PropertyDict::const_iterator q = dict.begin(); q != dict.end(); ++q)
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

extern "C" VALUE
IceRuby_Communicator_identityToString(VALUE self, VALUE id)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        Ice::Identity ident = getIdentity(id);
        string str = p->identityToString(ident);
        return createString(str);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_getImplicitContext(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        Ice::ImplicitContextPtr implicitContext = p->getImplicitContext();
        return createImplicitContext(implicitContext);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_getProperties(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        Ice::PropertiesPtr props = p->getProperties();
        return createProperties(props);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_getLogger(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        Ice::LoggerPtr logger = p->getLogger();
        return createLogger(logger);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_getDefaultRouter(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        optional<Ice::RouterPrx> router = p->getDefaultRouter();
        if (router)
        {
            volatile VALUE cls = callRuby(rb_path2class, "Ice::RouterPrx");
            assert(!NIL_P(cls));
            return createProxy(router.value(), cls);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_setDefaultRouter(VALUE self, VALUE router)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        optional<Ice::RouterPrx> proxy;
        if (!NIL_P(router))
        {
            if (!checkProxy(router))
            {
                throw RubyException(rb_eTypeError, "argument must be a proxy");
            }
            proxy = Ice::uncheckedCast<Ice::RouterPrx>(getProxy(router));
        }
        p->setDefaultRouter(proxy);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_getDefaultLocator(VALUE self)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        optional<Ice::LocatorPrx> locator = p->getDefaultLocator();
        if (locator)
        {
            volatile VALUE cls = callRuby(rb_path2class, "Ice::LocatorPrx");
            assert(!NIL_P(cls));
            return createProxy(locator.value(), cls);
        }
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_setDefaultLocator(VALUE self, VALUE locator)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);
        optional<Ice::LocatorPrx> proxy;
        if (!NIL_P(locator))
        {
            if (!checkProxy(locator))
            {
                throw RubyException(rb_eTypeError, "argument must be a proxy");
            }
            proxy = Ice::uncheckedCast<Ice::LocatorPrx>(getProxy(locator));
        }
        p->setDefaultLocator(proxy);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

extern "C" VALUE
IceRuby_Communicator_flushBatchRequests(VALUE self, VALUE compress)
{
    ICE_RUBY_TRY
    {
        Ice::CommunicatorPtr p = getCommunicator(self);

        volatile VALUE type = callRuby(rb_path2class, "Ice::CompressBatch");
        if (callRuby(rb_obj_is_instance_of, compress, type) != Qtrue)
        {
            throw RubyException(
                rb_eTypeError,
                "value for 'compress' argument must be an enumerator of Ice::CompressBatch");
        }
        volatile VALUE compressValue = callRuby(rb_funcall, compress, rb_intern("to_i"), 0);
        assert(TYPE(compressValue) == T_FIXNUM);
        Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(FIX2LONG(compressValue));
        p->flushBatchRequests(cb);
    }
    ICE_RUBY_CATCH
    return Qnil;
}

void
IceRuby::initCommunicator(VALUE iceModule)
{
    rb_define_module_function(iceModule, "initialize", CAST_METHOD(IceRuby_initialize), -1);
    rb_define_module_function(iceModule, "identityToString", CAST_METHOD(IceRuby_identityToString), -1);
    rb_define_module_function(iceModule, "stringToIdentity", CAST_METHOD(IceRuby_stringToIdentity), 1);

    _communicatorClass = rb_define_class_under(iceModule, "CommunicatorI", rb_cObject);
    rb_undef_alloc_func(_communicatorClass);
    rb_define_method(_communicatorClass, "destroy", CAST_METHOD(IceRuby_Communicator_destroy), 0);
    rb_define_method(_communicatorClass, "shutdown", CAST_METHOD(IceRuby_Communicator_shutdown), 0);
    rb_define_method(_communicatorClass, "isShutdown", CAST_METHOD(IceRuby_Communicator_isShutdown), 0);
    rb_define_method(_communicatorClass, "waitForShutdown", CAST_METHOD(IceRuby_Communicator_waitForShutdown), 0);
    rb_define_method(_communicatorClass, "stringToProxy", CAST_METHOD(IceRuby_Communicator_stringToProxy), 1);
    rb_define_method(_communicatorClass, "proxyToString", CAST_METHOD(IceRuby_Communicator_proxyToString), 1);
    rb_define_method(_communicatorClass, "propertyToProxy", CAST_METHOD(IceRuby_Communicator_propertyToProxy), 1);
    rb_define_method(_communicatorClass, "proxyToProperty", CAST_METHOD(IceRuby_Communicator_proxyToProperty), 2);
    rb_define_method(_communicatorClass, "identityToString", CAST_METHOD(IceRuby_Communicator_identityToString), 1);
    rb_define_method(_communicatorClass, "getImplicitContext", CAST_METHOD(IceRuby_Communicator_getImplicitContext), 0);
    rb_define_method(_communicatorClass, "getProperties", CAST_METHOD(IceRuby_Communicator_getProperties), 0);
    rb_define_method(_communicatorClass, "getLogger", CAST_METHOD(IceRuby_Communicator_getLogger), 0);
    rb_define_method(_communicatorClass, "getDefaultRouter", CAST_METHOD(IceRuby_Communicator_getDefaultRouter), 0);
    rb_define_method(_communicatorClass, "setDefaultRouter", CAST_METHOD(IceRuby_Communicator_setDefaultRouter), 1);
    rb_define_method(_communicatorClass, "getDefaultLocator", CAST_METHOD(IceRuby_Communicator_getDefaultLocator), 0);
    rb_define_method(_communicatorClass, "setDefaultLocator", CAST_METHOD(IceRuby_Communicator_setDefaultLocator), 1);
    rb_define_method(_communicatorClass, "flushBatchRequests", CAST_METHOD(IceRuby_Communicator_flushBatchRequests), 1);
}

Ice::CommunicatorPtr
IceRuby::getCommunicator(VALUE v)
{
    Ice::CommunicatorPtr* p = reinterpret_cast<Ice::CommunicatorPtr*>(DATA_PTR(v));
    assert(p);
    return *p;
}

VALUE
IceRuby::lookupCommunicator(const Ice::CommunicatorPtr& p)
{
    CommunicatorMap::iterator q = _communicatorMap.find(p);
    if (q != _communicatorMap.end())
    {
        return q->second;
    }
    return Qnil;
}

Ice::SliceLoaderPtr
IceRuby::lookupSliceLoader(const Ice::CommunicatorPtr& communicator)
{
    auto p = _sliceLoaderMap.find(communicator);
    assert(p != _sliceLoaderMap.end());
    return p->second;
}
