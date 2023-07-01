//**********************************************************
//Copyright Tabish Syed
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
// Created by tabish on 2023-06-30.
//

#include <itkOffset.h>

#include <vector>

#include "topology.h"


namespace topology{
    std::vector<itk::Offset<3>> neighbors18 = {
            {{-1,-1,0}}, {{-1,0,-1}}, {{-1,0,0}}, {{-1,0,1}}, {{-1,1,0}},
            {{0,-1,-1}}, {{0,-1,0}}, {{0,-1,1}}, {{0,0,-1}}, {{0,0,1}},
            {{0,1,-1}}, {{0,1,0}}, {{0,1,1}}, {{1,-1,0}}, {{1,0,-1}},
            {{1,0,0}}, {{1,0,1}}, {{1,1,0}}
    };

    std::vector<itk::Offset<3>> neighbors26 = {
            {{-1,-1,-1}},
            {{-1,-1,0}},
            {{-1,-1,1}},
            {{-1,0,-1}},
            {{-1,0,0}},
            {{-1,0,1}},
            {{-1,1,-1}},
            {{-1,1,0}},
            {{-1,1,1}},
            {{0,-1,-1}},
            {{0,-1,0}},
            {{0,-1,1}},
            {{0,0,-1}},
            {{0,0,1}},
            {{0,1,-1}},
            {{0,1,0}},
            {{0,1,1}},
            {{1,-1,-1}},
            {{1,-1,0}},
            {{1,-1,1}},
            {{1,0,-1}},
            {{1,0,0}},
            {{1,0,1}},
            {{1,1,-1}},
            {{1,1,0}},
            {{1,1,1}}
    };
    std::vector<std::vector<size_t>> graph26 = {
            {1,2,4,5,10,11,13},
            {1,2,3,4,5,6,10,11,12,13,14},
            {2,3,5,6,11,12,14},
            {1,2,4,5,7,8,10,11,13,15,16},
            {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17},
            {2,3,5,6,8,9,11,12,14,16,17},
            {4,5,7,8,13,15,16},
            {4,5,6,7,8,9,13,14,15,16,17},
            {5,6,8,9,14,16,17},
            {1,2,4,5,10,11,13,18,19,21,22},
            {1,2,3,4,5,6,10,11,12,13,14,18,19,20,21,22,23},
            {2,3,5,6,11,12,14,19,20,22,23},
            {1,2,4,5,7,8,10,11,13,15,16,18,19,21,22,24,25},
            {2,3,5,6,8,9,11,12,14,16,17,19,20,22,23,25,26},
            {4,5,7,8,13,15,16,21,22,24,25},
            {4,5,6,7,8,9,13,14,15,16,17,21,22,23,24,25,26},
            {5,6,8,9,14,16,17,22,23,25,26},
            {10,11,13,18,19,21,22},
            {10,11,12,13,14,18,19,20,21,22,23},
            {11,12,14,19,20,22,23},
            {10,11,13,15,16,18,19,21,22,24,25},
            {10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26},
            {11,12,14,16,17,19,20,22,23,25,26},
            {13,15,16,21,22,24,25},
            {13,14,15,16,17,21,22,23,24,25,26},
            {14,16,17,22,23,25,26}
    };
    std::vector<std::vector<size_t>> graph18 = {
            {1,3,7},
            {2,3,9},
            {1,2,3,4,5},
            {3,4,10},
            {3,5,12},
            {6,7,9},
            {1,6,7,8,14},
            {7,8,10},
            {2,6,9,11,15},
            {4,8,10,13,17},
            {9,11,12},
            {5,11,12,13,18},
            {10,12,13},
            {7,14,16},
            {9,15,16},
            {14,15,16,17,18},
            {10,16,17},
            {12,16,18}
    };
    std::vector<bool> n6 = {false,false,true,false,false,false,true,false,true,true,false,true,false,false,false,true,false,false};
    std::vector<itk::Offset<2>> neighbors8 = {
            {{-1, -1}},
            {{-1, 0}},
            {{-1, 1}},
            {{0,  1}},
            //{{0,  0}},
            {{1,  1}},
            {{1,  0}},
            {{1,  -1}},
            {{0,  -1}}
    };

	   ObjectPointType
    TopologicalLabel(unsigned Cbar, unsigned Cstar){

        if (Cbar==0)
            return ObjectPointType::Interior; //interior point
        else if (Cstar==0)
            return ObjectPointType::Isoloated; //isolated point
        else if ((Cbar==1)&&(Cstar==1))
            return ObjectPointType::Simple; //simple point
        else if ((Cbar==1)&&(Cstar==2))
            return ObjectPointType::Curve; //candidate curve point
        else if ((Cbar==1)&&(Cstar>2))
            return ObjectPointType::CurveCurveJunction; //junction of curves
        else if ((Cbar==2)&&(Cstar==1))
            return ObjectPointType::Surface; //candidate surface point
        else if ((Cbar==2)&&(Cstar>=2))
            return ObjectPointType::CurveSurfaceJunction; //junction between curve(s) and surface
        else if ((Cbar>2)&&(Cstar==1))
            return ObjectPointType::SurfaceSurfaceJunction; //junction of surfaces
        else if ((Cbar>2)&&(Cstar>=2))
            return ObjectPointType::SurfaceCurveJunction; //junction between surface(s) and curve
        else
            return ObjectPointType::Other;
    }

}
