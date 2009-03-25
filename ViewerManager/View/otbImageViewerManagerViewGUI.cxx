/*=========================================================================

Program:   ORFEO Toolbox
Language:  C++
Date:      $Date$
Version:   $Revision$


Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
See OTBCopyright.txt for details.


This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULA
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <FL/Fl_Text_Buffer.H>
#include "otbImageViewerManagerViewGUI.h"
#include "otbMacro.h"
#include "otbFileName.h"

#include "base/ossimFilename.h"
#include "base/ossimDirectory.h"

namespace otb
{
/**
 * Constructor
 */

ImageViewerManagerViewGUI
::ImageViewerManagerViewGUI():m_TemplateViewerName(""),m_DisplayedLabel("+ "),
			      m_UndisplayedLabel("- ")
{

  //
  m_VisuViewShowedList =  VisuViewShowedList::New();
  m_VisuView           =  VisuViewType::New();
  m_PreviewWidget      =  ImageWidgetType::New();
  m_pRenderingFuntion  = StandardRenderingFunctionType::New();
  m_PixelView          =  PixelDescriptionViewType::New();
  m_CurveWidget        =  CurvesWidgetType::New();

  //Get an instance of the model
  m_ImageViewerManagerModel = ImageViewerManagerModel::GetInstance();
  m_ImageViewerManagerModel->RegisterListener(this);

  //Create the component of the GUI
  this->CreateGUI();
  
  //Create buffer for the guiViewerInformation
  Fl_Text_Buffer * buffer = new Fl_Text_Buffer();
  this->guiViewerInformation->buffer(buffer);
  
  //init the previewWindow
  m_PreviewWidget->label("PreviewWidget");
  gPreviewWindow->add(m_PreviewWidget);
  gPreviewWindow->box(FL_NO_BOX);
  gPreviewWindow->resizable(gPreviewWindow);
  m_PreviewWidget->resize(gPreviewWindow->x(), gPreviewWindow->y(), gPreviewWindow->w(), gPreviewWindow->h() );
  
  // Init the widgets : PixelDescription, Full, Scroll, Zoom & Histogram 
  m_FullWindow       = new Fl_Window(500,500);
  m_ScrollWindow     = new Fl_Window(200,200);
  m_ZoomWindow       = new Fl_Window(200,200);
  m_PixelWindow      = new Fl_Window(200,200);
  m_HistogramWindow  = new Fl_Window(300,200);


  
  
  //
  m_CurveWidget->resize(0,0,300,200);
  m_HistogramWindow->add(m_CurveWidget);
  m_HistogramWindow->resize(0,0,300,200);
  m_Bhistogram          = HistogramCurveType::New();
  m_Rhistogram          = HistogramCurveType::New();
  m_Ghistogram          = HistogramCurveType::New();
  
  m_Red.Fill(0);
  m_Green.Fill(0);
  m_Blue.Fill(0);
  
  m_Red[0]  = 1.;   m_Red[3]   = 0.5;
  m_Green[1]= 1.;   m_Green[3] = 0.5;
  m_Blue[2] = 1.;   m_Blue[3]  = 0.5;
  
  m_Ghistogram->SetHistogramColor(m_Green);
  m_Ghistogram->SetLabelColor(m_Green);
  m_Bhistogram->SetHistogramColor(m_Blue);
  m_Bhistogram->SetLabelColor(m_Blue);
  m_Rhistogram->SetHistogramColor(m_Red);
  m_Rhistogram->SetLabelColor(m_Red);
  
  m_CurveWidget->AddCurve(m_Bhistogram);
  m_CurveWidget->AddCurve(m_Ghistogram);
  m_CurveWidget->AddCurve(m_Rhistogram);
  m_CurveWidget->SetXAxisLabel("Pixels");
  m_CurveWidget->SetYAxisLabel("Frequency");
    
  //Add a widget once
  m_PixelWindow->add(m_PixelView->GetPixelDescriptionWidget());

  //
  m_PackedWindow =  PackedWidgetManagerType::New(); 
}
  
  /**
   * 
   */
