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

#ifndef SKELTOOLS_TOPOLOGY_HXX
#define SKELTOOLS_TOPOLOGY_HXX

#include <itkNeighborhoodIterator.h>
#include <itkNeighborhood.h>

namespace topology
{
    template<typename TImage>
    unsigned computeCbar(typename TImage::Pointer image, typename TImage::IndexType index)
    {
        typename itk::ConstantBoundaryCondition<TImage> m_Accessor;

        unsigned regions = 0;
        std::queue<size_t> Q;
        std::vector<bool> visited(18,false);
        for(size_t i = 0; i < neighbors18.size(); ++i){ // starting point
            if( (n6[i]) && (! visited[i]) &&                       // not already visited
                (m_Accessor.GetPixel(index + neighbors18[i], image) <= 0)){     // is outside object
                ++regions; // new component
                Q.push(i);
                visited[i] = true;
                while(Q.size() > 0){
                    size_t current = Q.front();
                    Q.pop();
                    visited[current] = true;
                    for(size_t neighbor : graph18[current]){
                        --neighbor;
                        if((!visited[neighbor]) &&
                           (m_Accessor.GetPixel(index + neighbors18[neighbor], image) <= 0)){
                            visited[neighbor] = true;
                            Q.push(neighbor);
                        }
                    }
                }

            }
        }
        return regions;
    }

    template<typename TImage>
    unsigned computeCstar(typename TImage::Pointer image, typename TImage::IndexType index)
    {
        typename itk::ConstantBoundaryCondition<TImage> m_Accessor;

        unsigned regions = 0;
        std::queue<size_t> Q;
        std::vector<bool> visited(26,false);
        for(size_t i = 0; i < neighbors26.size(); ++i){ // starting point
            if( (! visited[i]) &&                       // not already visited
                (m_Accessor.GetPixel(index + neighbors26[i], image) > 0)){     // is in object
                ++regions; // new component
                Q.push(i);
                visited[i] = true;
                while(Q.size() > 0){
                    size_t current = Q.front();
                    Q.pop();
                    visited[current] = true;
                    for(size_t neighbor : graph26[current]){
                        --neighbor;
                        if((!visited[neighbor]) &&
                           (m_Accessor.GetPixel(index + neighbors26[neighbor], image) > 0)){
                            visited[neighbor] = true;
                            Q.push(neighbor);
                        }
                    }
                }

            }
        }
        return regions;
    }
    template< class TImage>
    bool IsEndPoint(typename TImage::Pointer image,typename TImage::IndexType index){
        using BoundaryConditionType = itk::ConstantBoundaryCondition<TImage>;
        using OutputNeighborhoodIteratorType = itk::NeighborhoodIterator<TImage, BoundaryConditionType>;
        typename OutputNeighborhoodIteratorType::RadiusType radius;
        radius.Fill(1);
        OutputNeighborhoodIteratorType nit(radius, image, image->GetLargestPossibleRegion());

        BoundaryConditionType cbc;
        nit.OverrideBoundaryCondition(&cbc);

        nit.SetLocation(index);

        int n=0;
        for( unsigned int i = 0; i < nit.Size(); i++ )
        {
            if ( nit.GetIndex() != nit.GetIndex( i ) && nit.GetPixel( i ) >0 ) //Belonging to the object - 26* connected
                ++n;
        }
        return n < 2;

    }
    template< class TImage>
    bool IsBoundaryPoint(typename TImage::Pointer image,typename TImage::IndexType index) {
        bool IsBoundaryPixel = false;
        using BoundaryConditionType = itk::ConstantBoundaryCondition<TImage>;
        using OutputNeighborhoodIteratorType = itk::NeighborhoodIterator<TImage, BoundaryConditionType>;
        typename OutputNeighborhoodIteratorType::RadiusType radius;
        radius.Fill(1);
        OutputNeighborhoodIteratorType nit(radius, image, image->GetLargestPossibleRegion());

        BoundaryConditionType cbc;
        nit.OverrideBoundaryCondition(&cbc);

        nit.SetLocation(index);

        if (nit.GetCenterPixel() > 0) {
            for (unsigned int i = 0; i < nit.Size() && !IsBoundaryPixel; i++) {
                if (nit.GetPixel(i) <= 0) {
                    IsBoundaryPixel = true;
                    break;
                }
            }
        }
        return IsBoundaryPixel;
    }
    template<class TImage>
    bool IsSimplePoint(typename TImage::Pointer image,typename TImage::IndexType index){
        auto cbar = computeCbar<TImage>(image, index);
        auto cstar = computeCstar<TImage>(image, index);
        return TopologicalLabel(cbar, cstar) == ObjectPointType::Simple;
    }
    template<class TImage>
    bool IsSimplePoint2d(typename TImage::Pointer image, typename TImage::IndexType index){
        typename itk::ConstantBoundaryCondition<TImage> accessor;
        std::array<int,8> nbrs = {{0,0,0,0,0,0,0,0}};
        int numNeighbors = 0;
        int numEdges = 0;
        for(size_t i = 0 ;i < neighbors8.size(); ++i){
            nbrs[i] = accessor.GetPixel(index + neighbors8[i], image) > 0;
            size_t j = (i+1)%8;
            nbrs[j] = accessor.GetPixel(index + neighbors8[j], image) > 0;
            if(nbrs[i] == 1 && nbrs[j] == 1){
                numNeighbors+=2;
                ++numEdges;
            }else if(nbrs[i] == 1 || nbrs[j] == 1){
                ++numNeighbors;
            }
        }
        //remove double counted
        numNeighbors /=2;
        //add corner diagonals if corner is 0
        for (size_t i = 0; i < 8; i+=2){
            numEdges += (nbrs[(i+7)%8] == 1 && nbrs[i] == 0 && nbrs[(i+1)%8] == 1)? 1: 0;
        }
        return (numNeighbors - numEdges == 1);
    }
}

#endif //SKELTOOLS_TOPOLOGY_HXX
