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

#include <itkImage.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <filesystem>
#include <itkLogger.h>
#include <itkStdStreamLogOutput.h>
#include <itkImageFileReader.h>

#include "itkCommandLineArgumentParser.h"
#include "itkHomotopicThinningImageFilter.h"
#include "config.h"


int thinning2d(const itk::CommandLineArgumentParser::Pointer &params,
                           const itk::Logger::Pointer &logger){

    logger->Info("Homotopic thinning for 2D image\n");
    std::string inputFilename, outputFoldername;
    params->GetCommandLineArgument("-input", inputFilename);
    params->GetCommandLineArgument("-outputFolder", outputFoldername);

	fs::path inputFilePath(inputFilename);
	fs::path outputFolderPath(outputFoldername);

    if(fs::exists(outputFolderPath)){
            if(fs::is_directory(outputFolderPath)) {
                logger->Warning("Directory " + outputFolderPath.string() + " already Exists\n");
                logger->Warning("Old files in " + outputFolderPath.string() + " may be overwritten\n");
            }else{
                logger->Critical("A file named " + outputFolderPath.string() + " already Exists\n");
                return EXIT_FAILURE;
            }
    }else{
      fs::create_directory(outputFolderPath);
      logger->Info("Created new directory " + outputFolderPath.string() + "\n");
    }

    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType,2>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFilePath);

    using HomotopicThinningFilter = itk::HomotopicThinningImageFilter<PixelType,2>;
    auto thinningFilter = HomotopicThinningFilter::New();
    thinningFilter->SetInput(reader->GetOutput());
    thinningFilter->SetLowerThreshold(1);

    using WriterType = itk::ImageFileWriter<ImageType>;
    auto writer = WriterType::New();
    writer->SetInput(thinningFilter->GetOutput());
    fs::path outputFilePath;
    outputFilePath = outputFolderPath/(inputFilePath.stem().string() +
                                       "_homotopic_skeleton" +
                                       inputFilePath.extension().string());
    writer->SetFileName(outputFilePath);

	try {
		writer->Update();
		logger->Info("Wrote file " + outputFilePath.string() + "\n");
	} catch (const itk::ExceptionObject &excep) {
		logger->Critical("Error Writing  File!\n");
		logger->Critical(excep.what() + std::string("\n"));
		return EXIT_FAILURE;
	}


    return EXIT_SUCCESS;
}


int thinning3d(const itk::CommandLineArgumentParser::Pointer &params,
                           const itk::Logger::Pointer &logger){

    logger->Info("Homotopic thinning for 3D image\n");
    std::string inputFilename, outputFoldername;
    params->GetCommandLineArgument("-input", inputFilename);
    params->GetCommandLineArgument("-outputFolder", outputFoldername);

	fs::path inputFilePath(inputFilename);
	fs::path outputFolderPath(outputFoldername);

    if(fs::exists(outputFolderPath)){
            if(fs::is_directory(outputFolderPath)) {
                logger->Warning("Directory " + outputFolderPath.string() + " already Exists\n");
                logger->Warning("Old files in " + outputFolderPath.string() + " may be overwritten\n");
            }else{
                logger->Critical("A file named " + outputFolderPath.string() + " already Exists\n");
                return EXIT_FAILURE;
            }
    }else{
      fs::create_directory(outputFolderPath);
      logger->Info("Created new directory " + outputFolderPath.string() + "\n");
    }

    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType,3>;
    using ReaderType = itk::ImageFileReader<ImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFilePath);

    using HomotopicThinningFilter = itk::HomotopicThinningImageFilter<PixelType,3>;
    auto thinningFilter = HomotopicThinningFilter::New();
    thinningFilter->SetInput(reader->GetOutput());
    thinningFilter->SetLowerThreshold(1);

    using WriterType = itk::ImageFileWriter<ImageType>;
    auto writer = WriterType::New();
    writer->SetInput(thinningFilter->GetOutput());
    fs::path outputFilePath;
    outputFilePath = outputFolderPath/(inputFilePath.stem().string() +
                                       "_homotopic_skeleton" +
                                       inputFilePath.extension().string());
    writer->SetFileName(outputFilePath);

	try {
		writer->Update();
		logger->Info("Wrote file " + outputFilePath.string() + "\n");
	} catch (const itk::ExceptionObject &excep) {
		logger->Critical("Error Writing  File!\n");
		logger->Critical(excep.what() + std::string("\n"));
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
    itk::CommandLineArgumentParser::Pointer params2d = itk::CommandLineArgumentParser::New();
    std::vector<std::string> p0;
    p0.push_back("-input");
    p0.push_back("./data/stick0.png");
    params2d->AddCommandLineArguments(p0);

    std::vector<std::string> p1;
    p1.push_back("-outputFolder");
    p1.push_back("./results");
    params2d->AddCommandLineArguments(p1);



    std::cout << "\n================================================================\n";
    std::cout << "Runing 2D example\n";
    std::cout << "-----------------------------------------------------------------\n";
    thinning2d(params2d, logger);
    std::cout << "\n================================================================\n";

	itk::CommandLineArgumentParser::Pointer params3d = itk::CommandLineArgumentParser::New();
    std::vector<std::string> p30;
    p30.push_back("-input");
    p30.push_back("./data/dinosaur.tif");
    params3d->AddCommandLineArguments(p30);

    std::vector<std::string> p31;
    p31.push_back("-outputFolder");
    p31.push_back("./results");
    params3d->AddCommandLineArguments(p31);


	std::cout << "Running 3D example\n";
    std::cout << "-----------------------------------------------------------------\n";
    thinning3d(params3d, logger);
    std::cout << "\n================================================================\n";
	
    return EXIT_SUCCESS;
}

