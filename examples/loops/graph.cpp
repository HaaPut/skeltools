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

#include "graph.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <functional>
#include <cassert>
#include <chrono>

#include <itkMesh.h>
#include <itkLineCell.h>
#include <itkMeshFileWriter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkConstantBoundaryCondition.h>
#include <itkNeighborhoodIterator.h>
#include <itkImageFileReader.h>

template<unsigned Dimension>
size_t Graph<Dimension>::getVertexCount() const {
    return vertices.size();
}

template<unsigned Dimension>
Vertex<Dimension> Graph<Dimension>::getVertexByIndex(size_t v) {
    return vertices[v];
}

template<unsigned Dimension>
bool Graph<Dimension>::findVertexById(size_t id) {
    auto it = std::find_if(vertices.begin(), vertices.end(), 
                           [id](const Vertex<Dimension>& v) { return v.id == id; });
    return it != vertices.end();
}

template<unsigned Dimension>
Vertex<Dimension> Graph<Dimension>::getVertexById(size_t id) {
    auto it = std::find_if(vertices.begin(), vertices.end(), 
                           [id](const Vertex<Dimension>& v) { return v.id == id; });
    if (it != vertices.end()) {
        return *it;
    }
    return Vertex<Dimension>(-1, std::vector<double>(Dimension, 0.0));
}

template<unsigned Dimension>
int Graph<Dimension>::getVertexIndex(size_t id) {
    auto it = std::find_if(vertices.begin(), vertices.end(), 
                           [id](const Vertex<Dimension>& v) { return v.id == id; });
    if (it != vertices.end()) {
        return std::distance(vertices.begin(), it);
    }
    std::cerr << "Unknown vertex ID: " << id << "\n";
    return -1;
}

template<unsigned Dimension>
std::list<size_t> Graph<Dimension>::getNeighboursByIndex(size_t v) {
    return this->adjList[v];
}

template<unsigned D>
void Graph<D>::addVertex(Vertex<D> v) {
    vertices.push_back(v);
    expandAdjecency(vertices.size());
}

template<unsigned D>
void Graph<D>::setVertexIndexLabel(size_t v, int label) {
    if (v >= vertices.size()) {
        std::cerr << "Attempting to set label of out of bound vertex. Index " 
                  << v << " >= " << vertices.size() << "\n";
        return;
    }
    vertices[v].label = label;
}

template<unsigned Dimension>
void Graph<Dimension>::addUndirectedEdge(size_t fromId, size_t toId) {
    size_t fromIndex = getVertexIndex(fromId);
    size_t toIndex = getVertexIndex(toId);
    assert(fromIndex < getVertexCount() && toIndex < getVertexCount());

    auto& fromNbrs = adjList[fromIndex];
    if (std::find(fromNbrs.begin(), fromNbrs.end(), toIndex) != fromNbrs.end()) {
        return; // Edge already exists
    }

    edges.emplace_back(fromId, toId);
    updateAdjecency(fromIndex, toIndex, true);
}

template<unsigned Dimension>
void Graph<Dimension>::addLabelledEdge(size_t fromId, size_t toId, int label) {
    edges.emplace_back(fromId, toId, label);
    updateAdjecency(getVertexIndex(fromId), getVertexIndex(toId), true);
}

template<unsigned Dimension>
void Graph<Dimension>::expandAdjecency(size_t size) {
    adjList.resize(std::max(getVertexCount(), size));
}

template<unsigned Dimension>
void Graph<Dimension>::updateAdjecency(size_t fromIndex, size_t toIndex, bool undirected) {
    auto& fromNbrs = adjList[fromIndex];
    if (std::find(fromNbrs.begin(), fromNbrs.end(), toIndex) == fromNbrs.end()) {
        fromNbrs.push_back(toIndex);
    }

    if (undirected) {
        auto& toNbrs = adjList[toIndex];
        if (std::find(toNbrs.begin(), toNbrs.end(), fromIndex) == toNbrs.end()) {
            toNbrs.push_back(fromIndex);
        }
    }
}

