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
// Created by tabish on 2023-10-10.
//

#include <itkImage.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <filesystem>
#include <itkLogger.h>
#include <itkStdStreamLogOutput.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkMultiplyImageFilter.h>
#include <itkChangeInformationImageFilter.h>

#include "itkCommandLineArgumentParser.h"
#include "itkSpokeFieldToAverageOutwardFluxImageFilter.h"
#include "itkAOFAnchoredMedialSurfaceImageFilter.h"
#include "itkMapToBoundaryImageFilter.h"
#include "flux.h"
#include "config.h"


using InputPixelType = unsigned char;
using RealPixelType = float;
constexpr unsigned Dimension = 3;
using InputImageType = itk::Image<InputPixelType,Dimension>;
using DistanceImageType = itk::Image<RealPixelType, Dimension>;
using SkeletonImageType = itk::Image<RealPixelType, Dimension>;

SkeletonImageType::Pointer
computeSkeleton(InputImageType::Pointer& object,
				const itk::CommandLineArgumentParser::Pointer &params,
				const itk::Logger::Pointer &logger){
    using SpokeFieldImageType = typename itk::Image<itk::Vector<float,Dimension>,Dimension>;

    auto distClosestPointPair =
            computeObjectSignedDistanceSpokesPair<InputImageType, DistanceImageType>(params, logger);
    auto spokeField = distClosestPointPair.second;

    using AOFFilterType = itk::SpokeFieldToAverageOutwardFluxImageFilter<SpokeFieldImageType, float>;
    typename AOFFilterType::Pointer aofFilter = AOFFilterType::New();
    aofFilter->SetInput(spokeField);
    aofFilter->Update();

	using ScaleFilterType = itk::MultiplyImageFilter<DistanceImageType, DistanceImageType, DistanceImageType>;
	auto medialThickness = ScaleFilterType::New();
	medialThickness->SetInput(distClosestPointPair.first);
	medialThickness->SetConstant(-2);
	medialThickness->Update();

    using MedialSurfaceFilterType = itk::AOFAnchoredMedialSurfaceImageFilter<InputImageType, SkeletonImageType>;
    typename MedialSurfaceFilterType::Pointer medialSurfaceFilter = MedialSurfaceFilterType::New();
    medialSurfaceFilter->SetInput(object);
    medialSurfaceFilter->SetAOFImage(aofFilter->GetOutput());
	medialSurfaceFilter->SetPriorityImage(medialThickness->GetOutput());
	medialSurfaceFilter->SetRadiusWeightedSkeleton(true);
    medialSurfaceFilter->SetAOFThreshold(-10);
	medialSurfaceFilter->SetQuick(true);
    medialSurfaceFilter->Update();

	using ChangeInfoFilterType = itk::ChangeInformationImageFilter<SkeletonImageType>;
	auto skelInfo = ChangeInfoFilterType::New();
	skelInfo->SetInput(medialSurfaceFilter->GetOutput());
	skelInfo->ChangeSpacingOn();
	skelInfo->SetOutputSpacing(object->GetSpacing());
	skelInfo->Update();

    auto medialSurface = skelInfo->GetOutput();
	return medialSurface;
}

int map2boundary(const itk::CommandLineArgumentParser::Pointer &params,
				 const itk::Logger::Pointer &logger) {

    logger->Info("Running map to boundary example\n");
    std::string inputFileName;
	params->GetCommandLineArgument("-input", inputFileName);

    //fs::path inputFilePath, rootPath, outputFolderPath;



    using ReaderType = itk::ImageFileReader<InputImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFileName);
	reader->Update();
	typename InputImageType::Pointer input = reader->GetOutput();


	typename SkeletonImageType::Pointer skeleton = computeSkeleton(input,params,logger);

	using OutputImageType = SkeletonImageType;
	using MappingFilterType = itk::MapToBoundaryImageFilter<InputImageType, SkeletonImageType,OutputImageType>;
	auto mappingFilter = MappingFilterType::New();
	mappingFilter->SetInput(input);
	mappingFilter->SetSkeletonImage(skeleton);

    std::string outputFileName;
	params->GetCommandLineArgument("-output", outputFileName);

    using WriterType = itk::ImageFileWriter<OutputImageType>;
    auto writer = WriterType::New();
	writer->SetInput(mappingFilter->GetOutput());
    writer->SetFileName(outputFileName);
	try{
		writer->Update();
		logger->Info("Wrote file " + std::string(outputFileName) + "\n");
	} catch (const itk::ExceptionObject &excep) {
		logger->Critical("Error Writing  File!\n");
		logger->Critical(std::string(excep.what()) + "\n");
		return EXIT_FAILURE;
	}
    return EXIT_SUCCESS;
}


int main(int argc, char* argv[]){
    itk::Logger::Pointer logger = itk::Logger::New();
    itk::StdStreamLogOutput::Pointer itkcout = itk::StdStreamLogOutput::New();
    itkcout->SetStream(std::cout);
    logger->SetLevelForFlushing(itk::LoggerBaseEnums::PriorityLevel::DEBUG);

    logger->AddLogOutput(itkcout);
    std::string humanReadableFormat = "[%b-%d-%Y, %H:%M:%S]";
    logger->SetHumanReadableFormat(humanReadableFormat);
    logger->SetTimeStampFormat(itk::LoggerBaseEnums::TimeStampFormat::HUMANREADABLE);

    // map to list module code, module name.
    itk::CommandLineArgumentParser::Pointer params = itk::CommandLineArgumentParser::New();
    std::vector<std::string> p0;
    p0.push_back("-input");
    p0.push_back("./data/dinosaur.tif");
    params->AddCommandLineArguments(p0);

    std::vector<std::string> p1;
    p1.push_back("-output");
    p1.push_back("dinosaur-mapped-thickness.tif");
    params->AddCommandLineArguments(p1);



    std::cout << "\n================================================================\n";
    std::cout << "Mapping medial radius to boundary\n";
    std::cout << "-----------------------------------------------------------------\n";
    map2boundary(params, logger);
    std::cout << "\n================================================================\n";

    return EXIT_SUCCESS;
}

