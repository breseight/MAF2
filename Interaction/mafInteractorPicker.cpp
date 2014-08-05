/*=========================================================================

 Program: MAF2
 Module: mafInteractorPicker
 Authors: Marco Petrone
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/


#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------


#include "mafInteractorPicker.h"
#include "mafEvent.h"

#include "mafView.h"
//#include "mafViewCompound.h"
#include "mafDeviceButtonsPadTracker.h"
#include "mafDeviceButtonsPadMouse.h"
#include "mafAvatar3D.h"

#include "mafEventInteraction.h"

#include "mafRWIBase.h"
#include "mafVME.h"
#include "mafTransform.h"

#include "vtkMAFSmartPointer.h"
#include "vtkCellPicker.h"
#include "vtkMAFRayCast3DPicker.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkRendererCollection.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"

#include <assert.h>

//------------------------------------------------------------------------------
mafCxxTypeMacro(mafInteractorPicker)
//------------------------------------------------------------------------------
mafInteractorPicker::mafInteractorPicker()
//------------------------------------------------------------------------------
{
  m_ContinuousPickingFlag = false;
}

//------------------------------------------------------------------------------
mafInteractorPicker::~mafInteractorPicker()
//------------------------------------------------------------------------------
{
}

//------------------------------------------------------------------------------
void mafInteractorPicker::OnEvent(mafEventBase *event)
//------------------------------------------------------------------------------
{
  Superclass::OnEvent(event);
  if (m_ContinuousPickingFlag)
  {
    if (mafDeviceButtonsPadMouse *mouse=mafDeviceButtonsPadMouse::SafeDownCast((mafDevice *)event->GetSender()))
    { 
      double mouse_pos[2];
      ((mafEventInteraction *)event)->Get2DPosition(mouse_pos);
      SendPickingInformation(mouse->GetView(), mouse_pos,VME_PICKING);
    }
  }
}
//----------------------------------------------------------------------------
void mafInteractorPicker::OnButtonDown(mafEventInteraction *e)
//----------------------------------------------------------------------------
{
  if (mafDeviceButtonsPadTracker *tracker=mafDeviceButtonsPadTracker::SafeDownCast((mafDevice *)e->GetSender()))
  { // is it a tracker?
    mafMatrix *tracker_pose = e->GetMatrix();
    // extract device avatar's renderer, no avatar == no picking
    mafAvatar *avatar = tracker->GetAvatar();
    if (avatar)
    {
      // compute pose in the world frame
      mafMatrix world_pose;
      mafAvatar3D *avatar3D=mafAvatar3D::SafeDownCast(avatar);
      if (avatar3D)
        avatar3D->TrackerToWorld(*tracker_pose,world_pose,mafAvatar3D::CANONICAL_TO_WORLD_SCALE);
      else
        world_pose = *tracker_pose;
      SendPickingInformation(avatar->GetView(),NULL,VME_PICKED,&world_pose,false);
    }
  }
  else if (mafDeviceButtonsPadMouse *mouse=mafDeviceButtonsPadMouse::SafeDownCast((mafDevice *)e->GetSender()))
  { 
    double mouse_pos[2];
    e->Get2DPosition(mouse_pos);
    SendPickingInformation(mouse->GetView(), mouse_pos);
  }
}

//----------------------------------------------------------------------------
void mafInteractorPicker::OnButtonUp(mafEventInteraction *e) 
//----------------------------------------------------------------------------
{
  if (m_ContinuousPickingFlag)
  {
    if (mafDeviceButtonsPadTracker *tracker=mafDeviceButtonsPadTracker::SafeDownCast((mafDevice *)e->GetSender()))
    { // is it a tracker?
      mafMatrix *tracker_pose = e->GetMatrix();
      // extract device avatar's renderer, no avatar == no picking
      mafAvatar *avatar = tracker->GetAvatar();
      if (avatar)
      {
        // compute pose in the world frame
        mafMatrix world_pose;
        mafAvatar3D *avatar3D=mafAvatar3D::SafeDownCast(avatar);
        if (avatar3D)
          avatar3D->TrackerToWorld(*tracker_pose,world_pose,mafAvatar3D::CANONICAL_TO_WORLD_SCALE);
        else
          world_pose = *tracker_pose;
        SendPickingInformation(avatar->GetView(),NULL,VME_PICKED,&world_pose,false);
      }
    }
    else if (mafDeviceButtonsPadMouse *mouse=mafDeviceButtonsPadMouse::SafeDownCast((mafDevice *)e->GetSender()))
    { 
      double mouse_pos[2];
      e->Get2DPosition(mouse_pos);
      SendPickingInformation(mouse->GetView(), mouse_pos);
    }
  }
}
//----------------------------------------------------------------------------
void mafInteractorPicker::SendPickingInformation(mafView *v, double *mouse_pos, int msg_id, mafMatrix *tracker_pos, bool mouse_flag)
//----------------------------------------------------------------------------
{
  bool picked_something = false;

  vtkCellPicker *cellPicker;
  vtkNEW(cellPicker);
  cellPicker->SetTolerance(0.001);
  if (v)
  {
    /*mafViewCompound *vc = mafViewCompound::SafeDownCast(v);
    if (vc)
      v = vc->GetSubView();*/ // the code is integrated into the GetRWI method of the mafViewCompound, so it is not necessary!
    if(mouse_flag)
    {
      vtkRendererCollection *rc = v->GetRWI()->GetRenderWindow()->GetRenderers();
      vtkRenderer *r = NULL;
      rc->InitTraversal();
      while(r = rc->GetNextItem())
      {
        if(cellPicker->Pick(mouse_pos[0],mouse_pos[1],0,r))
        {
          picked_something = true;
        }
      }
    }
    else
      picked_something = v->Pick(*tracker_pos);
    if (picked_something)
    {
      vtkPoints *p = vtkPoints::New();
      double pos_picked[3];
      cellPicker->GetPickPosition(pos_picked);
      p->SetNumberOfPoints(1);
      p->SetPoint(0,pos_picked);
      double scalar_value = 0;
      mafVME *pickedVME = v->GetPickedVme();
      if(pickedVME)
      {
        vtkDataSet *vtk_data = pickedVME->GetOutput()->GetVTKData();
        int pid = vtk_data->FindPoint(pos_picked);
        vtkDataArray *scalars = vtk_data->GetPointData()->GetScalars();
        if (scalars)
          scalars->GetTuple(pid,&scalar_value);
        mafEvent pick_event(this,msg_id,p);
        pick_event.SetDouble(scalar_value);
        pick_event.SetArg(pid);
		pick_event.SetView(v);
        mafEventMacro(pick_event);
        p->Delete();
      }
    }
  }
  vtkDEL(cellPicker);
}
