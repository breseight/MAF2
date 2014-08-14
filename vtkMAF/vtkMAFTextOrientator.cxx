/*=========================================================================

 Program: MAF2
 Module: vtkMAFTextOrientator
 Authors: Daniele Giunchi
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkMAFTextOrientator.h"

#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkPolyData.h"
#include "vtkInteractorObserver.h"


vtkCxxRevisionMacro(vtkMAFTextOrientator, "$Revision: 1.3.2.2 $");
vtkStandardNewMacro(vtkMAFTextOrientator);
//------------------------------------------------------------------------------
vtkMAFTextOrientator::vtkMAFTextOrientator()
//------------------------------------------------------------------------------
{
  TextSourceLeftActor = NULL;
  TextSourceLeftMapper = NULL;
  TextSourceLeft = NULL;

  TextSourceDownActor = NULL;
  TextSourceDownMapper = NULL;
  TextSourceDown = NULL;

  TextSourceRightActor = NULL;
  TextSourceRightMapper = NULL;
  TextSourceRight = NULL;

  TextSourceUpActor = NULL;
  TextSourceUpMapper = NULL;
  TextSourceUp = NULL;


  Dimension = 10;
  AttachPositionFlag = false;

  DisplayOffsetUp[0] = DisplayOffsetUp[1] = 0;  
  DisplayOffsetRight[0] = DisplayOffsetRight[1] = 0;
  DisplayOffsetDown[0] = DisplayOffsetDown[1] = 0;
  DisplayOffsetLeft[0] = DisplayOffsetLeft[1] = 0;

  OrientatorCreate();
}
//------------------------------------------------------------------------------
vtkMAFTextOrientator::~vtkMAFTextOrientator()
//------------------------------------------------------------------------------
{
  if(TextSourceLeftActor) TextSourceLeftActor->Delete();
  if(TextSourceLeftMapper) TextSourceLeftMapper->Delete();
  if(TextSourceLeft) TextSourceLeft->Delete();

  if(TextSourceDownActor) TextSourceDownActor->Delete();
  if(TextSourceDownMapper) TextSourceDownMapper->Delete();
  if(TextSourceDown) TextSourceDown->Delete();

  if(TextSourceRightActor) TextSourceRightActor->Delete();
  if(TextSourceRightMapper) TextSourceRightMapper->Delete();
  if(TextSourceRight) TextSourceRight->Delete();

  if(TextSourceUpActor) TextSourceUpActor->Delete();
  if(TextSourceUpMapper) TextSourceUpMapper->Delete();
  if(TextSourceUp) TextSourceUp->Delete();
	
}
//------------------------------------------------------------------------------
void vtkMAFTextOrientator::PrintSelf(ostream& os, vtkIndent indent)
//------------------------------------------------------------------------------
{
  this->Superclass::PrintSelf(os,indent);
}
//------------------------------------------------------------------------------
int vtkMAFTextOrientator::RenderOverlay(vtkViewport *viewport)
//------------------------------------------------------------------------------
{
  vtkRenderer *ren = static_cast<vtkRenderer *>(viewport);

  /*vtkCamera *cam = ren->GetActiveCamera();
  if(!cam->GetParallelProjection()) return 0;*/


  OrientatorUpdate(ren);
  this->Modified();

  if(TextSourceLeftActor->GetVisibility())
    TextSourceLeftActor->RenderOverlay(viewport);
  if(TextSourceDownActor->GetVisibility())
    TextSourceDownActor->RenderOverlay(viewport);
  if(TextSourceRightActor->GetVisibility())
    TextSourceRightActor->RenderOverlay(viewport);
  if(TextSourceUpActor->GetVisibility())
    TextSourceUpActor->RenderOverlay(viewport);

  return 1;
  
}
//------------------------------------------------------------------------------
int vtkMAFTextOrientator::RenderOpaqueGeometry(vtkViewport *viewport)
//------------------------------------------------------------------------------
{
	vtkRenderer *ren = static_cast<vtkRenderer *>(viewport);

	OrientatorUpdate(ren);
	this->Modified();

  if(TextSourceLeftActor->GetVisibility())
    TextSourceLeftActor->RenderOpaqueGeometry(viewport);
  if(TextSourceDownActor->GetVisibility())
    TextSourceDownActor->RenderOpaqueGeometry(viewport);
  if(TextSourceRightActor->GetVisibility())
    TextSourceRightActor->RenderOpaqueGeometry(viewport);
  if(TextSourceUpActor->GetVisibility())
    TextSourceUpActor->RenderOpaqueGeometry(viewport);
   
	return 0;
}

