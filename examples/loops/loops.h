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

#ifndef SKELTOOLS_LOOPS_H
#define SKELTOOLS_LOOPS_H

#include <iostream>
#include <vector>
#include <list>
#include <filesystem>

#include "itkCommandLineArgumentParser.h"
#include <itkLogger.h>

#include "graph.h"

template<typename AdjecencyValueType>
bool find_loop(std::vector<std::vector<AdjecencyValueType>>& adjMatrix, int parent, int current, std::list<int>& path);

int enumerateLoops(const std::string& adjacencyListPath,
                   const itk::Logger::Pointer& logger);

template<unsigned Dimension = 3>
[[nodiscard]] Graph<Dimension> extractLabelledSubGraph(Graph<Dimension>& edgeLabelledGraph, int label);

template<unsigned Dimension = 3>
[[nodiscard]] std::vector<std::list<size_t>> findSimpleGraphLoops(Graph<Dimension>& graph);

template<unsigned Dimension = 3>
bool find_loop(const std::vector<std::list<size_t>>& loopyAdjList, int parentIndex, int currentIndex, std::list<size_t>& path);

#endif //SKELTOOLS_LOOPS_H
