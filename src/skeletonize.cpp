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
#include <itkMultiplyImageFilter.h>

#include "util.h"
#include "flux.h"
#include "config.h"
#include "experiment.h"
#include "itkMedialCurveImageFilter.h"
#include "itkAOFAnchoredMedialCurveImageFilter.h"
#include "itkMedialSurfaceImageFilter.h"
#include "itkAOFAnchoredMedialSurfaceImageFilter.h"
#include "itkSpokeFieldToAverageOutwardFluxImageFilter.h"


template<typename ObjectImageType, typename OutputImageType>
static void
computeAOFAnchoredMedialCurve(typename ObjectImageType::Pointer objectImage,
                   itk::CommandLineArgumentParser::Pointer parser,
                   itk::Logger::Pointer logger){
    logger->Debug("Starting AOF computation for Anchored medial curve\n");
    constexpr unsigned Dimension = ObjectImageType::ImageDimension;
    using DistanceImageType = itk::Image<float, Dimension>;
    using SpokeFieldImageType = typename itk::Image<itk::Vector < float, Dimension>, Dimension > ;

    auto distClosestPointPair =
            computeObjectSignedDistanceSpokesPair<ObjectImageType, DistanceImageType>(parser, logger);
    auto spokeField = distClosestPointPair.second;

    using AOFFilterType = itk::SpokeFieldToAverageOutwardFluxImageFilter<SpokeFieldImageType, float>;
    typename AOFFilterType::Pointer aofFilter = AOFFilterType::New();
    aofFilter->SetInput(spokeField);
    aofFilter->Update();

    using MedialCurveFilterType = itk::AOFAnchoredMedialCurveImageFilter<ObjectImageType, OutputImageType>;
    typename MedialCurveFilterType::Pointer medialCurveFilter = MedialCurveFilterType::New();
    medialCurveFilter->SetInput(objectImage);

    medialCurveFilter->SetAOFImage(aofFilter->GetOutput());

    std::string priorityType;
    parser->GetCommandLineArgument("-priority", priorityType);
    if (priorityType == "distance") {
        using ScaleFilterType = itk::MultiplyImageFilter<DistanceImageType, DistanceImageType, DistanceImageType>;
        auto inverter = ScaleFilterType::New();
        inverter->SetInput(distClosestPointPair.first);
        inverter->SetConstant(-1);
        inverter->Update();
        medialCurveFilter->SetPriorityImage(inverter->GetOutput());//distClosestPointPair.first);
    }
    if(parser->ArgumentExists("-weighted")){
        medialCurveFilter->SetRadiusWeightedSkeleton(true);
    }else{
        medialCurveFilter->SetRadiusWeightedSkeleton(false);
    }

    float threshold = -30;
    if(parser->GetCommandLineArgument("-threshold",threshold)){
        logger->Info("Set End point threshold = " + std::to_string(threshold) + "\n");
    }else{
        logger->Debug("Set End point threshold to default " + std::to_string(threshold) + "\n");
    }

    medialCurveFilter->SetAOFThreshold(threshold);

	if(parser->ArgumentExists("-quick")){
		medialCurveFilter->SetQuick(true);
		logger->Info("Using quick mode: discarding all non-negative AOF point in initialization\n");
	}else{
		medialCurveFilter->SetQuick(false);
		logger->Debug("Using default mode: initializing with all interior points");
	}
    medialCurveFilter->Update();
    auto medialCurve = medialCurveFilter->GetOutput();

    std::string outputFileName;
    parser->GetCommandLineArgument("-output", outputFileName);

    writeImage<OutputImageType>(outputFileName, medialCurve, logger);
}


template<typename ObjectImageType, typename OutputImageType>
static void
computeMedialCurve(typename ObjectImageType::Pointer objectImage,
                   itk::CommandLineArgumentParser::Pointer parser,
                   itk::Logger::Pointer logger){
    using MedialCurveFilterType = itk::MedialCurveImageFilter<ObjectImageType, OutputImageType>;
    typename MedialCurveFilterType::Pointer medialCurveFilter = MedialCurveFilterType::New();
    medialCurveFilter->SetInput(objectImage);

    std::string priorityType;
    parser->GetCommandLineArgument("-priority", priorityType);
    if (priorityType == "distance") {
        medialCurveFilter->SetPriorityImage(nullptr);
    }
    if(parser->ArgumentExists("-weighted")){
        medialCurveFilter->SetRadiusWeightedSkeleton(true);
    }else{
        medialCurveFilter->SetRadiusWeightedSkeleton(false);
    }
    medialCurveFilter->Update();
    auto medialCurve = medialCurveFilter->GetOutput();

    std::string outputFileName;
    parser->GetCommandLineArgument("-output", outputFileName);

    writeImage<OutputImageType>(outputFileName, medialCurve, logger);
}


