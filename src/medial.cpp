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

#include "itkSpokeFieldToAverageOutwardFluxImageFilter.h"
#include "itkCommandLineArgumentParser.h"
#include "medial.h"
#include "config.h"
#include "flux.h"
#include "util.h"

#include <itkImage.h>
#include <itkLogger.h>
#include <itkImageIOBase.h>
#include <itkImageIOFactory.h>
#include <itkBinaryThresholdImageFilter.h>

template<typename InputPixelType, unsigned Dimension>
int approximateMedialSurface_impl(const itk::CommandLineArgumentParser::Pointer &parser,
                                  const itk::Logger::Pointer &logger){
    using ObjectImageType = itk::Image<InputPixelType, Dimension>;
    using DistanceValueType = float;//only float is supported by tiff writer
    using DistanceImageType = itk::Image<DistanceValueType, Dimension>;
    using SpokeFieldImageType = typename itk::Image<itk::Vector<DistanceValueType, Dimension>, Dimension>;

    using FluxValueType = DistanceValueType;
    using FluxImageType = itk::Image<FluxValueType, Dimension>;

    logger->Info("Starting AOF Approximate Medial Surface computation\n");
    std::string inputFileName;
    parser->GetCommandLineArgument("-input", inputFileName);
    fs::path inputFilePath(inputFileName);

    std::string outputFolderName;
    parser->GetCommandLineArgument("-outputFolder",outputFolderName);
    fs::path outputFolderPath(outputFolderName);

    fs::path distanceMapFilePath = outputFolderPath / (inputFilePath.stem().string() + "_signedDistanceMap.tif");
    logger->Debug("Set distance map filename to : " + distanceMapFilePath.string() + "\n");

    fs::path spokeFilePath = outputFolderPath / (inputFilePath.stem().string() + "_CPT.tif");
    logger->Debug("Set spoke field file path to : " + spokeFilePath.string() + "\n");

    typename DistanceImageType::Pointer distanceMap;
    typename SpokeFieldImageType::Pointer spokeField;

    if((!fs::exists(distanceMapFilePath) || !fs::exists(spokeFilePath))
       && parser->ArgumentExists("-useprecomputed")){
        logger->Warning("distance map/spoke file does not exist!"
                        " Ignoring -useprecomputed argument\n");
    }

    if (fs::exists(distanceMapFilePath)
        && fs::exists(spokeFilePath)
        && parser->ArgumentExists("-useprecomputed") ) {

        logger->Info("Reading precomputed distance map and spoke field\n");
        //distanceMap = readImage<DistanceImageType>(distanceMapFilePath.string(), logger);
        spokeField = readImage<SpokeFieldImageType>(spokeFilePath.string(), logger);
    }else{
        logger->Info("Computing distance map and Spoke field\n");
        auto distClosestPointPair =
                computeObjectSignedDistanceSpokesPair<ObjectImageType, DistanceImageType>(parser, logger);
        distanceMap = distClosestPointPair.first;
        if(parser->ArgumentExists("-writeIntermediate")){
            writeImage<DistanceImageType>(distanceMapFilePath, distanceMap, logger);
        }
        spokeField = distClosestPointPair.second;
        if(parser->ArgumentExists("-writeIntermediate")) {
            writeImage<SpokeFieldImageType>(spokeFilePath, spokeField, logger);
        }
    }
    fs::path aofFilePath = outputFolderPath / (inputFilePath.stem().string() + "_aof.tif");
    logger->Debug("Set AOF file path to : " + aofFilePath.string() + "\n");

    typename FluxImageType::Pointer aof;

    if (!fs::exists(aofFilePath)
        && parser->ArgumentExists("-useprecomputed")){
        logger->Warning("AOF file does not exist! Will ignore -useprecomputed\n");
    }

    if (fs::exists(aofFilePath)
        && parser->ArgumentExists("-useprecomputed")) {
        logger->Info("Reading already computed m_AOF map..\n");
        aof = readImage<FluxImageType >(aofFilePath,logger);
    }else {
        logger->Info("Starting AOF computation using Spoke Vector Field\n");
        using AOFFilterType = itk::SpokeFieldToAverageOutwardFluxImageFilter< SpokeFieldImageType, FluxValueType >;
        typename AOFFilterType::Pointer aofFilter = AOFFilterType::New();
        aofFilter->SetInput(spokeField);
        aofFilter->Update();
        aof = aofFilter->GetOutput();
        if(parser->ArgumentExists("-writeIntermediate")) {
            writeImage<FluxImageType>(aofFilePath.string(), aof, logger);
        }
    }

    // TODO: move this to a separate function and switch medial algorithms here instead.
    // Only thresholding required for approximate surface.
    using ThresholdFilterType = itk::BinaryThresholdImageFilter< FluxImageType , ObjectImageType >;
    typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();

    // (Number of samples * 2)/pi = 40
    // threshold value = 0.4
	double aofThresholdValue = 0.4;
	if(parser->GetCommandLineArgument("-approximate", aofThresholdValue)){
		logger->Info("Set AOF Threshold value to "+ std::to_string(aofThresholdValue) +"\n");
	}else{
		logger->Debug("Using default AOF Threshold : " + std::to_string(aofThresholdValue) + "\n");
	}
    thresholdFilter->SetLowerThreshold(-40*aofThresholdValue);
    thresholdFilter->SetUpperThreshold(std::numeric_limits<FluxValueType>::max());
    thresholdFilter->SetOutsideValue(1);
    thresholdFilter->SetInsideValue(0);
    thresholdFilter->SetInput(aof);

    thresholdFilter->Update();

    std::string outputFileName;
    fs::path skeletonFilePath;
    if(parser->GetCommandLineArgument("-output",outputFileName)){
        skeletonFilePath = outputFileName;
        logger->Debug("Set output skeleton path to " + skeletonFilePath.string() + "\n");
    }else {
        skeletonFilePath = outputFolderPath / (inputFilePath.stem().string() + "_approximateMedialSkeleotn.tif");
        logger->Debug("Using default skeleton path : " + skeletonFilePath.string() + "\n");
    }
    writeImage<ObjectImageType>(skeletonFilePath, thresholdFilter->GetOutput(), logger);

    return EXIT_SUCCESS;
}

