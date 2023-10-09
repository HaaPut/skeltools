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

#ifndef SKELTOOLS_itkAOFAnchoredSkeletonImageFilterBase_h
#define SKELTOOLS_itkAOFAnchoredSkeletonImageFilterBase_h

#include <queue>

#include "itkOrderedSkeletonizationImageFilterBase.h"

#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodIterator.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkConstantBoundaryCondition.h>

namespace itk {
    /// 1. manual instantation
    template<class TInputImage,
            class TOutputImage = TInputImage>
    class ITK_TEMPLATE_EXPORT  AOFAnchoredSkeletonImageFilterBase :
            public OrderedSkeletonizationImageFilterBase<TInputImage, TOutputImage> {
    public:
        /** Standard class typedefs. */
        using Self = AOFAnchoredSkeletonImageFilterBase;
        using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        static constexpr unsigned Dimension = TInputImage::ImageDimension;


        /** Run-time type information (and related methods). */
        itkTypeMacro(AOFAnchoredSkeletonImageFilterBase, OrderedSkeletonizationImageFilterBase);

        using IndexType = typename TOutputImage::IndexType;
        using InputPointerType = typename TInputImage::ConstPointer;

        using PixelType = typename TInputImage::PixelType;
        using OutputIteratorType = ImageRegionIterator<TOutputImage>;

        using AOFValueType = float;
        using AOFImageType = Image<AOFValueType, Dimension>;
        using AOFImagePointerType = typename AOFImageType::Pointer;
        using AOFImageConstIteratorType = ImageRegionConstIterator<AOFImageType>;

		using PriorityValueType = float;
        using PriorityImageType = Image<PriorityValueType, Dimension>;
        using PriorityImagePointerType = typename PriorityImageType::Pointer;
        using PriorityImageConstIteratorType = ImageRegionConstIterator<PriorityImageType>;


        void SetAOFImage(AOFImagePointerType aofImage){
            m_AOF = aofImage;
        }
        AOFImagePointerType GetAOFImage(){
            return m_AOF;
        }

        itkSetMacro(AOFThreshold, AOFValueType);
        itkGetConstMacro(AOFThreshold, AOFValueType);

		itkSetMacro(Quick, bool);
        itkGetConstMacro(Quick, bool);

    protected:
        AOFAnchoredSkeletonImageFilterBase();
        ~AOFAnchoredSkeletonImageFilterBase() = default;

		void Initialize() override;

		bool m_Quick;

        AOFImagePointerType m_AOF;
        AOFValueType m_AOFThreshold;
    };


} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAOFAnchoredSkeletonImageFilterBase.hxx"
#endif

#endif //SKELTOOLS_itkAOFAnchoredSkeletonImageFilterBase_hxx
