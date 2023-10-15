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
// Created by tabish on 2023-09-14.
//

#ifndef SKELTOOLS_itkLocalConnectivityImageFilter_h
#define SKELTOOLS_itkLocalConnectivityImageFilter_h

#include <unordered_set>

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkConstNeighborhoodIterator.h>
#include <itkImageToImageFilter.h>

namespace itk {
    /// 1. manual instantation
    template<typename TInputImage,
			 typename TOutputImage = TInputImage>
    class ITK_TEMPLATE_EXPORT  LocalConnectivityImageFilter :
            public ImageToImageFilter<TInputImage, TOutputImage> {
    public:
        /** Standard class typedefs. */
        using Self = LocalConnectivityImageFilter;
        using Superclass = ImageToImageFilter<TInputImage, TOutputImage>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        static constexpr unsigned Dimension = TInputImage::ImageDimension;


        /** Method for creation through the object factory */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro(LocalConnectivityImageFilter, ImageToImageFilter);

		using NeighborhoodIteratorType = itk::ConstNeighborhoodIterator<TInputImage>;
		using InputIteratorType = itk::ImageRegionConstIteratorWithIndex<TInputImage>;
		using OutputIteratorType = itk::ImageRegionIterator<TOutputImage>;

		using IndexType = typename TInputImage::IndexType;
		using WeightType = typename TInputImage::SizeType;
		using SetType = std::unordered_set<SizeValueType>;

		struct QPacketType{
			IndexType index;
			unsigned level;
		};

		itkSetMacro(MaxLevel, unsigned);
		itkGetConstMacro(MaxLevel, unsigned);

    protected:
        LocalConnectivityImageFilter();
        ~LocalConnectivityImageFilter() = default;

        void GenerateData() override;
        void PrintSelf(std::ostream &os, Indent indent) const override;
		void Initialize();
		SizeValueType LinearIndex(IndexType index);
	private:
		unsigned m_MaxLevel;
		WeightType m_Weight;
    };


} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLocalConnectivityImageFilter.hxx"
#endif

#endif //SKELTOOLS_itkLocalConnectivityImageFilter_hxx
