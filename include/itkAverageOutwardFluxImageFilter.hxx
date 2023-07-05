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

#ifndef _itkAverageOutwardFluxImageFilter_hxx
#define _itkAverageOutwardFluxImageFilter_hxx

#include "itkAverageOutwardFluxImageFilter.h"
#include <itkBinaryThresholdImageFilter.h>

namespace itk {

    template<class TInputImage, class TOutputImage>
    AverageOutwardFluxImageFilter<TInputImage, TOutputImage>::AverageOutwardFluxImageFilter() {
        this->NormalsToASphere();
    }

    template<class TInputImage, class TOutputImage>
    void
    AverageOutwardFluxImageFilter<TInputImage, TOutputImage>::NormalsToASphere() {
        ::std::random_device rd;  //Will be used to obtain a seed for the random number engine
        ::std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        ::std::uniform_real_distribution<double> dis(0.0, 1.0);
        constexpr unsigned N = 60;
        ::std::vector<VectorType> forces;
        VectorType n;
        for (size_t d = 0; d < Dimension; ++d) n[d] = 0;
        forces.push_back(n);
        n[0] = 1.0;
        this->m_Points.push_back(n);
        for (size_t count = 0; count < N - 1; ++count) {
            for (size_t d = 0; d < Dimension; ++d) {
                n[d] = dis(gen);
            }
            n.Normalize();
            //hope that no two point are the same.
            this->m_Points.push_back(n);
            forces.push_back(0.0 * n);
        }

        double r;
        VectorType forceVector;
        int iteration = 0;
        do {
            for (size_t i = 1; i < N; ++i) {
                forces[i] *= 0.0;                        // Initialize force vector zero
                for (size_t j = 0; j < N; ++j) {
                    if (i == j) continue;
                    forceVector = this->m_Points[i] - this->m_Points[j];
                    r = forceVector.GetNorm();
                    r *= r;
                    forceVector /= r;
                    forces[i] += forceVector; // Get contributing forces from other particles.
                }
            }
            for (size_t i = 1; i < N; ++i) {
                //move point on sphere.
                this->m_Points[i] += forces[i];
                this->m_Points[i].Normalize();
            }
        } while (++iteration < 50);
        itkDebugMacro("Finished Computing normals to sphere.");
    }


    template<class TInputImage, class TOutputImage>
    void
    AverageOutwardFluxImageFilter<TInputImage, TOutputImage>::ComputeSpokeField() {
        auto input =  this->GetInput();

        //flip object to get Closest Boundary Point Transform
        using ThresholdFilterType = BinaryThresholdImageFilter< InputImageType , BinaryImageType >;
        typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
        thresholdFilter->SetInput(input);
        thresholdFilter->SetLowerThreshold(NumericTraits<InputPixelType>::OneValue());
        thresholdFilter->SetUpperThreshold(NumericTraits<InputPixelType>::max());
        thresholdFilter->SetOutsideValue(NumericTraits<BinaryPixelType>::OneValue());
        thresholdFilter->SetInsideValue(NumericTraits<BinaryPixelType>::ZeroValue());

        auto distanceMapImageFilter = SignedDistanceMapImageFilterType::New();
        distanceMapImageFilter->SetInput(thresholdFilter->GetOutput());
        distanceMapImageFilter->SetInsideIsPositive(true);
        distanceMapImageFilter->UseImageSpacingOn();
        distanceMapImageFilter->Update();

        m_DistanceMap = distanceMapImageFilter->GetOutput();
        m_ClosestPointTransform = distanceMapImageFilter->GetVectorDistanceMap();

        itkDebugMacro("Finished Computing Spoke filed.");
    }


    template<class TInputImage, class OutputImageType>
    void
    AverageOutwardFluxImageFilter<TInputImage, OutputImageType>::GenerateData() {
        ComputeSpokeField();

        auto output = this->GetOutput();
        const auto input = this->GetInput();

        this->AllocateOutputs();

        auto spacing = input->GetSpacing();
        double minspacing = 1e9;
        for(size_t d = 0 ;d < Dimension; ++d) minspacing = std::min(minspacing,spacing[d]);

        auto aofIt = ImageRegionIteratorWithIndex< OutputImageType >(output, output->GetLargestPossibleRegion());
        auto dIt = ImageRegionIterator< DistanceImageType >(m_DistanceMap, m_DistanceMap->GetLargestPossibleRegion());

        double f = 0;
        VectorType spokeVector;
        IndexType currentIndex, spokeIndex, boundaryIndex;
        OffsetType spokeOffset;
        for (aofIt.GoToBegin(), dIt.GoToBegin(); !aofIt.IsAtEnd(); ++aofIt,++dIt) {
            f = 0.0;
            currentIndex = aofIt.GetIndex();
            if (dIt.Get() < -1.5*minspacing) {
                for (auto point: this->m_Points) {
                    //get spoke vector.
                    for (size_t d = 0; d < TInputImage::ImageDimension; ++d) {
                        spokeIndex[d] = std::floor((static_cast<double>(currentIndex[d]) + point[d] + 0.5));
                    }
                    spokeOffset = m_FieldAccessor.GetPixel(spokeIndex, m_ClosestPointTransform);
                    for (size_t d = 0; d < TInputImage::ImageDimension; ++d) {
                        boundaryIndex[d] = currentIndex[d] + spokeOffset[d];
                        spokeVector[d] = boundaryIndex[d] - (static_cast<double>(currentIndex[d]) + point[d] + 0.5);
                    }
                    spokeVector.Normalize();
                    //compute dot product of normalized vectors
                    f -= (spokeVector * point);
                }
            }
            aofIt.Set(f);
        }
    }


/**
*  Print Self
*/
    template<class TInputImage, class TOutputPixelType>
    void
    AverageOutwardFluxImageFilter<TInputImage, TOutputPixelType>::PrintSelf(std::ostream &os, Indent indent) const {
        Superclass::PrintSelf(os, indent);
        os << indent << "AverageOutwardFluxImageFilter." << std::endl;
    }

}

#endif
