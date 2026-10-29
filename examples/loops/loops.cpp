//**********************************************************
//Copyright 2026 Tabish Syed
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.
//**********************************************************
//
// Created by tabish on 2026-10-27.
//

#include "loops.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>

template<typename AdjecencyValueType>
bool find_loop(std::vector<std::vector<AdjecencyValueType>>& adjMatrix, int parent, int current, std::list<int>& path) {
    path.push_back(current);
    auto& destRow = adjMatrix[current];
    
    for (size_t nbr = 0; nbr < destRow.size(); ++nbr) {
        if (nbr != static_cast<size_t>(parent) && destRow[nbr] > 0) { // for each non-parent neighbour
            if (std::find(path.begin(), path.end(), nbr) != path.end()) { // loop detected
                return true;
            } else {
                if (find_loop(adjMatrix, current, static_cast<int>(nbr), path)) {
                    return true;
                }
            }
        }
    }
    
    assert(current == path.back());
    path.pop_back();
    return false;
}
template bool find_loop<bool>(std::vector<std::vector<bool>>&, int, int, std::list<int>&);

template<unsigned Dimension>
bool find_loop(const std::vector<std::list<size_t>>& loopyAdjList, int parentIndex, int currentIndex, std::list<size_t>& path) {
    path.push_back(currentIndex);
    
    for (size_t nbrIndex : loopyAdjList[currentIndex]) {
        if (nbrIndex != static_cast<size_t>(parentIndex)) { 
            if (std::find(path.begin(), path.end(), nbrIndex) != path.end()) { 
                return true;
            } else {
                if (find_loop<Dimension>(loopyAdjList, currentIndex, static_cast<int>(nbrIndex), path)) {
                    return true;
                }
            }
        }
    }
    
    assert(static_cast<size_t>(currentIndex) == path.back());
    path.pop_back();
    return false;
}
template bool find_loop<3>(const std::vector<std::list<size_t>>&, int, int, std::list<size_t>&);

template<unsigned Dimension>
std::vector<std::list<size_t>> findSimpleGraphLoops(Graph<Dimension>& graph) {
    kruskal_mst<Dimension>(graph);
    
    auto constructAdjList = [&graph]() -> std::vector<std::list<size_t>> {
        std::vector<std::list<size_t>> subAdjList(graph.getVertexCount());
        for (const auto& edge : graph.edges) {
            if (edge.label == 1) {
                size_t fromIndex = graph.getVertexIndex(edge.u);
                size_t toIndex = graph.getVertexIndex(edge.v);
                subAdjList[fromIndex].push_back(toIndex);
                if (edge.undirected) {
                    subAdjList[toIndex].push_back(fromIndex);
                }
            }
        }
        return subAdjList;
    };

    std::vector<std::list<size_t>> loops;
    
    for (auto& edge : graph.edges) {
        if (edge.label != 1) { // Pick unincluded edge
            edge.label = 1;    // Temporarily add edge to MST
            std::vector<std::list<size_t>> loopySubList = constructAdjList();
            edge.label = 0;    // Remove edge again
            
            std::list<size_t> path;
            size_t uIndex = graph.getVertexIndex(edge.u);
            
            // Extract execution outside the assert to prevent failure in Release builds
            bool foundLoop = find_loop<Dimension>(loopySubList, static_cast<int>(uIndex), static_cast<int>(uIndex), path);
            assert(foundLoop);
            
            if (foundLoop) {
                loops.push_back(path);
            }
        }
    }
    return loops;
}
template std::vector<std::list<size_t>> findSimpleGraphLoops<3>(Graph<3>& graph);

template<unsigned Dimension>
Graph<Dimension> extractLabelledSubGraph(Graph<Dimension>& edgeLabelledGraph, int label) {
    Graph<Dimension> subgraph;
    for (const auto& edge : edgeLabelledGraph.edges) {
        if (edge.label != label) continue;
        
        if (!subgraph.findVertexById(edge.u)) {
            subgraph.addVertex(edgeLabelledGraph.getVertexById(edge.u));
        }
        if (!subgraph.findVertexById(edge.v)) {
            subgraph.addVertex(edgeLabelledGraph.getVertexById(edge.v));
        }
        subgraph.addLabelledEdge(edge.u, edge.v, label);
    }
    return subgraph;
}
template Graph<3> extractLabelledSubGraph<3>(Graph<3>& edgeLabelledGraph, int label);