template<unsigned Dimension>
void Graph<Dimension>::reduce(double distanceThreshold) {
    struct EdgeType {
        size_t first;
        size_t second;
        EdgeType(size_t f, size_t s) : first(std::min(f, s)), second(std::max(f, s)) {}
        bool operator<(const EdgeType& rhs) const {
            return std::tie(first, second) < std::tie(rhs.first, rhs.second);
        }
    };

    std::set<EdgeType> newEdgeSet;
    std::set<size_t> newVertexSet;
    std::set<size_t> deleted;
    std::set<size_t> dirty;

    auto isDeleted = [&deleted](size_t id) { return deleted.count(id) > 0; };
    auto isDirty = [&dirty](size_t id) { return dirty.count(id) > 0; };

    for (size_t v = 0; v < adjList.size(); ++v) {
        const auto& current = vertices[v];

        if (adjList[v].size() == 2 && !isDirty(current.id)) {
            const auto& left = vertices[adjList[v].front()];
            const auto& right = vertices[adjList[v].back()];

            if (current.distance(left) > distanceThreshold || current.distance(right) > distanceThreshold) {
                newVertexSet.insert(current.id);
                newVertexSet.insert(left.id);
                newVertexSet.insert(right.id);
                newEdgeSet.insert({current.id, left.id});
                newEdgeSet.insert({current.id, right.id});
            } else {
                newVertexSet.erase(current.id);
                newEdgeSet.erase({current.id, left.id});
                newEdgeSet.erase({current.id, right.id});

                deleted.insert(current.id);
                dirty.insert(left.id);
                dirty.insert(right.id);

                newVertexSet.insert(left.id);
                newVertexSet.insert(right.id);
                newEdgeSet.insert({left.id, right.id});
            }
        } else {
            newVertexSet.insert(current.id);
            for (size_t toIndex : adjList[v]) {
                const auto& dest = vertices[toIndex];
                if (!isDeleted(dest.id)) {
                    newVertexSet.insert(dest.id);
                    newEdgeSet.insert({current.id, dest.id});
                }
            }
        }
    }

    std::vector<Vertex<Dimension>> newVertices;
    std::unordered_map<size_t, size_t> idToIndex;
    newVertices.reserve(newVertexSet.size());

    for (size_t id : newVertexSet) {
        if (!isDeleted(id)) {
            idToIndex[id] = newVertices.size();
            newVertices.push_back(getVertexById(id));
        }
    }

    std::vector<std::list<size_t>> newAdjList(newVertices.size());
    std::vector<Edge> newEdges;
    newEdges.reserve(newEdgeSet.size());

    for (const EdgeType& e : newEdgeSet) {
        if (isDeleted(e.first) || isDeleted(e.second)) continue;

        auto itFrom = idToIndex.find(e.first);
        auto itTo = idToIndex.find(e.second);

        if (itFrom != idToIndex.end() && itTo != idToIndex.end()) {
            size_t fromIndex = itFrom->second;
            size_t toIndex = itTo->second;

            newAdjList[fromIndex].push_back(toIndex);
            newAdjList[toIndex].push_back(fromIndex);
            newEdges.emplace_back(e.first, e.second);
        }
    }

    vertices = std::move(newVertices);
    edges = std::move(newEdges);
    adjList = std::move(newAdjList);
}

template<unsigned Dimension>
unsigned Graph<Dimension>::removeTriangles() {
    using IndexType = size_t;
    auto connected = [this](IndexType v1, IndexType v2) -> bool {
        return std::find(adjList[v1].begin(), adjList[v1].end(), v2) != adjList[v1].end();
    };

    unsigned count = 0;
    int oldPercent = -1;

    for (IndexType v = 0; v < adjList.size(); ++v) {
        if (adjList[v].size() >= 3) {
            std::vector<Vertex<Dimension>> triangle(3);
            triangle[0] = vertices[v];

            std::vector<IndexType> nbrs(adjList[v].begin(), adjList[v].end());
            bool foundTriangle = false;

            for (size_t i = 0; i < nbrs.size(); ++i) {
                for (size_t j = i + 1; j < nbrs.size(); ++j) {
                    if (connected(nbrs[i], nbrs[j])) {
                        triangle[1] = vertices[nbrs[i]];
                        triangle[2] = vertices[nbrs[j]];
                        foundTriangle = true;
                        --v; // Re-check this vertex on next iteration
                        break;
                    }
                }
                if (foundTriangle) break;
            }

            if (!foundTriangle) continue;
            ++count;

            std::vector<double> distances(3);
            for (size_t i = 0; i < 3; ++i) {
                distances[i] = triangle[i].distance(triangle[(i + 1) % 3]);
            }
            size_t largestedge = std::max_element(distances.begin(), distances.end()) - distances.begin();

            removeEdge(triangle[largestedge], triangle[(largestedge + 1) % 3]);
        }

        int percentage = (v * 100) / adjList.size();
        if (percentage != oldPercent) {
            std::cout << "\rRemoving triangles: " << percentage << "% done" << std::flush;
            oldPercent = percentage;
        }
    }
    std::cout << "\rRemoving triangles: 100% done\n";
    return count;
}

