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


#ifndef SKELTOOLS_UTIL_H
#define SKELTOOLS_UTIL_H

#include <string>
#include <itkImage.h>
#include <itkLogger.h>

template<typename TImage>
void writeImage(const std::string & filePath, const typename TImage::Pointer &image,
                const itk::Logger::Pointer &logger);

template<typename TImage>
typename TImage::Pointer readImage(const std::string & filePath,
                                   const itk::Logger::Pointer &logger);

#include "util.hxx"

#endif //SKELTOOLS_UTIL_H
