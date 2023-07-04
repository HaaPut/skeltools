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
// Created by tabish on 2023-07-03.
//
#ifndef SKELTOOLS_itkMedialCurveImageFilter_hxx
#define SKELTOOLS_itkMedialCurveImageFilter_hxx
#include "itkMedialCurveImageFilter.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkImageFileWriter.h>
#include "topology.h"

namespace itk {
    template<class TInputImage, class TOutputImage>
    MedialCurveImageFilter<TInputImage, TOutputImage>::MedialCurveImageFilter() {
        m_PriorityImage = nullptr;
    }

    template<class TInputImage, class TOutputImage>
    bool
    MedialCurveImageFilter<TInputImage, TOutputImage>::IsSimple(IndexType index) {
        return ::topology::IsSimplePoint<TOutputImage>(this->m_Skeleton, index);
    }

    template<class TInputImage, class TOutputImage>
    bool MedialCurveImageFilter<TInputImage, TOutputImage>::IsEnd(IndexType index) {
        return ::topology::IsEndPoint<TOutputImage>(this->m_Skeleton, index);
    }

    template<class TInputImage, class TOutputImage>
    bool MedialCurveImageFilter<TInputImage, TOutputImage>::IsBoundary(IndexType index) {
        return ::topology::IsBoundaryPoint<TOutputImage>(this->m_Skeleton, index);
    }


    template<class TInputImage, class TOutputImage>
    void
    MedialCurveImageFilter<TInputImage, TOutputImage>::Initialize() {
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

        if(m_PriorityImage == nullptr) {
            using PriorityFilterType = DanielssonDistanceMapImageFilter<TInputImage, PriorityImageType>;
            auto priorityFilter = PriorityFilterType::New();
            priorityFilter->SetInput(binaryInput);
            priorityFilter->UseImageSpacingOn();
            priorityFilter->Update();
            this->m_PriorityImage = priorityFilter->GetOutput();
            m_PriorityImage->ReleaseDataFlagOff();
        }
        this->m_Skeleton = this->GetOutput();
        this->AllocateOutputs();

        this->m_Queued = TOutputImage::New();
        this->m_Queued->SetSpacing(this->m_PriorityImage->GetSpacing());
        this->m_Queued->SetOrigin(this->m_PriorityImage->GetOrigin());
        this->m_Queued->SetRegions(this->m_PriorityImage->GetRequestedRegion());
        this->m_Queued->Allocate();

        OutputIteratorType skit(this->m_Skeleton, this->m_Skeleton->GetLargestPossibleRegion());
        InputConstIteratorType it(binaryInput, binaryInput->GetLargestPossibleRegion());
        it.GoToBegin();
        skit.GoToBegin();
        while (!skit.IsAtEnd()) {
            if(it.Get() == 0) {
                skit.Set(1);
            }else{
                skit.Set(0);
            }
            ++it;
            ++skit;
        }
    }

    template<class TInputImage, class TOutputImage>
    void
    MedialCurveImageFilter<TInputImage, TOutputImage>::GenerateData() {
        Initialize();

        //Iterators
        PriorityImageConstIteratorType dit(this->m_PriorityImage, this->m_PriorityImage->GetRequestedRegion());
        OutputIteratorType skit(this->m_Skeleton, this->m_Skeleton->GetRequestedRegion());
        OutputIteratorType qit(this->m_Queued, this->m_Queued->GetRequestedRegion());

        typename OutputNeighborhoodIteratorType::RadiusType radius;
        radius.Fill(1);
        PriorityNeighborhoodIteratorType dnit(radius, this->m_PriorityImage,
                                              this->m_PriorityImage->GetRequestedRegion());
        radius.Fill(1);
        OutputNeighborhoodIteratorType sknit(radius, this->m_Skeleton, this->m_Skeleton->GetRequestedRegion());
        radius.Fill(1);
        OutputNeighborhoodIteratorType qnit(radius, this->m_Queued, this->m_Queued->GetRequestedRegion());

        //First step...
        IndexType q;
        PriorityValueType priority = 0;
        HeapType heap;
        Pixel node;

        for (skit.GoToBegin(), dit.GoToBegin(), qit.GoToBegin();
             !skit.IsAtEnd() && !dit.IsAtEnd() && !qit.IsAtEnd(); ++skit, ++dit, ++qit) {
            q = skit.GetIndex();
            if (this->IsBoundary(q)) {
                if (IsSimple(q)) {
                    //Simple pixel
                    node.SetIndex(q);
                    priority = dit.Get();
                    node.SetValue(priority);
                    heap.push(node);
                    qit.Set(1);
                } else qit.Set(0);
            } else {
                qit.Set(0);
            }
        }

        //Second step

        IndexType r;

        while (!heap.empty()) {

            node = heap.top();
            heap.pop();

            q = node.GetIndex();

            qnit.SetLocation(q);
            qnit.SetCenterPixel(0);

            if (this->IsSimple(q)) {
                if (this->IsEnd(q)) {
                    //do nothing
                } else {
                    sknit.SetLocation(q);
                    sknit.SetCenterPixel(0); //Deletion from object
                    dnit.SetLocation(q);

                    for (unsigned int i = 0; i < sknit.Size(); i++) {
                        if (sknit.GetPixel(i) > 0) {
                            //Object pixel
                            r = sknit.GetIndex(i);
                            if (qnit.GetPixel(i) == 0) {
                                //Not queued pixel
                                if (this->IsSimple(r)) {
                                    priority = dnit.GetPixel(i);
                                    node.SetIndex(r);
                                    node.SetValue(priority);
                                    heap.push(node);
                                    qnit.SetPixel(i, 1);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

/**
*  Print Self
*/
    template<class TInputImage, class TOutputImage>
    void
    MedialCurveImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream &os, Indent indent) const {
        Superclass::PrintSelf(os, indent);
        os << indent << "MedialCurveImageFilter." << std::endl;
    }
}
#endif //SKELTOOLS_itkMedialCurveImageFilter_hxx
