// Copyright (c) ZeroC, Inc.

#include "Ice/Value.h"
#include "Ice/Demangle.h"
#include "Ice/InputStream.h"
#include "Ice/LocalExceptions.h"
#include "Ice/OutputStream.h"
#include "Ice/SlicedData.h"

#include <algorithm>
#include <deque>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    // RAII helper class that detects cycles when printing class instances.
    class CycleChecker final
    {
    public:
        CycleChecker(const Value* value, ostream& os) : _os(os)
        {
            // Retrieve current stack, if any.
            void*& currentVoidStar = _os.pword(_cycleCheckIndex);
            if (currentVoidStar)
            {
                auto* currentStack = static_cast<deque<const Value*>*>(currentVoidStar);
                if (find(currentStack->begin(), currentStack->end(), value) == currentStack->end())
                {
                    // No cycle detected.
                    _currentStack = currentStack;
                }
                // else _currentStack remains null and CycleChecker doesn't do anything.
            }
            else // new stack
            {
                _currentStack = &_localStack;
                currentVoidStar = _currentStack;
            }

            if (_currentStack)
            {
                _currentStack->push_front(value);
            }
        }

        ~CycleChecker()
        {
            if (_currentStack)
            {
                _currentStack->pop_front();

                if (_currentStack == &_localStack)
                {
                    // Clear value in os.
                    _os.pword(_cycleCheckIndex) = nullptr;
                }
            }
            assert(_localStack.size() == 0);
        }

        [[nodiscard]] bool good() const noexcept { return _currentStack; }

    private:
        ostream& _os;
        deque<const Value*> _localStack;
        deque<const Value*>* _currentStack{nullptr}; // may or may not point to _localStack

        static const int _cycleCheckIndex;
    };

    const int CycleChecker::_cycleCheckIndex = ios_base::xalloc(); // NOLINT(cert-err58-cpp)
}

Ice::Value::~Value() = default;

void
Ice::Value::ice_print(ostream& os) const
{
    string className{demangle(typeid(*this).name())};
    // On Windows, the class name is prefixed with "class "; we removed it.
    if (className.compare(0, 6, "class ") == 0)
    {
        className.erase(0, 6);
    }

    os << className << '{';
    const CycleChecker cycleChecker{this, os};
    if (cycleChecker.good())
    {
        ice_printFields(os);
    }
    else
    {
        os << "...already printed...";
    }
    os << '}';
}

void
Ice::Value::ice_printFields(ostream&) const
{
}

void
Ice::Value::_iceWrite(Ice::OutputStream* os) const
{
    os->startValue(_slicedData);
    _iceWriteImpl(os);
    os->endValue();
}

void
Ice::Value::_iceRead(Ice::InputStream* is)
{
    is->startValue();
    _iceReadImpl(is);
    _slicedData = is->endValue();
}

const char*
Ice::Value::ice_id() const noexcept
{
    return ice_staticId();
}

const char*
Ice::Value::ice_staticId() noexcept
{
    return "::Ice::Object";
}

SlicedDataPtr
Ice::Value::ice_getSlicedData() const
{
    return _slicedData;
}

Ice::ValuePtr
Ice::Value::_iceCloneImpl() const
{
    throw std::logic_error("clone is not implemented for this class");
}