int medial(const itk::CommandLineArgumentParser::Pointer &parser,
           const itk::Logger::Pointer &logger) {

    logger->Info("Average outward flux based medial Module\n");
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
        logger->Debug("Output Folder Set to: " + outputFolderPath.string() + "\n");
    }else{
        outputFolderPath  = outputFolderName;
        logger->Debug("Set Output Folder to " + outputFolderPath.string() + "\n");
    }

    logger->Info("Input  Filename : " + inputFilePath.string() + "\n");
    logger->Debug("Checking Input file....\n");

    //Check if input file exists
    if (!fs::exists(inputFilePath)) {
        logger->Critical("File " + inputFilePath.string() + " not found\n");
        return EXIT_FAILURE;
    }

    logger->Info("Output Folder : "  + outputFolderPath.string() + "\n");
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

    // Switch algorithms

    if(parser->ArgumentExists("-approximate")) {
        logger->Info("Running Approximate Medial Surface computation\n");
        switch (dimensions) {
            case 2:
                switch (componentType) {
                    case itk::ImageIOBase::UCHAR:
                        return approximateMedialSurface_impl<unsigned char, 2>(parser, logger);
                    case itk::ImageIOBase::USHORT:
                        return approximateMedialSurface_impl<unsigned short, 2>(parser, logger);
                    case itk::ImageIOBase::FLOAT:
                        return approximateMedialSurface_impl<float, 2>(parser, logger);
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
                        return approximateMedialSurface_impl<unsigned char, 3>(parser, logger);
                    case itk::ImageIOBase::USHORT:
                        return approximateMedialSurface_impl<unsigned short, 3>(parser, logger);
                    case itk::ImageIOBase::FLOAT:
                        return approximateMedialSurface_impl<float, 3>(parser, logger);
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
    }else{
        logger->Warning("Unknown Medial computation algorithm\n");
        logger->Info("Available Algorithms include: \n"
                     " \t-approximate\n");
    }
    return EXIT_FAILURE;
}