//------------------------------------------------------------------------------
void vtkMAFTextOrientator::OrientatorCreate()
//------------------------------------------------------------------------------
{
  //left
  TextSourceLeft = vtkTextSource::New();
  TextSourceLeft->BackingOn();
  TextSourceLeft->SetText("");
  TextSourceLeft->SetForegroundColor(1.0,1.0,1.0);
  TextSourceLeft->SetBackgroundColor(0.0,0.0,0.0);
  TextSourceLeft->Update();

  TextSourceLeftMapper = vtkPolyDataMapper2D::New();
  TextSourceLeftMapper->SetInput(TextSourceLeft->GetOutput());
  
  TextSourceLeftActor = vtkActor2D::New();
  TextSourceLeftActor->SetMapper(TextSourceLeftMapper);

  //down
  TextSourceDown = vtkTextSource::New();
  TextSourceDown->BackingOn();
  TextSourceDown->SetText("");
  TextSourceDown->SetForegroundColor(1.0,1.0,1.0);
  TextSourceDown->SetBackgroundColor(0.0,0.0,0.0);
  TextSourceDown->Update();

  TextSourceDownMapper = vtkPolyDataMapper2D::New();
  TextSourceDownMapper->SetInput(TextSourceDown->GetOutput());

  TextSourceDownActor = vtkActor2D::New();
  TextSourceDownActor->SetMapper(TextSourceDownMapper);

  //right
  TextSourceRight = vtkTextSource::New();
  TextSourceRight->BackingOn();
  TextSourceRight->SetText("");
  TextSourceRight->SetForegroundColor(1.0,1.0,1.0);
  TextSourceRight->SetBackgroundColor(0.0,0.0,0.0);
  TextSourceRight->Update();

  TextSourceRightMapper = vtkPolyDataMapper2D::New();
  TextSourceRightMapper->SetInput(TextSourceRight->GetOutput());

  TextSourceRightActor = vtkActor2D::New();
  TextSourceRightActor->SetMapper(TextSourceRightMapper);

  //up
  TextSourceUp = vtkTextSource::New();
  TextSourceUp->BackingOn();
  TextSourceUp->SetText("");
  TextSourceUp->SetForegroundColor(1.0,1.0,1.0);
  TextSourceUp->SetBackgroundColor(0.0,0.0,0.0);
  TextSourceUp->Update();

  TextSourceUpMapper = vtkPolyDataMapper2D::New();
  TextSourceUpMapper->SetInput(TextSourceUp->GetOutput());

  TextSourceUpActor = vtkActor2D::New();
  TextSourceUpActor->SetMapper(TextSourceUpMapper);
}

