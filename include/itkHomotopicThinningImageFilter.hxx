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
// Created by tabish on 2023-06-30.
//

#ifndef SKELTOOLS_itkHomotopicThinningImageFilter_hxx
#define SKELTOOLS_itkHomotopicThinningImageFilter_hxx

#include "itkHomotopicThinningImageFilter.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <itkImageFileWriter.h>
#include <itkApproximateSignedDistanceMapImageFilter.h>
#include <itkSignedMaurerDistanceMapImageFilter.h>
#include <itkDanielssonDistanceMapImageFilter.h>
#include <string>
#include <functional>

namespace itk{

    template< class PixelType>
    HomotopicThinningImageFilter<PixelType , 3>::HomotopicThinningImageFilter(){
        //m_Accessor.SetConstant(NumericTraits<PixelType>::OneValue());
        //this->DebugOn();
        m_RemoveCount = 0;
        m_InsideValue = NumericTraits<PixelType>::OneValue();
        m_OutsideValue = NumericTraits<PixelType>::ZeroValue();
        m_ChamferFilter = ChamferType::New();
        m_ThresholdFilter = ChamferThresholdFilterType::New();
    }

    template< class PixelType>
    HomotopicThinningImageFilter<PixelType , 2>::HomotopicThinningImageFilter(){
        m_Accessor.SetConstant(NumericTraits<PixelType>::OneValue());
        m_RemoveCount = 0;
        m_InsideValue = NumericTraits<PixelType>::OneValue();
        m_OutsideValue = NumericTraits<PixelType>::ZeroValue();
        m_ChamferFilter = ChamferType::New();
        m_ThresholdFilter = ChamferThresholdFilterType::New();
    }

