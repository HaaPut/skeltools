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
// Created by tabish on 2023-09-29.
//
#ifndef SKELTOOLS_itkMapToBoundaryImageFilter_hxx
#define SKELTOOLS_itkMapToBoundaryImageFilter_hxx

#include "itkMapToBoundaryImageFilter.h"
#include "topology.h"

namespace itk {
    template<class TInputImage, class TSkeletonImage, class TOutputImage>
    MapToBoundaryImageFilter<TInputImage, TSkeletonImage, TOutputImage>::MapToBoundaryImageFilter() {
        m_SkeletonImage = nullptr;
    }


    template<class TInputImage,class TSkeletonImage, class TOutputImage>
    void
    MapToBoundaryImageFilter<TInputImage, TSkeletonImage, TOutputImage>::GenerateData() {
		typename SampleType::Pointer samples = SampleType::New();
		SkeletonIteratorType skIt = OutputIteratorType(m_SkeletonImage, m_SkeletonImage->GetLargestPossibleRegion());
		skIt.GoToBegin();
		MeasurementVectorType mv;
		while(!skIt.IsAtEnd()){
			if(skIt.Get() >  0) {
				m_SkeletonImage->TransformIndexToPhysicalPoint(skIt.GetIndex(), mv);
				samples->PushBack(mv);
			}
			++skIt;
		}

		typename TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();

		treeGenerator->SetSample(samples);
	    treeGenerator->SetBucketSize(16);
		treeGenerator->Update();

		auto tree = treeGenerator->GetOutput();

		MeasurementVectorType queryPoint;
		unsigned int  numberOfNeighbors = 1;
		typename TreeType::InstanceIdentifierVectorType neighbors;

		this->AllocateOutputs();
		auto output = this->GetOutput();
		auto input = this->GetInput();

		typename TInputImage::Pointer inputPtr(const_cast<TInputImage*>(input));

		OutputIteratorType outputIt = OutputIteratorType(output, output->GetLargestPossibleRegion());
		InputIteratorType ipIt = InputIteratorType(input, input->GetLargestPossibleRegion());

		ipIt.GoToBegin();
		outputIt.GoToBegin();
		//BUG:using m_skeleton image for index <->Point conversion for now
		//so no need to enforce spacing match on input and skeleton..
		while(!ipIt.IsAtEnd()){
			auto index = ipIt.GetIndex();
			if(::topology::IsBoundaryPoint<TInputImage>(inputPtr, index)){
				m_SkeletonImage->TransformIndexToPhysicalPoint(index, queryPoint);
				tree->Search(queryPoint, numberOfNeighbors, neighbors);
				mv = tree->GetMeasurementVector(neighbors[0]);
				m_SkeletonImage->TransformPhysicalPointToIndex(mv, index);
				outputIt.Set(m_SkeletonImage->GetPixel(index));
			}else{
				outputIt.Set(0);
			}
			++ipIt;
			++outputIt;
		}
    }

/**
*  Print Self
*/
    template<class TInputImage,class TSkeletonImage, class TOutputImage>
    void
    MapToBoundaryImageFilter<TInputImage, TSkeletonImage, TOutputImage>::PrintSelf(std::ostream &os, Indent indent) const {
        Superclass::PrintSelf(os, indent);
        os << indent << "MapToBoundaryImageFilter." << std::endl;
    }
}
#endif //SKELTOOLS_itkMapToBoundaryImageFilter_hxx
