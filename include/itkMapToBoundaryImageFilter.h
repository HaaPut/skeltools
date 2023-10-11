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

#ifndef SKELTOOLS_itkMapToBoundaryImageFilter_h
#define SKELTOOLS_itkMapToBoundaryImageFilter_h

#include <itkListSample.h>
#include <itkKdTreeGenerator.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

//#include <itkNeighborhoodIterator.h>
//#include <itkDanielssonDistanceMapImageFilter.h>
//#include <itkConstantBoundaryCondition.h>

namespace itk {
    /// 1. manual instantation
    template<typename TInputImage,
			 typename TSkeletonImage = TInputImage,
			 typename TOutputImage = TSkeletonImage>
    class ITK_TEMPLATE_EXPORT  MapToBoundaryImageFilter :
            public ImageToImageFilter<TInputImage, TOutputImage> {
    public:
        /** Standard class typedefs. */
        using Self = MapToBoundaryImageFilter;
        using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        static constexpr unsigned Dimension = TInputImage::ImageDimension;


        /** Method for creation through the object factory */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro(MapToBoundaryImageFilter, ImageToImageFilter);

        using SkeletonPointerType = typename TSkeletonImage::Pointer;
		using SkeletonIteratorType = ImageRegionConstIteratorWithIndex<TSkeletonImage>;

        using OutputPointerType = typename TOutputImage::Pointer;
		using OutputIteratorType = itk::ImageRegionIteratorWithIndex<TOutputImage>;

		// using OutputPixelType = typename TOutputImage::PixelType;
		using InputPixelType = typename TInputImage::PixelType;

		using MeasurementVectorType = Point<float, Dimension>;
		using SampleType = Statistics::ListSample<MeasurementVectorType>;
		using TreeGeneratorType = Statistics::KdTreeGenerator<SampleType>;
		using TreeType = typename TreeGeneratorType::KdTreeType;

		using InputIteratorType = itk::ImageRegionConstIteratorWithIndex<TInputImage>;

		//using SkeletonIteratorType = itk::ImageRegionConstIteratorWithIndex<TSkeletonImage>;

		// typename TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();

        // using BoundaryConditionType = ConstantBoundaryCondition<TOutputImage>;
        // using OutputIteratorType = ImageRegionIterator<TOutputImage>;
        // using InputConstIteratorType = ImageRegionConstIterator<TInputImage>;
        // using OutputNeighborhoodIteratorType = itk::NeighborhoodIterator<TOutputImage, BoundaryConditionType>;
        // using IndexType = typename TOutputImage::IndexType;
        // using SizeType = typename TOutputImage::SizeType;
        // using RegionType = ImageRegion<Dimension>;
        // using PixelType = typename TInputImage::PixelType;


        // using PriorityValueType = float;
        // using PriorityImageType = Image<PriorityValueType, Dimension>;
        // using PriorityImagePointerType = typename PriorityImageType::Pointer;
        // using PriorityImageConstIteratorType = ImageRegionConstIterator<PriorityImageType>;
        // using PriorityNeighborhoodIteratorType = itk::NeighborhoodIterator<PriorityImageType>;
        // typedef
        // struct Pixel {
        // private:
        //     IndexType pixelIndex;
        //     PriorityValueType priority;

        // public:
        //     PriorityValueType GetPriority() const { return priority; };

        //     void SetIndex(IndexType p) { pixelIndex = p; };

        //     void SetValue(PriorityValueType v) { priority = v; };

        //     IndexType GetIndex() { return pixelIndex; };

        //     PriorityValueType GetValue() { return priority; };
        // } Pixel;

        // struct Greater : public std::binary_function<Pixel, Pixel, bool> {
        // public:
        //     bool operator()(const Pixel &p1, const Pixel &p2) const {
        //         return p1.GetPriority() > p2.GetPriority();
        //     }
        // };

        // using HeapContainer = std::vector<Pixel>;
        // using HeapType = std::priority_queue<Pixel, HeapContainer, Greater>;

        void SetSkeletonImage(SkeletonPointerType skeletonImage){
            m_SkeletonImage = skeletonImage;
        }
        SkeletonPointerType GetSkeletonImage(){
            return m_SkeletonImage;
        }


    protected:
        MapToBoundaryImageFilter();
        ~MapToBoundaryImageFilter() = default;

        void GenerateData() override;
        void PrintSelf(std::ostream &os, Indent indent) const override;

        SkeletonPointerType m_SkeletonImage;

    };


} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMapToBoundaryImageFilter.hxx"
#endif

#endif //SKELTOOLS_itkMapToBoundaryImageFilter_hxx
