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

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageIOBase.h>

#include "config.h"
#include "experiment.h"
#include "itkMedialCurveImageFilter.h"


template <typename InputPixelType, unsigned int Dimension>
int experiment_impl(const itk::CommandLineArgumentParser::Pointer &parser,
                    const itk::Logger::Pointer &logger) {

    logger->Info("Experiment\n");
    std::string inputFileName;
	parser->GetCommandLineArgument("-input", inputFileName);

    fs::path inputFilePath, rootPath, outputFolderPath;

    using InputImageType = itk::Image<InputPixelType,Dimension>;

    using ReaderType = itk::ImageFileReader<InputImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFileName);

	using OutputImageType = InputImageType;

	//....
    std::string outputFileName;
	parser->GetCommandLineArgument("-output", outputFileName);

    using WriterType = itk::ImageFileWriter<OutputImageType>;
    auto writer = WriterType::New();
    //writer->SetInput(outputFilter->GetOutput());
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


int experiment(const itk::CommandLineArgumentParser::Pointer &parser,
                 const itk::Logger::Pointer &logger) {

    logger->Info("Experimental Module\n");
    std::string inputFileName, outputFileName;
	std::string outputFolderName;

    fs::path inputFilePath, outputFilePath;
	fs::path outputFolderPath;

    bool hasInputFilePath =
        parser->GetCommandLineArgument("-input", inputFileName);
    bool hasOutputFilePath =
        parser->GetCommandLineArgument("-output", outputFileName);
    bool hasOutputFolderPath =
        parser->GetCommandLineArgument("-outputFolder", outputFolderName);

    // Check the input image.
    if (!hasInputFilePath) {
        logger->Error("Input file not specified\n");
        return EXIT_FAILURE;
    }else{
        inputFilePath = inputFileName;
    }

	// Set output folder
	if (!hasOutputFolderPath){
		if(hasOutputFilePath){
			outputFolderPath = outputFilePath.parent_path();
		}else{
			outputFolderPath = inputFilePath.parent_path();
		}
		std::vector<std::string> newargs;
        newargs.push_back("-outputFolder");
        newargs.push_back(outputFolderPath.string());
        parser->AddCommandLineArguments(newargs);
	}else{
		outputFolderPath  = outputFolderName;
	}

	// Set output filename
    if (!hasOutputFilePath) {
        outputFilePath = outputFolderPath/(inputFilePath.stem().string() +
										   "_experiment" +
										   inputFilePath.extension().string());
        std::vector<std::string> newargs;
        newargs.push_back("-output");
        newargs.push_back(outputFilePath.string());
        parser->AddCommandLineArguments(newargs);
    }else{
        outputFilePath = outputFileName;
    }

    logger->Info("Input  Filename : " + inputFilePath.string() + "\n");
	logger->Debug("Checking Input file....\n");

	//Check if input file exists
    if (!fs::exists(inputFilePath)) {
		logger->Critical("File " + inputFilePath.string() + " not found\n");
		return EXIT_FAILURE;
    }

	logger->Info("Output Filename : "  + outputFilePath.string() + "\n");
	logger->Debug("Setting up output folders...\n");

	// Create folder for to write output files
	if(fs::exists(outputFolderPath)){
		if(fs::is_directory(outputFolderPath)) {
			logger->Warning("Directory " + outputFolderPath.string() + " already exists\n");
			logger->Warning("Old files in " + outputFolderPath.string() + " may be overwritten\n");
		}else{
			logger->Critical("A file named " + outputFolderPath.string() + " already exists\n");
			logger->Critical("Remove file " + outputFolderPath.string() + " or change the output filename\n");
			return EXIT_FAILURE;
		}
	}else{
		fs::create_directory(outputFolderPath);
		logger->Info("Created new directory " + outputFolderPath.string() + "\n");
	}

    itk::ImageIOBase::Pointer imageIO =
        itk::ImageIOFactory::CreateImageIO(inputFileName.c_str(), itk::ImageIOFactory::FileModeType::ReadMode);
    imageIO->SetFileName(inputFileName);
    imageIO->ReadImageInformation();
    itk::ImageIOBase::IOPixelType pixelType = imageIO->GetPixelType();
    itk::ImageIOBase::IOComponentType componentType = imageIO->GetComponentType();
    unsigned int dimensions = imageIO->GetNumberOfDimensions();

    logger->Info("Component Type  : " +
                 imageIO->GetComponentTypeAsString(componentType) + "\n");
    logger->Info("Pixel Type      : " +
                 itk::ImageIOBase::GetPixelTypeAsString(pixelType) + "\n");
    logger->Info("Image Dimension : " + std::to_string(dimensions) + "\n");

    switch (dimensions){
    case 2:
        switch (componentType){
        case itk::ImageIOBase::UCHAR:
            return experiment_impl<unsigned char,2>(parser, logger);
        case itk::ImageIOBase::USHORT:
            return experiment_impl<unsigned short, 2>(parser, logger);
        case itk::ImageIOBase::FLOAT:
            return experiment_impl<float, 2>(parser, logger);
        case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
        default:
          logger->Critical("Unknown ComponentType :: " +
                           std::string(typeid(componentType).name()) +
                           " in 2 dimensions\n");
        }
        break;
    case 3:
       switch (componentType) {
       case itk::ImageIOBase::UCHAR:
          return experiment_impl<unsigned char, 3>(parser, logger);
       case itk::ImageIOBase::USHORT:
         return experiment_impl<unsigned short, 3>(parser, logger);
       case itk::ImageIOBase::FLOAT:
          return experiment_impl<float, 3>(parser, logger);
       case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
       default:
         logger->Critical("Unknown ComponentType :: " +
                          std::string(typeid(componentType).name()) +
                          " in 3 dimensions\n");
       }
       break;
    default:
        logger->Critical("File not supported..\n");
    }
    return EXIT_FAILURE;
}
