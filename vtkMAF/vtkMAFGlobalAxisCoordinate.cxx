/*=========================================================================

  Program:   Multimod Fundation Library
  Module:    $RCSfile: vtkMAFGlobalAxisCoordinate.cxx,v $
  Language:  C++
  Date:      $Date: 2008-07-03 11:27:45 $
  Version:   $Revision: 1.1 $
  Authors:   Silvano Imboden 
  Project:   MultiMod Project (www.ior.it/multimod)

==========================================================================*/

#include "vtkMAFGlobalAxisCoordinate.h"
#include "vtkViewport.h"
#include "vtkObjectFactory.h"

#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"

//vtkCxxRevisionMacro(vtkMAFGlobalAxisCoordinate, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkMAFGlobalAxisCoordinate);

//----------------------------------------------------------------------------
vtkMAFGlobalAxisCoordinate::vtkMAFGlobalAxisCoordinate()
//----------------------------------------------------------------------------
{
  this->CoordinateSystem = VTK_USERDEFINED;

	camera = vtkCamera::New();
	renderer = vtkRenderer::New();
  renderer->SetActiveCamera(camera);
	renderwindow = vtkRenderWindow::New();
  renderwindow->AddRenderer(renderer);
  renderwindow->SetSize(40,40);
}
//----------------------------------------------------------------------------
vtkMAFGlobalAxisCoordinate::~vtkMAFGlobalAxisCoordinate()
//----------------------------------------------------------------------------
{
	camera->Delete();
  renderer->Delete();
  renderwindow->Delete();
}
//----------------------------------------------------------------------------
double *vtkMAFGlobalAxisCoordinate::GetComputedUserDefinedValue(vtkViewport *viewport)
//----------------------------------------------------------------------------
{
	vtkRenderer *r = (vtkRenderer *)viewport;
  vtkCamera *c = r->GetActiveCamera();

  camera->SetViewAngle(c->GetViewAngle());
  camera->SetPosition(c->GetPosition());
  camera->SetFocalPoint(c->GetFocalPoint());
  camera->SetViewUp(c->GetViewUp());
  camera->SetParallelProjection(c->GetParallelProjection());
  camera->SetParallelScale(c->GetParallelScale());

	double b = 0.5;
	renderer->ResetCamera(-b,b,-b,b,-b,b);

  double w[4];
	w[0] = this->Value[0];
	w[1] = this->Value[1];
	w[2] = this->Value[2];
	w[3] = 1;

	renderer->SetWorldPoint(w);
	renderer->WorldToDisplay();
	renderer->GetDisplayPoint(ComputedUserDefinedValue);  

  return this->ComputedUserDefinedValue;
}