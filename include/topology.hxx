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
}

#endif //SKELTOOLS_TOPOLOGY_HXX