//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetTextLeft(const char * inputString)
//----------------------------------------------------------------------------
{
	TextSourceLeft->Delete();
	TextSourceLeft = NULL;
	TextSourceLeft = vtkTextSource::New();
	TextSourceLeftMapper->SetInput(TextSourceLeft->GetOutput());
	TextSourceLeft->SetText(inputString);
	TextSourceLeft->Update();

	//alignment
	double b[6];
	TextSourceLeft->GetOutput()->GetBounds(b);
	vtkTransform *transformLeft = vtkTransform::New();
	transformLeft->Translate(.5*(b[0]-b[1]),.5*(b[2]-b[3]),.5*(b[4]-b[5]));
	transformLeft->Update();
	vtkTransformPolyDataFilter *tpdfLeft = vtkTransformPolyDataFilter::New();
	tpdfLeft->SetTransform(transformLeft);
	tpdfLeft->SetInput(TextSourceLeft->GetOutput());
	tpdfLeft->Update();

	TextSourceLeft->GetOutput()->DeepCopy(tpdfLeft->GetOutput());
	TextSourceLeft->GetOutput()->Update();

	tpdfLeft->Delete();
	transformLeft->Delete();
}
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetTextDown(const char * inputString)
//----------------------------------------------------------------------------
{
	TextSourceDown->Delete();
	TextSourceDown = NULL;
	TextSourceDown = vtkTextSource::New();
	TextSourceDownMapper->SetInput(TextSourceDown->GetOutput());
	TextSourceDown->SetText(inputString);
	TextSourceDown->Update();
	//alignment
	double b[6];
	TextSourceDown->GetOutput()->GetBounds(b);
	vtkTransform *transformDown = vtkTransform::New();
	transformDown->Translate(.5*(b[0]-b[1]),.5*(b[2]-b[3]),.5*(b[4]-b[5]));
	transformDown->Update();
	vtkTransformPolyDataFilter *tpdfDown = vtkTransformPolyDataFilter::New();
	tpdfDown->SetTransform(transformDown);
	tpdfDown->SetInput(TextSourceDown->GetOutput());
	tpdfDown->Update();

	TextSourceDown->GetOutput()->DeepCopy(tpdfDown->GetOutput());
	TextSourceDown->GetOutput()->Update();

	tpdfDown->Delete();
	transformDown->Delete();
} 
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetTextRight(const char * inputString)
//----------------------------------------------------------------------------
{
	TextSourceRight->Delete();
	TextSourceRight = NULL;
	TextSourceRight = vtkTextSource::New();
	TextSourceRightMapper->SetInput(TextSourceRight->GetOutput());
	TextSourceRight->SetText(inputString);
	TextSourceRight->Update();
	//alignment
	double b[6];
	TextSourceRight->GetOutput()->GetBounds(b);
	vtkTransform *transformRight = vtkTransform::New();
	transformRight->Translate(.5*(b[0]-b[1]),.5*(b[2]-b[3]),.5*(b[4]-b[5]));
	transformRight->Update();
	vtkTransformPolyDataFilter *tpdfRight = vtkTransformPolyDataFilter::New();
	tpdfRight->SetTransform(transformRight);
	tpdfRight->SetInput(TextSourceRight->GetOutput());
	tpdfRight->Update();

	TextSourceRight->GetOutput()->DeepCopy(tpdfRight->GetOutput());
	TextSourceRight->GetOutput()->Update();

	tpdfRight->Delete();
	transformRight->Delete();
}

//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetTextUp(const char * inputString)
//----------------------------------------------------------------------------
{
	TextSourceUp->Delete();
	TextSourceUp = NULL;
	TextSourceUp = vtkTextSource::New();
	TextSourceUpMapper->SetInput(TextSourceUp->GetOutput());
	TextSourceUp->SetText(inputString);
	TextSourceUp->Update();
	//alignment
	double b[6];
	TextSourceUp->GetOutput()->GetBounds(b);
	vtkTransform *transformUp = vtkTransform::New();
	transformUp->Translate(.5*(b[0]-b[1]),.5*(b[2]-b[3]),.5*(b[4]-b[5]));
	transformUp->Update();
	vtkTransformPolyDataFilter *tpdfUp = vtkTransformPolyDataFilter::New();
	tpdfUp->SetTransform(transformUp);
	tpdfUp->SetInput(TextSourceUp->GetOutput());
	tpdfUp->Update();

	TextSourceUp->GetOutput()->DeepCopy(tpdfUp->GetOutput());
	TextSourceUp->GetOutput()->Update();

	tpdfUp->Delete();
	transformUp->Delete();
}

