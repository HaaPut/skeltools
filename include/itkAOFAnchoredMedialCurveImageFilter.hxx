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
#ifndef SKELTOOLS_itkAOFAnchoredMedialCurveImageFilter_hxx
#define SKELTOOLS_itkAOFAnchoredMedialCurveImageFilter_hxx

#include "itkAOFAnchoredMedialCurveImageFilter.h"
#include "topology.h"

namespace itk {
    template<class TInputImage, class TOutputImage>
    bool
    AOFAnchoredMedialCurveImageFilter<TInputImage, TOutputImage>::IsSimple(IndexType index) {
        return ::topology::IsSimplePoint<TOutputImage>(this->m_Skeleton, index);
    }

    template<class TInputImage, class TOutputImage>
    bool AOFAnchoredMedialCurveImageFilter<TInputImage, TOutputImage>::IsEnd(IndexType index) {
        return ::topology::IsEndPoint<TOutputImage>(this->m_Skeleton, index) && this->m_AOF->GetPixel(index) < this->m_AOFThreshold;
    }

    template<class TInputImage, class TOutputImage>
    bool AOFAnchoredMedialCurveImageFilter<TInputImage, TOutputImage>::IsBoundary(IndexType index) {
        return ::topology::IsBoundaryPoint<TOutputImage>(this->m_Skeleton, index);
    }

/**
*  Print Self
*/
    template<class TInputImage, class TOutputImage>
    void
    AOFAnchoredMedialCurveImageFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream &os, Indent indent) const {
        Superclass::PrintSelf(os, indent);
        os << indent << "AofAnchoredMedialCurveImageFilter." << std::endl;
    }
}
#endif //SKELTOOLS_itkAOFAnchoredMedialCurveImageFilter_hxx