template<unsigned Dimension>
void Graph<Dimension>::removeEdge(Vertex<Dimension>& A, Vertex<Dimension>& B) {
    size_t a = A.id;
    size_t b = B.id;

    // 1. Remove from edge list safely utilizing std::remove_if
    edges.erase(std::remove_if(edges.begin(), edges.end(),
        [a, b](const Edge& e) {
            return (e.u == (int)a && e.v == (int)b) || (e.u == (int)b && e.v == (int)a);
        }), edges.end());

    // 2. Remove from adjacency lists
    size_t va = getVertexIndex(a);
    size_t vb = getVertexIndex(b);
    
    if (va != (size_t)-1 && vb != (size_t)-1) {
        adjList[va].remove(vb);
        adjList[vb].remove(va);
    }
}

template<unsigned Dimension>
unsigned Graph<Dimension>::removeQuads() {
    using IndexType = size_t;
    auto haveCommonNbr = [this](IndexType v1, IndexType v2, IndexType ignore) -> int {
        std::unordered_set<IndexType> n1;
        for (auto& nbr : adjList[v1]) {
            if (nbr != ignore) n1.insert(nbr);
        }
        for (auto& nbr : adjList[v2]) {
            if (n1.count(nbr)) return nbr;
        }
        return -1;
    };

    unsigned count = 0;
    int oldPercent = -1;

    for (IndexType v = 0; v < adjList.size(); ++v) {
        if (adjList[v].size() >= 3) {
            std::vector<Vertex<Dimension>> quad(4);
            quad[0] = vertices[v];

            std::vector<IndexType> nbrs(adjList[v].begin(), adjList[v].end());
            bool foundQuad = false;

            for (size_t i = 0; i < nbrs.size(); ++i) {
                for (size_t j = i + 1; j < nbrs.size(); ++j) {
                    int common = haveCommonNbr(nbrs[i], nbrs[j], v);
                    if (common >= 0) {
                        quad[1] = vertices[nbrs[i]];
                        quad[2] = vertices[common];
                        quad[3] = vertices[nbrs[j]];
                        foundQuad = true;
                        --v;
                        break;
                    }
                }
                if (foundQuad) break;
            }

            if (!foundQuad) continue;
            ++count;

            std::vector<double> distances(4);
            for (size_t i = 0; i < 4; ++i) {
                distances[i] = quad[i].distance(quad[(i + 1) % 4]);
            }
            size_t largestedge = std::max_element(distances.begin(), distances.end()) - distances.begin();

            removeEdge(quad[largestedge], quad[(largestedge + 1) % 4]);
        }

        int percentage = (v * 100) / adjList.size();
        if (percentage != oldPercent) {
            std::cout << "\rRemoving quads: " << percentage << "% done" << std::flush;
            oldPercent = percentage;
        }
    }
    std::cout << "\rRemoving quads: 100% done\n";
    return count;
}

template<unsigned Dimension>
void Graph<Dimension>::writetxt(const fs::path& filepath) {
    std::ofstream graphFile(filepath);
    if (!graphFile.is_open()) return;

    graphFile << getVertexCount() << "\n";
    for (size_t v = 0; v < vertices.size(); ++v) {
        const auto& vertex = vertices[v];
        
        for (size_t d = 0; d < Dimension; ++d) {
            graphFile << std::fixed << std::setprecision(1) << vertex.location[d] << " ";
        }
        graphFile << "\n" << adjList[v].size() << "\n";
        
        for (const auto& nbr : adjList[v]) {
            graphFile << nbr << " ";
        }
        graphFile << "\n";
    }
}