//----------------------------------------------------------------------------
void vtkMAFTextOrientator::ComputeWorldToDisplay(vtkRenderer *ren, double x, double y, double z, double displayPt[3])
//----------------------------------------------------------------------------
{
	if ( !ren ) 
		return;

	ren->SetWorldPoint(x, y, z, 1.0);
	ren->WorldToDisplay();
	ren->GetDisplayPoint(displayPt);
}

//----------------------------------------------------------------------------
void vtkMAFTextOrientator::OrientatorUpdate(vtkRenderer *ren)
//----------------------------------------------------------------------------
{
  int *renderSize;
  renderSize = ren->GetSize();
  
  int middleX = renderSize[0]/2;
  int middleY = renderSize[1]/2;

  
  
  if(AttachPositionFlag == false)
  {
    //left
	double displayPt[3];	
	//double displayPt0[3] = {0,0,0};
	//ComputeWorldToDisplay (ren, 0., 0., 0., displayPt0);

	//ComputeWorldToDisplay (ren, TextSourceLeftActor->GetWidth(), 0., 0., displayPt);
	
    TextSourceLeftActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    TextSourceLeftActor->SetPosition(1.5*Dimension, middleY);
	

    //down
	//ComputeWorldToDisplay (ren, 0., TextSourceDownActor->GetHeight(), 0., displayPt);
    TextSourceDownActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    TextSourceDownActor->SetPosition(middleX,  1.5*Dimension);

    //right
	//ComputeWorldToDisplay (ren, TextSourceRightActor->GetWidth(), 0., 0., displayPt);
    TextSourceRightActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    TextSourceRightActor->SetPosition(renderSize[0] - 1.5*Dimension, middleY);

    //up
	//ComputeWorldToDisplay (ren, 0., TextSourceUpActor->GetHeight(), 0., displayPt);
    TextSourceUpActor->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
    TextSourceUpActor->SetPosition(middleX, renderSize[1] - 1.5*Dimension);
  }
  else
  {
    

    double displayUp[3];
    ren->SetWorldPoint(AttachPositionUp[0],AttachPositionUp[1],AttachPositionUp[2],1.);
    ren->WorldToDisplay();
    ren->GetDisplayPoint(displayUp);
    int temporaryXUp,temporaryYUp;
    temporaryXUp = displayUp[0] + DisplayOffsetUp[0];
    temporaryYUp = displayUp[1]+ DisplayOffsetUp[1];

    if(temporaryXUp < 10)
      temporaryXUp = 10;
    if(temporaryXUp > renderSize[0] - 10)
      temporaryXUp = renderSize[0] - 10;

    if(temporaryYUp < 15)
      temporaryYUp = 15;
    if(temporaryYUp > renderSize[1] - 15)
      temporaryYUp = renderSize[1] - 15;

    TextSourceUpActor->SetPosition(temporaryXUp, temporaryYUp);


    double displayRight[3];
    ren->SetWorldPoint(AttachPositionRight[0],AttachPositionRight[1],AttachPositionRight[2],1.);
    ren->WorldToDisplay();
    ren->GetDisplayPoint(displayRight);
    int temporaryXRight,temporaryYRight;
    temporaryXRight = displayRight[0] + DisplayOffsetRight[0];
    temporaryYRight = displayRight[1]+ DisplayOffsetRight[1];

    if(temporaryXRight < 10)
      temporaryXRight = 10;
    if(temporaryXRight > renderSize[0] - 10)
      temporaryXRight = renderSize[0] - 10;

    if(temporaryYRight < 15)
      temporaryYRight = 15;
    if(temporaryYRight > renderSize[1] - 15)
      temporaryYRight = renderSize[1] - 15;


    TextSourceRightActor->SetPosition(temporaryXRight, temporaryYRight);

    double displayDown[3];
    ren->SetWorldPoint(AttachPositionDown[0],AttachPositionDown[1],AttachPositionDown[2],1.);
    ren->WorldToDisplay();
    ren->GetDisplayPoint(displayDown);
    int temporaryXDown,temporaryYDown;
    temporaryXDown = displayDown[0] + DisplayOffsetDown[0];
    temporaryYDown = displayDown[1] + DisplayOffsetDown[1];

    if(temporaryXDown < 10)
      temporaryXDown = 10;
    if(temporaryXDown > renderSize[0] - 10)
      temporaryXDown = renderSize[0] - 10;

    if(temporaryYDown < 15)
      temporaryYDown = 15;
    if(temporaryYDown > renderSize[1] - 15)
      temporaryYDown = renderSize[1] - 15;

    TextSourceDownActor->SetPosition(temporaryXDown, temporaryYDown);

    double displayLeft[3];
    ren->SetWorldPoint(AttachPositionLeft[0],AttachPositionLeft[1],AttachPositionLeft[2],1.);
    ren->WorldToDisplay();
    ren->GetDisplayPoint(displayLeft);
    int temporaryXLeft,temporaryYLeft;
    temporaryXLeft = displayLeft[0] + DisplayOffsetLeft[0];
    temporaryYLeft = displayLeft[1] + DisplayOffsetLeft[1];

    if(temporaryXLeft < 10)
      temporaryXLeft = 10;
    if(temporaryXLeft > renderSize[0] - 10)
      temporaryXLeft = renderSize[0] - 10;

    if(temporaryYLeft < 15)
      temporaryYLeft = 15;
    if(temporaryYLeft > renderSize[1] - 15)
      temporaryYLeft = renderSize[1] - 15;


    TextSourceLeftActor->SetPosition(temporaryXLeft, temporaryYLeft);
  }
  
}
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetBackgroundVisibility(bool show)
//----------------------------------------------------------------------------
{
  TextSourceLeft->SetBacking(show);
  TextSourceLeft->Update();
  TextSourceDown->SetBacking(show);
  TextSourceDown->Update();
  TextSourceRight->SetBacking(show);
  TextSourceRight->Update();
  TextSourceUp->SetBacking(show);
  TextSourceUp->Update();
}