    template< class PixelType, unsigned Dimension>
    void
    HomotopicThinningImageFilter<PixelType , Dimension>::GenerateInputRequestedRegion()
    {
        // call the superclass' implementation of this method
        Superclass::GenerateInputRequestedRegion();

        // get pointers to the input and output
        auto inputPtr = const_cast<InputImageType *>(this->GetInput());
        auto outputPtr = this->GetOutput();

        if (!inputPtr || !outputPtr)
        {
            return;
        }

        // get a copy of the input requested region (should equal the output
        // requested region)
        typename InputImageType::RegionType inputRequestedRegion = inputPtr->GetRequestedRegion();

        // pad the input requested region by one, the size of neighbourhood for labelling
        inputRequestedRegion.PadByRadius(1);

        // crop the input requested region at the input's largest possible region
        if (inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()))
        {
            inputPtr->SetRequestedRegion(inputRequestedRegion);
            return;
        }
        else
        {
            // Couldn't crop the region (requested region is outside the largest
            // possible region).  Throw an exception.

            // store what we tried to request (prior to trying to crop)
            inputPtr->SetRequestedRegion(inputRequestedRegion);

            // build an exception
            InvalidRequestedRegionError e(__FILE__, __LINE__);
            e.SetLocation(ITK_LOCATION);
            e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
            e.SetDataObject(inputPtr);
            throw e;
        }
    }

    template< class PixelType>
    bool HomotopicThinningImageFilter<PixelType , 3>::isRemovable(IndexType index){
        auto cbar = ::topology::computeCbar<OutputImageType>(m_Output, index);
        auto cstar = ::topology::computeCstar<OutputImageType>(m_Output, index);
        auto label = ::topology::TopologicalLabel(cbar,cstar);
        if(label == ::topology::ObjectPointType::Simple)
            return true;
        return false;
    }

    template< class PixelType>
    void
    HomotopicThinningImageFilter<PixelType , 3>::GenerateData() {
        this->AllocateOutputs();
        InputImagePointer input = const_cast<InputImageType *>(this->GetInput(0));
        this->m_Output = this->GetOutput(0);
        this->m_RemoveCount = 0;
        this->m_Count = 0;
        const InternalPixelType maximumDistance = (static_cast<InternalPixelType>(m_MaxIterations))*m_MinSpacing;

        input->SetRequestedRegionToLargestPossibleRegion();

        m_Output->ReleaseDataFlagOff();

        InputConstIteratorType skit = InputConstIteratorType(input, input->GetRequestedRegion());
        OutputIteratorType outIt = OutputIteratorType(m_Output, m_Output->GetRequestedRegion());

        PixelType value;
        skit.GoToBegin();
        outIt.GoToBegin();
        while(!skit.IsAtEnd()){
            value = skit.Get();
            if(value >= this->m_LowerThreshold)
                outIt.Set(m_InsideValue);
            else
                outIt.Set(m_OutsideValue);
            ++skit;
            ++outIt;
        }

        m_ThresholdFilter->SetInput(input);
        m_ThresholdFilter->SetLowerThreshold( this->m_LowerThreshold);
        m_ThresholdFilter->SetUpperThreshold(NumericTraits<PixelType>::max());
        m_ThresholdFilter->SetInsideValue(maximumDistance+10*m_MinSpacing);
        m_ThresholdFilter->SetOutsideValue(NumericTraits<InternalPixelType>::ZeroValue());

        m_ChamferFilter->SetInput(m_ThresholdFilter->GetOutput());
        m_ChamferFilter->SetMaximumDistance(maximumDistance+m_MinSpacing);
        m_ChamferFilter->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());
        m_ChamferFilter->Update();
        auto distanceMap = m_ChamferFilter->GetOutput();

        using DistIteratorType = ImageRegionIteratorWithIndex< InternalImageType >;
        DistIteratorType dIt = DistIteratorType(distanceMap, distanceMap->GetRequestedRegion());

        using NodeType = std::pair<itk::Index<3>, float>;
        const auto cmp = [](NodeType const& left, NodeType const& right) { return left.second > right.second; };
        std::priority_queue<NodeType, std::vector<NodeType>, decltype(cmp)> q(cmp);

        dIt.GoToBegin();
        while(!dIt.IsAtEnd()) {
            if(dIt.Get() > 0){
                q.push({dIt.GetIndex(), std::abs(dIt.Get())});
            }
            ++dIt;
        }

        float current_distance = 0;
        IndexType current_index;
        //unsigned label;
        while(current_distance < maximumDistance && !q.empty()){
            NodeType current_node = q.top();
            current_distance = current_node.second;
            current_index = current_node.first;
            q.pop();
            if (isRemovable(current_index)){
                m_Output->SetPixel(current_index, m_OutsideValue);
                ++this->m_RemoveCount;
            }
            ++this->m_Count;
        }
        itkDebugMacro("Removed " + std::to_string(this->m_RemoveCount) + " of " + std::to_string(this->m_Count) + " voxels");
    }

    template< class PixelType>
    bool
    HomotopicThinningImageFilter<PixelType , 2>::isSimple2(IndexType index){
        std::array<int,8> nbrs = {{0,0,0,0,0,0,0,0}};
        int numNeighbors = 0;
        int numEdges = 0;
        for(size_t i = 0 ;i < m_Neighbors8.size(); ++i){
            nbrs[i] = m_Accessor.GetPixel(index + m_Neighbors8[i], m_Output) > 0;
            size_t j = (i+1)%8;
            nbrs[j] = m_Accessor.GetPixel(index + m_Neighbors8[j], m_Output) > 0;
            if(nbrs[i] == 1 && nbrs[j] == 1){
                numNeighbors+=2;
                ++numEdges;
            }else if(nbrs[i] == 1 || nbrs[j] == 1){
                ++numNeighbors;
            }
        }
        //remove double counted
        numNeighbors /=2;
        //add corner diagonals if corner is 0
        for (size_t i = 0; i < 8; i+=2){
            numEdges += (nbrs[(i+7)%8] == 1 && nbrs[i] == 0 && nbrs[(i+1)%8] == 1)? 1: 0;
        }
        return (numNeighbors - numEdges == 1);
    }

    template< class PixelType>
    void
    HomotopicThinningImageFilter<PixelType , 2>::GenerateData() {
        this->AllocateOutputs();
        auto input = const_cast<InputImageType *>(this->GetInput(0));
        this->m_Output = this->GetOutput(0);
        const InternalPixelType maximumDistance = (static_cast<InternalPixelType>(m_MaxIterations))*m_MinSpacing;
        input->SetRequestedRegionToLargestPossibleRegion();

        m_Output->ReleaseDataFlagOff();
        InputConstIteratorType skit = InputConstIteratorType(input, input->GetRequestedRegion());
        OutputIteratorType outIt = OutputIteratorType(m_Output, m_Output->GetRequestedRegion());

        PixelType value;
        skit.GoToBegin();
        outIt.GoToBegin();
        while(!skit.IsAtEnd()){
            value = skit.Get();
            if(value >= this->m_LowerThreshold)
                outIt.Set(m_InsideValue);
            else
                outIt.Set(m_OutsideValue);
            ++skit;
            ++outIt;
        }

        m_ThresholdFilter->SetInput(input);
        m_ThresholdFilter->SetLowerThreshold( this->m_LowerThreshold);
        m_ThresholdFilter->SetUpperThreshold(NumericTraits<PixelType>::max());
        m_ThresholdFilter->SetInsideValue(maximumDistance+10*m_MinSpacing);
        m_ThresholdFilter->SetOutsideValue(NumericTraits<InternalPixelType>::ZeroValue());

        auto writer = itk::ImageFileWriter<InternalImageType>::New();
        writer->SetInput(m_ThresholdFilter->GetOutput());
        writer->SetFileName("/tmp/t0.tif");
        writer->Update();

        m_ChamferFilter->SetInput(m_ThresholdFilter->GetOutput());
        m_ChamferFilter->SetMaximumDistance(maximumDistance+1);
        m_ChamferFilter->SetNumberOfWorkUnits(this->GetNumberOfWorkUnits());

        // Create the distance map
        m_ChamferFilter->Update();
        auto distanceMap = m_ChamferFilter->GetOutput();

        using DistIteratorType = ImageRegionIteratorWithIndex< InternalImageType >;
        DistIteratorType dIt = DistIteratorType(distanceMap, distanceMap->GetRequestedRegion());

        using NodeType = std::pair<itk::Index<2>, float>;
        const auto cmp = [](NodeType left, NodeType right) { return left.second > right.second; };
        std::priority_queue<NodeType, std::vector<NodeType>, decltype(cmp)> q(cmp);

        dIt.GoToBegin();
        while(!dIt.IsAtEnd()) {
            if(dIt.Get() > 0){
                q.push({dIt.GetIndex(), dIt.Get()});
            }
            ++dIt;
        }

        float current_distance = 0;
        IndexType current_index;
        this->m_RemoveCount = 0;
        this->m_Count = 0;
        while(current_distance < maximumDistance && !q.empty()){
            NodeType current_node = q.top();
            current_distance = current_node.second;
            current_index = current_node.first;
            q.pop();
            if(isSimple2(current_index)){
                m_Output->SetPixel(current_index,m_OutsideValue);
                ++this->m_RemoveCount;
            }
            ++this->m_Count;
        }
        itkDebugMacro("Removed " + std::to_string(this->m_RemoveCount) + " of " + std::to_string(this->m_Count) + " voxels");
    }
/**
 *  Print Self
 */
    template< class PixelType , unsigned Dimension>
    void
    HomotopicThinningImageFilter<PixelType , Dimension>::PrintSelf(std::ostream& os, Indent indent) const
    {
        Superclass::PrintSelf(os,indent);
        os << indent << "Boundary distance ordered homotopy preserving  thinning in " << Dimension << " dimensions" << std::endl;
    }
} // namespace itk

#endif //itkHomotopicThinningImageFilter_hxx