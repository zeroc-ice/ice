// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef NODE_I_H
#define NODE_I_H

#include <Complex.h>
#include <Ice/ObjectFactory.h>

namespace Complex
{

class NumberNodeI : public NumberNode
{
public:

    NumberNodeI()
    {
    }

    NumberNodeI(int number)
    {
	this->number = number;
    }

    virtual int calc(const Ice::Current&)
    {
	return number;
    }
};

class AddNodeI : public AddNode
{
public:

    AddNodeI()
    {
    }

    AddNodeI(const NodePtr& left, const NodePtr& right)
    {
	this->left = left;
	this->right = right;
    }

    virtual int calc(const Ice::Current&)
    {
	return left->calc() + right->calc();
    }
};

class MultiplyNodeI : public MultiplyNode
{
public:

    MultiplyNodeI()
    {
    }

    MultiplyNodeI(const NodePtr& left, const NodePtr& right)
    {
	this->left = left;
	this->right = right;
    }

    virtual int calc(const Ice::Current&)
    {
	return left->calc() * right->calc();
    }
};

class ObjectFactoryI : public Ice::ObjectFactory
{
public:

    virtual Ice::ObjectPtr create(const std::string& type)
    {
	if(type == "::Complex::MultiplyNode")
	{
	    return new MultiplyNodeI();
	}
	if(type == "::Complex::AddNode")
	{
	    return new AddNodeI();
	}
	if(type == "::Complex::NumberNode")
	{
	    return new NumberNodeI();
	}
	std::cout << "create: " << type << std::endl;
	assert(false);
	return 0;
    }

    virtual void destroy()
    {
	// Nothing to do
    }
};

} // End namespace Complex

#endif
