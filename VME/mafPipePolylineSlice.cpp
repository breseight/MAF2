/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafPipePolylineSlice.cpp,v $
  Language:  C++
  Date:      $Date: 2007-02-21 17:22:47 $
  Version:   $Revision: 1.4 $
  Authors:   Daniele Giunchi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include "mafPipePolylineSlice.h"
#include "mafSceneNode.h"
#include "mmgGui.h"
#include "mafAxes.h"

#include "mmaMaterial.h"
#include "mafVMEGenericAbstract.h"
#include "mafVMEPolyline.h"
#include "mafVMEOutputPolyline.h"
#include "mafAbsMatrixPipe.h"

#include "vtkMAFAssembly.h"
#include "vtkRenderer.h"
#include "vtkOutlineCornerFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkPointData.h"
#include "vtkFixedCutter.h"
#include "vtkPlane.h"
#include "vtkMAFToLinearTransform.h"
#include "vtkTubeFilter.h"

#include <vector>

//----------------------------------------------------------------------------
mafCxxTypeMacro(mafPipePolylineSlice);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mafPipePolylineSlice::mafPipePolylineSlice()
:mafPipe()
//----------------------------------------------------------------------------
{
  m_Mapper          = NULL;
  m_Actor           = NULL;
  m_OutlineBox      = NULL;
  m_OutlineMapper   = NULL;
  m_OutlineProperty = NULL;
  m_OutlineActor    = NULL;
  m_Cutter          = NULL;
  m_Plane	    = NULL;
//m_Tube = NULL;
//m_TubeRadial = NULL;

  m_Origin[0] = 0;
  m_Origin[1] = 0;
  m_Origin[2] = 0;

  m_Normal[0] = 0;
  m_Normal[1] = 0;
  m_Normal[2] = 0;

  m_ScalarVisibility = 0;
  m_RenderingDisplayListFlag = 0;
  m_Border=1;
  m_Radius=1;
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::Create(mafSceneNode *n)
//----------------------------------------------------------------------------
{
  Superclass::Create(n);
  
  m_Selected = false;
  m_Mapper          = NULL;
  m_Actor           = NULL;
  m_OutlineBox      = NULL;
  m_OutlineMapper   = NULL;
  m_OutlineProperty = NULL;
  m_OutlineActor    = NULL;
  m_Axes            = NULL;

  assert(m_Vme->GetOutput()->IsMAFType(mafVMEOutputPolyline));
  mafVMEOutputPolyline *polyline_output = mafVMEOutputPolyline::SafeDownCast(m_Vme->GetOutput());
  assert(polyline_output);
  polyline_output->Update();
  vtkPolyData *data = vtkPolyData::SafeDownCast(polyline_output->GetVTKData());
  data->Update();
  assert(data);
  vtkDataArray *scalars = data->GetPointData()->GetScalars();
  double sr[2] = {0,1};

	//////////////////////////////////
  vtkNEW(m_Tube);
	m_Tube->UseDefaultNormalOn();
	m_Tube->SetInput(data);
	m_Tube->SetRadius(m_Radius);
	m_Tube->SetCapping(1);
	m_Tube->SetNumberOfSides(16);
	m_Tube->Update();
	
	data = m_Tube->GetOutput();
	//////////////////////////////////

	m_Plane	= vtkPlane::New();
	m_Cutter = vtkFixedCutter::New();

	m_Plane->SetOrigin(m_Origin);
	m_Plane->SetNormal(m_Normal);
	vtkMAFToLinearTransform* m_VTKTransform = vtkMAFToLinearTransform::New();
  m_VTKTransform->SetInputMatrix(m_Vme->GetAbsMatrixPipe()->GetMatrixPointer());
	m_Plane->SetTransform(m_VTKTransform);

	m_Cutter->SetInput(data);
	m_Cutter->SetCutFunction(m_Plane);
	m_Cutter->Update();

  if(scalars != NULL)
  {
    m_ScalarVisibility = 1;
    scalars->GetRange(sr);
  }

  m_Mapper = vtkPolyDataMapper::New();

  m_Mapper->SetInput(m_Cutter->GetOutput());
 
  
  m_Mapper->SetScalarVisibility(m_ScalarVisibility);
  m_Mapper->SetScalarRange(sr);
  
	if(m_Vme->IsAnimated())
  {
    m_RenderingDisplayListFlag = 1;
    m_Mapper->ImmediateModeRenderingOn();	 //avoid Display-Lists for animated items.
  }
	else
  {
    m_RenderingDisplayListFlag = 0;
    m_Mapper->ImmediateModeRenderingOff();
  }

  m_Actor = vtkActor::New();
  m_Actor->SetMapper(m_Mapper);

  m_Actor->GetProperty()->SetLineWidth (1);
  m_AssemblyFront->AddPart(m_Actor);

  // selection highlight
  m_OutlineBox = vtkOutlineCornerFilter::New();
	m_OutlineBox->SetInput(data);  

	m_OutlineMapper = vtkPolyDataMapper::New();
	m_OutlineMapper->SetInput(m_OutlineBox->GetOutput());

	m_OutlineProperty = vtkProperty::New();
	m_OutlineProperty->SetColor(1,1,1);
	m_OutlineProperty->SetAmbient(1);
	m_OutlineProperty->SetRepresentationToWireframe();
	m_OutlineProperty->SetInterpolationToFlat();

	m_OutlineActor = vtkActor::New();
	m_OutlineActor->SetMapper(m_OutlineMapper);
	m_OutlineActor->VisibilityOff();
	m_OutlineActor->PickableOff();
	m_OutlineActor->SetProperty(m_OutlineProperty);

  m_AssemblyFront->AddPart(m_OutlineActor);

  m_Axes = new mafAxes(m_RenFront, m_Vme);
  m_Axes->SetVisibility(0);

  /*
  m_axes = NULL;
	if(m_use_axes) m_axes = new mafAxes(m_ren1,m_Vme);
	if(m_use_axes) m_axes->SetVisibility(0);
	*/
  //CreateGui();
}
//----------------------------------------------------------------------------
mafPipePolylineSlice::~mafPipePolylineSlice()
//----------------------------------------------------------------------------
{
  m_AssemblyFront->RemovePart(m_Actor);
  m_AssemblyFront->RemovePart(m_OutlineActor);


  vtkDEL(m_Mapper);
  vtkDEL(m_Actor);
  vtkDEL(m_Tube);
  //vtkDEL(m_TubeRadial);
  vtkDEL(m_OutlineBox);
  vtkDEL(m_OutlineMapper);
  vtkDEL(m_OutlineProperty);
  vtkDEL(m_OutlineActor);
  vtkDEL(m_Plane);
  vtkDEL(m_Cutter);
  cppDEL(m_Axes);
	//@@@ if(m_use_axes) wxDEL(m_axes);  
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::Select(bool sel)
//----------------------------------------------------------------------------
{
	m_Selected = sel;
	if(m_Actor->GetVisibility()) 
	{
			//m_OutlineActor->SetVisibility(sel);
      //m_Axes->SetVisibility(sel);
	}
}
//----------------------------------------------------------------------------
mmgGui *mafPipePolylineSlice::CreateGui()
//----------------------------------------------------------------------------
{
  assert(m_Gui == NULL);
  m_Gui = new mmgGui(this);
  m_Gui->FloatSlider(ID_BORDER_CHANGE,_("Border"),&m_Border,1.0,5.0);
  return m_Gui;
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId()) 
    {
	  case ID_BORDER_CHANGE:
		  {
			  m_Actor->GetProperty()->SetLineWidth(m_Border);
			  m_Actor->Modified();
			  mafEventMacro(mafEvent(this,CAMERA_UPDATE));
		  }
	  break;
      default:
        mafEventMacro(*e);
      break;
    }
  }
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::SetSlice(double *Origin)
//----------------------------------------------------------------------------
{
	m_Origin[0] = Origin[0];
	m_Origin[1] = Origin[1];
	m_Origin[2] = Origin[2];
	
	if(m_Plane && m_Cutter)
	{
		m_Plane->SetOrigin(m_Origin);
		m_Cutter->SetCutFunction(m_Plane);
		m_Cutter->Update();
	}
  if(m_Vme != NULL)
    m_Actor->GetProperty()->SetColor(((mafVMEPolyline *)m_Vme)->GetMaterial()->m_Diffuse);
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::SetNormal(double *Normal)
//----------------------------------------------------------------------------
{
	m_Normal[0] = Normal[0];
	m_Normal[1] = Normal[1];
	m_Normal[2] = Normal[2];


	if(m_Plane && m_Cutter)
	{
		m_Plane->SetNormal(m_Normal);
		m_Cutter->SetCutFunction(m_Plane);
		m_Cutter->Update();
	}
}
//----------------------------------------------------------------------------
double mafPipePolylineSlice::GetThickness()
//----------------------------------------------------------------------------
{
	return m_Border;
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::SetThickness(double thickness)
//----------------------------------------------------------------------------
{
	m_Border=thickness;
	m_Actor->GetProperty()->SetLineWidth(m_Border);
  m_Actor->GetProperty()->SetPointSize(m_Border);

  m_Actor->GetProperty()->SetColor(((mafVMEPolyline *)m_Vme)->GetMaterial()->m_Diffuse);
  
  m_Actor->Modified();
	mafEventMacro(mafEvent(this,CAMERA_UPDATE));
}
//----------------------------------------------------------------------------
double mafPipePolylineSlice::GetRadius()
//----------------------------------------------------------------------------
{
//	return m_Radius;
	return -1;
}
//----------------------------------------------------------------------------
void mafPipePolylineSlice::SetRadius(double radius)
//----------------------------------------------------------------------------
{
	/*m_Radius=radius;
  m_Tube->SetRadius(m_Radius);
  m_Tube->Update();
  m_Actor->GetProperty()->SetColor(((mafVMEPolyline *)m_Vme)->GetMaterial()->m_Diffuse);
	
	mafEventMacro(mafEvent(this,CAMERA_UPDATE));*/
}