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
// Created by tabish on 2023-10-10.
//

#ifndef SKELTOOLS_itkAOFAnchoredMedialSurfaceImageFilter_h
#define SKELTOOLS_itkAOFAnchoredMedialSurfaceImageFilter_h

#include "itkAOFAnchoredSkeletonImageFilterBase.h"

namespace itk {
    /// 1. manual instantation
    template<class TInputImage,
            class TOutputImage = TInputImage>
    class ITK_TEMPLATE_EXPORT  AOFAnchoredMedialSurfaceImageFilter :
            public AOFAnchoredSkeletonImageFilterBase<TInputImage, TOutputImage> {
    public:
        /** Standard class typedefs. */
        using Self = AOFAnchoredMedialSurfaceImageFilter;
        using Superclass = AOFAnchoredSkeletonImageFilterBase<TInputImage, TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        static constexpr unsigned Dimension = TInputImage::ImageDimension;

        /** Method for creation through the object factory */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro(AOFAnchoredMedialSurfaceImageFilter, AOFAnchoredSkeletonImageFilterBase);

        using IndexType = typename TOutputImage::IndexType;

    protected:
        AOFAnchoredMedialSurfaceImageFilter();
        ~AOFAnchoredMedialSurfaceImageFilter() = default;

        bool IsEnd(IndexType index) override;
        bool IsSimple(IndexType index) override;
        bool IsBoundary(IndexType index) override;

		void PrintSelf(std::ostream &os, Indent indent) const override;

    };


} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAOFAnchoredMedialSurfaceImageFilter.hxx"
#endif

#endif //SKELTOOLS_itkAOFAnchoredMedialSurfaceImageFilter_hxx
