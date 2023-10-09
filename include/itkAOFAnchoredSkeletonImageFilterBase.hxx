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
// Created by tabish on 2023-07-30.
//
#ifndef SKELTOOLS_itkAOFAnchoredSkeletonImageFilterBase_hxx
#define SKELTOOLS_itkAOFAnchoredSkeletonImageFilterBase_hxx

#include "itkAOFAnchoredSkeletonImageFilterBase.h"
#include "topology.h"

namespace itk {
    template<class TInputImage, class TOutputImage>
    AOFAnchoredSkeletonImageFilterBase<TInputImage, TOutputImage>::AOFAnchoredSkeletonImageFilterBase() {
        m_AOFThreshold = -30.0;
        m_AOF = nullptr;
		m_Quick = false;
    }

    template<class TInputImage, class TOutputImage>
    void
    AOFAnchoredSkeletonImageFilterBase<TInputImage, TOutputImage>::Initialize() {
        InputPointerType input = this->GetInput();

        using BinaryImageGeneratorType = BinaryThresholdImageFilter<TInputImage, TInputImage>;
        typename BinaryImageGeneratorType::Pointer binaryImageGenerator = BinaryImageGeneratorType::New();
        binaryImageGenerator->SetInput(input);
        binaryImageGenerator->SetLowerThreshold(NumericTraits<PixelType>::OneValue());
        binaryImageGenerator->SetUpperThreshold(NumericTraits<PixelType>::max());
        binaryImageGenerator->SetOutsideValue(NumericTraits<PixelType>::OneValue());
        binaryImageGenerator->SetInsideValue(NumericTraits<PixelType>::ZeroValue());
        binaryImageGenerator->Update();
        auto binaryInput = binaryImageGenerator->GetOutput();
        PriorityImagePointerType distanceImage = nullptr;

        if(this->m_PriorityImage == nullptr) {
            using PriorityFilterType = DanielssonDistanceMapImageFilter<TInputImage, PriorityImageType>;
            auto priorityFilter = PriorityFilterType::New();
            priorityFilter->SetInput(binaryInput);
            priorityFilter->UseImageSpacingOn();
            priorityFilter->Update();
            this->m_PriorityImage = priorityFilter->GetOutput();
            this->m_PriorityImage->ReleaseDataFlagOff();
            distanceImage = this->m_PriorityImage;
        }else{
            auto distanceFilter = DanielssonDistanceMapImageFilter<TInputImage, PriorityImageType>::New();
            distanceFilter->SetInput(binaryInput);
            distanceFilter->UseImageSpacingOn();
            distanceFilter->Update();
            distanceImage = distanceFilter->GetOutput();
        }
        assert(distanceImage != nullptr && "Distance image cannot be nullptr\n");

        this->m_Skeleton = this->GetOutput();
        this->AllocateOutputs();

        this->m_Queued = TOutputImage::New();
        this->m_Queued->SetSpacing(this->m_PriorityImage->GetSpacing());
        this->m_Queued->SetOrigin(this->m_PriorityImage->GetOrigin());
        this->m_Queued->SetRegions(this->m_PriorityImage->GetRequestedRegion());
        this->m_Queued->Allocate();

        OutputIteratorType skit(this->m_Skeleton, this->m_Skeleton->GetLargestPossibleRegion());
        PriorityImageConstIteratorType dIt(distanceImage, distanceImage->GetLargestPossibleRegion() );
		AOFImageConstIteratorType aofIt(m_AOF, m_AOF->GetLargestPossibleRegion() );

		aofIt.GoToBegin();
        dIt.GoToBegin();
        skit.GoToBegin();
        while (!skit.IsAtEnd()) {
            PriorityValueType value = dIt.Get();
            if(value > 0 ){
				if(!m_Quick ||
				   (m_Quick && aofIt.Get() < 0)) {
					if(this->m_RadiusWeightedSkeleton) {
						skit.Set(value);
					}else{
						skit.Set(1);
					}
				}
            }else{
                skit.Set(0);
            }
            ++dIt;
            ++skit;
			++aofIt;
        }
    }

}
#endif //SKELTOOLS_itkAOFAnchoredSkeletonImageFilterBase_hxx