void
ImageViewerManagerViewGUI
::OpenImage()
{
  std::string pikedFileName="";
  char * cfname ;
  cfname = fl_file_chooser("Pick an image file", "*.*",pikedFileName.c_str());
  
  if (cfname == NULL || strlen(cfname)<1)
    {
      otbMsgDebugMacro(<<"Empty file name!");
      return ;
    }
  
  Fl::check();
  guiMainWindow->redraw();
  
  //Put the status of the last image 
  m_DisplayStatusList.push_back(false);
  //m_DisplayedWidgetList.push_back(NULL);

  // Call the Controller
  m_ImageViewerManagerController->OpenInputImage(cfname);



}

/**
 * Handle the notification of the model
 */
void
ImageViewerManagerViewGUI
::ImageViewerManagerNotify()
{
  if(m_ImageViewerManagerModel->GetHasImageOpened())
    this->AddImageListName();
  
  //Update the 
  if(m_ImageViewerManagerModel->GetHasChangedChannelOrder())
    {
      
      std::cout << " ViewerNotification " << std::endl;
      unsigned int selectedItem = guiImageList->value();
      if (selectedItem == 0)
	return;        // no image selected, return

      //DipalyPreviewWidget
      this->DisplayPreviewWidget(selectedItem);
                
      //Update the widget dispalyed
      if(m_DisplayStatusList[selectedItem-1])
	{
	  this->Display(selectedItem);
	  m_FullWindow->redraw();
	  m_ScrollWindow->redraw();
	  m_ZoomWindow->redraw();
	}

      //Update Information
      this->UpdateInformation(selectedItem);
    }
}

/**
 * CloseImage , Send the notification to the controller then to the model
 */
void
ImageViewerManagerViewGUI
::CloseImage()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
    return;        // no image selected, return
  
  //Call the controller
  m_ImageViewerManagerController->CloseImage(selectedItem);

  //Erase the item selected
  guiImageList->remove(selectedItem);
  m_PreviewWidget->hide();
  
  if(m_DisplayStatusList[selectedItem-1])
    {
      this->Undisplay(selectedItem);
    }
  m_DisplayStatusList.erase( m_DisplayStatusList.begin()+(selectedItem-1));

}
/**
 * Show the mainWindow 
 */
void
ImageViewerManagerViewGUI
::Show()
{
  guiMainWindow->show();
}

/**
 * Update the filename
 */
void
ImageViewerManagerViewGUI
::AddImageListName()
{
  //Update the Image List widget
  unsigned int len     = m_ImageViewerManagerModel->GetObjectList().size();
  std::string fileName = m_ImageViewerManagerModel->GetObjectList().at(len-1).fileName;
  int slashIndex       = fileName.find_last_of("/",fileName.size());
  
  itk::OStringStream oss;
  oss<<m_UndisplayedLabel;
  oss<<m_TemplateViewerName<<fileName.substr(slashIndex+1,fileName.size());
  guiImageList->add(oss.str().c_str());
  guiImageList->redraw();
}

/**
 * Quit GUI
 */
void
ImageViewerManagerViewGUI
::SelectAction()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
    {
      // no image selected, return
      return;
    }
  
  //DipalyPreviewWidget
  this->DisplayPreviewWidget(selectedItem);
    
  //Update SelectedImageInformation 
  this->UpdateInformation(selectedItem);
  
  //Udpate the ViewerGUISetup
  this->UpdateViewerSetupWindow(selectedItem);
}
/**
 * Quit GUI
 */
void
ImageViewerManagerViewGUI
::DisplayPreviewWidget(unsigned int selectedItem)
{
  //Build the m_PreviewWidget
  VisuModelPointerType rendering = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRendering;
    
  m_PreviewWidget->ClearBuffer();
  ImageViewerManagerModelType::ViewerImageType * quickLook = rendering->GetRasterizedQuicklook();
  m_PreviewWidget->ReadBuffer(quickLook, quickLook->GetLargestPossibleRegion());

  double newIsotropicZoom = this->UpdatePreviewWidgetIsotropicZoom(quickLook->GetLargestPossibleRegion().GetSize());  
  m_PreviewWidget->SetIsotropicZoom(newIsotropicZoom);
  m_PreviewWidget ->show();
  m_PreviewWidget->redraw();
}



/**
 * Compute the size of the 
 */
