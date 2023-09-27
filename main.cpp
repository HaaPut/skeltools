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
#include "skeletonize.h"
#include "experiment.h"
#include "homotopic.h"
#include "medial.h"

namespace fs = std::filesystem;


std::string helpstring() {
    std::ostringstream ss;
    ss << "Medial Tools\n";
    ss << "$ skeltool -[modulename] -input <path_to_input_image>\n\n";
    ss << "Options:: \n";
    ss << "===========================================\n";

    ss << "\t -input\n";
    ss << "\t\t(required) path to input file\n";

    ss << "\t -outputFolder\n";
    ss << "\t\t folder path to write output image to\n";

    ss << "\t -output\n";
    ss << "\t\t path to output file\n";

    ss << "\t -h, --help\n";
    ss << "\t\t display this help\n";

    ss << "Modules:: \n";
    ss << "===========================================\n";

    ss << "\t -homotopic       :: homotopic thinning\n";
    ss << "\t -aof             :: average outward flux based methods\n";
    ss << "\t -priority \"type\" ::(default distance) priority weighted heap based skeletonization\n";

    //-------------------------------------------------------------------------
    ss << "Homotopic Options:: \n";
    ss << "===========================================\n";

	ss << "\t\t -threshold T      ::(default 1)(object threshold for binary object)\n";

    //------------------------------------------------------------------------
    ss << "AOF Options:: \n";
    ss << "===========================================\n";

    ss << "\t\t Algorithms:\n";
    ss << "\t\t\t -approximate [T]    :: Medial Surface by thresholding (@optional T {0.4}) AOF values\n";
    ss << "\t\t -useprecomputed       :: Use precomputed intermediate images for distance transform, AOF\n";
    ss << "\t\t -writeIntermediate    :: Write intermediate distance transform, spoke field, AOF images\n";
    ss << "\t\t -spacing x y..        :: size of image voxel\n";
    ss << "\t\t -smooth V             :: (default 1 px) Variance of gaussian used for smoothing input image\n";
    ss << "\t\t -lthreshold           :: Lower threshold for generating binary object\n";
    ss << "\t\t -uthreshold           :: Upper threshold for generating binary object\n";

    //------------------------------------------------------------------------
    ss << "Priority Options:: \n";
    ss << "===========================================\n";
    ss << "\t\t -curve                :: medial curve algorithm\n";
    ss << "\t\t -fillholes            :: fill object holes before skeletonization\n";
    ss << "\t\t -weighted             :: radius weighted skeleton\n";
    ss << "\t\t -spacing x y..        :: size of image voxel\n";
    ss << "\t\t -smooth V             :: (default 1 px) Variance of gaussian used for smoothing input image\n";
    ss << "\t\t -lthreshold           :: Lower threshold for generating binary object\n";
    ss << "\t\t -uthreshold           :: Upper threshold for generating binary object\n";
    ss << "\t\t -anchor [aof,""]      :: (optional, default none)use anchored end points\n";

    //------------------------------------------------------------------------

    ss << "\n\n";
    ss << "Examples:\n";
    ss << "=========\n";
    ss << "Compute homotopic skeleton:\n";
    ss << "$./skeltool -homotopic -input data/dinosaur.tif -outputFolder results\n";
    ss << "------------------------------------------\n";
    ss << "Compute Approximate Medial Surface:\n";
    ss << "$ ./skeltool -aof -approximate  -input data/dinosaur.tif -smooth 0.5 -useprecomputed -writeIntermediate -outputFolder results\n";
    ss << "------------------------------------------\n";
    ss << "Compute distance weighted medial curve:\n";
    ss << "$ ./skeltool -priority distance -curve -input data/dinosaur.tif -smooth 0.2 -fillholes -anchor aof -outputFolder results\n";
    ss << "------------------------------------------\n";
    ss << "\n\n";

    std::string helpString(ss.str());
    return helpString;
}