template<typename ObjectImageType, typename OutputImageType>
static void
computeAOFAnchoredMedialSurface(typename ObjectImageType::Pointer objectImage,
                   itk::CommandLineArgumentParser::Pointer parser,
                   itk::Logger::Pointer logger){
    logger->Debug("Starting AOF computation for anchored medial surface\n");
    constexpr unsigned Dimension = ObjectImageType::ImageDimension;
    using DistanceImageType = itk::Image<float, Dimension>;
    using SpokeFieldImageType = typename itk::Image<itk::Vector < float, Dimension>, Dimension > ;

    auto distClosestPointPair =
            computeObjectSignedDistanceSpokesPair<ObjectImageType, DistanceImageType>(parser, logger);
    auto spokeField = distClosestPointPair.second;

    using AOFFilterType = itk::SpokeFieldToAverageOutwardFluxImageFilter<SpokeFieldImageType, float>;
    typename AOFFilterType::Pointer aofFilter = AOFFilterType::New();
    aofFilter->SetInput(spokeField);
    aofFilter->Update();

    using MedialSurfaceFilterType = itk::AOFAnchoredMedialSurfaceImageFilter<ObjectImageType, OutputImageType>;
    typename MedialSurfaceFilterType::Pointer medialSurfaceFilter = MedialSurfaceFilterType::New();
    medialSurfaceFilter->SetInput(objectImage);

    medialSurfaceFilter->SetAOFImage(aofFilter->GetOutput());

    std::string priorityType;
    parser->GetCommandLineArgument("-priority", priorityType);
    if (priorityType == "distance") {
        using ScaleFilterType = itk::MultiplyImageFilter<DistanceImageType, DistanceImageType, DistanceImageType>;
        auto inverter = ScaleFilterType::New();
        inverter->SetInput(distClosestPointPair.first);
        inverter->SetConstant(-1);
        inverter->Update();
        medialSurfaceFilter->SetPriorityImage(inverter->GetOutput());//distClosestPointPair.first);
    }
    if(parser->ArgumentExists("-weighted")){
        medialSurfaceFilter->SetRadiusWeightedSkeleton(true);
    }else{
        medialSurfaceFilter->SetRadiusWeightedSkeleton(false);
    }

    float threshold = -10;
    if(parser->GetCommandLineArgument("-threshold",threshold)){
        logger->Info("Set End point threshold = " + std::to_string(threshold) + "\n");
    }else{
        logger->Debug("Set End point threshold to default " + std::to_string(threshold) + "\n");
    }

    medialSurfaceFilter->SetAOFThreshold(threshold);

	if(parser->ArgumentExists("-slow")){
		medialSurfaceFilter->SetQuick(false);
		logger->Info("Using non-quick mode: Using all interior points for skeleton initialization\n");
	}else{
		medialSurfaceFilter->SetQuick(true);
		logger->Debug("Using default quick mode: discarding all non-negative AOF point in initialization\n");
	}
    medialSurfaceFilter->Update();
    auto medialSurface = medialSurfaceFilter->GetOutput();

    std::string outputFileName;
    parser->GetCommandLineArgument("-output", outputFileName);

    writeImage<OutputImageType>(outputFileName, medialSurface, logger);
}


template<typename ObjectImageType, typename OutputImageType>
static void
computeMedialSurface(typename ObjectImageType::Pointer objectImage,
                   itk::CommandLineArgumentParser::Pointer parser,
                   itk::Logger::Pointer logger){
    using MedialSurfaceFilterType = itk::MedialSurfaceImageFilter<ObjectImageType, OutputImageType>;
    typename MedialSurfaceFilterType::Pointer medialSurfaceFilter = MedialSurfaceFilterType::New();
    medialSurfaceFilter->SetInput(objectImage);

    std::string priorityType;
    parser->GetCommandLineArgument("-priority", priorityType);
    if (priorityType == "distance") {
        medialSurfaceFilter->SetPriorityImage(nullptr);
    }
    if(parser->ArgumentExists("-weighted")){
        medialSurfaceFilter->SetRadiusWeightedSkeleton(true);
    }else{
        medialSurfaceFilter->SetRadiusWeightedSkeleton(false);
    }
    medialSurfaceFilter->Update();
    auto medialSurface = medialSurfaceFilter->GetOutput();

    std::string outputFileName;
    parser->GetCommandLineArgument("-output", outputFileName);

    writeImage<OutputImageType>(outputFileName, medialSurface, logger);
}


