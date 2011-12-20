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

#include "otbImage.h"
#include "otbImageList.h"
#include "itkListSample.h"
#include "itkFixedArray.h"

#include "otbSparseWvltToAngleMapperListFilter.h"

int otbSparseWvltToAngleMapperListFilterNewTest ( int argc, char * argv[] )
{
  const unsigned int Dimension = 2;
  typedef double PixelType;
  typedef otb::Image< PixelType, Dimension > ImageType;
  typedef otb::ImageList< ImageType > ImageListType;

  typedef itk::Statistics::ListSample< itk::FixedArray< ImageType::PixelType, 1 > > SampleType;

  typedef otb::SparseWvltToAngleMapperListFilter< ImageListType, SampleType > FilterType;

  FilterType::Pointer filter = FilterType::New();

  return EXIT_SUCCESS;
}
