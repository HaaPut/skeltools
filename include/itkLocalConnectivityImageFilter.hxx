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
// Created by tabish on 2023-10-14.
//

#ifndef SKELTOOLS_itkLocalConnectivityImageFilter_hxx
#define SKELTOOLS_itkLocalConnectivityImageFilter_hxx

#include "itkLocalConnectivityImageFilter.h"

namespace itk {
    template<class TInputImage, class TOutputImage>
    LocalConnectivityImageFilter<TInputImage, TOutputImage>::LocalConnectivityImageFilter() {
		m_MaxLevel = 3;
	}

	template<class TInputImage, class TOutputImage>
	SizeValueType
	LocalConnectivityImageFilter<TInputImage, TOutputImage>::LinearIndex(IndexType index){
		SizeValueType linIndex = 0;
		for(size_t d = 0; d < Dimension; ++d){
			linIndex += m_Weight[d] * index[d];
		}
		return linIndex;
	}

	template<class TInputImage, class TOutputImage>
	void
    LocalConnectivityImageFilter<TInputImage, TOutputImage>::Initialize() {
		auto region = this->GetInput()->GetLargestPossibleRegion();
		m_Weight = region.GetSize();
		//R,C,Z(R,C)
		for(size_t d =1; d < Dimension-1; ++d){
			m_Weight[d] *= m_Weight[d-1];
		}//R, R*C , Z (R,C)
		for(size_t d = 1; d < Dimension; ++d){
			m_Weight[d] = m_Weight[d-1];
		}// R, R, R*C (R,R)
		m_Weight[0] = 1;
		// 1,R,R*C (1,R)

		this->AllocateOutputs();
	}

    template<class TInputImage, class TOutputImage>
    void
    LocalConnectivityImageFilter<TInputImage, TOutputImage>::GenerateData() {
		Initialize();
		auto input = this->GetInput();
		auto output = this->GetOutput();
		typename NeighborhoodIteratorType::RadiusType radius;
		radius.Fill(1);
		NeighborhoodIteratorType skIt(radius, input, input->GetLargestPossibleRegion());
		InputIteratorType ipIt(input, input->GetLargestPossibleRegion());
		OutputIteratorType outIt(output, output->GetLargestPossibleRegion());

		outIt.GoToBegin();
		ipIt.GoToBegin();
		while(!ipIt.IsAtEnd()){
			typename TOutputImage::PixelType count = 0;
			if(ipIt.Get() > 0){
				SetType visited;
				visited.insert(LinearIndex(ipIt.GetIndex()));
				std::queue<QPacketType> q;
				q.push({ipIt.GetIndex(),0});
				while(!q.empty()){
					auto qp = q.front();
					q.pop();
					if(qp.level >= m_MaxLevel) continue;
					++count;
					skIt.SetLocation(qp.index);
					auto current_level = qp.level;
					for(unsigned k = 0; k < skIt.Size(); ++k){
						if(skIt.GetPixel(k) > 0){
							auto idx = skIt.GetIndex(k);
							auto lidx = LinearIndex(idx);
							if(visited.find(lidx) == visited.end()){
								q.push({idx, current_level+1});
								visited.insert(lidx);
							}
						}
					}
				}
				outIt.Set(count);
			}else{
				outIt.Set(0);
			}
			++outIt;
			++ipIt;
		}
    }

/**
*  Print Self
*/
    template<class TInputImage, class TOutputImage>
    void
    LocalConnectivityImageFilter<TInputImage,  TOutputImage>::PrintSelf(std::ostream &os, Indent indent) const {
        Superclass::PrintSelf(os, indent);
        os << indent << "LocalConnectivityImageFilter." << std::endl;
    }
}
#endif //SKELTOOLS_itkLocalConnectivityImageFilter_hxx
