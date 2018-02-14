// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <FreezeScript/Parser.h>
#include <FreezeScript/GrammarUtil.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

using namespace std;

namespace FreezeScript
{

class EntityNodePrinter : public EntityNodeVisitor
{
public:

    EntityNodePrinter(ostream& os) :
        _os(os), _first(true)
    {
    }

    virtual void
    visitIdentifier(const std::string& name)
    {
        if(_first)
        {
            _first = false;
        }
        else
        {
            _os << '.';
        }
        _os << name;
    }

    virtual void
    visitElement(const NodePtr& value)
    {
        assert(!_first);
        _os << '[';
        value->print(_os);
        _os << ']';
    }

private:

    ostream& _os;
    bool _first;
};

} // End of namespace FreezeScript

//
// Globals required by the Bison grammar.
//
FreezeScript::DataFactoryPtr FreezeScript::parseDataFactory;
FreezeScript::ErrorReporterPtr FreezeScript::parseErrorReporter;
FreezeScript::NodePtr FreezeScript::parseResult;
int FreezeScript::parseLine;

namespace
{

string _input;
string::size_type _pos;

IceUtil::Mutex* _parserMutex = 0;

class Init
{
public:

    Init()
    {
        _parserMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete _parserMutex;
        _parserMutex = 0;
    }
};

Init init;

}

//
// parseExpression
//
FreezeScript::NodePtr
FreezeScript::parseExpression(const string& expr, const DataFactoryPtr& factory, const ErrorReporterPtr& errorReporter)
{
    //
    // The bison grammar is not thread-safe.
    //
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> sync(_parserMutex);

    parseDataFactory = factory;
    parseErrorReporter = errorReporter;
    parseLine = 1;

    parseErrorReporter->setExpression(expr);

    _input = expr;
    _pos = 0;

    int status = freeze_script_parse();
    if(status != 0)
    {
        parseResult = 0;
    }

    parseErrorReporter->clearExpression();
    parseErrorReporter = 0;

    return parseResult;
}

//
// getInput supplies characters to the lexical scanner.
//
int
FreezeScript::getInput(char* buf, int /*maxSize*/)
{
    if(_pos < _input.length())
    {
        buf[0] = _input[_pos];
        _pos++;
        return 1;
    }
    else
    {
        return 0;
    }
}

//
// EvaluateException
//
FreezeScript::EvaluateException::EvaluateException(const char* file, int line, const string& reason) :
    IceUtil::Exception(file, line), _reason(reason)
{
}

FreezeScript::EvaluateException::~EvaluateException() throw()
{
}

const char* FreezeScript::EvaluateException::_name = "FreezeScript::EvaluateException";

string
FreezeScript::EvaluateException::ice_name() const
{
    return _name;
}

void
FreezeScript::EvaluateException::ice_print(ostream& out) const
{
    Ice::Exception::ice_print(out);
    out << ":\nerror occurred while evaluating expression";
    if(!_reason.empty())
    {
        out << ":\n" << _reason;
    }
}

FreezeScript::EvaluateException*
FreezeScript::EvaluateException::ice_clone() const
{
    return new EvaluateException(ice_file(), ice_line(), _reason);
}

void
FreezeScript::EvaluateException::ice_throw() const
{
    throw *this;
}

string
FreezeScript::EvaluateException::reason() const
{
    return _reason;
}

//
// SymbolTable
//
FreezeScript::SymbolTable::~SymbolTable()
{
}

//
// Node
//
FreezeScript::Node::~Node()
{
}

//
// BinaryNode
//
FreezeScript::BinaryNode::BinaryNode(BinaryOperator op, const DataFactoryPtr& factory, const NodePtr& left,
                                  const NodePtr& right) :
    _op(op), _factory(factory), _left(left), _right(right)
{
}

