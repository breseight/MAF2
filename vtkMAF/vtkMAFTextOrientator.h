/*=========================================================================

  Program:   Multimod Fundation Library
  Module:    $RCSfile: vtkMAFTextOrientator.h,v $
  Language:  C++
  Date:      $Date: 2007-12-17 10:01:51 $
  Version:   $Revision: 1.3 $
  Authors:   Daniele Giunchi
  Project:   MultiMod Project (www.ior.it/multimod)

==========================================================================
  Copyright (c) 2002/2003 
  CINECA - Interuniversity Consortium (www.cineca.it)
  v. Magnanelli 6/3
  40033 Casalecchio di Reno (BO)
  Italy
  ph. +39-051-6171411 (90 lines) - Fax +39-051-6132198
========================================================================= */

#ifndef __vtkMAFTextOrientator_h
#define __vtkMAFTextOrientator_h


#include "vtkMAFConfigure.h" //??


#include "vtkActor2D.h"
#include "vtkTextSource.h"


class vtkViewport;
class vtkCamera;
class vtkRenderer;
class vtkProperty2D;
class vtkPolyDataMapper2D;
class vtkMapper;


/**
an actor displaying world X and Y axes as an Actor2D.
Camera must be in ParallelProjection and aligned with world axis
*/

//-----------------------------------------------------------------------------
class VTK_vtkMAF_EXPORT vtkMAFTextOrientator : public vtkActor2D
//-----------------------------------------------------------------------------
{
 public:
  vtkTypeRevisionMacro(vtkMAFTextOrientator,vtkActor2D);
  void PrintSelf(ostream& os, vtkIndent indent);
  static	vtkMAFTextOrientator *New();
  
  
  enum ID_TEXT_ORIENTATOR
  {
    ID_ACTOR_LEFT = 0,
    ID_ACTOR_DOWN,
    ID_ACTOR_RIGHT,
    ID_ACTOR_UP
  };
  
  const char*   GetTextLeft(){return m_TextSourceLeft->GetText();}
  const char*   GetTextDown(){return m_TextSourceDown->GetText();} 
  const char*   GetTextRight(){return m_TextSourceRight->GetText();}
  const char*   GetTextUp(){return m_TextSourceUp->GetText();}

  void 	  SetTextLeft(const char * inputString){m_TextSourceLeft->SetText(inputString);}
  void 	  SetTextDown(const char * inputString){m_TextSourceDown->SetText(inputString);} 
  void 	  SetTextRight(const char * inputString){m_TextSourceRight->SetText(inputString);}
  void 	  SetTextUp(const char * inputString){m_TextSourceUp->SetText(inputString);}
  
  void SetSingleActorVisibility(int actor, bool show);
  
  /** Set Visibility for background*/
  void SetBackgroundVisibility(bool show);

  /** Set text color*/
  void SetTextColor(double red, double green, double blue);

  /** Set background color*/
  void SetBackgroundColor(double red, double green, double blue);

  /** Set scale actor*/
  void SetScale(double multiple);

protected:
					vtkMAFTextOrientator();
					~vtkMAFTextOrientator();
	//methods
	
	void			OrientatorCreate();	
	void			OrientatorUpdate(vtkRenderer *ren);
   
  int	 RenderOverlay(vtkViewport *viewport);
  int	 RenderOpaqueGeometry(vtkViewport *viewport);      
  int	 RenderTranslucentGeometry(vtkViewport *viewport)  {return 0;}

  //variables
  int                     m_Dimension;

  vtkActor2D						 *m_TextSourceLeftActor;
  vtkPolyDataMapper2D		 *m_TextSourceLeftMapper;
  vtkTextSource          *m_TextSourceLeft;

  vtkActor2D						 *m_TextSourceDownActor;
  vtkPolyDataMapper2D		 *m_TextSourceDownMapper;
  vtkTextSource          *m_TextSourceDown;

  vtkActor2D						 *m_TextSourceRightActor;
  vtkPolyDataMapper2D		 *m_TextSourceRightMapper;
  vtkTextSource          *m_TextSourceRight;

  vtkActor2D						 *m_TextSourceUpActor;
  vtkPolyDataMapper2D		 *m_TextSourceUpMapper;
  vtkTextSource          *m_TextSourceUp;
	
private:
  // hide the two paraOrientator Render() method from the user and the compiler.
  virtual void Render(vtkRenderer *, vtkMapper *) {};
private:
  vtkMAFTextOrientator(const vtkMAFTextOrientator&);  	// Not implemented.
  void operator=(const vtkMAFTextOrientator&);  // Not implemented.
};
#endif