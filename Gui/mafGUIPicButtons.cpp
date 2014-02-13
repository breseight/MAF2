/*=========================================================================
  Program:   Multimod Application Framework
  Module:    $RCSfile: mafGUIPicButtons.cpp,v $
  Language:  C++
  Date:      $Date: 2009-05-14 12:27:49 $
  Version:   $Revision: 1.4.2.2 $
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

#include "mafGUIPicButtons.h"  
#include "mafDecl.h"
#include "mafPics.h"
#include "mafGUIPicButton.h"
#include "mafGUIValidator.h"

#include <fstream>
#include <sstream>
#include <iterator>


//----------------------------------------------------------------------------
// mafGUIPicButtons
//----------------------------------------------------------------------------
BEGIN_EVENT_TABLE(mafGUIPicButtons,mafGUIPanel)
  
END_EVENT_TABLE()
//----------------------------------------------------------------------------
#define mafGUIPicButtonsStyle wxNO_BORDER | wxWANTS_CHARS | wxTAB_TRAVERSAL

enum MOVIECTRL_WIDGETS_ID
{
  
};
//----------------------------------------------------------------------------
mafGUIPicButtons::mafGUIPicButtons( wxWindow* parent,wxWindowID id, int numberOfButtons, int numberOfColumns)
:mafGUIPanel(parent,id,wxDefaultPosition,wxDefaultSize,mafGUIPicButtonsStyle)       
//----------------------------------------------------------------------------
{
  m_Sizer = NULL;
  m_Listener = NULL;
  m_NumberOfButtons = numberOfButtons;
  m_NumberOfColumns = numberOfColumns;
  m_ActiveButtonId = -1;
  m_ButtonsType = ID_RADIO_BUTTON;
}
//----------------------------------------------------------------------------
mafGUIPicButtons::mafGUIPicButtons(wxWindow* parent,wxWindowID id, mafString filename, int numberOfColumns)
	:mafGUIPanel(parent,id,wxDefaultPosition,wxDefaultSize,mafGUIPicButtonsStyle)       
//----------------------------------------------------------------------------
{
	m_Sizer = NULL;
	m_Listener = NULL;

	// calculate from file
	LoadFile(filename);
	m_NumberOfButtons = m_Pictures.size(); 
	m_NumberOfColumns = numberOfColumns;
	m_ActiveButtonId = -1;
	m_ButtonsType = ID_RADIO_BUTTON;
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::LoadFile(mafString filename)   
//----------------------------------------------------------------------------
{
	m_NumberOfButtons = 0;
	m_Pictures.clear();
	m_ButtonsInfo.clear();

	// load file from here
	std::string line;
	ifstream myfile (filename.GetCStr());
	if (myfile.is_open())
	{
		while ( myfile.good() )
		{
			std::getline (myfile,line);

			std::istringstream buf(line);
			std::istream_iterator<std::string> beg(buf), end;
			std::vector<std::string> tokens(beg, end);

			int h=0 , size = tokens.size();
			std::vector<wxString> tokens2;
			for(;h<size;++h) {
				tokens2.push_back(tokens.at(h).c_str());
			}

			m_Pictures.push_back(tokens[0].c_str());
			m_ButtonsInfo.push_back(tokens2);
			
		}
		myfile.close();
	} else {
		mafLogMessage("File not found");
	}
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::GetInfoButton(int index, std::vector< std::string > &infos)
//----------------------------------------------------------------------------
{
	infos.clear();
	infos.resize(m_ButtonsInfo.size());
	std::copy(m_ButtonsInfo[index].begin(), m_ButtonsInfo[index].end(), infos.begin());
}
//----------------------------------------------------------------------------
mafGUIPicButtons::~mafGUIPicButtons()
//----------------------------------------------------------------------------
{
  
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::Create()
//----------------------------------------------------------------------------
{
  assert(m_Pictures.size() != 0);
  int rows = m_NumberOfButtons / m_NumberOfColumns;

  assert(m_NumberOfButtons != 0 && rows != 0);

  m_Sizer =  new wxFlexGridSizer( rows, m_NumberOfColumns, 2, 2 );

  for(int i=0; i<m_NumberOfButtons;i++)
  {
    m_PicButtons.push_back(new mafGUIPicButton(this, m_Pictures[m_PicButtons.size()].c_str(), FIRST_BUTTON_ID + m_PicButtons.size(), this,7));
	m_CheckList.push_back(false);
  }

  for(int i = 0; i < m_PicButtons.size(); i++)
  {
    m_Sizer->Add(m_PicButtons[i],0,0);
  }

  this->SetAutoLayout( TRUE );
  this->SetSizer( m_Sizer );
  m_Sizer->Fit(this);
  m_Sizer->SetSizeHints(this);
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {

	int index = -1;
	if(m_ButtonsType == ID_RADIO_BUTTON) {
		for(int i = 0; i < m_PicButtons.size(); i++)
		{
		  if(m_PicButtons[i]->GetId() == e->GetId())
		  {
			ActivateButton(i, true);
			index = i;
		  }
		  else
		  {
			ActivateButton(i, false);
		  }
		}
	} else if(m_ButtonsType == ID_BUTTON) {
		for(int i = 0; i < m_PicButtons.size(); i++)
		{
		    if(m_PicButtons[i]->GetId() == e->GetId())
			{
				index = i;
			}
		}
	} else if (m_ButtonsType == ID_CHECK_BUTTON) {
		for(int i = 0; i < m_PicButtons.size(); i++)
		{
			if(m_PicButtons[i]->GetId() == e->GetId())
			{
				if(m_CheckList[i] == true) {
					ActivateButton(i, false);
				} else {
					ActivateButton(i, true);
				}
				index = i;
			}
		}
	}

    e->SetSender(this);
    e->SetArg(index);
    e->SetId(GetId());
    mafEventMacro(*e);
  }
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::Update()
//----------------------------------------------------------------------------
{
  for(int i = 0; i < m_PicButtons.size(); i++)
  {
    m_PicButtons[i]->SetEventId(i + FIRST_BUTTON_ID);
  }
  TransferDataToWindow();
  mafYield();

  this->FitInside();
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::ActivateButton(int index, bool activated)
//----------------------------------------------------------------------------
{
  if(activated)
  {
    //if(index == m_ActiveButtonId) return;
    mafString activePicture;
    activePicture << m_Pictures[index].c_str();
    activePicture << "_";
    activePicture << "ACTIVE";
    m_PicButtons[index]->SetBitmap(activePicture.GetCStr());
    m_ActiveButtonId = index;
  }
  else
  {
    m_PicButtons[index]->SetBitmap(m_Pictures[index].c_str());
  }
  m_CheckList[index] = activated;

  Update();
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::DeActivateAllButtons()
//----------------------------------------------------------------------------
{
  int i = 0;
  for(; i < m_NumberOfButtons; i++)
  {
    this->ActivateButton(i,false);
  }
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::EnableButton(int index, bool enable)
	//----------------------------------------------------------------------------
{
	if(!enable)
	{
		mafString enablePicture;
		enablePicture << m_Pictures[index].c_str();
		enablePicture << "_";
		enablePicture << "DISABLE";
		m_PicButtons[index]->SetBitmap(enablePicture.GetCStr());
	}
	else
	{
		m_PicButtons[index]->SetBitmap(m_Pictures[index].c_str());
	}
	m_PicButtons[index]->Enable(enable); // deactive completely the button clicking
	Update();
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::DisableAllButtons()
	//----------------------------------------------------------------------------
{
	int i = 0;
	for(; i < m_NumberOfButtons; i++)
	{
		this->EnableButton(i,false);
	}
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::SetPictureVector(std::vector<wxString> &pictures)
//----------------------------------------------------------------------------
{
  assert(pictures.size() == m_NumberOfButtons);

  m_Pictures.clear();
  for(int i = 0; i< pictures.size();i++)
  {
    m_Pictures.push_back(pictures[i]);
  }
}
//----------------------------------------------------------------------------
void mafGUIPicButtons::SetToolTip(int index, const char* toolTip)
//----------------------------------------------------------------------------
{
  if(m_PicButtons[index])
  {
    m_PicButtons[index]->SetToolTip(toolTip);
    m_PicButtons[index]->Update();
  }
}
/*/----------------------------------------------------------------------------
void mafGUIPicButtons::ShowButton(int index, bool enable)
//----------------------------------------------------------------------------
{
  if(m_PicButtons[index])
  {
    m_PicButtons[index]->Show(enable);
    m_PicButtons[index]->Update();
  }
  this->SetAutoLayout( TRUE );
  this->SetSizer( m_Sizer );
  m_Sizer->Fit(this);
  m_Sizer->SetSizeHints(this);
}*/