template<unsigned Dimension>
void Graph<Dimension>::writemesh(const fs::path& filepath) {
    using PixelType = size_t;
    using MeshType = itk::Mesh<PixelType, Dimension>;
    using CellType = typename MeshType::CellType;
    using LineType = itk::LineCell<CellType>;
    using CellAutoPointer = typename CellType::CellAutoPointer;

    typename MeshType::Pointer mesh = MeshType::New();
    
    for (size_t i = 0; i < getVertexCount(); ++i) {
        auto node = getVertexByIndex(i);
        typename MeshType::PointType point;
        for (size_t d = 0; d < Dimension; ++d) {
            point[d] = node.location[d];
        }
        mesh->SetPoint(i, point);
        mesh->SetPointData(i, node.label);
    }
    
    for (size_t i = 0; i < edges.size(); ++i) {
        CellAutoPointer line;
        line.TakeOwnership(new LineType);
        size_t from = getVertexIndex(edges[i].u);
        size_t to = getVertexIndex(edges[i].v);
        line->SetPointId(0, from);
        line->SetPointId(1, to);
        mesh->SetCell(i, line);
    }
    
    using MeshSkeletonWriter = itk::MeshFileWriter<MeshType>;
    typename MeshSkeletonWriter::Pointer meshWriter = MeshSkeletonWriter::New();
    meshWriter->SetFileName(filepath.string());
    meshWriter->SetInput(mesh);
    
    try {
        meshWriter->Update();
    } catch (itk::ExceptionObject& e) {
        std::cerr << e.what() << "\nFailed write to " << filepath << "\n";
    }
}

template class Graph<3>;

template<unsigned Dimension>
std::ostream& operator<<(std::ostream& os, Vertex<Dimension> V) {
    os << "V(" << V.id << ") @[";
    for (size_t d = 0; d < Dimension; ++d) {
        os << V.location[d] << (d < Dimension - 1 ? "," : "");
    }
    os << "]";
    return os;
}
template std::ostream& operator<<<3>(std::ostream& os, Vertex<3> V);

std::ostream& operator<<(std::ostream& os, Edge e) {
    os << "-" << e.u << "-(" << e.weight << ")-" << e.v << "-";
    return os;
}

template<unsigned Dimension>
int kruskal_mst(Graph<Dimension>& graph) {
    int VertexCount = graph.getVertexCount();
    auto& edges = graph.edges;
    std::vector<int> parent(VertexCount), rank(VertexCount);

    for (int i = 0; i < VertexCount; ++i) {
        parent[i] = i;
        rank[i] = 0;
    }

    std::function<int(int)> find_set = [&](int v) {
        if (parent[v] == v) return v;
        return parent[v] = find_set(parent[v]);
    };

    auto union_set = [&](int a, int b) {
        a = find_set(a);
        b = find_set(b);
        if (a != b) {
            if (rank[a] < rank[b]) std::swap(a, b);
            parent[b] = a;
            if (rank[a] == rank[b]) ++rank[a];
        }
    };

    std::sort(edges.begin(), edges.end(), std::greater<Edge>());
    
    for (auto& edge : edges) {
        if (find_set(edge.u) != find_set(edge.v)) {
            edge.label = 1;
            union_set(edge.u, edge.v);
        }
    }
    return EXIT_SUCCESS;
}

template int kruskal_mst<3>(Graph<3>&);

