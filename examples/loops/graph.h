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

#ifndef SKELTOOLS_GRAPH_H
#define SKELTOOLS_GRAPH_H

#include <iostream>
#include <vector>
#include <list>
#include <array>
#include <cmath>
#include <itkImage.h>
#include <filesystem>

#include "itkCommandLineArgumentParser.h"
#include <itkLogger.h>

namespace fs = std::filesystem;

struct Edge {
    int u, v;
    double weight;
    int label{};
    bool undirected;

    Edge(int u, int v, double weight) : u{u}, v{v}, weight{weight}, label{0}, undirected{true} {}
    Edge(int u, int v, int label) : u{u}, v{v}, weight{1.0}, label{label}, undirected{true} {}
    Edge(int u, int v) : u{u}, v{v}, weight{1.0}, label{0}, undirected{true} {}

    bool operator<(const Edge& other) const {
        return weight < other.weight; // sort in ascending order
    }
    bool operator>(const Edge& other) const {
        return weight > other.weight; // sort in descending order
    }
};

std::ostream& operator<<(std::ostream& os, Edge e);

template<unsigned Dim = 3>
class Vertex {
public:
    static constexpr unsigned Dimension = Dim;
    size_t id;
    std::array<double, Dimension> location;
    int label;

    Vertex() : id(0), location({0, 0, 0}), label(0) {}

    template<typename T>
    Vertex(size_t id, const std::vector<T>& loc) : id{id}, label{0} {
        for (size_t d = 0; d < Dimension; ++d) {
            location[d] = static_cast<double>(loc[d]);
        }
    }

    template<typename T>
    Vertex(size_t id, const std::array<T, Dimension>& loc) : id{id}, label{0} {
        for (size_t d = 0; d < Dimension; ++d) {
            location[d] = static_cast<double>(loc[d]);
        }
    }

    [[nodiscard]] double distance(const Vertex<Dimension>& other) const {
        double dist = 0.0;
        for (size_t d = 0; d < Dimension; ++d) {
            dist += (location[d] - other.location[d]) * (location[d] - other.location[d]);
        }
        return std::sqrt(dist);
    }
};

template<unsigned Dimension>
std::ostream& operator<<(std::ostream& os, Vertex<Dimension> V);


template<unsigned Dim = 3>
class Graph {
public:
    static constexpr unsigned Dimension = Dim;
    std::vector<Edge> edges;
    std::vector<Vertex<Dimension>> vertices;
    std::vector<std::list<size_t>> adjList;

public:
    Graph() = default;

    [[nodiscard]] size_t getVertexCount() const;
    Vertex<Dim> getVertexByIndex(size_t v);
    Vertex<Dim> getVertexById(size_t id);
    int getVertexIndex(size_t id);
    std::list<size_t> getNeighboursByIndex(size_t v);
    
    void addVertex(Vertex<Dim> v);
    void setVertexIndexLabel(size_t v, int label);
    void addUndirectedEdge(size_t fromId, size_t toId);
    void addLabelledEdge(size_t fromId, size_t toId, int label);
    void expandAdjecency(size_t size);
    void updateAdjecency(size_t fromIndex, size_t toIndex, bool undirected);
    
    void writetxt(const fs::path& filepath);
    void writemesh(const fs::path& filepath);
    
    void reduce(double distanceThreshold);
    unsigned removeTriangles();
    void removeEdge(Vertex<Dimension>& A, Vertex<Dimension>& B);
    unsigned removeQuads();
    bool findVertexById(size_t id);
};

template<unsigned Dimension = 3>
int kruskal_mst(Graph<Dimension>& graph);


template<typename VoxelSkeltonType>
int voxelSketonToGraph( typename VoxelSkeltonType::Pointer const & skeleton,
                        int reduction, double doubleThreshold, fs::path& outputFolderPath,
                       const itk::Logger::Pointer& logger);

#endif //SKELTOOLS_GRAPH_H
