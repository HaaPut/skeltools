//**********************************************************
//Copyright 2023 Tabish Syed
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
// Created by tabish on 2023-07-05.
//


#ifndef _itkAverageOutwardFluxImageFilter_h
#define _itkAverageOutwardFluxImageFilter_h

#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkSignedDanielssonDistanceMapImageFilter.h>
#include <vector>
#include <random>
#include <cmath>


namespace itk
{

/// \brief This class computes the average outward flux associated to an object From its
/// Spoke Vectors. For detailed explanation on Spoke vectors see for example"
/// "S. Bouix, K. Siddiqi, and A. Tannenbaum. Flux driven automatic centerline extraction".


template< class TInputImage,
	      class TOutputImage = Image<float,TInputImage::ImageDimension>>
    class ITK_TEMPLATE_EXPORT  AverageOutwardFluxImageFilter:
	public ImageToImageFilter<TInputImage, TOutputImage >
{
	public:
        using OutputImageType = TOutputImage;
        using InputImageType = TInputImage;
        /** Standard class typedefs. */
        using Self = AverageOutwardFluxImageFilter;
        using Superclass = ImageToImageFilter<InputImageType, OutputImageType>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        static constexpr unsigned Dimension = InputImageType::ImageDimension;
		/** Method for creation through the object factory */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(AverageOutwardFluxImageFilter, ImageToImageFilter );

        using InputPixelType = typename InputImageType::PixelType;
        using IndexType = typename InputImageType::IndexType;
        using OffsetType = typename InputImageType::OffsetType;
        using VectorType = Vector<double, Dimension>;

        using BinaryPixelType = unsigned char;
        using BinaryImageType = Image<BinaryPixelType, Dimension>;
        using DistanceImageType = Image<float, Dimension>;

        using SignedDistanceMapImageFilterType = SignedDanielssonDistanceMapImageFilter<BinaryImageType, DistanceImageType>;
        using OffSetImageType =  typename SignedDistanceMapImageFilterType::VectorImageType ;

        using BoundaryConditionType = itk::ZeroFluxNeumannBoundaryCondition<OffSetImageType>;
	protected:
        AverageOutwardFluxImageFilter();
        ~AverageOutwardFluxImageFilter() = default;

        /// \brief Compute the AOF.
        void GenerateData() override;

        void PrintSelf(std::ostream& os, Indent indent) const;
        void ComputeInwardFluxOn(){
            m_InwardFlux = true;
        }
private:
        void NormalsToASphere();
        void ComputeSpokeField();

        std::vector< VectorType > m_Points;
        typename DistanceImageType::Pointer m_DistanceMap;
        typename OffSetImageType::Pointer m_ClosestPointTransform;
        BoundaryConditionType m_FieldAccessor;
        bool m_InwardFlux;
    };
} // end namespace itk
#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAverageOutwardFluxImageFilter.hxx"
#endif

#endif // _itkAverageOutwardFluxImageFilter_h