FreezeScript::DataPtr
FreezeScript::BinaryNode::evaluate(const SymbolTablePtr& st)
{
    DataPtr result;

    switch(_op)
    {
    case BinOpOr:
    {
        DataPtr leftValue = _left->evaluate(st);
        if(leftValue->booleanValue())
        {
            result = leftValue;
        }
        else
        {
            result = _right->evaluate(st);
        }
        break;
    }

    case BinOpAnd:
    {
        DataPtr leftValue = _left->evaluate(st);
        if(!leftValue->booleanValue())
        {
            result = leftValue;
        }
        else
        {
            result = _right->evaluate(st);
        }
        break;
    }

    case BinOpMul:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        IntegerDataPtr ileft = IntegerDataPtr::dynamicCast(leftValue);
        IntegerDataPtr iright = IntegerDataPtr::dynamicCast(rightValue);
        if(ileft && iright)
        {
            result = _factory->createInteger(leftValue->integerValue() * rightValue->integerValue(), true);
        }
        else
        {
            result = _factory->createDouble(leftValue->doubleValue(true) * rightValue->doubleValue(true), true);
        }
        break;
    }

    case BinOpDiv:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        IntegerDataPtr ileft = IntegerDataPtr::dynamicCast(leftValue);
        IntegerDataPtr iright = IntegerDataPtr::dynamicCast(rightValue);
        if(ileft && iright)
        {
            result = _factory->createInteger(leftValue->integerValue() / rightValue->integerValue(), true);
        }
        else
        {
            result = _factory->createDouble(leftValue->doubleValue(true) / rightValue->doubleValue(true), true);
        }
        break;
    }

    case BinOpMod:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        result = _factory->createInteger(leftValue->integerValue() % rightValue->integerValue(), true);
        break;
    }

    case BinOpAdd:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        StringDataPtr sleft = StringDataPtr::dynamicCast(leftValue);
        StringDataPtr sright = StringDataPtr::dynamicCast(rightValue);
        IntegerDataPtr ileft = IntegerDataPtr::dynamicCast(leftValue);
        IntegerDataPtr iright = IntegerDataPtr::dynamicCast(rightValue);
        if(sleft || sright)
        {
            if(sleft && sright)
            {
                result = _factory->createString(leftValue->stringValue() + rightValue->stringValue(), true);
            }
            else
            {
                _factory->getErrorReporter()->error("string concatenation requires two string arguments");
            }
        }
        else if(ileft && iright)
        {
            result = _factory->createInteger(leftValue->integerValue() + rightValue->integerValue(), true);
        }
        else
        {
            result = _factory->createDouble(leftValue->doubleValue(true) + rightValue->doubleValue(true), true);
        }
        break;
    }

    case BinOpSub:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        IntegerDataPtr ileft = IntegerDataPtr::dynamicCast(leftValue);
        IntegerDataPtr iright = IntegerDataPtr::dynamicCast(rightValue);
        if(ileft && iright)
        {
            result = _factory->createInteger(leftValue->integerValue() - rightValue->integerValue(), true);
        }
        else
        {
            result = _factory->createDouble(leftValue->doubleValue(true) - rightValue->doubleValue(true), true);
        }
        break;
    }

    case BinOpLess:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        bool b = leftValue < rightValue;
        result = _factory->createBoolean(b, true);
        break;
    }

    case BinOpGreater:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        bool b = (leftValue < rightValue) || (leftValue == rightValue);
        result = _factory->createBoolean(!b, true);
        break;
    }

    case BinOpLessEq:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        bool b = (leftValue < rightValue) || (leftValue == rightValue);
        result = _factory->createBoolean(b, true);
        break;
    }

    case BinOpGrEq:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        bool b = leftValue < rightValue;
        result = _factory->createBoolean(!b, true);
        break;
    }

    case BinOpEq:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        bool b = leftValue == rightValue;
        result = _factory->createBoolean(b, true);
        break;
    }

    case BinOpNotEq:
    {
        DataPtr leftValue = _left->evaluate(st);
        DataPtr rightValue = _right->evaluate(st);
        bool b = leftValue == rightValue;
        result = _factory->createBoolean(!b, true);
        break;
    }
    }

    if(!result)
    {
        throw EvaluateException(__FILE__, __LINE__, "invalid operands to operator " + opToString(_op));
    }

    return result;
}

void
FreezeScript::BinaryNode::print(ostream& os) const
{
    os << opToString(_op) << ": left=";
    _left->print(os);
    os << ", right=";
    _right->print(os);
}

string
FreezeScript::BinaryNode::opToString(BinaryOperator op)
{
    switch(op)
    {
    case BinOpOr:
        return "OR";

    case BinOpAnd:
        return "AND";

    case BinOpMul:
        return "*";

    case BinOpDiv:
        return "/";

    case BinOpMod:
        return "%";

    case BinOpAdd:
        return "+";

    case BinOpSub:
        return "-";

    case BinOpLess:
        return "<";

    case BinOpGreater:
        return ">";

    case BinOpLessEq:
        return "<=";

    case BinOpGrEq:
        return ">=";

    case BinOpEq:
        return "==";

    case BinOpNotEq:
        return "!=";
    }

    assert(false);
    return string();
}

//
// UnaryNode
//
FreezeScript::UnaryNode::UnaryNode(UnaryOperator op, const DataFactoryPtr& factory, const NodePtr& right) :
    _op(op), _factory(factory), _right(right)
{
}