//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetTextColor(double red, double green, double blue)
//----------------------------------------------------------------------------
{
  TextSourceLeft->SetForegroundColor(red,green,blue);
  TextSourceLeft->Update();
  TextSourceDown->SetForegroundColor(red,green,blue);
  TextSourceDown->Update();
  TextSourceRight->SetForegroundColor(red,green,blue);
  TextSourceRight->Update();
  TextSourceUp->SetForegroundColor(red,green,blue);
  TextSourceUp->Update();
}
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetBackgroundColor(double red, double green, double blue)
//----------------------------------------------------------------------------
{
  TextSourceLeft->SetBackgroundColor(red,green,blue);
  TextSourceLeft->Update();
  TextSourceDown->SetBackgroundColor(red,green,blue);
  TextSourceDown->Update();
  TextSourceRight->SetBackgroundColor(red,green,blue);
  TextSourceRight->Update();
  TextSourceUp->SetBackgroundColor(red,green,blue);
  TextSourceUp->Update();
}
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetTransform(double multiple, double angleLeft[3], double angleDown[3], double angleRight[3], double angleUp[3])
//----------------------------------------------------------------------------
{
  Dimension *= multiple;

  //left
  vtkTransform *transformLeft = vtkTransform::New();
  transformLeft->Scale(multiple,multiple,multiple);
  transformLeft->RotateX(angleLeft[0]);
  transformLeft->RotateY(angleLeft[1]);
  transformLeft->RotateZ(angleLeft[2]);
  transformLeft->Update();

  vtkTransformPolyDataFilter *tpdfLeft = vtkTransformPolyDataFilter::New();
  tpdfLeft->SetTransform(transformLeft);
  tpdfLeft->SetInput(TextSourceLeft->GetOutput());
  tpdfLeft->Update();
  TextSourceLeft->GetOutput()->DeepCopy(tpdfLeft->GetOutput());
  TextSourceLeft->GetOutput()->Update();
  tpdfLeft->Delete();
  transformLeft->Delete();

  //down
  vtkTransform *transformDown = vtkTransform::New();
  transformDown->Scale(multiple,multiple,multiple);
  transformDown->RotateX(angleDown[0]);
  transformDown->RotateY(angleDown[1]);
  transformDown->RotateZ(angleDown[2]);
  transformDown->Update();
  vtkTransformPolyDataFilter *tpdfDown = vtkTransformPolyDataFilter::New();
  tpdfDown->SetTransform(transformDown);
  tpdfDown->SetInput(TextSourceDown->GetOutput());
  tpdfDown->Update();
  TextSourceDown->GetOutput()->DeepCopy(tpdfDown->GetOutput());
  TextSourceDown->GetOutput()->Update();
  tpdfDown->Delete();
  transformDown->Delete();
  

  //right
  vtkTransform *transformRight = vtkTransform::New();
  transformRight->Scale(multiple,multiple,multiple);
  transformRight->RotateX(angleRight[0]);
  transformRight->RotateY(angleRight[1]);
  transformRight->RotateZ(angleRight[2]);
  transformRight->Update();
  vtkTransformPolyDataFilter *tpdfRight = vtkTransformPolyDataFilter::New();
  tpdfRight->SetTransform(transformRight);
  tpdfRight->SetInput(TextSourceRight->GetOutput());
  tpdfRight->Update();
  TextSourceRight->GetOutput()->DeepCopy(tpdfRight->GetOutput());
  TextSourceRight->GetOutput()->Update();
  tpdfRight->Delete();
  transformRight->Delete();


  //up
  vtkTransform *transformUp = vtkTransform::New();
  transformUp->Scale(multiple,multiple,multiple);
  transformUp->RotateX(angleUp[0]);
  transformUp->RotateY(angleUp[1]);
  transformUp->RotateZ(angleUp[2]);
  transformUp->Update();
  vtkTransformPolyDataFilter *tpdfUp = vtkTransformPolyDataFilter::New();
  tpdfUp->SetTransform(transformUp);
  tpdfUp->SetInput(TextSourceUp->GetOutput());
  tpdfUp->Update();
  TextSourceUp->GetOutput()->DeepCopy(tpdfUp->GetOutput());
  TextSourceUp->GetOutput()->Update();
  tpdfUp->Delete();
  transformUp->Delete();
}
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetSingleActorVisibility(int actor, bool show)
//----------------------------------------------------------------------------
{
  switch(actor)
  {
  case ID_ACTOR_LEFT:
    TextSourceLeftActor->SetVisibility(show);
    TextSourceLeftActor->Modified();
    break;
  case ID_ACTOR_DOWN:
    TextSourceDownActor->SetVisibility(show);
    TextSourceDownActor->Modified();
    break;
  case ID_ACTOR_RIGHT:
    TextSourceRightActor->SetVisibility(show);
    TextSourceRightActor->Modified();
    break;
  case ID_ACTOR_UP:
    TextSourceUpActor->SetVisibility(show);
    TextSourceUpActor->Modified();
    break;
  }
}
//----------------------------------------------------------------------------
void vtkMAFTextOrientator::SetAttachPositions(double up[3], double right[3], double Down[3], double left[3])
//----------------------------------------------------------------------------
{
   AttachPositionUp[0] = up[0];
   AttachPositionUp[1] = up[1];
   AttachPositionUp[2] = up[2];

   AttachPositionRight[0] = right[0];
   AttachPositionRight[1] = right[1];
   AttachPositionRight[2] = right[2];

   AttachPositionDown[0] = Down[0];
   AttachPositionDown[1] = Down[1];
   AttachPositionDown[2] = Down[2];

   AttachPositionLeft[0] = left[0];
   AttachPositionLeft[1] = left[1];
   AttachPositionLeft[2] = left[2];
}