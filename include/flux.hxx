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


#ifndef SKELTOOLS_FLUX_HXX
#define SKELTOOLS_FLUX_HXX

#include <utility>
#include <sstream>
#include <vector>

#include <itkImage.h>
#include <itkLogger.h>
#include <itkImageFileReader.h>
#include <itkChangeInformationImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkSignedDanielssonDistanceMapImageFilter.h>

#include "itkCommandLineArgumentParser.h"


template<class TObjectImage, class TDistanceImage>
std::pair< typename TDistanceImage::Pointer,
           typename itk::Image<itk::Vector<float, TObjectImage::ImageDimension>,TObjectImage::ImageDimension>::Pointer>
computeObjectSignedDistanceSpokesPair(const itk::CommandLineArgumentParser::Pointer &parser,
                                      const itk::Logger::Pointer &logger){
    logger->Info("Starting computation of Distance map + spoke vector field\n");
    static_assert(TObjectImage::ImageDimension == TDistanceImage::ImageDimension);
    using ObjectImageType = TObjectImage;
    using DistanceImageType = TDistanceImage;
    using InternalImageType = DistanceImageType;
    constexpr unsigned Dimension = ObjectImageType::ImageDimension;
    using FieldImageType = itk::Image<itk::Vector<float,Dimension>, Dimension>;


    std::string  inputFilename;
    if (!parser->GetCommandLineArgument("-input", inputFilename)) {
        logger->Error("Input file not specified\n");
        return {};
    }else{
        logger->Info("Set input object filename to : " + inputFilename + "\n" );
    }

    using ReaderType = itk::ImageFileReader<ObjectImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFilename);
    reader->Update();

    std::vector<float> objectSpacing(Dimension,1);
    if(parser->GetCommandLineArgument("-spacing",objectSpacing)){
        logger->Debug("Read user spacing\n");
    }else{
        logger->Debug("Using default image spacing\n");
    }

    typename ObjectImageType::SpacingType spacing;
    for(size_t d = 0 ; d < Dimension; ++d) {
        spacing[d] = static_cast<typename ObjectImageType::SpacingValueType>(objectSpacing[d]);
    }

    std::stringstream ss;
    ss << "Set input spacing : (";
    for(size_t d = 0; d < Dimension; ++d) ss << spacing[d] << ",";
    ss << ")\n";
    logger->Info(ss.str());
    ss.str("");

    using InformationChangeFilterTypeFloat = itk::ChangeInformationImageFilter<ObjectImageType>;
    auto changeSpacing = InformationChangeFilterTypeFloat::New();
    changeSpacing->SetInput(reader->GetOutput());
    changeSpacing->SetOutputSpacing(spacing);
    changeSpacing->ChangeSpacingOn();
    typename ObjectImageType::Pointer image = changeSpacing->GetOutput();

    using GaussianFilterType = itk::DiscreteGaussianImageFilter<ObjectImageType , InternalImageType >;
    typename GaussianFilterType::Pointer smoothingFilter = GaussianFilterType::New();
    smoothingFilter->SetInput(image);
    double smoothingVariance = 1;
    if(parser->GetCommandLineArgument("-smooth",smoothingVariance)){
        logger->Debug("Set smoothing variance to " + std::to_string(smoothingVariance) + "\n");
    }else{
        logger->Debug("Using default object smoothing variance to " + std::to_string(smoothingVariance) + "\n");
    }
    std::vector<float> varVector(Dimension);
    for(size_t d = 0; d < Dimension; ++d){
        varVector[d] = smoothingVariance * spacing[d];
    }

    ss << "Set smoothing variance to : (";
    for(size_t d = 0; d < Dimension; ++d) ss << varVector[d] << ",";
    ss << ")\n";
    logger->Info(ss.str());
    ss.str("");
    smoothingFilter->SetVariance(varVector.data());

    smoothingFilter->SetUseImageSpacingOff();


    using ThresholdFilterType = itk::BinaryThresholdImageFilter< InternalImageType , ObjectImageType >;
    typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();

    double lthresh = 0.5;
    if(parser->GetCommandLineArgument("-lthreshold", lthresh)){
        logger->Debug("Set lower object threshold to : " + std::to_string(lthresh) + "\n");
    }else{
        logger->Debug("Set lower object threshold to default : " + std::to_string(lthresh) + "\n");
    }
    thresholdFilter->SetLowerThreshold(lthresh);

    double uthresh = std::numeric_limits<typename ThresholdFilterType::InputImageType::PixelType>::max();
    if(parser->GetCommandLineArgument("-uthreshold", uthresh)){
        logger->Debug("Set upper object threshold to : " + std::to_string(uthresh) + "\n");
    }else{
        logger->Debug("Set upper object threshold to default : " + std::to_string(uthresh) + "\n");
    }

    thresholdFilter->SetUpperThreshold(uthresh);
    thresholdFilter->SetOutsideValue(1);
    thresholdFilter->SetInsideValue(0);
    thresholdFilter->SetInput(smoothingFilter->GetOutput());

    logger->Info("Started Signed distance map computation \n");
    using SignedDistanceMapImageFilterType = itk::SignedDanielssonDistanceMapImageFilter<ObjectImageType, DistanceImageType>;
    typename SignedDistanceMapImageFilterType::Pointer distanceMapImageFilter = SignedDistanceMapImageFilterType::New();
    distanceMapImageFilter->SetInput(thresholdFilter->GetOutput());
    // inside true because threshold inverts the object.
    distanceMapImageFilter->SetInsideIsPositive(true);
    distanceMapImageFilter->Update();
    typename DistanceImageType::Pointer distanceMap = distanceMapImageFilter->GetOutput();
    using OffSetImageType =  typename SignedDistanceMapImageFilterType::VectorImageType ;
    typename OffSetImageType::Pointer closestPointTransform = distanceMapImageFilter->GetVectorDistanceMap();

    //Casting Offset Image to Floating Point Vector field.
    typename FieldImageType::Pointer castField = FieldImageType::New();
    castField->SetOrigin(distanceMap->GetOrigin());
    castField->SetSpacing(distanceMap->GetSpacing());
    castField->SetRegions(distanceMap->GetLargestPossibleRegion());
    castField->Allocate();
    itk::ImageRegionIterator<FieldImageType > wit(castField, castField->GetLargestPossibleRegion());
    itk::ImageRegionIterator<DistanceImageType > dit(distanceMap, distanceMap->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<OffSetImageType> cpit(closestPointTransform, closestPointTransform->GetLargestPossibleRegion());
    typename FieldImageType::PixelType castValue;

    double maxSpacing = *std::max_element(objectSpacing.begin(),objectSpacing.end());

    dit.GoToBegin();
    wit.GoToBegin();
    cpit.GoToBegin();
    while(!wit.IsAtEnd()){
        auto current = cpit.Get();
        auto dist = dit.Get();
        //zero out field at the boundary where it tend to be all over the place.
        float multiplier = dist < -1.5*maxSpacing ? 1:0;
        for(size_t d = 0; d < Dimension; ++d){
            castValue[d] = static_cast<float>(current.GetElement(d)) * multiplier;
        }
        wit.Set(castValue);
        ++cpit;
        ++wit;
        ++dit;
    }

    std::pair<typename DistanceImageType::Pointer, typename FieldImageType::Pointer> retVal;
    retVal.first = distanceMap;
    retVal.second = castField;
    return retVal;
}
#endif //SKELTOOLS_FLUX_HXX
