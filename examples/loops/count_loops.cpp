#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include <itkLogger.h>
#include <itkStdStreamLogOutput.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkChangeInformationImageFilter.h>
#include <itkMultiplyImageFilter.h>

#include "itkAOFAnchoredMedialCurveImageFilter.h"
#include "itkSpokeFieldToAverageOutwardFluxImageFilter.h"
#include "itkCommandLineArgumentParser.h" 
#include "graph.h"
#include "loops.h"
#include "flux.h"


namespace fs = std::filesystem;

// -------------------------------------------------------------------------
// Helper: Compute Medial Curve
// -------------------------------------------------------------------------
template<typename ObjectImageType, typename OutputImageType>
typename OutputImageType::Pointer
computeAOFAnchoredMedialCurve(typename ObjectImageType::Pointer objectImage,
                              itk::CommandLineArgumentParser::Pointer params,
                              itk::Logger::Pointer logger) {
    logger->Debug("Starting AOF computation for Anchored medial curve\n");
    constexpr unsigned Dimension = ObjectImageType::ImageDimension;
    using DistanceImageType = itk::Image<float, Dimension>;
    using SpokeFieldImageType = typename itk::Image<itk::Vector<float, Dimension>, Dimension>;

    auto distClosestPointPair =
            computeObjectSignedDistanceSpokesPair<ObjectImageType, DistanceImageType>(params, logger);
    auto spokeField = distClosestPointPair.second;

    using AOFFilterType = itk::SpokeFieldToAverageOutwardFluxImageFilter<SpokeFieldImageType, float>;
    typename AOFFilterType::Pointer aofFilter = AOFFilterType::New();
    aofFilter->SetInput(spokeField);
    aofFilter->Update();

    using MedialCurveFilterType = itk::AOFAnchoredMedialCurveImageFilter<ObjectImageType, OutputImageType>;
    typename MedialCurveFilterType::Pointer medialCurveFilter = MedialCurveFilterType::New();
    medialCurveFilter->SetInput(objectImage);

    medialCurveFilter->SetAOFImage(aofFilter->GetOutput());

    // Set distance priority
    using ScaleFilterType = itk::MultiplyImageFilter<DistanceImageType, DistanceImageType, DistanceImageType>;
    auto inverter = ScaleFilterType::New();
    inverter->SetInput(distClosestPointPair.first);
    inverter->SetConstant(-1);
    inverter->Update();
    
    medialCurveFilter->SetPriorityImage(inverter->GetOutput());
    medialCurveFilter->SetRadiusWeightedSkeleton(false);

    float threshold = -30;

    medialCurveFilter->SetAOFThreshold(threshold);
    medialCurveFilter->SetQuick(false);
    logger->Debug("Using default mode: initializing with all interior points\n");

    medialCurveFilter->Update();
    auto medialCurve = medialCurveFilter->GetOutput();

    std::string outputFileName;

    return medialCurve;
}


int main(int argc, char* argv[]) {
    // 1. Logger Setup
    itk::Logger::Pointer logger = itk::Logger::New();
    itk::StdStreamLogOutput::Pointer itkcout = itk::StdStreamLogOutput::New();
    itkcout->SetStream(std::cout);
    logger->SetPriorityLevel(itk::LoggerBaseEnums::PriorityLevel::INFO);
    logger->SetLevelForFlushing(itk::LoggerBaseEnums::PriorityLevel::DEBUG);
    logger->SetTimeStampFormat(itk::LoggerBaseEnums::TimeStampFormat::HUMANREADABLE);
    logger->SetHumanReadableFormat("%H:%M:%S ");
    logger->AddLogOutput(itkcout);


    std::string inputFileName = "./data/donuts.tif";
    
    itk::CommandLineArgumentParser::Pointer params = itk::CommandLineArgumentParser::New();
    std::vector<std::string> p0;
    p0.push_back("-input");
    p0.push_back(inputFileName);
    params->AddCommandLineArguments(p0);
    
    
    std::string outputFoldername = "./data";
    
    std::vector<std::string> p1;
    p1.push_back("-outputFolder");
    p1.push_back(outputFoldername);
    params->AddCommandLineArguments(p1);
    
    fs::path outputFolderpath = outputFoldername;


    unsigned reduction = 4;

    std::vector<double> spacing{1, 1, 1.0};

    double distanceThreshold = 1e3;
    
    //-----------------------------------------------------------
    // Generate Curve Skeleton from Object
    //-----------------------------------------------------------
    
    constexpr unsigned Dimension = 3;
    using ObjectPixelType = unsigned char;
    using ObjectImageType = itk::Image<ObjectPixelType, Dimension>;
    using ObjectReaderType = itk::ImageFileReader<ObjectImageType>;
    
    auto objectReader = ObjectReaderType::New();
    objectReader->SetFileName(inputFileName);
    
    try {
        objectReader->Update();
    } catch (const itk::ExceptionObject& e) {
        logger->Critical(std::string("Failed to read input object image: ") + e.what() + "\n");
        return EXIT_FAILURE;
    }
    
    auto objectImage = objectReader->GetOutput();
    logger->Debug("Loaded object image file " + inputFileName + "\n");

    using VoxelSkeletonImageType = ObjectImageType;
    using OutputImageType = VoxelSkeletonImageType;
    auto voxelSkeleton = computeAOFAnchoredMedialCurve<ObjectImageType, OutputImageType>(
        objectImage, params, logger
    );

    /*
    using WriterType = itk::ImageFileWriter<VoxelSkeltonImageType>;
    auto skeletonWriter = WriterType::New();
    skeletonWriter->SetInput(voxelSkeleton);
    skeletonWriter->ReleaseDataFlagOff();
    
    skeletonWriter->SetFileName("./data/donut_skeleton.tif");
    try{
        skeletonWriter->Update();
	logger->Info("Wrote skeleton file \n");
    } catch (const itk::ExceptionObject &excep) {
	logger->Critical("Error Writing  File!\n");
	logger->Critical(std::string(excep.what()) + "\n");
	return EXIT_FAILURE;
    }
    */
    
    //----------------------------------------------------------------
    // Graph Construction & Reduction
    //----------------------------------------------------------------
    if (voxelSkeletonToGraph<VoxelSkeletonImageType>(voxelSkeleton, reduction, distanceThreshold, outputFolderpath, logger) != EXIT_SUCCESS) {
        logger->Critical("VoxelSketon To Graph Construction failed.\n");
        return EXIT_FAILURE;
    }

    //------------------------------------------------------------------
    // Enumerate Loops
    //------------------------------------------------------------------
    
    std::string adjacencyListPath = "./data/skeletonGraphList-" + std::to_string(reduction)+ ".txt";
    if (enumerateLoops(adjacencyListPath, logger) != EXIT_SUCCESS) {
        logger->Critical("Loop Enumeration failed.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
