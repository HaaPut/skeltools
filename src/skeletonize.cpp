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
#include <string>
#include <set>
#include <algorithm>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageIOBase.h>
#include <itkBinaryFillholeImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkChangeInformationImageFilter.h>

#include "util.h"
#include "config.h"
#include "experiment.h"
#include "itkMedialCurveImageFilter.h"


template <typename InputPixelType, unsigned int Dimension>
int skeletonize_impl(const itk::CommandLineArgumentParser::Pointer &parser,
                    const itk::Logger::Pointer &logger) {

    logger->Info("Skeltoniation module\n");
    std::string inputFileName;
    parser->GetCommandLineArgument("-input", inputFileName);

    //fs::path inputFilePath, rootPath, outputFolderPath;

    using ObjectImageType = itk::Image<InputPixelType,Dimension>;
    using FloatImageType = itk::Image<float, Dimension>;
    using OutputImageType = ObjectImageType;

    using ReaderType = itk::ImageFileReader<ObjectImageType>;
    auto reader = ReaderType::New();
    reader->SetFileName(inputFileName);

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

    using GaussianFilterType = itk::DiscreteGaussianImageFilter<ObjectImageType , FloatImageType >;
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


    using ThresholdFilterType = itk::BinaryThresholdImageFilter< FloatImageType , ObjectImageType >;
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
    InputPixelType objectInsideValue = itk::NumericTraits<InputPixelType>::OneValue();
    InputPixelType objectOutsideValue = itk::NumericTraits<InputPixelType>::ZeroValue();

    thresholdFilter->SetOutsideValue(objectOutsideValue);
    thresholdFilter->SetInsideValue(objectInsideValue);
    thresholdFilter->SetInput(smoothingFilter->GetOutput());
    typename ObjectImageType::Pointer objectImage;
    if(parser->ArgumentExists("-fillholes")){
        logger->Info("Filling holes in the object\n");
        using HoleFillingFilterType = itk::BinaryFillholeImageFilter<ObjectImageType>;
        typename HoleFillingFilterType::Pointer filledFilter = HoleFillingFilterType::New();
        filledFilter->SetInput(thresholdFilter->GetOutput());
        filledFilter->SetForegroundValue(objectInsideValue);
        filledFilter->Update();
        objectImage = filledFilter->GetOutput();
    }else{
        logger->Info("Using smoothed object without hole filling\n");
        objectImage = thresholdFilter->GetOutput();
    }

    using MedialCurveFilterType = itk::MedialCurveImageFilter<ObjectImageType, OutputImageType>;
    typename MedialCurveFilterType::Pointer medialCurveFilter = MedialCurveFilterType::New();
    medialCurveFilter->SetInput(objectImage);

    std::string priorityType;
    parser->GetCommandLineArgument("-priority", priorityType);
    if(priorityType == "distance"){
        medialCurveFilter->SetPriorityImage(nullptr);
    }
    medialCurveFilter->Update();
    typename OutputImageType::Pointer medialCurve = medialCurveFilter->GetOutput();

    std::string outputFileName;
    parser->GetCommandLineArgument("-output", outputFileName);

    writeImage<OutputImageType>(outputFileName, medialCurve, logger);

    return EXIT_SUCCESS;
}


int skeletonize(const itk::CommandLineArgumentParser::Pointer &parser,
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
    std::set<std::string> allowedPriorityTypes = {"distance"};
    std::string priorityType = "distance";
    if(parser->GetCommandLineArgument("-priority",priorityType)){
        std::transform(priorityType.begin(), priorityType.end(), priorityType.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if(allowedPriorityTypes.find(priorityType) == allowedPriorityTypes.end()){
            logger->Warning("Unknown priority type " + priorityType + "\n");
            priorityType = "distance";
            logger->Warning("Reset priority type to distance\n");
        }
    }else{
        // this branch should never be taken
        logger->Debug("This shouldn't have happened \n");
        logger->Info("Priority Type set to default " + priorityType + "\n");
    }

    if(parser->ArgumentExists("-curve")) {
        logger->Info("Running Medial Curve computation\n");
        // Set output filename
        if (!hasOutputFilePath) {
            outputFilePath = outputFolderPath/(inputFilePath.stem().string() +
                                               "_" + priorityType +
                                               "_curve" +
                                               inputFilePath.extension().string());
            std::vector<std::string> newargs;
            newargs.push_back("-output");
            newargs.push_back(outputFilePath.string());
            parser->AddCommandLineArguments(newargs);
            logger->Debug("Output Filename Set to: " + outputFilePath.string() + "\n");
        }else{
            outputFilePath = outputFileName;
            logger->Debug("Set Output Filename to " + outputFilePath.string() + "\n");
        }
        switch (dimensions) {
            //case 2:
            case 3:
                switch (componentType) {
                    case itk::ImageIOBase::UCHAR:
                        return skeletonize_impl<unsigned char, 3>(parser, logger);
                    case itk::ImageIOBase::USHORT:
                        return skeletonize_impl<unsigned short, 3>(parser, logger);
                    case itk::ImageIOBase::FLOAT:
                        return skeletonize_impl<float, 3>(parser, logger);
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
        logger->Warning("Unknown priority weighted Skeletonization algorithm\n");
        logger->Info("Available Algorithms include: \n"
                     " \t-curve\n");
    }
    return EXIT_FAILURE;
}