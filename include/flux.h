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


#ifndef SKELTOOLS_FLUX_H
#define SKELTOOLS_FLUX_H

#include <utility>
#include <itkImage.h>
#include <itkVector.h>
#include <itkLogger.h>

#include "itkCommandLineArgumentParser.h"

template<class TObjectImage, class TDistanceImage>
std::pair< typename TDistanceImage::Pointer,
        typename itk::Image<itk::Vector<float, TObjectImage::ImageDimension>,TObjectImage::ImageDimension>::Pointer>
computeObjectSignedDistanceSpokesPair(const itk::CommandLineArgumentParser::Pointer &parser,
                                      const itk::Logger::Pointer &logger);

#include "flux.hxx"
#endif //SKELTOOLS_FLUX_H
