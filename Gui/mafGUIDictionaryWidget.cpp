/*=========================================================================

 Program: MAF2
 Module: mafGUIDictionaryWidget
 Authors: Daniele Giunchi
 
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

#include "mafGUIDictionaryWidget.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/laywin.h>
#include <wx/listctrl.h>
#include <wx/dialog.h>

#include "mafDecl.h"
#include "mafEvent.h"
#include "mafTagArray.h"

#include "mafGUIListCtrl.h"
#include "mafGUIDictionaryWidget.h"

#include "mafVME.h"
#include "mafVMERoot.h"
#include "mafVMELandmarkCloud.h"

#include "mafXMLStorage.h"
#include "mafStorageElement.h"



enum DICTIONARY_WIDGET_ID
{
	LOAD_DICT = MINID,
};
//----------------------------------------------------------------------------
mafGUIDictionaryWidget::mafGUIDictionaryWidget(wxWindow *parent, int id)
//----------------------------------------------------------------------------
{  
  m_NumItem = 0;
  m_Items = NULL;
  m_Vme   = NULL;
  m_File = "";

  m_List = new mafGUIListCtrl(parent,id,false,true);
  m_List->Show(false);
  m_List->SetListener(this);
  m_List->SetSize(wxSize(450,400));

  if(m_File != "") LoadDictionary(m_File);
}
//----------------------------------------------------------------------------
mafGUIDictionaryWidget::~mafGUIDictionaryWidget()
//----------------------------------------------------------------------------
{
  for(int i=0; i<m_NumItem; i++) delete m_Items[i];
  delete [] m_Items; 
  cppDEL(m_List);
}
//----------------------------------------------------------------------------
void mafGUIDictionaryWidget::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
	if(mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
	  switch(e->GetId())
	  {
			case ITEM_SELECTED:
	      mafEventMacro(*e);
			break;
	  }
	}
}
//----------------------------------------------------------------------------
void mafGUIDictionaryWidget::LoadDictionary()
//----------------------------------------------------------------------------
{
  wxString wild_dict	= "Dictionary file (*.dic)|*.dic|All files (*.*)|*.*";
  wxString m_dict		  = mafGetApplicationDirectory().c_str();
  m_dict = m_dict + "\\Config\\Dictionary\\";
	wxString file       = mafGetOpenFile(m_dict,wild_dict,"Choose Dictionary File",m_List).c_str(); 
	if(file != "") LoadDictionary(file);
}
//----------------------------------------------------------------------------
void mafGUIDictionaryWidget::LoadDictionary(wxString file)
//----------------------------------------------------------------------------
{
  m_File = file;
  
  // XML storage to restore
  mafXMLStorage restore;
  restore.SetURL(m_File);
  restore.SetFileType("DIC");
  restore.SetVersion("2.0");

  // create a new object to restore into
  mafStorableDictionary *storeDict = new mafStorableDictionary;
  restore.SetDocument(storeDict);
  restore.Restore();
  
	if(m_NumItem) delete [] m_Items;

  m_NumItem = storeDict->m_StrVector.size();
  m_Items = new wxString*[m_NumItem];

  m_List->Reset();
  m_List->SetColumnLabel(0, "names already in use are displayed with the red icon");
  for(int i=0; i<m_NumItem; i++)
  {
     m_Items[i] = new wxString(storeDict->m_StrVector[i]);
     m_List->AddItem(i,*m_Items[i]);
  }
  
  storeDict->Delete();
  ValidateAllItem();
}
//----------------------------------------------------------------------------
void mafGUIDictionaryWidget::SetCloud(mafVME *vme)
//----------------------------------------------------------------------------
{
  m_Vme = vme;
	ValidateAllItem();
}
//----------------------------------------------------------------------------
// SIL - to be removed
void mafGUIDictionaryWidget::ValidateItem(wxString item, bool valid)
//----------------------------------------------------------------------------
{
  ITEM_ICONS icon = (valid) ? ITEM_GRAY : ITEM_RED;

  for(int i=0; i<m_NumItem; i++)
  {
    if ( *m_Items[i] == item )
    {
      m_List->SetItemIcon(i,icon);
      break;
    }
  }
}
//----------------------------------------------------------------------------
void mafGUIDictionaryWidget::ValidateAllItem(bool valid)
//----------------------------------------------------------------------------
{
  if(!m_Vme)
  {
		for(int i=0; i<m_NumItem; i++)
		{
      m_List->SetItemIcon(i,ITEM_GRAY);
		}
	}
  else
	{
    mafVMELandmarkCloud* lc = (mafVMELandmarkCloud*)m_Vme;
		for(int i=0; i<m_NumItem; i++)
		{
			ITEM_ICONS icon = (lc->FindInTreeByName(*m_Items[i])) ? ITEM_RED : ITEM_GRAY ;
			m_List->SetItemIcon(i,icon);
		}
	}
}

//------------------------------------------------------------------------------
mafCxxTypeMacro(mafStorableDictionary);
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
mafStorableDictionary::mafStorableDictionary()
//------------------------------------------------------------------------------
{
}
//------------------------------------------------------------------------------
mafStorableDictionary::~mafStorableDictionary()
//------------------------------------------------------------------------------
{
  m_StrVector.resize(0);
}
//------------------------------------------------------------------------------
// example of de-serialization code
int mafStorableDictionary::InternalRestore(mafStorageElement *element)
//------------------------------------------------------------------------------
{
  m_StrVector.resize(element->GetChildren()[0]->GetChildren().size());
  
  if(element->RestoreVectorN("Dictionary",m_StrVector,element->GetChildren()[0]->GetChildren().size(),"DItem"))
    return MAF_ERROR;
  return MAF_OK;
}
