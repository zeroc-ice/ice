// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Transform/Node.h>

using namespace std;

namespace Transform
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

} // End of namespace Transform

ostream&
operator<<(ostream& os, const Transform::EntityNodePtr& entity)
{
    Transform::EntityNodePrinter printer(os);
    entity->visit(printer);
    return os;
}

//
// EvaluateException
//
Transform::EvaluateException::EvaluateException(const char* file, int line, const string& reason) :
    IceUtil::Exception(file, line), _reason(reason)
{
}

string
Transform::EvaluateException::ice_name() const
{
    return "Transform::EvaluateException";
}

void
Transform::EvaluateException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
    out << ":\nerror occurred while evaluating expression";
    if(!_reason.empty())
    {
        out << ":\n" << _reason;
    }
}

IceUtil::Exception*
Transform::EvaluateException::ice_clone() const
{
    return new EvaluateException(ice_file(), ice_line(), _reason);
}

void
Transform::EvaluateException::ice_throw() const
{
    throw *this;
}

string
Transform::EvaluateException::reason() const
{
    return _reason;
}

//
// SymbolTable
//
Transform::SymbolTable::~SymbolTable()
{
}

//
// Node
//
Transform::Node::~Node()
{
}

//
// BinaryNode
//
Transform::BinaryNode::BinaryNode(BinaryOperator op, const DataFactoryPtr& factory, const NodePtr& left,
                                  const NodePtr& right) :
    _op(op), _factory(factory), _left(left), _right(right)
{
}

Transform::DataPtr
Transform::BinaryNode::evaluate(SymbolTable& st)
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
        IntegerDataPtr ileft = IntegerDataPtr::dynamicCast(leftValue);
        IntegerDataPtr iright = IntegerDataPtr::dynamicCast(rightValue);
        if(ileft && iright)
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
    }

    if(!result)
    {
        throw EvaluateException(__FILE__, __LINE__, "invalid operands to operator " + opToString(_op));
    }

    return result;
}

void
Transform::BinaryNode::print(ostream& os) const
{
    os << opToString(_op) << ": left=";
    _left->print(os);
    os << ", right=";
    _right->print(os);
}

string
Transform::BinaryNode::opToString(BinaryOperator op)
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
    }

    assert(false);
    return string();
}

//
// UnaryNode
//
Transform::UnaryNode::UnaryNode(UnaryOperator op, const DataFactoryPtr& factory, const NodePtr& right) :
    _op(op), _factory(factory), _right(right)
{
}

Transform::DataPtr
Transform::UnaryNode::evaluate(SymbolTable& st)
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
Transform::UnaryNode::print(ostream& os) const
{
    os << opToString(_op) << ": right=";
    _right->print(os);
}

string
Transform::UnaryNode::opToString(UnaryOperator op)
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
Transform::DataNode::DataNode(const DataPtr& data) :
    _data(data)
{
}

Transform::DataPtr
Transform::DataNode::evaluate(SymbolTable&)
{
    return _data;
}

void
Transform::DataNode::print(ostream& os) const
{
    _data->print(os);
}

//
// EntityNodeVisitor
//
Transform::EntityNodeVisitor::~EntityNodeVisitor()
{
}

//
// EntityNode
//
Transform::DataPtr
Transform::EntityNode::evaluate(SymbolTable& st)
{
    DataPtr result = st.getValue(this);
    if(!result)
    {
        ostringstream ostr;
        print(ostr);
        throw EvaluateException(__FILE__, __LINE__, "unknown entity `" + ostr.str() + "'");
    }
    return result;
}

void
Transform::EntityNode::print(ostream& os) const
{
    EntityNodePrinter printer(os);
    visit(printer);
}

void
Transform::EntityNode::append(const EntityNodePtr& next)
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
Transform::IdentNode::IdentNode(const string& value) :
    _value(value)
{
}

string
Transform::IdentNode::getValue() const
{
    return _value;
}

void
Transform::IdentNode::visit(EntityNodeVisitor& visitor) const
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
Transform::ElementNode::ElementNode(const NodePtr& value) :
    _value(value)
{
}

Transform::NodePtr
Transform::ElementNode::getValue() const
{
    return _value;
}

void
Transform::ElementNode::visit(EntityNodeVisitor& visitor) const
{
    visitor.visitElement(_value);
    if(_next)
    {
        _next->visit(visitor);
    }
}

//
// ConstantNode
//
Transform::ConstantNode::ConstantNode(const string& value) :
    _value(value)
{
}

Transform::DataPtr
Transform::ConstantNode::evaluate(SymbolTable& st)
{
    DataPtr result = st.getConstantValue(_value);
    if(!result)
    {
        throw EvaluateException(__FILE__, __LINE__, "unknown constant `" + _value + "'");
    }
    return result;
}

void
Transform::ConstantNode::print(ostream& os) const
{
    os << _value;
}