void parseGraph(Graph<3>& graph, std::ifstream& graphFile, int pass) {
    unsigned N;
    if (!(graphFile >> N)) return; // vertex_count
    
    for (size_t i = 0; i < N; ++i) {
        std::array<double, 3> location{};
        unsigned nbrCount, nbr;
        
        for (size_t d = 0; d < 3; ++d) {
            graphFile >> location[d];
        }
        if (pass == 1) {
            graph.addVertex(Vertex<3>(i, location));
        }
        
        graphFile >> nbrCount;
        for (size_t n = 0; n < nbrCount; ++n) {
            graphFile >> nbr;
            if (pass == 2) {
                graph.addUndirectedEdge(i, nbr);
            }
        }
    }
}

void readAdjList(Graph<3>& graph, const fs::path& adjecencyListPath) {
    std::ifstream graphFile(adjecencyListPath);
    if (!graphFile.is_open()) return;
    
    //std::cout << "First pass over adjList\n";
    parseGraph(graph, graphFile, 1);
    
    graphFile.clear();
    graphFile.seekg(0);
    
    //std::cout << "Running second pass\n";
    parseGraph(graph, graphFile, 2);
    
    //std::cout << "Finished parsing graph file\n";
}

int enumerateLoops(const std::string& inputAdjecencyList,
                   const itk::Logger::Pointer& logger) {
    constexpr unsigned Dimension = 3;
    
    fs::path inputAdjecencyListpath = inputAdjecencyList;
    
    fs::path outputFolderpath = inputAdjecencyListpath.parent_path().string();
    
    if (!fs::exists(inputAdjecencyListpath)) {
        logger->Critical("File " + inputAdjecencyList + " Not Found \n");
        return EXIT_FAILURE;
    } else {
        logger->Info("Input Graph Adj List file: " + inputAdjecencyListpath.string() + "\n");
    }

    Graph<Dimension> graph;
    readAdjList(graph, inputAdjecencyListpath);

    std::vector<std::list<size_t>> loops = findSimpleGraphLoops(graph);
    std::sort(loops.begin(), loops.end(),
              [](const std::list<size_t>& left, const std::list<size_t>& right) { 
                  return left.size() > right.size(); 
              });

    std::vector<double> loopLength;
    logger->Info("Found Following loops:\n");
    
    for (auto& loop : loops) {
        double length = 0;
        auto prev_vert = graph.getVertexByIndex(loop.front());
        auto curr_vert = prev_vert;
        
        for (auto& v : loop) {
            std::cout << v << "-";
            curr_vert = graph.getVertexByIndex(v);
            length += curr_vert.distance(prev_vert);
            prev_vert = curr_vert;
        }
        
        // Connect back to the start to close the loop calculation
        prev_vert = graph.getVertexByIndex(loop.front());
        length += curr_vert.distance(prev_vert);
        loopLength.push_back(length);
        
        std::cout << " : (" << std::fixed << std::setprecision(2) << length << ") nm\n";
    }
    std::cout << "\n";

    fs::path loopFilepath = outputFolderpath / (inputAdjecencyListpath.stem().string() + "-loops.txt");
    fs::path loopLabelledMeshpath = outputFolderpath / (inputAdjecencyListpath.stem().string() + "-loops.vtk");
    
    Graph<3> outputGraph = graph;

    std::ofstream loopFile(loopFilepath);
    logger->Info("Writing loop file " + loopFilepath.string() + "\n");
    loopFile << loops.size() << "\n";
    
    for (size_t l = 0; l < loops.size(); ++l) {
        loopFile << loops[l].size() << "\n";
        for (auto& e : loops[l]) {
            loopFile << e << " ";
            outputGraph.setVertexIndexLabel(e, l + 1);
        }
        loopFile << "\n";
    }
    
    logger->Info("Writing loop labelled mesh " + loopLabelledMeshpath.string() + "\n");
    outputGraph.writemesh(loopLabelledMeshpath);

    return EXIT_SUCCESS;
}
