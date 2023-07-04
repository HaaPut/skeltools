//**********************************************************
//Copyright 2021 Affan Jilani
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
// Created by affan on 2021-12-03.

#include <cstdlib>
#include <itkImage.h>
#include<itkConstantBoundaryCondition.h>
#include <queue>

#ifndef SKELTOOLS_TOPOLOGY_H
#define SKELTOOLS_TOPOLOGY_H

namespace topology
{
    extern std::vector<itk::Offset<3>> neighbors18;
    extern std::vector<itk::Offset<3>> neighbors26;
    extern std::vector<std::vector<size_t>> graph26;
    extern std::vector<std::vector<size_t>> graph18;
    extern std::vector<bool> n6;
    extern std::vector<itk::Offset<2>> neighbors8;

    enum class ObjectPointType{
        Interior = 2, //interior point
        Isoloated = 3, //isolated point
        Simple =  4, //simple point
        Curve = 5, //candidate curve point
        CurveCurveJunction = 6, //junction of curves
        Surface = 7, //candidate surface point
        CurveSurfaceJunction = 8,  //junction between curve(s) and surface
        SurfaceSurfaceJunction = 9, //junction of surfaces
        SurfaceCurveJunction = 10, //junction between surface(s) and curve
        End=11, //End point
        Anchor=12, //User defined non-removable point
        Other = 0
    };

    template<typename TImage>
    unsigned computeCbar(typename TImage::Pointer image, typename TImage::IndexType index);

    template<typename TImage>
    unsigned computeCstar(typename TImage::Pointer image, typename TImage::IndexType index);

    ObjectPointType
    TopologicalLabel(unsigned Cbar, unsigned Cstar);

    template< class TImage>
    bool IsEndPoint(typename TImage::Pointer image,typename TImage::IndexType index);

    template< class TImage>
    bool IsBoundaryPoint(typename TImage::Pointer image,typename TImage::IndexType index);

    template<class TImage>
    bool IsSimplePoint(typename TImage::Pointer image,typename TImage::IndexType index);
}

#include "topology.hxx"

#endif //SKELTOOLS_TOPOLOGY_H