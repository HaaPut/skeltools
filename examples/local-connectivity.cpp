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
// Created by tabish on 2023-10-14.
//

#include <iostream>

#include <itkStdStreamLogOutput.h>
#include <itkLogger.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include "itkCommandLineArgumentParser.h"
#include "itkHomotopicThinningImageFilter.h"
#include "itkLocalConnectivityImageFilter.h"
#include "flux.h"
#include "config.h"


using InputPixelType = unsigned char;
using RealPixelType = float;
constexpr unsigned Dimension = 3;
using InputImageType = itk::Image<InputPixelType,Dimension>;
using DistanceImageType = itk::Image<RealPixelType, Dimension>;
using SkeletonPixelType = InputPixelType;
using SkeletonImageType = itk::Image<SkeletonPixelType, Dimension>;


SkeletonImageType::Pointer
computeSkeleton(InputImageType::Pointer& object,
				const itk::CommandLineArgumentParser::Pointer &params,
				const itk::Logger::Pointer &logger){
	logger->Debug("Starting skeleton computation\n");
    using HomotopicThinningFilter = itk::HomotopicThinningImageFilter<InputPixelType,Dimension>;
    auto thinningFilter = HomotopicThinningFilter::New();
    thinningFilter->SetInput(object);

    double objectThreshold = 1;
    thinningFilter->SetLowerThreshold(static_cast<InputPixelType>(objectThreshold));
    logger->Debug("Set object threshold to " + std::to_string(objectThreshold) + "\n");
    thinningFilter->Update();
    auto medialSurface = thinningFilter->GetOutput();

    return medialSurface;
}

int compute_gm(const itk::CommandLineArgumentParser::Pointer &params,
			const itk::Logger::Pointer &logger) {

    logger->Info("Running map to boundary example\n");
    std::string inputFileName;
    params->GetCommandLineArgument("-input", inputFileName);

    using ReaderType = itk::ImageFileReader<InputImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFileName);
    reader->Update();
    typename InputImageType::Pointer input = reader->GetOutput();

    typename SkeletonImageType::Pointer skeleton = computeSkeleton(input,params,logger);


    using LocalConnectivityFilterType = itk::LocalConnectivityImageFilter<SkeletonImageType, SkeletonImageType>;
    auto localConnectivityFeature = LocalConnectivityFilterType::New();
    localConnectivityFeature->SetInput(skeleton);
    unsigned radius=1;
    params->GetCommandLineArgument("-radius", radius);
    localConnectivityFeature->SetMaxLevel(radius);
    localConnectivityFeature->Update();

    auto output = localConnectivityFeature->GetOutput();

    std::string outputFileName;
    params->GetCommandLineArgument("-output", outputFileName);

    using WriterType = itk::ImageFileWriter<SkeletonImageType>;
    auto writer = WriterType::New();
    writer->SetInput(output);
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
    p0.push_back("./data/chair.tif");
    params->AddCommandLineArguments(p0);

    std::vector<std::string> p1;
    p1.push_back("-output");
    p1.push_back("chair-local-conn.tif");
    params->AddCommandLineArguments(p1);

    std::vector<std::string> p2;
    p2.push_back("-radius");
    p2.push_back("4");
    params->AddCommandLineArguments(p2);


    std::cout << "\n================================================================\n";
    std::cout << "Local gamma measure:~ Volume of local ball of fixed radius\n";
    std::cout << "-----------------------------------------------------------------\n";
    compute_gm(params, logger);
    std::cout << "\n================================================================\n";

    return EXIT_SUCCESS;
}

