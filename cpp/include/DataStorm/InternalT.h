// Copyright (c) ZeroC, Inc.

#pragma once

#include "Config.h"
#include "Ice/Demangle.h"
#include "Ice/Ice.h"
#include "InternalI.h"
#include "Types.h"

#if defined(__clang__)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow-field-in-constructor"
#elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#endif

namespace DataStorm
{
    template<typename K, typename V, typename U> class Sample;
}

namespace DataStormI
{
    template<typename T> class has_communicator_parameter
    {
        template<typename TT, typename SS>
        static auto testE(int) noexcept
            -> decltype(TT::encode(std::declval<Ice::CommunicatorPtr&>(), std::declval<SS&>()), std::true_type());

        template<typename, typename> static auto testE(...) -> std::false_type;

        template<typename TT, typename SS>
        static auto testD(int) noexcept
            -> decltype(TT::decode(std::declval<Ice::CommunicatorPtr&>(), Ice::ByteSeq()), std::true_type());

        template<typename, typename> static auto testD(...) -> std::false_type;

    public:
        static constexpr bool value =
            decltype(testE<DataStorm::Encoder<T>, T>(0))::value && decltype(testD<DataStorm::Decoder<T>, T>(0))::value;
    };

    template<typename T, typename Enabler = void> struct EncoderT
    {
        static Ice::ByteSeq encode(const Ice::CommunicatorPtr&, const T& value)
        {
            return DataStorm::Encoder<T>::encode(value);
        }
    };

    template<typename T, typename Enabler = void> struct DecoderT
    {
        static T decode(const Ice::CommunicatorPtr&, const Ice::ByteSeq& data)
        {
            return DataStorm::Decoder<T>::decode(data);
        }
    };

    template<typename T> struct EncoderT<T, std::enable_if_t<has_communicator_parameter<T>::value>>
    {
        static Ice::ByteSeq encode(const Ice::CommunicatorPtr& communicator, const T& value)
        {
            return DataStorm::Encoder<T>::encode(communicator, value);
        }
    };

    template<typename T> struct DecoderT<T, std::enable_if_t<has_communicator_parameter<T>::value>>
    {
        static T decode(const Ice::CommunicatorPtr& communicator, const Ice::ByteSeq& data)
        {
            return DataStorm::Decoder<T>::decode(communicator, data);
        }
    };

    template<typename T> class is_streamable
    {
        template<typename TT, typename SS>
        static auto test(int) noexcept -> decltype(std::declval<SS&>() << std::declval<TT>(), std::true_type());

        template<typename, typename> static auto test(...) noexcept -> std::false_type;

    public:
        static constexpr bool value = decltype(test<T, std::ostream>(0))::value;
    };

    template<typename T, typename Enabler = void> struct Stringifier
    {
        static std::string toString(const T& value)
        {
            std::ostringstream os;
            os << IceInternal::demangle(typeid(value).name()) << '(' << &value << ')';
            return os.str();
        }
    };

    template<typename T> struct Stringifier<T, std::enable_if_t<is_streamable<T>::value>>
    {
        static std::string toString(const T& value)
        {
            std::ostringstream os;
            os << value;
            return os.str();
        }
    };

    template<typename T> class AbstractElementT : public virtual Element
    {
    public:
        template<typename TT> AbstractElementT(TT&& v, std::int64_t id) : _value(std::forward<TT>(v)), _id(id) {}

        [[nodiscard]] std::string toString() const override
        {
            std::ostringstream os;
            os << _id << ':' << Stringifier<T>::toString(_value);
            return os.str();
        }

        [[nodiscard]] Ice::ByteSeq encode(const Ice::CommunicatorPtr& communicator) const override
        {
            return EncoderT<T>::encode(communicator, _value);
        }

        [[nodiscard]] std::int64_t getId() const override { return _id; }

        [[nodiscard]] const T& get() const { return _value; }

    protected:
        const T _value;
        const std::int64_t _id;
    };

    template<typename K, typename V>
    class AbstractFactoryT : public std::enable_shared_from_this<AbstractFactoryT<K, V>>
    {
        /// A custom deleter to remove the element from the factory when the shared_ptr is deleted.
        /// The deleter is used by elements created by the factory.
        struct Deleter
        {
            void operator()(V* obj)
            {
                if (auto factory = _factory.lock())
                {
                    factory->remove(obj);
                }
                delete obj;
            }

            std::weak_ptr<AbstractFactoryT<K, V>> _factory;

        } _deleter;

