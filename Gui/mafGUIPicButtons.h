/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGUIPicButtons.h,v $
  Language:  C++
  Date:      $Date: 2009-10-29 15:15:00 $
  Version:   $Revision: 1.4.2.2 $
  Authors:   Daniele Giunchi
==========================================================================
  Copyright (c) 2002/2004
  CINECA - Interuniversity Consortium (www.cineca.it) 
=========================================================================*/

#ifndef __mafGUIPicButtons_H__
#define __mafGUIPicButtons_H__

//----------------------------------------------------------------------------
// Include:
//----------------------------------------------------------------------------
#include "mafDecl.h"
#include "mafEvent.h"
#include "mafObserver.h"
#include "mafGUIPanel.h"

//----------------------------------------------------------------------------
// forward references :
//----------------------------------------------------------------------------
class mafGUIPicButton;    
class mafGUIValidator; 

//----------------------------------------------------------------------------
// Constants :
//----------------------------------------------------------------------------
#define ID_TIMER  702
#define MOVIE_BUTTONS_NUM 6

//----------------------------------------------------------------------------
// mafGUIPicButtons :
/** mafGUIPicButtons is a wxPanel with a set of widget to handle time. */
//----------------------------------------------------------------------------
class MAF_EXPORT mafGUIPicButtons: public mafGUIPanel , public mafObserver
{
public:
  mafGUIPicButtons(wxWindow* parent,wxWindowID id = -1, int numberOfButtons = 1, int numberOfColumns = 1);
  mafGUIPicButtons(wxWindow* parent,wxWindowID id = -1, mafString filename = "", int numberOfColumns = 1);
  virtual ~mafGUIPicButtons();

  enum ID_BUTTON_TYPE {
	  ID_RADIO_BUTTON,
	  ID_BUTTON,
      ID_CHECK_BUTTON,
  };

  void SetListener(mafObserver *Listener) {m_Listener = Listener;};

  void OnEvent(mafEventBase *maf_event);
  void SetPictureVector(std::vector<wxString> &pictures);
  void Create();
  void ActivateButton(int index, bool activated);
  void DeActivateAllButtons();
  void EnableButton(int index, bool activated);
  void DisableAllButtons();
  void SetToolTip(int index, const char* toolTip);

  int GetActiveButton(){return m_ActiveButtonId;}
  void SetButtonsType(int type) {m_ButtonsType = type;};
  bool GetCheckStatus(int index) {return m_CheckList[index];}
  
  void GetInfoButton(int index, std::vector< std::string > &infos);
  //void ShowButton(int index, bool enable);

protected:
  //----------------------------------------------------------------------------
  // constant
  //----------------------------------------------------------------------------
  enum ID_FIRST_EVENT
  {
    FIRST_BUTTON_ID = MINID,
  };

  wxFlexGridSizer		  *m_Sizer;
  //mafGUIPicButton		*m_TimeBarButtons[MOVIE_BUTTONS_NUM];
  std::vector<mafGUIPicButton *> m_PicButtons;
  std::vector<wxString> m_Pictures;
  std::vector<std::vector<wxString> >m_ButtonsInfo;

  mafObserver     *m_Listener;
  int              m_NumberOfButtons;
  int              m_NumberOfColumns;
  int              m_ActiveButtonId;
  int              m_ButtonsType;
  std::vector<bool> m_CheckList;

  /** Update the movie ctrl interface. */
  void Update();
  void LoadFile(mafString filename);

DECLARE_EVENT_TABLE()
};
#endif
