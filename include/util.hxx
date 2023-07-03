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


#ifndef SKELTOOLS_UTILS_HXX
#define SKELTOOLS_UTILS_HXX

#include <string>
#include <itkLogger.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>

template<typename TImage>
void writeImage(const std::string & filePath, const typename TImage::Pointer &image,
                const itk::Logger::Pointer &logger) {
    using FileWriterType = itk::ImageFileWriter<TImage>;
    typename FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(filePath );
    writer->SetInput( image );
    try
    {
        writer->Update();
        logger->Debug( "Wrote file " + std::string(writer->GetFileName()) + "\n");
    }catch (itk::ExceptionObject &e){
        logger->Critical( "Failed writing " + std::string(writer->GetFileName()) + "\n");
        logger->Critical( std::string(e.what()) + "\n");
    }
}

template<typename TImage>
typename TImage::Pointer readImage(const std::string & filePath,
                                   const itk::Logger::Pointer &logger) {
    using FileReaderType = itk::ImageFileReader<TImage>;
    typename FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(filePath );
    try
    {
        reader->Update();
        logger->Debug("Read file " + std::string(reader->GetFileName()) + "\n");
    }catch (itk::ExceptionObject &e){
        logger->Critical("Failed reading  " + std::string(reader->GetFileName()) + "\n");
        logger->Critical(std::string(e.what()) + "\n");
    }
    return reader->GetOutput();
}

#endif //SKELTOOLS_UTILS_HXX
