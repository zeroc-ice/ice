
[["js:es6-module"]]

#pragma once

#include <Demo/Square.ice>
#include <Demo/Circle.ice>
#include <Glacier2/Session.ice>

module Demo
{

module gx
{

interface Canvas
{
    void paintSquare(Square square);
    void paintCircle(Circle circle);

};

interface Session extends Glacier2::Session
{
    void destroySession();
};

};

};