double
ImageViewerManagerViewGUI
::UpdatePreviewWidgetIsotropicZoom(SizeType size)
{
  int h = gPreviewWindow->h();
  int w = gPreviewWindow->w();
  
  double zoomW = static_cast<double>(w)/static_cast<double>(size[0]);
  double zoomH = static_cast<double>(h)/static_cast<double>(size[1]);
  
  return std::min(zoomW,zoomH);
}

/**
 * Show Hide
 */
void
ImageViewerManagerViewGUI
::ShowHide()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
    {
      // no image selected, return
      return;
    }
  
  /* check what to do */
  if(!m_DisplayStatusList[selectedItem-1])
    {
      //check if there are displayed images : Close them if yes
      for(unsigned int i = 0; i<m_DisplayStatusList.size() ; i++)    
	{
	  if(m_DisplayStatusList[i])
	    {
	      this->UpdateImageListShowed(i+1, m_UndisplayedLabel);
	      m_DisplayStatusList[i] = false;
	    }
	}
      //Display the new image
      m_DisplayStatusList[selectedItem-1] = true;
      this->UpdateImageListShowed(selectedItem, m_DisplayedLabel);
      this->Display(selectedItem);
    }
  else
    {
      m_DisplayStatusList[selectedItem-1] = false;
      this->UpdateImageListShowed(selectedItem, m_UndisplayedLabel);
      this->Undisplay(selectedItem);
    }
}

/**
 * Display the three widget
 */
void
ImageViewerManagerViewGUI
::Display(unsigned int selectedItem)
{
  //- Get the view stored in the model 
  //- Build the widgets 
  
  //First get the histogram list
  m_pRenderingFuntion = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion;
  m_Rhistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pLayer->GetHistogramList()->GetNthElement(m_pRenderingFuntion->GetRedChannelIndex()));
  m_Ghistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pLayer->GetHistogramList()->GetNthElement(m_pRenderingFuntion->GetGreenChannelIndex()));
  m_Bhistogram->SetHistogram(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pLayer->GetHistogramList()->GetNthElement(m_pRenderingFuntion->GetBlueChannelIndex()));
  
  //build the 
  VisuViewPointerType currentVisuView = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pVisuView;
  m_PixelView->SetModel(m_ImageViewerManagerModel->GetPixelModel());
  
  
  m_PackedWindow->RegisterFullWidget(currentVisuView->GetFullWidget());
  m_PackedWindow->RegisterScrollWidget(currentVisuView->GetScrollWidget());
  m_PackedWindow->RegisterZoomWidget(currentVisuView->GetZoomWidget());
  m_PackedWindow->RegisterPixelInformationWidget(m_PixelView->GetPixelDescriptionWidget());
  //m_PackedWindow->RegisterHistogramWidget(m_CurveWidget);
  m_PackedWindow->Show();

  
  
//   currentVisuView->GetFullWidget()->show();
//   //m_PixelView->GetPixelDescriptionWidget()->show();
//   currentVisuView->GetScrollWidget()->show();
//   currentVisuView->GetZoomWidget()->show();

    
//   //   //Pixel Description

//    //   m_PixelWindow->label("Pixel Description");
//    //   m_PixelWindow->resizable(m_PixelView->GetPixelDescriptionWidget());
//    //   m_PixelWindow->show();
//     m_PackedWindow->m_PixelInformationGroup->resizable(m_PixelView->GetPixelDescriptionWidget());
//     m_PixelView->GetPixelDescriptionWidget()->resize(m_PackedWindow->m_PixelInformationGroup->x(), m_PackedWindow->m_PixelInformationGroup->y(),
// 						     m_PackedWindow->m_PixelInformationGroup->w(),m_PackedWindow->m_PixelInformationGroup->h());
    
//     //   //FullWidget
//     //   m_FullWindow->label("Full Window");
//     //   m_FullWindow->add(currentVisuView->GetFullWidget());
//     //   m_FullWindow->resizable(currentVisuView->GetFullWidget());
//     //   m_FullWindow->show(); 
    