FreezeScript::DataPtr
FreezeScript::UnaryNode::evaluate(const SymbolTablePtr& st)
{
    DataPtr result;

    switch(_op)
    {
    case UnaryOpNeg:
    {
        DataPtr rightValue = _right->evaluate(st);
        IntegerDataPtr iright = IntegerDataPtr::dynamicCast(rightValue);
        if(iright)
        {
            result = _factory->createInteger(-rightValue->integerValue(), true);
        }
        else
        {
            result = _factory->createDouble(-rightValue->doubleValue(), true);
        }
        break;
    }

    case UnaryOpNot:
    {
        DataPtr rightValue = _right->evaluate(st);
        result = _factory->createBoolean(!rightValue->booleanValue(), true);
        break;
    }
    }

    if(!result)
    {
        throw EvaluateException(__FILE__, __LINE__, "invalid operand to operator " + opToString(_op));
    }

    return result;
}

void
FreezeScript::UnaryNode::print(ostream& os) const
{
    os << opToString(_op) << ": right=";
    _right->print(os);
}

string
FreezeScript::UnaryNode::opToString(UnaryOperator op)
{
    switch(op)
    {
    case UnaryOpNeg:
        return "-";

    case UnaryOpNot:
        return "!";
    }

    assert(false);
    return string();
}

//
// DataNode
//
FreezeScript::DataNode::DataNode(const DataPtr& data) :
    _data(data)
{
}

FreezeScript::DataPtr
FreezeScript::DataNode::evaluate(const SymbolTablePtr&)
{
    return _data;
}

void
FreezeScript::DataNode::print(ostream& /*os*/) const
{
    // TODO
    //_data->print(os);
}

//
// EntityNodeVisitor
//
FreezeScript::EntityNodeVisitor::~EntityNodeVisitor()
{
}

//
// EntityNode
//
FreezeScript::DataPtr
FreezeScript::EntityNode::evaluate(const SymbolTablePtr& st)
{
    DataPtr result = st->getValue(this);
    if(!result)
    {
        ostringstream ostr;
        print(ostr);
        throw EvaluateException(__FILE__, __LINE__, "unknown entity `" + ostr.str() + "'");
    }
    return result;
}

void
FreezeScript::EntityNode::print(ostream& os) const
{
    EntityNodePrinter printer(os);
    visit(printer);
}

void
FreezeScript::EntityNode::append(const EntityNodePtr& next)
{
    if(_next)
    {
        _next->append(next);
    }
    else
    {
        _next = next;
    }
}

//
// IdentNode
//
FreezeScript::IdentNode::IdentNode(const string& value) :
    _value(value)
{
}

string
FreezeScript::IdentNode::getValue() const
{
    return _value;
}

void
FreezeScript::IdentNode::visit(EntityNodeVisitor& visitor) const
{
    visitor.visitIdentifier(_value);
    if(_next)
    {
        _next->visit(visitor);
    }
}

//
// ElementNode
//
FreezeScript::ElementNode::ElementNode(const NodePtr& value) :
    _value(value)
{
}

FreezeScript::NodePtr
FreezeScript::ElementNode::getValue() const
{
    return _value;
}

void
FreezeScript::ElementNode::visit(EntityNodeVisitor& visitor) const
{
    visitor.visitElement(_value);
    if(_next)
    {
        _next->visit(visitor);
    }
}

//
// FunctionNode
//
FreezeScript::FunctionNode::FunctionNode(const string& name, const NodeList& args) :
    _name(name), _args(args)
{
}

FreezeScript::DataPtr
FreezeScript::FunctionNode::evaluate(const SymbolTablePtr& st)
{
    DataPtr target;
    if(_target)
    {
        target = _target->evaluate(st);
    }
    DataList args;
    for(NodeList::iterator p = _args.begin(); p != _args.end(); ++p)
    {
        args.push_back((*p)->evaluate(st));
    }
    return st->invokeFunction(_name, target, args);
}

void
FreezeScript::FunctionNode::print(ostream& os) const
{
    if(_target)
    {
        _target->print(os);
        os << '.';
    }
    os << _name << '(';
    for(NodeList::const_iterator p = _args.begin(); p != _args.end(); ++p)
    {
        if(p != _args.begin())
        {
            os << ", ";
        }
        (*p)->print(os);
    }
    os << ')';
}

void
FreezeScript::FunctionNode::setTarget(const EntityNodePtr& target)
{
    _target = target;
}

//
// ConstantNode
//
FreezeScript::ConstantNode::ConstantNode(const string& value) :
    _value(value)
{
}

FreezeScript::DataPtr
FreezeScript::ConstantNode::evaluate(const SymbolTablePtr& st)
{
    DataPtr result = st->getConstantValue(_value);
    if(!result)
    {
        throw EvaluateException(__FILE__, __LINE__, "unknown constant `" + _value + "'");
    }
    return result;
}

void
FreezeScript::ConstantNode::print(ostream& os) const
{
    os << _value;
}

//
// Stream insertion for an entity node.
//
ostream&
operator<<(ostream& os, const FreezeScript::EntityNodePtr& entity)
{
    FreezeScript::EntityNodePrinter printer(os);
    entity->visit(printer);
    return os;
}
