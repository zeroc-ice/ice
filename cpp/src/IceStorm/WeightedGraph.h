// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef WEIGHTED_GRAPH_H
#define WEIGHTED_GRAPH_H

#include <IceUtil/Config.h>
#include <vector>
#include <list>
#include <map>

namespace IceStorm
{

//
// Forward declaration.
//
class SAXGraphHandler;

//
// A weighted graph. This class uses internally a vertex, adjacency
// matrix representation. Each vertex has a name. The [i,j] of the
// adjacency graph represents the the edge from _vertices[i] to
// _vertices[j].
//
class WeightedGraph
{
public:

    WeightedGraph(bool = false);
    ~WeightedGraph();

    //
    // Parse the graph from the given XML file. Returns false in event
    // of a parse error.
    //
    bool parse(const std::string&);

    //
    // Compute a new adjacency matrix with the given cost.
    //
    void compute(std::vector<int>&, int);

    //
    // Render the edge table in ASCII format.
    //
    void dump(std::ostream&);

    //
    // Swap the adjacency matrix. This could be actual swap.
    //
    void swap(const std::vector<int>&);

    //
    // Get all the vertices.
    //
    std::vector<std::string> getVertices() const;

    //
    // Get the set of edges for a vertex. The return data is the name
    // of the second vertex, and the cost.
    //
    std::vector<std::pair<std::string, int> > getEdgesFor(const std::string&) const;

private:

    //
    // Recursive function used in computation of new edge set.
    //
    void visit(unsigned int, int, std::list<std::pair<unsigned int, int> >&, int);

    //
    // Callbacks from parsing routines
    //
    void addVertex(const std::string&);
    void addEdge(const std::string&, const std::string&, int);

    void error();

    //
    // List of vertices.
    //
    std::vector<std::string> _vertices;

    //
    // Lookup table - maps from vertex name to vertex index.
    //
    std::map<std::string, int> _vlookup;

    //
    // The adjacency matrix. -1 indicates no edge, otherwise the value
    // is the cost.
    //
    std::vector<int> _edges;

    //
    // Is the graph reflective? That is if there is edge (v1, v2,
    // cost) then there is (v2, v1, cost).
    //
    bool _reflective;

    int _error;

    friend class GraphHandler;
};

} // End namespace IceStorm

#endif