//     m_PackedWindow->m_FullGroup->add(currentVisuView->GetFullWidget());
//     m_PackedWindow->m_FullGroup->resizable(currentVisuView->GetFullWidget());
//     currentVisuView->GetFullWidget()->resize(m_PackedWindow->m_FullGroup->x(), m_PackedWindow->m_FullGroup->y(),
// 					     m_PackedWindow->m_FullGroup->w(),m_PackedWindow->m_FullGroup->h());
    
//     //   m_FullWindow->redraw();
    
//     //   //ScrollWidget
//     //   m_ScrollWindow->label("Scroll Window");
//     //   m_ScrollWindow->add((currentVisuView->GetScrollWidget()));
//     //   m_ScrollWindow->resizable(currentVisuView->GetScrollWidget());
//     //   m_ScrollWindow->show();
//     m_PackedWindow->m_QuicklookGroup->add(currentVisuView->GetScrollWidget());
//     m_PackedWindow->m_QuicklookGroup->resizable(currentVisuView->GetScrollWidget());
//     currentVisuView->GetScrollWidget()->resize(m_PackedWindow->m_QuicklookGroup->x(),m_PackedWindow->m_QuicklookGroup->y(),
// 					       m_PackedWindow->m_QuicklookGroup->w(),m_PackedWindow->m_QuicklookGroup->h());
//     //   m_ScrollWindow->redraw();
    
//     //   //Zoom Widget
//     //   m_ZoomWindow->label("Zoom Window");
//     //   m_ZoomWindow->add(currentVisuView->GetZoomWidget());
//     //   m_ZoomWindow->resizable(currentVisuView->GetZoomWidget());
//     //   m_ZoomWindow->show();
    
//     m_PackedWindow->m_ZoomGroup->add(currentVisuView->GetZoomWidget());
//     m_PackedWindow->m_ZoomGroup->resizable(currentVisuView->GetZoomWidget());
//     currentVisuView->GetZoomWidget()->resize(m_PackedWindow->m_ZoomGroup->x(),m_PackedWindow->m_ZoomGroup->y(),
// 					    m_PackedWindow->m_ZoomGroup->w(),m_PackedWindow->m_ZoomGroup->h());
    
//     //   m_ZoomWindow->redraw();
    
//     // adding histograms rendering
//     // Get the renderingFunction  

    
  
  //     // m_HistogramWindow->show();
//     m_PackedWindow->m_HistogramsGroup->add(m_CurveWidget);
//     m_CurveWidget->resize(m_PackedWindow->m_HistogramsGroup->x(),m_PackedWindow->m_HistogramsGroup->y(),
// 			 m_PackedWindow->m_HistogramsGroup->w(),m_PackedWindow->m_HistogramsGroup->h());
    
//     //   m_CurveWidget->redraw();
    
    //Display Everything
  

  //currentVisuView->GetFullWidget()->show();
  //m_PixelView->GetPixelDescriptionWidget()->show();
  //currentVisuView->GetScrollWidget()->show();
  //currentVisuView->GetZoomWidget()->show();
  //m_CurveWidget->show();
    
}

/**
 * Update the guiImageList
 * Put a  "+" if the view is being showed, or a "-" otherwise in the begining of the imageName
 */
void
ImageViewerManagerViewGUI
::UpdateImageListShowed(unsigned int selectedItem, std::string status)
{

  /* Update the ImageList using the status label "+" or "-" */
  std::string fileName = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).fileName;
  int slashIndex = fileName.find_last_of("/",fileName.size());
  
  itk::OStringStream oss;
  oss<<status;
  oss<<m_TemplateViewerName<<fileName.substr(slashIndex+1,fileName.size());
  guiImageList->text(selectedItem,oss.str().c_str());
  oss.str("");
}

/**
 * Hide all the widget opened
 */
void
ImageViewerManagerViewGUI
::Undisplay(unsigned int selectedItem)
{
  //   m_FullWindow->hide();
  //   m_ScrollWindow->hide(); 
  //   m_ZoomWindow->hide();
  //   m_PixelWindow->hide(); 
  //   m_HistogramWindow->hide();


  //m_PackedWindow->m_Window->hide();
}
/**
 * Hide all the widget opened
 */
