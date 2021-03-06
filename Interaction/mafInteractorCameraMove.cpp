/*=========================================================================

 Program: MAF2
 Module: mafInteractorCameraMove
 Authors: Paolo Quadrani & Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "mafDefines.h"

#include "mafInteractorCameraMove.h"
#include "mafDeviceButtonsPadMouse.h"
#include "mafAvatar3D.h"
#include "mafInteractor.h"

#include "mafEventInteraction.h"
#include "vtkMath.h"
#include "vtkRendererCollection.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkTransform.h"

#include <assert.h>

//------------------------------------------------------------------------------
mafCxxTypeMacro(mafInteractorCameraMove)
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
mafInteractorCameraMove::mafInteractorCameraMove()
//------------------------------------------------------------------------------
{
  m_MotionFactor   = 10.0;
  m_State = MOUSE_CAMERA_NONE;

  m_StartButton = -1;
  m_InteractionFlag = 0;
  m_CurrentCamera = NULL;
  m_MousePose[0] = m_MousePose[1] = 0;
  m_AutoResetClippingRange = true;
}

//------------------------------------------------------------------------------
mafInteractorCameraMove::~mafInteractorCameraMove()
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
void mafInteractorCameraMove::OnEvent(mafEventBase *event)
//------------------------------------------------------------------------------
{
  assert(event);
  assert(event->GetSender());
  
  mafID id=event->GetId();
  mafID channel=event->GetChannel();

  if (channel == MCH_INPUT && m_InteractionFlag)
  {
    mafEventInteraction *e = (mafEventInteraction *)event;
    mafDeviceButtonsPadMouse *mouse = mafDeviceButtonsPadMouse::SafeDownCast(GetDevice());
    
    // if the event comes from tracker which started the interaction continue...
    if (id == mafDeviceButtonsPadMouse::GetMouse2DMoveId() && mouse)
    {
      if (!m_CurrentCamera)
        return;

	    double pos2d[2];
      e->Get2DPosition(pos2d);
      m_MousePose[0] = (int)pos2d[0];
      m_MousePose[1] = (int)pos2d[1];
      
      OnMouseMove();

      m_LastMousePose[0] = m_MousePose[0];
      m_LastMousePose[1] = m_MousePose[1];
      
      return;
    }
  }
    
  Superclass::OnEvent(event);
}
//------------------------------------------------------------------------------
int mafInteractorCameraMove::StartInteraction(mafDeviceButtonsPadMouse *mouse)
//------------------------------------------------------------------------------
{
  SetRenderer(mouse->GetRenderer());
  if (m_Renderer)
  {
    m_CurrentCamera = m_Renderer->GetActiveCamera();
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnButtonDown(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
  m_ButtonPressed = e->GetButton();
  double pos[2];
  e->Get2DPosition(pos);
  m_LastMousePose[0] = m_MousePose[0] = (int)pos[0];
  m_LastMousePose[1] = m_MousePose[1] = (int)pos[1];

  mafDeviceButtonsPadMouse *mouse = (mafDeviceButtonsPadMouse *)e->GetSender();
  StartInteraction(mouse);

  switch(m_ButtonPressed) 
  {
    case MAF_LEFT_BUTTON:
      OnLeftButtonDown(e);
  	break;
    case MAF_MIDDLE_BUTTON:
      OnMiddleButtonDown(e);
  	break;
    case MAF_RIGHT_BUTTON:
      OnRightButtonDown(e);
  	break;
  }
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnButtonUp(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
  m_ButtonPressed = e->GetButton();
  
  switch(m_ButtonPressed) 
  {
    case MAF_LEFT_BUTTON:
      OnLeftButtonUp();
  	break;
    case MAF_MIDDLE_BUTTON:
      OnMiddleButtonUp();
  	break;
    case MAF_RIGHT_BUTTON:
      OnRightButtonUp();
  	break;
  }
  m_Renderer->GetRenderWindow()->SetDesiredUpdateRate(0.001);
  m_Renderer->GetRenderWindow()->Render();
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnMouseMove() 
//----------------------------------------------------------------------------
{ 

  if (m_Renderer)
  {
	 m_Renderer->GetRenderWindow()->SetDesiredUpdateRate(15.0);
  }

  switch (this->m_State) 
  {
    case MOUSE_CAMERA_ROTATE:
      this->Rotate();
    break;
    case MOUSE_CAMERA_LINKED_ROTATE:
      LinkedRotate();
      return;
    break;
    case MOUSE_CAMERA_PAN:
      this->Pan();
    break;
    case MOUSE_CAMERA_LINKED_PAN:
      LinkedPan();
      return;
    break;
    case MOUSE_CAMERA_DOLLY:
      this->Dolly();
    break;
    case MOUSE_CAMERA_LINKED_DOLLY:
      LinkedDolly();
      return;
    break;
    case MOUSE_CAMERA_SPIN:
      this->Spin();
    break;
  }

  
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnLeftButtonDown(mafEventInteraction *e)
//----------------------------------------------------------------------------
{ 
  if (e->GetModifier(MAF_SHIFT_KEY)) 
  {
    if (e->GetModifier(MAF_CTRL_KEY)) 
      this->StartDolly();
    else 
      this->StartPan();
  } 
  else 
  {
    if (e->GetModifier(MAF_CTRL_KEY)) 
      this->StartSpin();
    else 
      this->StartRotate();
  }
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnMiddleButtonDown(mafEventInteraction *e) 
//----------------------------------------------------------------------------
{
  StartPan();
}
//----------------------------------------------------------------------------
bool mafInteractorCameraMove::CameraIsPresent()
//----------------------------------------------------------------------------
{
  bool cam_is_present = false;
  for (int c=0; c<m_LinkedCamera.size(); c++) 
  {
    if (m_LinkedCamera[c] == m_CurrentCamera) 
    {
      cam_is_present = true;
      break;
    }
  }
  return cam_is_present;
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnRightButtonDown(mafEventInteraction *e) 
//----------------------------------------------------------------------------
{
  StartDolly();
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnLeftButtonUp()
//----------------------------------------------------------------------------
{
  switch (this->m_State) 
  {
    case MOUSE_CAMERA_DOLLY:
    case MOUSE_CAMERA_LINKED_DOLLY:
      this->EndDolly();
    break;
    case MOUSE_CAMERA_PAN:
    case MOUSE_CAMERA_LINKED_PAN:
      this->EndPan();
    break;
    case MOUSE_CAMERA_SPIN:
      this->EndSpin();
    break;
    case MOUSE_CAMERA_ROTATE:
    case MOUSE_CAMERA_LINKED_ROTATE:
      this->EndRotate();
    break;
  }
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnMiddleButtonUp()
//----------------------------------------------------------------------------
{
  switch (this->m_State) 
  {
    case MOUSE_CAMERA_PAN:
    case MOUSE_CAMERA_LINKED_PAN:
      this->EndPan();
      break;
  }
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::OnRightButtonUp()
//----------------------------------------------------------------------------
{
  switch (this->m_State) 
  {
    case MOUSE_CAMERA_DOLLY:
    case MOUSE_CAMERA_LINKED_DOLLY:
      this->EndDolly();
      break;
  }
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::Rotate()
//----------------------------------------------------------------------------
{
  if (m_Renderer == NULL)
    return;

	if (m_CurrentCamera->GetParallelProjection()) return; 

  int dx = m_MousePose[0] - m_LastMousePose[0];
  int dy = m_MousePose[1] - m_LastMousePose[1];
  
  int *size = m_Renderer->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];
  
  double rxf = (double)dx * delta_azimuth * this->m_MotionFactor;
  double ryf = (double)dy * delta_elevation * this->m_MotionFactor;
  
  m_CurrentCamera->Azimuth(rxf);
  m_CurrentCamera->Elevation(ryf);
  m_CurrentCamera->OrthogonalizeViewUp();

  if (m_AutoResetClippingRange) ResetClippingRange();
  m_Renderer->GetRenderWindow()->Render();
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::Spin()
//----------------------------------------------------------------------------
{
  if (m_Renderer == NULL)
    return;

	if (m_CurrentCamera->GetParallelProjection()) return; 

  double *center = m_Renderer->GetCenter();

  double newAngle = 
    atan2((double)m_MousePose[1] - (double)center[1],
          (double)m_MousePose[0] - (double)center[0]);

  double oldAngle = 
    atan2((double)m_LastMousePose[1] - (double)center[1],
          (double)m_LastMousePose[0] - (double)center[0]);
  
  newAngle *= vtkMath::RadiansToDegrees();
  oldAngle *= vtkMath::RadiansToDegrees();

  m_CurrentCamera->Roll(newAngle - oldAngle);
  m_CurrentCamera->OrthogonalizeViewUp();
      
  if (m_AutoResetClippingRange)  ResetClippingRange();
  m_Renderer->GetRenderWindow()->Render();
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::Pan()
//----------------------------------------------------------------------------
{
  if (this->m_Renderer == NULL)
    return;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];
  
  // Calculate the focal depth since we'll be using it a lot

  m_CurrentCamera->GetFocalPoint(viewFocus);
  ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);
  focalDepth = viewFocus[2];

  ComputeDisplayToWorld((double)m_MousePose[0], (double)m_MousePose[1], focalDepth, newPickPoint);
    
  // Has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop

  ComputeDisplayToWorld((double)m_LastMousePose[0],
                              (double)m_LastMousePose[1],
                              focalDepth, 
                              oldPickPoint);
  
  // Camera motion is reversed

  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];
  
  m_CurrentCamera->GetFocalPoint(viewFocus);
  m_CurrentCamera->GetPosition(viewPoint);
  m_CurrentCamera->SetFocalPoint(motionVector[0] + viewFocus[0],
                        motionVector[1] + viewFocus[1],
                        motionVector[2] + viewFocus[2]);

  m_CurrentCamera->SetPosition(motionVector[0] + viewPoint[0],
                      motionVector[1] + viewPoint[1],
                      motionVector[2] + viewPoint[2]);
      
  if (m_AutoResetClippingRange)  ResetClippingRange();
  m_Renderer->GetRenderWindow()->Render();
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::Dolly()
//----------------------------------------------------------------------------
{
  if (m_Renderer == NULL)
    return;
  
  double *center = m_Renderer->GetCenter();

  int dy = m_MousePose[1] - m_LastMousePose[1];
  double dyf = this->m_MotionFactor * (double)(dy) / (double)(center[1]);
  double zoomFactor = pow((double)1.1, dyf);
  
  if (m_CurrentCamera->GetParallelProjection())
    m_CurrentCamera->SetParallelScale(m_CurrentCamera->GetParallelScale()/zoomFactor);
  else
    m_CurrentCamera->Dolly(zoomFactor);
  
  if (m_AutoResetClippingRange)  ResetClippingRange();
	m_Renderer->GetRenderWindow()->Render();
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::LinkedDolly()
//----------------------------------------------------------------------------
{
  if (m_Renderer == NULL)
    return;

  double *center = m_Renderer->GetCenter();

  int dy = m_MousePose[1] - m_LastMousePose[1];
  double dyf = this->m_MotionFactor * (double)(dy) / (double)(center[1]);
  double zoomFactor = pow((double)1.1, dyf);

  for (int c=0; c<m_LinkedCamera.size(); c++) 
  {
    if (m_LinkedCamera[c]->GetParallelProjection())
      m_LinkedCamera[c]->SetParallelScale(m_CurrentCamera->GetParallelScale()/zoomFactor);
    else
      m_LinkedCamera[c]->Dolly(zoomFactor);
  }

  mafEvent e(this,CAMERA_UPDATE);
  InvokeEvent(&e, MCH_UP);
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::LinkedPan()
//----------------------------------------------------------------------------
{
  if (this->m_Renderer == NULL)
    return;

  double viewFocus[4], focalDepth, viewPoint[3];
  double newPickPoint[4], oldPickPoint[4], motionVector[3];

  for (int c=0; c<m_LinkedCamera.size(); c++) 
  {
    // Calculate the focal depth since we'll be using it a lot
    m_LinkedCamera[c]->GetFocalPoint(viewFocus);
    ComputeWorldToDisplay(viewFocus[0], viewFocus[1], viewFocus[2], viewFocus);
    focalDepth = viewFocus[2];

    ComputeDisplayToWorld((double)m_MousePose[0], (double)m_MousePose[1], focalDepth, newPickPoint);

    // Has to recalc old mouse point since the viewport has moved,
    // so can't move it outside the loop
    ComputeDisplayToWorld((double)m_LastMousePose[0],(double)m_LastMousePose[1],focalDepth, oldPickPoint);

    // Camera motion is reversed
    motionVector[0] = oldPickPoint[0] - newPickPoint[0];
    motionVector[1] = oldPickPoint[1] - newPickPoint[1];
    motionVector[2] = oldPickPoint[2] - newPickPoint[2];

    m_LinkedCamera[c]->GetFocalPoint(viewFocus);
    m_LinkedCamera[c]->GetPosition(viewPoint);
    m_LinkedCamera[c]->SetFocalPoint(motionVector[0] + viewFocus[0],
                                   motionVector[1] + viewFocus[1],
                                   motionVector[2] + viewFocus[2]);

    m_LinkedCamera[c]->SetPosition(motionVector[0] + viewPoint[0],
                                 motionVector[1] + viewPoint[1],
                                 motionVector[2] + viewPoint[2]);
  }

  mafEvent e(this,CAMERA_UPDATE);
  InvokeEvent(&e, MCH_UP);
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::LinkedRotate()
//----------------------------------------------------------------------------
{
  if (m_Renderer == NULL)
    return;

  if (m_CurrentCamera->GetParallelProjection()) return; 

  int dx = m_MousePose[0] - m_LastMousePose[0];
  int dy = m_MousePose[1] - m_LastMousePose[1];

  int *size = m_Renderer->GetRenderWindow()->GetSize();

  double delta_elevation = -20.0 / size[1];
  double delta_azimuth = -20.0 / size[0];

  double rxf = (double)dx * delta_azimuth * this->m_MotionFactor;
  double ryf = (double)dy * delta_elevation * this->m_MotionFactor;

  for (int c=0; c<m_LinkedCamera.size(); c++)
  {
    m_LinkedCamera[c]->Azimuth(rxf);
    m_LinkedCamera[c]->Elevation(ryf);
    m_LinkedCamera[c]->OrthogonalizeViewUp();
  }

  mafEvent e(this,CAMERA_UPDATE);
  InvokeEvent(&e, MCH_UP);
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::ResetClippingRange()
//----------------------------------------------------------------------------
{
  // 20.12.2010: Modified by Simone Brazzale:
  // Added patch in the presence of the third layer.

  vtkRendererCollection *rc = m_Renderer->GetRenderWindow()->GetRenderers();
  rc->InitTraversal();

  // Layers can be ONE or TWO; the third layer ALWAYS_VISIBLE doesn't count, as implemented in mafRWI.
  int numberOfLayers = m_Renderer->GetRenderWindow()->GetNumberOfLayers();

  // The order of the renderers has been set in mafRWI.
  // This is ALWAYS VISIBLE RENDERER 
	vtkRenderer *rAV = rc->GetNextItem(); 	 
  // This is FRONT RENDERER
	vtkRenderer *rFR = rc->GetNextItem(); 	 
  // This can be BACK RENDERER or a renderer added by vtkMAFOrientationWidget.
  // It is BACK RENDERER only if  numberOfLayers==3.
  vtkRenderer *rBR = NULL;
  if (numberOfLayers==3)
  {
	  rBR = rc->GetNextItem(); 	 
  }

	double b1[6],b2[6],b3[6],b[6];
  if(rFR==NULL)
	{
	} 
	else if (rAV==NULL && rBR==NULL)
	{
     rFR->ResetCameraClippingRange(); 
	}
  else if (rAV)
  {
    // We have the tird layer (always visible), with default bounds (-1,1,-1,1,-1,1).
    // The clipping range must be computed with the bounds of the front renderer!
		rFR->ComputeVisiblePropBounds(b1);
		rAV->ComputeVisiblePropBounds(b2);

		if(b1[0] == VTK_LARGE_FLOAT && b2[0] == VTK_LARGE_FLOAT)
		{
			rFR->ResetCameraClippingRange();
		} 
		else if (b1[0] == VTK_LARGE_FLOAT )
		{
			rFR->ResetCameraClippingRange(b2);
		}
		else
		{
      // WORKAROUND 
      // The only actor shown is the Axis actor in the bottom left angle:
      // it must not be taken into account.
      if (b2[0]==-1 && b2[1]==1 && b2[2]==-1 && b2[3]==1 && b2[4]==-1 && b2[5]==1)
      {
        b[0] = b1[0];
			  b[2] = b1[2];
        b[4] = b1[4];
        b[1] = b1[1];
			  b[3] = b1[3];
			  b[5] = b1[5];
      }
      // WORKAROUND 
      // There are other actors (like the GIZMO): 
      // we must take them into account.
      else
      {
        b[0] = (b1[0]<b2[0]) ?	b1[0] : b2[0];    
			  b[2] = (b1[2]<b2[2]) ?	b1[2] : b2[2];    
			  b[4] = (b1[4]<b2[4]) ?	b1[4] : b2[4];    
			  b[1] = (b1[1]>b2[1]) ?	b1[1] : b2[1];    
			  b[3] = (b1[3]>b2[3]) ?	b1[3] : b2[3];    
			  b[5] = (b1[5]>b2[5]) ?	b1[5] : b2[5];  
      }
			rFR->ResetCameraClippingRange(b);
		}

    // We have also the back renderer.
    // The clipping range must be matched between the back renderer bounds and the already calculated one.	
    if (rBR)
  	{
	  	rBR->ComputeVisiblePropBounds(b3);

		  if (b3[0] == VTK_LARGE_FLOAT )
		  {
        // do nothing
			}
		  else
		  {
			  b[0] = (b[0]<b3[0]) ?	b[0] : b3[0];    
			  b[2] = (b[2]<b3[2]) ?	b[2] : b3[2];    
			  b[4] = (b[4]<b3[4]) ?	b[4] : b3[4];    
			  b[1] = (b[1]>b3[1]) ?	b[1] : b3[1];    
			  b[3] = (b[3]>b3[3]) ?	b[3] : b3[3];    
			  b[5] = (b[5]>b3[5]) ?	b[5] : b3[5];    
			  rFR->ResetCameraClippingRange(b);
		  }
	  }
  }
}


//----------------------------------------------------------------------------
void mafInteractorCameraMove::StartRotate() 
//----------------------------------------------------------------------------
{
  if (m_State != MOUSE_CAMERA_NONE) 
  {
    return;
  }

  int state = (m_LinkedCamera.size() != 0 && CameraIsPresent()) ? MOUSE_CAMERA_LINKED_ROTATE : MOUSE_CAMERA_ROTATE;
  this->StartState(state);
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::EndRotate() 
//----------------------------------------------------------------------------
{
  if (m_State != MOUSE_CAMERA_ROTATE  &&
      m_State != MOUSE_CAMERA_LINKED_ROTATE &&
      m_State != MOUSE_CAMERA_LINKED_PAN) 
  {
    return;
  }
  this->StopState();
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::StartZoom() 
//----------------------------------------------------------------------------
{
  if (this->m_State != MOUSE_CAMERA_NONE) 
  {
    return;
  }
  this->StartState(MOUSE_CAMERA_ZOOM);
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::EndZoom() 
//----------------------------------------------------------------------------
{
  if (this->m_State != MOUSE_CAMERA_ZOOM) 
  {
    return;
  }
  this->StopState();
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::StartPan() 
//----------------------------------------------------------------------------
{
  if (this->m_State != MOUSE_CAMERA_NONE) 
  {
    return;
  }
  int state = (m_LinkedCamera.size() != 0 && CameraIsPresent()) ? MOUSE_CAMERA_LINKED_PAN : MOUSE_CAMERA_PAN;
  this->StartState(state);
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::EndPan() 
//----------------------------------------------------------------------------
{
  if (m_State != MOUSE_CAMERA_PAN &&
      m_State != MOUSE_CAMERA_LINKED_PAN) 
  {
    return;
  }
  this->StopState();
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::StartSpin() 
//----------------------------------------------------------------------------
{
  if (this->m_State != MOUSE_CAMERA_NONE) 
  {
    return;
  }
  this->StartState(MOUSE_CAMERA_SPIN);
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::EndSpin() 
//----------------------------------------------------------------------------
{
  if (this->m_State != MOUSE_CAMERA_SPIN) 
  {
    return;
  }
  this->StopState();
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::StartDolly() 
//----------------------------------------------------------------------------
{
  if (this->m_State != MOUSE_CAMERA_NONE) 
  {
    return;
  }
  int state = (m_LinkedCamera.size() != 0 && CameraIsPresent()) ? MOUSE_CAMERA_LINKED_DOLLY : MOUSE_CAMERA_DOLLY;
  this->StartState(state);
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::EndDolly() 
//----------------------------------------------------------------------------
{
    if (m_State != MOUSE_CAMERA_DOLLY &&
        m_State != MOUSE_CAMERA_LINKED_DOLLY)
    {
      return;
    }
    this->StopState();
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::StartState(int newstate) 
//----------------------------------------------------------------------------
{
  this->m_State = newstate;
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::StopState() 
//----------------------------------------------------------------------------
{
  this->m_State = MOUSE_CAMERA_NONE;
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::AddLinkedCamera(vtkCamera *cam)
//----------------------------------------------------------------------------
{
  m_LinkedCamera.push_back(cam);
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::RemoveLinkedCamera(vtkCamera *cam)
//----------------------------------------------------------------------------
{
  for (std::vector<vtkCamera *>::iterator it = m_LinkedCamera.begin(); it != m_LinkedCamera.end(); it++) 
  {
    if (*it == cam) 
    {
      m_LinkedCamera.erase(it);
      break;
    }
  }
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::RemoveAllLinkedCamera()
//----------------------------------------------------------------------------
{
  m_LinkedCamera.clear();
}
//----------------------------------------------------------------------------
void mafInteractorCameraMove::AutoResetClippingRangeOff()
//----------------------------------------------------------------------------
{
	m_AutoResetClippingRange = false;
}

//----------------------------------------------------------------------------
void mafInteractorCameraMove::AutoResetClippingRangeOn()
//----------------------------------------------------------------------------
{
	m_AutoResetClippingRange = true;
}
