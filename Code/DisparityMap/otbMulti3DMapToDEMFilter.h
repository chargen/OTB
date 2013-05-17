/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbMulti3DMapToDEMFilter_h
#define __otbMulti3DMapToDEMFilter_h

#include "itkImageToImageFilter.h"
#include "otbGenericRSTransform.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegionConstIterator.h"
#include "otbVectorImage.h"
#include "otbImage.h"
#include "otbStreamingMinMaxVectorImageFilter.h"
#include "itkImageRegionSplitter.h"
#include "otbObjectList.h"

namespace otb
{

namespace CellFusionMode
{
enum CellFusionMode {
  MIN = 0,
  MAX = 1,
  MEAN = 2,
  ACC = 3 //return accumulator for debug purpose
  };
}


/** \class Multi3DMapToDEMFilter
 *  \brief Project N 3D images (long,lat,alti) into a regular DEM in the chosen map projection system.
 *
 *  This filter uses a group of N 3D points images and project then onto a regular DEM grid.
 *  The 3D coordinates (sorted by band) are : longitude , latitude (in degree, wrt WGS84) and altitude (in meters)
 *
 *  \sa FineRegistrationImageFilter
 *  \sa MultiDisparityMapTo3DFilter
 *
 *  \ingroup Streamed
 *  \ingroup Threaded
 *
 */
template <class T3DImage =  otb::VectorImage<double,2>,
          class TMaskImage = otb::Image<unsigned char>, class TOutputDEMImage = otb::Image<double> >
class ITK_EXPORT Multi3DMapToDEMFilter :
    public itk::ImageToImageFilter<T3DImage,TOutputDEMImage>
{
public:
  /** Standard class typedef */
  typedef Multi3DMapToDEMFilter                       Self;
  typedef itk::ImageToImageFilter<T3DImage,
                                  TOutputDEMImage>             Superclass;
  typedef itk::SmartPointer<Self>                           Pointer;
  typedef itk::SmartPointer<const Self>                     ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(Multi3DMapToDEMFilter, ImageToImageFilter);

  /** Usefull typedefs */
  typedef T3DImage                InputMapType;
  typedef TOutputDEMImage         OutputImageType;
  typedef TMaskImage              MaskImageType;
  typedef otb::Image<unsigned int>                   AccumulatorImageType;

  typedef typename OutputImageType::RegionType         RegionType;
  typedef typename OutputImageType::PixelType          DEMPixelType;
  typedef typename AccumulatorImageType::PixelType     AccumulatorPixelType;

  typedef typename InputMapType::PixelType             MapPixelType;
  typedef typename InputMapType::InternalPixelType   InputInternalPixelType;
  // 3D RS transform
  // TODO: Allow to tune precision (i.e. double or float)
  typedef double                  PrecisionType;
  typedef otb::GenericRSTransform
    <PrecisionType,3,3>           RSTransformType;

  // 3D points
  typedef typename RSTransformType::InputPointType  TDPointType;

  typedef otb::ImageKeywordlist                     ImageKeywordListType;
  
  typedef std::map
    <unsigned int,
     itk::ImageRegionConstIterator<InputMapType> >   MapIteratorList;
  
  typedef std::map
    <unsigned int,
     itk::ImageRegionConstIterator<MaskImageType> >      MaskIteratorList;
  
  typedef otb::StreamingMinMaxVectorImageFilter<InputMapType> StreamingMinMaxVImageFilterType;

  typedef double ValueType;
  typedef itk::VariableLengthVector<ValueType> MeasurementType;

  typedef itk::ImageRegionSplitter<2>   SplitterType;
  typedef otb::ObjectList<SplitterType>      SplitterListType;

  /** Set the number of 3D images (referred earlier as N) */
  void SetNumberOf3DMaps(unsigned int nb);
  
  /** Get the number of 3D images (referred earlier as N) */
  unsigned int GetNumberOf3DMaps();
  
  /** Set 3D map input at  corresponding 'index' */
  void Set3DMapInput(unsigned int index, const T3DImage * hmap);
  

  /** Set mask associated to 3D maps  'index'
   * (optional, pixels with a null mask value are ignored)
   */
  void SetMaskInput(unsigned int index, const TMaskImage * mask);

  /** Get the inputs */
  const T3DImage * Get3DMapInput(unsigned int index) const;
  const TMaskImage  * GetMaskInput(unsigned int index) const;

  /** Get DEM output*/
   const TOutputDEMImage * GetDEMOutput() const;
   TOutputDEMImage * GetDEMOutput();

   /** Set/Get macro for DEM grid step */
   itkSetMacro(DEMGridStep, double);
   itkGetConstReferenceMacro(DEMGridStep, double);

   /** Set/Get macro for DEM grid step */
   itkSetMacro(CellFusionMode, int);
   itkGetConstReferenceMacro(CellFusionMode, int);

   /** Set/Get macro for DEM grid step */
   itkSetMacro(NoDataValue, DEMPixelType);
   itkGetConstReferenceMacro(NoDataValue, DEMPixelType);


   /** Set keywordlist of the 3D map  'index' */
   void SetMapKeywordList(unsigned int index, const ImageKeywordListType kwl);

   /** Get keywordlist of the 3D map 'index' */
   const ImageKeywordListType & GetMapKeywordList(unsigned int index) const;

protected:
  /** Constructor */
  Multi3DMapToDEMFilter();

  /** Destructor */
  virtual ~Multi3DMapToDEMFilter();

  /** Generate output information */
  virtual void GenerateOutputInformation();

  /** Generate input requrested region */
  virtual void GenerateInputRequestedRegion();

  /** Before threaded generate data */
  virtual void BeforeThreadedGenerateData();
  
  /** Threaded generate data */
  virtual void ThreadedGenerateData(const RegionType & outputRegionForThread, int threadId);

  /** After threaded generate data */
  virtual void AfterThreadedGenerateData();

private:
  Multi3DMapToDEMFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Keywordlist of each map */
  std::vector<ImageKeywordListType> m_MapKeywordLists;

  
  /** Reference sensor image transform */
  RSTransformType::Pointer m_ReferenceToGroundTransform;
  
  /** Moving sensor image transforms */
  std::vector<RSTransformType::Pointer> m_MapToGroundTransform;

  std::vector<MeasurementType> m_MapMinVal;
  std::vector<MeasurementType> m_MapMaxVal;
  /** DEM grid step (in meters) */
  double m_DEMGridStep;

  /** Temporary DEMs for multithreading */
   std::vector<typename OutputImageType::Pointer> m_TempDEMRegions;
   /** Temporary accumulator for multithreading and mean calculus*/
   std::vector<typename AccumulatorImageType::Pointer> m_TempDEMAccumulatorRegions;


  std::vector<unsigned int> m_NumberOfSplit; // number of split for each map
  // std::vector<int> m_ThreadProcessed; //
  /** Region splitter for input disparity maps */
  SplitterListType::Pointer m_MapSplitterList;

  DEMPixelType m_NoDataValue;
  int m_CellFusionMode;


};
} // end namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbMulti3DMapToDEMFilter.txx"
#endif

#endif