void
ImageViewerManagerViewGUI
::HideAll()
{
  // Set the display Label to undislayed
  for(unsigned int i = 0; i<m_DisplayStatusList.size() ; i++)    
    {
      if(m_DisplayStatusList[i])
	{
	  std::cout << "cest le " << i+1 << " qui est affiche  "<< std::endl; 
	  this->UpdateImageListShowed(i+1, m_UndisplayedLabel);
	  m_DisplayStatusList[i] = false;
	}
    }
  // Close the opened widget

  //packeduWindow->m_Window->hide();

  
//   m_FullWindow->hide();
//   m_ScrollWindow->hide(); 
//   m_ZoomWindow->hide();
//   m_PixelWindow->hide();
//   m_HistogramWindow->hide();
}

/**
 * Quit GUI
 */
void
ImageViewerManagerViewGUI
::Quit()
{
  m_FullWindow->hide();
  m_ScrollWindow->hide(); 
  m_ZoomWindow->hide();
  m_PixelWindow->hide();
  guiMainWindow->hide();
}

/**
 *
 */
void
ImageViewerManagerViewGUI
::UpdateInformation(unsigned int selectedItem)
{
  itk::OStringStream oss;
  oss.str("");
  std::string selectedImageName = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).fileName;
  // Clear the info buffer
  guiViewerInformation->buffer()->remove(0,guiViewerInformation->buffer()->length());
  oss<<"Filename: "<<selectedImageName<<std::endl;
  guiViewerInformation->insert(oss.str().c_str());
  oss.str("");
  oss<<"Image information:"<<std::endl;
  guiViewerInformation->insert(oss.str().c_str());
  oss.str("");
  oss<<"Number of bands: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader->GetOutput()->GetNumberOfComponentsPerPixel();
  oss<<" - Size: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader->GetOutput()->GetLargestPossibleRegion().GetSize()<<std::endl;

  guiViewerInformation->insert(oss.str().c_str());
  oss.str("");

  //update band information
  if(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader->GetOutput()->GetNumberOfComponentsPerPixel()>=3)
    {
      oss<<"RGB Composition: ";
      oss<<" Band 1: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion->GetRedChannelIndex();
      oss<<" Band 2: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion->GetGreenChannelIndex();
      oss<<" Band 3: "<<m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion->GetBlueChannelIndex()<<std::endl;
    }
  
  guiViewerInformation->insert(oss.str().c_str());
  oss.str("");
}

/**
 *
 */
void
ImageViewerManagerViewGUI
::UpdateViewerSetupWindow(unsigned int selectedItem)
{
  
  ImageViewerManagerModelType::ReaderPointerType reader = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pReader;
  m_NbComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

  itk::OStringStream oss;
  oss.str("");
  
  for (unsigned int i = 0;i<m_NbComponent;++i)
  {
    oss.str("");
    oss<<i+1;
    guiGrayscaleChannelChoice->add(oss.str().c_str());
    guiRedChannelChoice->add(oss.str().c_str());
    guiGreenChannelChoice->add(oss.str().c_str());
    guiBlueChannelChoice->add(oss.str().c_str());
    guiGrayscaleChannelChoice->add(oss.str().c_str());
    guiRealChannelChoice->add(oss.str().c_str());
    guiImaginaryChannelChoice->add(oss.str().c_str());
  }

  switch(m_NbComponent){
  case 1 :
    this->GrayScaleSet();
    break;
  case 4 : 
    this->RGBSet();
    break;
  case 3 : 
    this->RGBSet();
    break;    
  default:
    this->ComplexSet();
  }
  
  guiViewerSetupName->value(m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).fileName.c_str());
  
}

/**
 * RGBSet();
 */
void 
ImageViewerManagerViewGUI
::RGBSet()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
    {
      // no image selected, return
      return;
    }

  StandardRenderingFunctionType::Pointer renderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion;
  
  guiViewerSetupColorMode->set();
  guiViewerSetupComplexMode->clear();
  guiViewerSetupGrayscaleMode->clear();
  guiGrayscaleChannelChoice->deactivate();
  guiRealChannelChoice->deactivate();
  guiImaginaryChannelChoice->deactivate();
  bModulus->deactivate();
  bPhase->deactivate();
  
  guiRedChannelChoice->activate();
  guiGreenChannelChoice->activate();
  guiBlueChannelChoice->activate();

  guiRedChannelChoice->value(std::min(renderingFunction->GetRedChannelIndex(),m_NbComponent-1));
  guiGreenChannelChoice->value(std::min(renderingFunction->GetGreenChannelIndex(),m_NbComponent-1));
  guiBlueChannelChoice->value(std::min(renderingFunction->GetBlueChannelIndex(),m_NbComponent-1));
  
}