    public:
        AbstractFactoryT() = default;

        void init() { _deleter = Deleter{std::enable_shared_from_this<AbstractFactoryT<K, V>>::shared_from_this()}; }

        template<typename F, typename... Args>
        [[nodiscard]] std::shared_ptr<typename V::BaseClassType> create(F&& value, Args&&... args)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return createImpl(std::forward<F>(value), std::forward<Args>(args)...);
        }

        [[nodiscard]] std::vector<std::shared_ptr<typename V::BaseClassType>> create(std::vector<K> values)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<std::shared_ptr<typename V::BaseClassType>> seq;
            seq.reserve(values.size());
            for (auto& v : values)
            {
                seq.push_back(createImpl(std::move(v)));
            }
            return seq;
        }

    protected:
        friend struct Deleter;

        [[nodiscard]] std::shared_ptr<typename V::BaseClassType> getImpl(std::int64_t id) const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto p = _elementsById.find(id);
            if (p != _elementsById.end())
            {
                return p->second.lock();
            }
            return nullptr;
        }

        template<typename F, typename... Args> [[nodiscard]] std::shared_ptr<V> createImpl(F&& value, Args&&... args)
        {
            // Called with _mutex locked

            auto p = _elements.find(value);
            if (p != _elements.end())
            {
                auto k = p->second.lock();
                if (k)
                {
                    return k;
                }

                // The key is being removed concurrently by the deleter, remove it now to allow the insertion of a new
                // key. The deleter won't remove the new key.
                _elements.erase(p);
            }

            auto k =
                std::shared_ptr<V>(new V(std::forward<F>(value), std::forward<Args>(args)..., ++_nextId), _deleter);
            _elements[k->get()] = k;
            _elementsById[k->getId()] = k;
            return k;
        }

        void remove(V* v)
        {
            // Make sure to declare the variable outside the synchronization in case the element needs
            // to be deleted if it's not the same.
            std::shared_ptr<V> e;
            std::lock_guard<std::mutex> lock(_mutex);
            auto p = _elements.find(v->get());
            if (p != _elements.end())
            {
                e = p->second.lock();
                if (e && e.get() == v)
                {
                    _elements.erase(p);
                }
            }
            _elementsById.erase(v->getId());
        }

        mutable std::mutex _mutex;
        std::map<K, std::weak_ptr<V>> _elements;
        std::map<std::int64_t, std::weak_ptr<V>> _elementsById;
        std::int64_t _nextId{1};
    };

    template<typename K> class KeyT final : public Key, public AbstractElementT<K>
    {
    public:
        [[nodiscard]] std::string toString() const final { return "k" + AbstractElementT<K>::toString(); }

        using AbstractElementT<K>::AbstractElementT;
        using BaseClassType = Key;
    };

    template<typename K> class KeyFactoryT final : public KeyFactory, public AbstractFactoryT<K, KeyT<K>>
    {
    public:
        using AbstractFactoryT<K, KeyT<K>>::AbstractFactoryT;

        [[nodiscard]] std::shared_ptr<Key> get(std::int64_t id) const final
        {
            return AbstractFactoryT<K, KeyT<K>>::getImpl(id);
        }

        [[nodiscard]] std::shared_ptr<Key>
        decode(const Ice::CommunicatorPtr& communicator, const Ice::ByteSeq& data) final
        {
            return AbstractFactoryT<K, KeyT<K>>::create(DecoderT<K>::decode(communicator, data));
        }

        [[nodiscard]] static std::shared_ptr<KeyFactoryT<K>> createFactory()
        {
            auto f = std::make_shared<KeyFactoryT<K>>();
            f->init();
            return f;
        }
    };

    template<typename T> class TagT final : public Tag, public AbstractElementT<T>
    {
    public:
        [[nodiscard]] std::string toString() const final { return "t" + AbstractElementT<T>::toString(); }

        using AbstractElementT<T>::AbstractElementT;
        using BaseClassType = Tag;
    };

    template<typename T> class TagFactoryT final : public TagFactory, public AbstractFactoryT<T, TagT<T>>
    {
    public:
        using AbstractFactoryT<T, TagT<T>>::AbstractFactoryT;

        [[nodiscard]] std::shared_ptr<Tag> get(std::int64_t id) const final
        {
            return AbstractFactoryT<T, TagT<T>>::getImpl(id);
        }

        [[nodiscard]] std::shared_ptr<Tag>
        decode(const Ice::CommunicatorPtr& communicator, const Ice::ByteSeq& data) final
        {
            return AbstractFactoryT<T, TagT<T>>::create(DecoderT<T>::decode(communicator, data));
        }

        [[nodiscard]] static std::shared_ptr<TagFactoryT<T>> createFactory()
        {
            auto f = std::make_shared<TagFactoryT<T>>();
            f->init();
            return f;
        }
    };

    template<typename Key, typename Value, typename UpdateTag>
    class SampleT final : public Sample, public std::enable_shared_from_this<SampleT<Key, Value, UpdateTag>>
    {
    public:
        SampleT(
            std::string session,
            std::string origin,
            std::int64_t id,
            DataStorm::SampleEvent event,
            const std::shared_ptr<DataStormI::Key>& key,
            const std::shared_ptr<DataStormI::Tag>& tag,
            Ice::ByteSeq value,
            std::int64_t timestamp)
            : Sample(std::move(session), std::move(origin), id, event, key, tag, std::move(value), timestamp),
              _hasValue(false)
        {
        }

        SampleT(DataStorm::SampleEvent event) : Sample(event), _hasValue(false) {}

        SampleT(DataStorm::SampleEvent event, Value value) : Sample(event), _hasValue(true), _value(std::move(value)) {}

        SampleT(Ice::ByteSeq value, const std::shared_ptr<Tag>& tag)
            : Sample(DataStorm::SampleEvent::PartialUpdate, tag),
              _hasValue(false)
        {
            _encodedValue = std::move(value);
        }

        [[nodiscard]] DataStorm::Sample<Key, Value, UpdateTag> get()
        {
            auto impl = std::enable_shared_from_this<SampleT<Key, Value, UpdateTag>>::shared_from_this();
            return DataStorm::Sample<Key, Value, UpdateTag>(impl);
        }

        [[nodiscard]] const Key& getKey()
        {
            assert(key);
            return std::static_pointer_cast<KeyT<Key>>(key)->get();
        }

        [[nodiscard]] const Value& getValue() const { return _value; }

        [[nodiscard]] UpdateTag getTag() const
        {
            return tag ? std::static_pointer_cast<TagT<UpdateTag>>(tag)->get() : UpdateTag();
        }

        void setValue(Value value)
        {
            _value = std::move(value);
            _hasValue = true;
        }

        [[nodiscard]] bool hasValue() const final { return _hasValue; }

        void setValue(const std::shared_ptr<Sample>& sample) final
        {
            if (sample)
            {
                _value = DataStorm::Cloner<Value>::clone(
                    std::static_pointer_cast<DataStormI::SampleT<Key, Value, UpdateTag>>(sample)->getValue());
            }
            else
            {
                _value = Value{};
            }
            _hasValue = true;
        }

        [[nodiscard]] const Ice::ByteSeq& encode(const Ice::CommunicatorPtr& communicator) final
        {
            if (_encodedValue.empty())
            {
                _encodedValue = encodeValue(communicator);
            }
            return _encodedValue;
        }

        [[nodiscard]] Ice::ByteSeq encodeValue(const Ice::CommunicatorPtr& communicator) final
        {
            assert(_hasValue || event == DataStorm::SampleEvent::Remove);
            return EncoderT<Value>::encode(communicator, _value);
        }

        void decode(const Ice::CommunicatorPtr& communicator) final
        {
            if (!_encodedValue.empty())
            {
                _hasValue = true;
                _value = DecoderT<Value>::decode(communicator, _encodedValue);
                _encodedValue.clear();
            }
        }

    private:
        bool _hasValue;
        Value _value;
    };

    template<typename Key, typename Value, typename UpdateTag> class SampleFactoryT final : public SampleFactory
    {
    public:
        [[nodiscard]] std::shared_ptr<Sample> create(
            std::string session,
            std::string origin,
            std::int64_t id,
            DataStorm::SampleEvent type,
            const std::shared_ptr<DataStormI::Key>& key,
            const std::shared_ptr<DataStormI::Tag>& tag,
            Ice::ByteSeq value,
            std::int64_t timestamp) final
        {
            return std::make_shared<SampleT<Key, Value, UpdateTag>>(
                std::move(session),
                std::move(origin),
                id,
                type,
                key,
                tag,
                std::move(value),
                timestamp);
        }
    };

    template<typename C, typename V> class FilterT final : public Filter, public AbstractElementT<C>
    {
    public:
        template<typename CC, typename FF>
        FilterT(CC&& criteria, std::string name, FF lambda, std::int64_t id)
            : AbstractElementT<C>::AbstractElementT(std::forward<CC>(criteria), id),
              _name(std::move(name)),
              _lambda(std::move(lambda))
        {
        }

        [[nodiscard]] std::string toString() const final { return "f" + AbstractElementT<C>::toString(); }

        [[nodiscard]] bool match(const std::shared_ptr<Filterable>& value) const final
        {
            return _lambda(std::static_pointer_cast<V>(value)->get());
        }

        [[nodiscard]] const std::string& getName() const final { return _name; }

        using BaseClassType = Filter;

    private:
        std::string _name;
        std::function<bool(const typename std::remove_reference<decltype(std::declval<V>().get())>::type&)> _lambda;
    };

    template<typename C, typename V>
    class FilterFactoryT final : public FilterFactory, public AbstractFactoryT<C, FilterT<C, V>>
    {
    public:
        FilterFactoryT() = default;

        [[nodiscard]] std::shared_ptr<Filter> get(std::int64_t id) const final
        {
            return AbstractFactoryT<C, FilterT<C, V>>::getImpl(id);
        }

        [[nodiscard]] static std::shared_ptr<FilterFactoryT<C, V>> createFactory()
        {
            auto f = std::make_shared<FilterFactoryT<C, V>>();
            f->init();
            return f;
        }
    };

    template<typename ValueT> class FilterManagerT final : public FilterManager
    {
        using Value = std::remove_reference_t<decltype(std::declval<ValueT>().get())>;

        struct Factory
        {
            virtual ~Factory() = default;

            [[nodiscard]] virtual std::shared_ptr<Filter> get(std::int64_t) const = 0;

            [[nodiscard]] virtual std::shared_ptr<Filter> decode(const Ice::CommunicatorPtr&, const Ice::ByteSeq&) = 0;
        };

        template<typename Criteria> struct FactoryT final : Factory
        {
            FactoryT(std::string name, std::function<std::function<bool(const Value&)>(const Criteria&)> lambda)
                : name(std::move(name)),
                  lambda(std::move(lambda))
            {
            }

            [[nodiscard]] std::shared_ptr<Filter> create(const Criteria& criteria)
            {
                return std::static_pointer_cast<FilterT<Criteria, ValueT>>(
                    filterFactory.create(criteria, name, lambda(criteria)));
            }

            [[nodiscard]] std::shared_ptr<Filter> get(std::int64_t id) const final { return filterFactory.get(id); }

            [[nodiscard]] std::shared_ptr<Filter>
            decode(const Ice::CommunicatorPtr& communicator, const Ice::ByteSeq& data) final
            {
                return create(DecoderT<Criteria>::decode(communicator, data));
            }

            const std::string name;
            std::function<std::function<bool(const Value&)>(const Criteria&)> lambda;
            FilterFactoryT<Criteria, ValueT> filterFactory;
        };

    public:
        template<typename Criteria>
        [[nodiscard]] std::shared_ptr<Filter> create(const std::string& name, const Criteria& criteria)
        {
            auto p = _factories.find(name);
            if (p == _factories.end())
            {
                throw std::invalid_argument("unknown filter '" + name + "'");
            }

            auto factory = dynamic_cast<FactoryT<Criteria>*>(p->second.get());
            if (!factory)
            {
                throw std::invalid_argument("filter '" + name + "' type doesn't match");
            }

            return factory->create(criteria);
        }

        [[nodiscard]] std::shared_ptr<Filter>
        decode(const Ice::CommunicatorPtr& communicator, const std::string& name, const Ice::ByteSeq& data) final
        {
            auto p = _factories.find(name);
            if (p == _factories.end())
            {
                return nullptr;
            }

            return p->second->decode(communicator, data);
        }

        [[nodiscard]] std::shared_ptr<Filter> get(const std::string& name, std::int64_t id) const final
        {
            auto p = _factories.find(name);
            if (p == _factories.end())
            {
                return nullptr;
            }

            return p->second->get(id);
        }

        template<typename Criteria>
        void set(std::string name, std::function<std::function<bool(const Value&)>(const Criteria&)> lambda)
        {
            if (lambda)
            {
                auto factory = std::make_unique<FactoryT<Criteria>>(name, std::move(lambda));
                _factories.emplace(std::move(name), std::move(factory));
            }
            else
            {
                _factories.erase(name);
            }
        }

    private:
        // A map containing the filter factories, indexed by the filter name.
        std::map<std::string, std::unique_ptr<Factory>> _factories;
    };
}

#if defined(__clang__)
#    pragma clang diagnostic pop
#elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