int main(int argc, char* argv[]){
    itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
    itk::Logger::Pointer logger = itk::Logger::New();
    itk::StdStreamLogOutput::Pointer itkcout = itk::StdStreamLogOutput::New();
    itk::StdStreamLogOutput::Pointer itkfout = itk::StdStreamLogOutput::New();
    itkcout->SetStream(std::cout);
    logger->SetLevelForFlushing(itk::LoggerBaseEnums::PriorityLevel::DEBUG);

#ifdef DEBUG_BUILD
    logger->SetPriorityLevel(itk::LoggerBaseEnums::PriorityLevel::DEBUG);
#else
    logger->SetPriorityLevel(itk::LoggerBaseEnums::PriorityLevel::CRITICAL);
#endif
    logger->AddLogOutput(itkcout);
    std::string humanReadableFormat = "[%b-%d-%Y, %H:%M:%S]";
    logger->SetHumanReadableFormat(humanReadableFormat);
    logger->SetTimeStampFormat(itk::LoggerBaseEnums::TimeStampFormat::HUMANREADABLE);

    // map to list module code, module name.
    std::map<int, std::string> modesMap;
    modesMap[0] = "Homotopic Thinning\n";
    modesMap[1] = "Average Outward Flux Methods\n";
    modesMap[2] = "Priority weighted Skeletonization Methods\n";
    modesMap[93] = "Undocumented/Experimental\n";

    std::vector<std::string> modules;
    modules.emplace_back("-homotopic");
    modules.emplace_back("-aof");
    modules.emplace_back("-priority");
    modules.emplace_back("-experiment");

    parser->SetCommandLineArguments(argc, argv);
    parser->SetProgramHelpText(helpstring());

    if(parser->CheckForRequiredArguments() == itk::CommandLineArgumentParser::HELPREQUESTED){
        return EXIT_SUCCESS;
    }
    if(parser->CheckForRequiredArguments() == itk::CommandLineArgumentParser::FAILED ||
       !parser->ExactlyOneExists(modules)){
        std::cout << parser->GetProgramHelpText() << std::endl;
        return EXIT_FAILURE;
    }
    std::string logFileName="/tmp/skelTools.log";

    if(parser->GetCommandLineArgument("-logfile", logFileName)){
        std::ofstream fileStream(logFileName);
        itkfout->SetStream(fileStream);
        logger->AddLogOutput(itkfout);
    }

	if(parser->ArgumentExists("-vv")){
		if(logger->GetPriorityLevel() < itk::LoggerBaseEnums::PriorityLevel::DEBUG ){
			logger->SetPriorityLevel(itk::LoggerBaseEnums::PriorityLevel::DEBUG);
			logger->Info("Set Log level to Debug\n");
		}else{
			logger->Write(itk::LoggerBaseEnums::PriorityLevel::MUSTFLUSH, "Log level already above Debug\n");
		}
	}else if(parser->ArgumentExists("-v")){
		if(logger->GetPriorityLevel() < itk::LoggerBaseEnums::PriorityLevel::INFO ){
			logger->SetPriorityLevel(itk::LoggerBaseEnums::PriorityLevel::INFO);
			logger->Info("Set Log level to Info\n");
		}else{
			logger->Write(itk::LoggerBaseEnums::PriorityLevel::MUSTFLUSH, "Log level already above Info\n");
		}
	}

    logger->Info( "Starting Skeletonization Tools\n");

    int module = 0;
    if(parser->ArgumentExists("-homotopic")){
        module = 0;
        logger->Info("Selected : " + modesMap[module] + "\n");
    } else if (parser->ArgumentExists("-aof")) {
        module = 1;
        logger->Info("Selected : " + modesMap[module] + "\n");
    }else if (parser->ArgumentExists("-priority")) {
        module = 2;
        logger->Info("Selected : " + modesMap[module] + "\n");
    }  else if (parser->ArgumentExists("-experiment")) {
        module = 93;
        logger->Info("Selected : " + modesMap[module] + "\n");
    } else {
        logger->Info("Using default : " + modesMap[module] + "\n");
    }

    switch(module){
        case 0: // homotopic thinning
			logger->Debug("Homotopic Thinning case\n");
			homotopic(parser, logger);
            break;
        case 1: // aof based medial methods
            logger->Debug("Flux based medial methods case\n");
            medial(parser, logger);
            break;
        case 2: // priority queue based skeletonization
            logger->Debug("Priority queue methods case\n");
            skeletonize(parser, logger);
            break;
	    case 93: // experimental
			logger->Debug("Experimental case\n");
            experiment(parser, logger);
            break;
        default:
            std::cout<<"Unknown Module"<<std::endl;
    }
    return EXIT_SUCCESS;
}