template <typename InputPixelType, unsigned int Dimension>
int skeletonize_impl(const itk::CommandLineArgumentParser::Pointer &parser,
                    const itk::Logger::Pointer &logger) {

    logger->Info("Skeltoniation module\n");
    std::string inputFileName;
    parser->GetCommandLineArgument("-input", inputFileName);

    //fs::path inputFilePath, rootPath, outputFolderPath;

    using ObjectImageType = itk::Image<InputPixelType,Dimension>;
    using FloatImageType = itk::Image<float, Dimension>;

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
    objectImage->Update();

    std::string anchorType;
    parser->GetCommandLineArgument("-anchor", anchorType);
    if (anchorType == "aof") {
        if (parser->ArgumentExists("-weighted")) {
			if (parser->ArgumentExists("-surface")){
				logger->Info("Running radius weighted AOF Anchored medial surface\n");
				computeAOFAnchoredMedialSurface<ObjectImageType, FloatImageType>(objectImage, parser, logger);
			}else{
				logger->Info("Running radius weighted AOF Anchored medial curve\n");
				computeAOFAnchoredMedialCurve<ObjectImageType, FloatImageType>(objectImage, parser, logger);
			}
        } else {
			if(parser->ArgumentExists("-curve")){
				logger->Info("Running unweighted AOF Anchored medial curve\n");
				computeAOFAnchoredMedialCurve<ObjectImageType, ObjectImageType>(objectImage, parser, logger);
			}else{
				logger->Info("Running unweighted AOF Anchored medial surface\n");
				computeAOFAnchoredMedialSurface<ObjectImageType, ObjectImageType>(objectImage, parser, logger);
			}
        }

    }
    else {
        if (parser->ArgumentExists("-weighted")) {
			if(parser->ArgumentExists("-surface")){
				logger->Info("Running radius weighted medial surface\n");
				computeMedialSurface<ObjectImageType, FloatImageType>(objectImage, parser, logger);
			}else{
				logger->Info("Running radius weighted medial curve\n");
				computeMedialCurve<ObjectImageType, FloatImageType>(objectImage, parser, logger);
			}
        } else {
			if(parser->ArgumentExists("-surface")){
				logger->Info("Running unweighted medial surface\n");
				computeMedialCurve<ObjectImageType, ObjectImageType>(objectImage, parser, logger);
			}else{
				logger->Info("Running unweighted medial curve\n");
				computeMedialSurface<ObjectImageType, ObjectImageType>(objectImage, parser, logger);
			}
        }
    }
    return EXIT_SUCCESS;
}


int skeletonize(const itk::CommandLineArgumentParser::Pointer &parser,
                const itk::Logger::Pointer &logger) {

    logger->Info("Priority queue skeletonize Module\n");
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
            outputFolderPath = fs::path(outputFileName).parent_path();
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
    itk::CommonEnums::IOPixel pixelType = imageIO->GetPixelType();
    itk::CommonEnums::IOComponent componentType = imageIO->GetComponentType();
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

    std::set<std::string> allowedAnchorTypes = {"","aof"};
    std::string anchorType = "";
    if(parser->GetCommandLineArgument("-anchor", anchorType)){
        std::transform(anchorType.begin(), anchorType.end(), anchorType.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if(allowedAnchorTypes.find(anchorType) == allowedAnchorTypes.end()){
            logger->Warning("Unknown anchor type " + anchorType + "\n");
            anchorType = "";
            logger->Warning("Reset anchor type to none\n");
        }
    }else{
        logger->Info("Default none type anchor  \n");
    }



	logger->Info("Running Medial Skeleton computation\n");
	// Set output filename
	if (!hasOutputFilePath) {
		outputFilePath = outputFolderPath/(inputFilePath.stem().string() +
										   "_" + priorityType +
										   "_skeleton" +
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
		case itk::CommonEnums::IOComponent::UCHAR:
			return skeletonize_impl<unsigned char, 3>(parser, logger);
		case itk::CommonEnums::IOComponent::USHORT:
			return skeletonize_impl<unsigned short, 3>(parser, logger);
		case itk::CommonEnums::IOComponent::FLOAT:
			return skeletonize_impl<float, 3>(parser, logger);
		case itk::CommonEnums::IOComponent::UNKNOWNCOMPONENTTYPE:
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
