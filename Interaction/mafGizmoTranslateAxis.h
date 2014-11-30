/*=========================================================================

 Program: MAF2
 Module: mafGizmoTranslateAxis
 Authors: Stefano Perticoni
 
 Copyright (c) B3C
 All rights reserved. See Copyright.txt or
 http://www.scsitaly.com/Copyright.htm for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __mafGizmoTranslateAxis_H__
#define __mafGizmoTranslateAxis_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafEvent.h"
#include "mafGizmoInterface.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafInteractorGenericMouse;
class mafInteractorCompositorMouse;
class mafVMEGizmo;
class mafMatrix;
class mafVME;
class vtkConeSource;
class vtkCylinderSource;
class vtkTransformPolyDataFilter;
class vtkTransform;

/** Basic gizmo component used to perform constrained translation on one axis.
  
  @sa mafGizmoTranslate 
*/
class MAF_EXPORT mafGizmoTranslateAxis: public mafGizmoInterface 
{
public:
           mafGizmoTranslateAxis(mafVME *input, mafObserver *listener = NULL, mafString name = "");
  virtual ~mafGizmoTranslateAxis(); 
  
  /** 
  Set the gizmo generating vme; the gizmo will be centered on this vme*/
  void SetInput(mafVME *vme); 
  mafVME *GetInput() {return this->m_InputVme;};

  //----------------------------------------------------------------------------
  // events handling 
  //----------------------------------------------------------------------------
  
  /** Set the event receiver object*/
  void  SetListener(mafObserver *Listener) {m_Listener = Listener;};
  
  /** Events handling*/        
  virtual void OnEvent(mafEventBase *maf_event);
  
  //----------------------------------------------------------------------------
  // axis setting 
  //----------------------------------------------------------------------------

  /** Axis enum*/
  enum AXIS {X = 0, Y, Z};
  
  /** Set/Get gizmo axis, default axis is X*/        
  void SetAxis(int axis); 
  int  GetAxis() {return m_Axis;}; 
  
  //----------------------------------------------------------------------------
  // highlight and show 
  //----------------------------------------------------------------------------

  /** Highlight the gizmo*/
  void Highlight(bool highlight);
    
  /** Show the gizmo */
  void Show(bool show);

  //----------------------------------------------------------------------------
  // cone stuff
  //----------------------------------------------------------------------------
    
  /** Set/Get the length of the cone*/
  void   SetConeLength(double length);
  double GetConeLength() {return m_ConeLength;};

  /** Set/Get the length of the cone*/
  void   SetConeRadius(double radius);
  double GetConeRadius() {return m_ConeRadius;};
 
  //----------------------------------------------------------------------------
  // cylinder stuff
  //----------------------------------------------------------------------------
 
  /** Set/Get the length of the cylinder*/
  void   SetCylinderLength(double length);
  double GetCylinderLength() {return m_CylinderLength;};
  
  /** 
  Set the abs pose */
  void SetAbsPose(mafMatrix *absPose);
  mafMatrix *GetAbsPose();

  /**
  Set the constraint modality for the given axis; allowed constraint modality are:
  LOCK, FREE, BOUNDS, SNAP_STEP, SNAP_ARRAY.*/
  void SetConstraintModality(int axis, int constrainModality);

  /** Set the step value for snap step constraint type for the given axis*/
  void SetStep(int axis, double step);

protected:
  /** 
  Set the constrain ref sys */
  void SetRefSysMatrix(mafMatrix *constrain);

  /** Cone gizmo */
  mafVMEGizmo *m_ConeGizmo;

  /** cylinder gizmo*/
  mafVMEGizmo *m_CylGizmo;

  /** Register input vme*/
  mafVME *m_InputVme;

  enum GIZMOPARTS {CYLINDER = 0, CONE};
  
  /** Register the gizmo axis */
  int m_Axis;
  
  /** Cone source*/
  vtkConeSource *m_Cone;

  /** Cone length*/
  double m_ConeLength;
  double m_ConeRadius;

  /** Cylinder source*/
  vtkCylinderSource *m_Cylinder;
  
  /** Cylinder length*/
  double m_CylinderLength;

  /** Cylinder and cone gizmo vme data*/
  //mafVmeData *GizmoData[2];

  /** translate PDF for cylinder and cone*/
  vtkTransformPolyDataFilter *m_TranslatePDF[2];
  
  /** translation transform for cylinder and cone*/
  vtkTransform *m_TranslateTr[2];
 
  /** rotate PDF for cylinder and cone*/
  vtkTransformPolyDataFilter *m_RotatePDF[2];

  /** rotation transform for cylinder and cone*/
  vtkTransform *m_RotationTr;
  
  /** Create vtk objects needed*/
  void CreatePipeline();

  /** Create isa stuff */
  void CreateISA();

  /** isa compositor*/
  mafInteractorCompositorMouse *m_IsaComp[2];

  /** isa generic*/
  mafInteractorGenericMouse *m_IsaGen[2];

  /** Gizmo color setting facilities; part can be CYLINDER or CONE*/
  void SetColor(int part, double col[3]);
  void SetColor(int part, double colR, double colG, double colB);
  void SetColor(double cylCol[3], double coneCol[3]);
  void SetColor(double cylR, double cylG, double cylB, double coneR, double coneG, double coneB);

  /**
  Register the event receiver object*/
  mafObserver *m_Listener;

  /** test friend */
  friend class mafGizmoTranslateAxisTest;
};
#endif
