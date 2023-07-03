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


#ifndef SKELTOOLS_MEDIAL_H
#define SKELTOOLS_MEDIAL_H

#include "itkCommandLineArgumentParser.h"
#include <itkLogger.h>

int medial(const itk::CommandLineArgumentParser::Pointer &parser,
           const itk::Logger::Pointer &logger);
#endif //SKELTOOLS_MEDIAL_H
