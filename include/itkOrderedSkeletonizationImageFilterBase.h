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
// Created by tabish on 2023-10-03.
//

#ifndef SKELTOOLS_itkOrderedSkeletonizationImageFilterBase_h
#define SKELTOOLS_itkOrderedSkeletonizationImageFilterBase_h

#include <queue>

#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkNeighborhoodIterator.h>
//#include <itkDanielssonDistanceMapImageFilter.h>
#include <itkConstantBoundaryCondition.h>

namespace itk {
    /// 1. manual instantation
    template<class TInputImage,
            class TOutputImage = TInputImage>
    class ITK_TEMPLATE_EXPORT  OrderedSkeletonizationImageFilterBase :
            public ImageToImageFilter<TInputImage, TOutputImage> {
    public:
        /** Standard class typedefs. */
        using Self = OrderedSkeletonizationImageFilterBase;
        using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        static constexpr unsigned Dimension = TInputImage::ImageDimension;


        /** Run-time type information (and related methods). */
        itkTypeMacro(OrderedSkeletonizationImageFilterBase, ImageToImageFilter);

        using InputPointerType = typename TInputImage::ConstPointer;
        using OutputPointerType = typename TOutputImage::Pointer;
        using OutputPixelType = typename TOutputImage::PixelType;

        using BoundaryConditionType = ConstantBoundaryCondition<TOutputImage>;
        using OutputIteratorType = ImageRegionIterator<TOutputImage>;
        using InputConstIteratorType = ImageRegionConstIterator<TInputImage>;
        using OutputNeighborhoodIteratorType = itk::NeighborhoodIterator<TOutputImage, BoundaryConditionType>;
        using IndexType = typename TOutputImage::IndexType;
        using SizeType = typename TOutputImage::SizeType;
        using RegionType = ImageRegion<Dimension>;
        using PixelType = typename TInputImage::PixelType;


        using PriorityValueType = float;
        using PriorityImageType = Image<PriorityValueType, Dimension>;
        using PriorityImagePointerType = typename PriorityImageType::Pointer;
        using PriorityImageConstIteratorType = ImageRegionConstIterator<PriorityImageType>;
        using PriorityNeighborhoodIteratorType = itk::NeighborhoodIterator<PriorityImageType>;
        typedef
        struct Pixel {
        private:
            IndexType pixelIndex;
            PriorityValueType priority;

        public:
            PriorityValueType GetPriority() const { return priority; };

            void SetIndex(IndexType p) { pixelIndex = p; };

            void SetValue(PriorityValueType v) { priority = v; };

            IndexType GetIndex() { return pixelIndex; };

            PriorityValueType GetValue() { return priority; };
        } Pixel;

        struct Greater{
			//: public std::binary_function<Pixel, Pixel, bool> {
			//public:
            bool operator()(const Pixel &p1, const Pixel &p2) const {
                return p1.GetPriority() > p2.GetPriority();
            }
        };

        using HeapContainer = std::vector<Pixel>;
        using HeapType = std::priority_queue<Pixel, HeapContainer, Greater>;

        void SetPriorityImage(PriorityImagePointerType priorityImage){
            m_PriorityImage = priorityImage;
        }
        PriorityImagePointerType GetPriorityImage(){
            return m_PriorityImage;
        }

        itkSetMacro(RadiusWeightedSkeleton,bool);
        itkGetConstMacro(RadiusWeightedSkeleton, bool);

    protected:
        OrderedSkeletonizationImageFilterBase();
        ~OrderedSkeletonizationImageFilterBase() = default;

        void GenerateData() override;

        virtual bool IsEnd(IndexType index) = 0;
        virtual void Initialize();
        OutputPointerType m_Skeleton;

        virtual bool IsSimple(IndexType index) = 0;
        virtual bool IsBoundary(IndexType index) = 0;

        OutputPointerType m_Queued;
        PriorityImagePointerType m_PriorityImage;
        bool m_RadiusWeightedSkeleton;
    };


} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkOrderedSkeletonizationImageFilterBase.hxx"
#endif

#endif //SKELTOOLS_itkOrderedSkeletonizationImageFilterBase_hxx