/**
 * GrayScale();
 */
void 
ImageViewerManagerViewGUI
::GrayScaleSet()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
    {
      // no image selected, return
      return;
    }

  StandardRenderingFunctionType::Pointer renderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion;
  
  guiViewerSetupGrayscaleMode->set();
  guiViewerSetupComplexMode->clear();
  guiViewerSetupColorMode->clear();
  
  guiRealChannelChoice->deactivate();
  guiImaginaryChannelChoice->deactivate();
  bModulus->deactivate();
  bPhase->deactivate();
  guiRedChannelChoice->deactivate();
  guiGreenChannelChoice->deactivate();
  guiBlueChannelChoice->deactivate();

  guiGrayscaleChannelChoice->activate();
  guiGrayscaleChannelChoice->value(std::min(renderingFunction->GetRedChannelIndex(),m_NbComponent-1));
}


void
ImageViewerManagerViewGUI
::ComplexSet()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
  {
    // no image selected, return
    return;
  }
  
  StandardRenderingFunctionType::Pointer renderingFunction = m_ImageViewerManagerModel->GetObjectList().at(selectedItem-1).pRenderFuntion;
  
  guiViewerSetupComplexMode->set();
  guiViewerSetupColorMode->clear();
  guiViewerSetupGrayscaleMode->clear();
  guiGrayscaleChannelChoice->deactivate();
  guiRedChannelChoice->deactivate();
  guiGreenChannelChoice->deactivate();
  guiBlueChannelChoice->deactivate();
  guiRealChannelChoice->activate();
  guiImaginaryChannelChoice->activate();
  bModulus->activate();
  bPhase->activate();
  guiRealChannelChoice->value(std::min(renderingFunction->GetRedChannelIndex(),m_NbComponent-1));
  guiImaginaryChannelChoice->value(std::min(renderingFunction->GetGreenChannelIndex(),m_NbComponent-1));
}


/**
 * ViewerSetup();
 */
void 
ImageViewerManagerViewGUI
::ViewerSetup()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
  {
    // no image selected, return
    return;
  }
  guiViewerSetupWindow->show();
}

void 
ImageViewerManagerViewGUI
::ViewerSetupOk()
{
  unsigned int selectedItem = guiImageList->value();
  if (selectedItem == 0)
  {
    // no image selected, return
    return;
  }
  
  if (guiViewerSetupColorMode->value())
    {
      m_ImageViewerManagerController->UpdateRGBChannelOrder(atoi(guiRedChannelChoice->value())-1,
							    atoi(guiGreenChannelChoice->value())-1,
							    atoi(guiBlueChannelChoice->value())-1,
							    selectedItem);
    }
  else if (guiViewerSetupGrayscaleMode->value())
    {
      m_ImageViewerManagerController->UpdateGrayScaleChannelOrder(atoi(guiGrayscaleChannelChoice->value())-1,
								  selectedItem);
    }
  else if (guiViewerSetupComplexMode->value())
    {
      if (bModulus->value())
	{
	  m_ImageViewerManagerController->UpdateModulusChannelOrder(atoi(guiRealChannelChoice->value())-1,
								   atoi(guiImaginaryChannelChoice->value())-1,
								   selectedItem);
	}
      else
	{
	  m_ImageViewerManagerController->UpdatePhaseChannelOrder(atoi(guiRealChannelChoice->value())-1,
								  atoi(guiImaginaryChannelChoice->value())-1,
								  selectedItem);
	}
    }
}

void 
ImageViewerManagerViewGUI
::ViewerSetupCancel()
{
  guiViewerSetupWindow->hide();
}

/**
 * PrintSelf Method
 */

void
ImageViewerManagerViewGUI
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}
} // End namespace otb