template<typename VoxelSkeltonImageType> 
int voxelSketonToGraph(typename VoxelSkeltonImageType::Pointer const & voxelSkeleton,
                         int reduction, double distanceThreshold, fs::path& outputFolderpath,
                         const itk::Logger::Pointer& logger) {
    using IndexType = typename VoxelSkeltonImageType::IndexType;
    constexpr unsigned Dimension = VoxelSkeltonImageType::ImageDimension;
    using SkeletonPixelType = typename VoxelSkeltonImageType::PixelType;

    std::string reductionLevel = (reduction > 0) ? std::to_string(reduction) : "inf";
    auto spacing = voxelSkeleton->GetSpacing();
    
    Graph<Dimension> graph;
    std::unordered_map<size_t, size_t> nodeIdFromLinIndex;
    auto indexWeight = voxelSkeleton->GetLargestPossibleRegion().GetSize();

    for (int d = static_cast<int>(Dimension) - 2; d >= 0; --d) {
        indexWeight[d] *= indexWeight[d + 1];
    }

    auto linIndex = [&indexWeight](IndexType index) -> size_t {
        size_t lIndex = 0;
        for (int d = static_cast<int>(Dimension) - 1; d >= 0; --d) {
            lIndex += index[d] * indexWeight[d];
        }
        return lIndex;
    };

    typename VoxelSkeltonImageType::SizeType radius;
    radius.Fill(1);

    using BoundaryConditionType = itk::ConstantBoundaryCondition<VoxelSkeltonImageType>;
    using VoxelSkeltonIteratorType = itk::NeighborhoodIterator<VoxelSkeltonImageType, BoundaryConditionType>;
    VoxelSkeltonIteratorType skit(radius, voxelSkeleton, voxelSkeleton->GetLargestPossibleRegion());
    using VoxelNeighborhoodType = typename VoxelSkeltonIteratorType::NeighborhoodType;

    size_t voxelCount = 0;
    for (skit.GoToBegin(); !skit.IsAtEnd(); ++skit) {
        if (skit.GetCenterPixel() > 0) {
            skit.SetCenterPixel(itk::NumericTraits<SkeletonPixelType>::OneValue());
        }
        ++voxelCount;
    }

    logger->Info("Starting Graph Construction\n");
    size_t done = 0;
    int oldPercent = -1;

    for (skit.GoToBegin(); !skit.IsAtEnd(); ++skit, ++done) {
        if (skit.GetCenterPixel() > 0) {
            IndexType index = skit.GetIndex();
            auto lIndex = linIndex(index);
            
            size_t vertexId;
            auto findIt = nodeIdFromLinIndex.find(lIndex);
            if (findIt != nodeIdFromLinIndex.end()) {
                vertexId = findIt->second;
            } else {
                vertexId = graph.getVertexCount();
                nodeIdFromLinIndex[lIndex] = vertexId;
                graph.expandAdjecency(vertexId + 1);
                
                std::array<double, Dimension> location{};
                for (size_t d = 0; d < Dimension; ++d) {
                    location[d] = index[d] * spacing[d];
                }
                graph.addVertex(Vertex<Dimension>(vertexId, location));
            }

            VoxelNeighborhoodType nbd = skit.GetNeighborhood();
            for (size_t n = 0; n < nbd.Size(); ++n) {
                if (skit.GetPixel(n) > 0) {
                    IndexType toIndex = skit.GetIndex(n);
                    auto linTo = linIndex(toIndex);
                    
                    size_t to;
                    auto toIt = nodeIdFromLinIndex.find(linTo);
                    if (toIt == nodeIdFromLinIndex.end()) {
                        to = graph.getVertexCount();
                        nodeIdFromLinIndex[linTo] = to;
                        graph.expandAdjecency(to + 1);
                        
                        std::array<double, Dimension> location{};
                        for (size_t d = 0; d < Dimension; ++d) {
                            location[d] = toIndex[d] * spacing[d];
                        }
                        graph.addVertex(Vertex<Dimension>(to, location));
                    } else {
                        to = toIt->second;
                    }
                    
                    if (vertexId != to) {
                        graph.addUndirectedEdge(vertexId, to);
                    }
                }
            }
        }

        int percentage = (done * 100) / voxelCount;
        if (percentage != oldPercent) {
            std::cout << "\rGraph Construction: " << percentage << "% done" << std::flush;
            oldPercent = percentage;
        }
    }
    std::cout << "\n";

    logger->Info("Fix junction Triangles\n");
    unsigned count = graph.removeTriangles();
    logger->Debug("Removed " + std::to_string(count) + " Triangles\n");

    logger->Info("Fix Quads\n");
    count = graph.removeQuads();
    logger->Debug("Removed " + std::to_string(count) + " Quads\n");

    logger->Info("Starting Graph Reduction\n");
    size_t vertexCount = -1;
    
    logger->Info("Distance threshold for graph reduction = " + std::to_string(distanceThreshold) + "\n");
    for (int i = 1; i <= reduction; ++i) {
        graph.reduce(distanceThreshold);
        
        if (vertexCount == graph.vertices.size()) {
            logger->Warning("Vertex count unchanged after reduction round " + std::to_string(i) + "\n");
            reductionLevel = "inf";
            break;
        }
        vertexCount = graph.vertices.size();
    }

    std::string name = "skeletonGraphList-" + reductionLevel + ".txt";
    graph.writetxt(outputFolderpath / name);

    name = "skeletonMesh-" + reductionLevel + ".vtk";
    graph.writemesh(outputFolderpath / name);

    return EXIT_SUCCESS;
}

using ImageType = typename itk::Image<unsigned char, 3>;
template int voxelSketonToGraph<ImageType>(ImageType::Pointer const&, int, double, fs::path&, const itk::Logger::Pointer& );

