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

#ifndef SKELTOOLS_itkHomotopicThinningImageFilter_h
#define SKELTOOLS_itkHomotopicThinningImageFilter_h

#include <itkBinaryThresholdImageFilter.h>
#include <itkConstantBoundaryCondition.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageToImageFilter.h>
#include <itkFastChamferDistanceImageFilter.h>
#include <vector>
#include <map>
#include <itkMacro.h>

#include "topology.h"


namespace itk
{

    template< typename PixelType, unsigned Dimension>
    class ITK_TEMPLATE_EXPORT  HomotopicThinningImageFilter:
        public ImageToImageFilter<Image<PixelType, Dimension>, Image<PixelType, Dimension> >
    {
    public:
        /** Standard class typedefs. */
        using InputImageType = Image<PixelType, Dimension>;
        using Self = HomotopicThinningImageFilter;
        using Superclass = ImageToImageFilter<InputImageType, InputImageType>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        /** Method for creation through the object factory */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro( SkeletonImageFilter, ImageToImageFilter );

        using InputPointerType = typename InputImageType::ConstPointer;

        using OutputImageType = InputImageType;
        using OutputPointerType = typename InputImageType::Pointer;

        using InputConstIteratorType = ImageRegionConstIterator< InputImageType >;
        using OutputIteratorType = ImageRegionIteratorWithIndex< InputImageType >;
        using IndexType = typename InputImageType::IndexType;

        void
        GenerateInputRequestedRegion() override;
    protected:
        HomotopicThinningImageFilter() = default;
        ~HomotopicThinningImageFilter() = default;


        void GenerateData() override;

        void PrintSelf(std::ostream& os, Indent indent) const;
    };

    //---------------------------------------------------------------------
    template< typename PixelType>
    class ITK_TEMPLATE_EXPORT  HomotopicThinningImageFilter<PixelType, 3>:
        public ImageToImageFilter<Image<PixelType, 3>, Image<PixelType, 3> >
    {
    public:
        /** Standard class typedefs. */
        using InputImageType = Image<PixelType, 3>;
        using Self = HomotopicThinningImageFilter;
        using Superclass = ImageToImageFilter<InputImageType, InputImageType>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        /** Method for creation through the object factory */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro( SkeletonImageFilter, ImageToImageFilter );

        using InputImagePointer = typename Superclass::InputImagePointer;
        using OutputImageType = InputImageType;
        using OutputImagePointer = typename InputImageType::Pointer;

        using InputConstIteratorType = ImageRegionConstIterator< InputImageType >;
        using OutputIteratorType = ImageRegionIteratorWithIndex< OutputImageType >;
        using IndexType = typename InputImageType::IndexType;
        using BoundaryConditionType = ConstantBoundaryCondition<InputImageType>;

        using InternalPixelType = float;
        using InternalImageType = Image<InternalPixelType,3>;
        using ChamferThresholdFilterType = itk::BinaryThresholdImageFilter<InputImageType, InternalImageType>;
        using ChamferType = FastChamferDistanceImageFilter<InternalImageType, InternalImageType>;

        //void
        //GenerateInputRequestedRegion() override;

        itkSetMacro(MaxIterations, unsigned);
        itkGetConstMacro(MaxIterations, unsigned);

        itkSetMacro(LowerThreshold, PixelType);
        itkGetConstMacro(LowerThreshold, PixelType);

        itkGetConstMacro(RemoveCount, double);

        itkGetConstMacro(InsideValue, PixelType);

        itkGetConstMacro(OutsideValue, PixelType);
      protected:
        HomotopicThinningImageFilter();
        ~HomotopicThinningImageFilter() = default;

        void GenerateData() override;

    private:
        unsigned m_MaxIterations = NumericTraits<unsigned>::max();
        PixelType m_LowerThreshold = NumericTraits<PixelType>::OneValue();
        double m_MinSpacing = 1;
        typename ChamferType::Pointer m_ChamferFilter;
        typename ChamferThresholdFilterType::Pointer m_ThresholdFilter;

        OutputImagePointer m_Output;
        PixelType m_InsideValue;
        PixelType m_OutsideValue;
        double m_RemoveCount;
        double m_Count;
        bool isRemovable(IndexType index);

        BoundaryConditionType  m_Accessor;
    };

//--------------------------------------------------------------------------------------
    template <typename PixelType>
    class ITK_TEMPLATE_EXPORT HomotopicThinningImageFilter<PixelType, 2>
            : public ImageToImageFilter<Image<PixelType, 2>, Image<PixelType, 2>> {
    public:
        /** Standard class typedefs. */
        using InputImageType = Image<PixelType, 2>;
        using Self = HomotopicThinningImageFilter;
        using Superclass = ImageToImageFilter<InputImageType, InputImageType>;
        using Pointer = SmartPointer<Self>;
        using ConstPointer = SmartPointer<const Self>;

        /** Method for creation through the object factory */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro( HomotopicThinningImageFilter, ImageToImageFilter );

        //using InputPointerType = typename InputImageType::ConstPointer;

        using OutputImageType = InputImageType;
        using OutputImagePointer = typename InputImageType::Pointer;

        using InputConstIteratorType = ImageRegionConstIterator< InputImageType >;
        using OutputIteratorType = ImageRegionIteratorWithIndex< InputImageType >;
        using IndexType = typename InputImageType::IndexType;
        using BoundaryConditionType = ConstantBoundaryCondition<InputImageType>;

        using InternalPixelType = float;
        using InternalImageType = Image<InternalPixelType,2>;
        using ChamferThresholdFilterType = itk::BinaryThresholdImageFilter<InputImageType, InternalImageType>;
        using ChamferType = FastChamferDistanceImageFilter<InternalImageType, InternalImageType>;

        itkSetMacro(MaxIterations, unsigned);
        itkGetConstMacro(MaxIterations, unsigned);

        itkSetMacro(LowerThreshold, PixelType);
        itkGetConstMacro(LowerThreshold, PixelType);

        itkGetConstMacro(RemoveCount, double);
        itkGetConstMacro(Count, double);

        itkSetMacro(InsideValue, PixelType);
        itkGetConstMacro(InsideValue, PixelType);

        itkGetConstMacro(OutsideValue, PixelType);

    protected:
        HomotopicThinningImageFilter();
        ~HomotopicThinningImageFilter() = default;

        void GenerateData() override;
        bool isSimple2(IndexType index);

    private:
        //bool m_SaveProgressImage = true;
        typename ChamferType::Pointer m_ChamferFilter;
        typename ChamferThresholdFilterType::Pointer m_ThresholdFilter;
        PixelType m_LowerThreshold =
                NumericTraits<PixelType>::OneValue();
        unsigned m_MaxIterations = NumericTraits<unsigned>::max();
        float m_MinSpacing = 1;
        double m_RemoveCount = 0;
        double m_Count = 0;
        PixelType m_InsideValue;
        PixelType m_OutsideValue;

        OutputImagePointer m_Output;

        //order is important here
        //0 1 2
        //7 x 3
        //6 5 4
        const std::vector<itk::Offset<2>> m_Neighbors8 = {
                {{-1, -1}},
                {{-1, 0}},
                {{-1, 1}},
                {{0,  1}},
                //{{0,  0}},
                {{1,  1}},
                {{1,  0}},
                {{1,  -1}},
                {{0,  -1}}
        };
        BoundaryConditionType m_Accessor;
    };
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkHomotopicThinningImageFilter.hxx"
#endif

#endif //itkHomotopicThinningImageFilter_h
