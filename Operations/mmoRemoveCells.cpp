/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: mmoRemoveCells.cpp,v $
Language:  C++
Date:      $Date: 2007-03-21 11:36:07 $
Version:   $Revision: 1.4 $
Authors:   Stefano Perticoni
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

#include "mmoRemoveCells.h"
#include "wx/busyinfo.h"

#include "mafDecl.h"
#include "mmgGui.h"

#include "mmgDialog.h"

#include "mafRWIBase.h"
#include "mafRWI.h"
#include "mmdMouse.h"

#include "mmgButton.h"
#include "mmgValidator.h"
#include "mmgPicButton.h"
#include "mmgFloatSlider.h"

#include "mafVME.h"
#include "mafVMESurface.h"
#include "mafVMEOutput.h"
#include "mmiSelectCell.h"
#include "mmaVolumeMaterial.h"

#include "vtkLight.h"
#include "vtkCamera.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "mafVMEVolumeGray.h"

#include "vtkCell.h"
#include "vtkMath.h"
#include "vtkDataSet.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyData.h"
#include "vtkPlane.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkVolume.h"
#include "vtkRemoveCellsFilter.h"
#include "vtkMath.h"
#include "vtkIdList.h"
#include "vtkTriangle.h"

const int ID_REGION = 0;

//----------------------------------------------------------------------------
mafCxxTypeMacro(mmoRemoveCells);
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
mmoRemoveCells::mmoRemoveCells(wxString label) :
mafOp(label)
//----------------------------------------------------------------------------
{
  m_OpType  = OPTYPE_OP;
  m_Canundo = true;
  m_Dialog = NULL;
  m_Rwi = NULL;

  m_SelectCellInteractor  = NULL;

  m_rcf = NULL;

  m_Diameter = 20;
  m_MinBrushSize = 0;
  m_MaxBrushMSize = 100;

  m_Mesh = NULL;
  m_UnselectCells = 0;
  
  m_NeighborCellPointIds = NULL;
  m_InputPreserving = false;

  m_ResultPolydata	  = NULL;
  m_OriginalPolydata  = NULL;
}
//----------------------------------------------------------------------------
mmoRemoveCells::~mmoRemoveCells()
//----------------------------------------------------------------------------
{
  vtkDEL(m_Mesh);
  vtkDEL(m_ResultPolydata);
  vtkDEL(m_OriginalPolydata);
}
//----------------------------------------------------------------------------
mafOp* mmoRemoveCells::Copy()
//----------------------------------------------------------------------------
{
  /** return a copy of itself, needs to put it into the undo stack */
  return new mmoRemoveCells(m_Label);
}
//----------------------------------------------------------------------------
bool mmoRemoveCells::Accept(mafNode* vme)
//----------------------------------------------------------------------------
{
  return vme != NULL && vme->IsMAFType(mafVMESurface);
}
//----------------------------------------------------------------------------
void mmoRemoveCells::OpRun()
//----------------------------------------------------------------------------
{
  
  vtkNEW(m_ResultPolydata);
  m_ResultPolydata->DeepCopy((vtkPolyData*)((mafVME *)m_Input)->GetOutput()->GetVTKData());

  vtkNEW(m_OriginalPolydata);
  m_OriginalPolydata->DeepCopy((vtkPolyData*)((mafVME *)m_Input)->GetOutput()->GetVTKData());

  int result = OP_RUN_CANCEL;

  CreateSurfacePipeline();
  InitializeMesh();

  if (m_TestMode == false)
  {
    CreateOpDialog();

    int ret_dlg = m_Dialog->ShowModal();
    if( ret_dlg == wxID_OK )
    {
      result = OP_RUN_OK;

      RemoveCells();
    }
    else 
    {
      result = OP_RUN_CANCEL;
    }

    DeleteOpDialog();

    mafEventMacro(mafEvent(this,result));
  }
}
//----------------------------------------------------------------------------
void mmoRemoveCells::OpDo()
//----------------------------------------------------------------------------
{
  ((mafVMESurface *)m_Input)->SetData(m_ResultPolydata,((mafVME *)m_Input)->GetTimeStamp());
  mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}
//----------------------------------------------------------------------------
void mmoRemoveCells::OpUndo()
//----------------------------------------------------------------------------
{
  ((mafVMESurface *)m_Input)->SetData(m_OriginalPolydata,((mafVME *)m_Input)->GetTimeStamp());
  mafEventMacro(mafEvent(this, CAMERA_UPDATE));
}
//----------------------------------------------------------------------------
// widget ID's
//----------------------------------------------------------------------------
enum EXTRACT_ISOSURFACE_ID
{
  ID_FIT = MINID,
  ID_DIAMETER,
  ID_DELETE,
  ID_OK,
  ID_UNSELECT,
  ID_CANCEL,
};
//----------------------------------------------------------------------------
void mmoRemoveCells::CreateOpDialog()
//----------------------------------------------------------------------------
{
  wxBusyCursor wait;

  //===== setup interface ====
  m_Dialog = new mmgDialog("Remove Cells", mafCLOSEWINDOW | mafRESIZABLE);
  
  m_Rwi = new mafRWI(m_Dialog,ONE_LAYER,false);
  m_Rwi->SetListener(this);
  m_Rwi->CameraSet(CAMERA_PERSPECTIVE);

  m_Rwi->m_RenderWindow->SetDesiredUpdateRate(0.0001f);
  m_Rwi->SetSize(0,0,800,800);
  m_Rwi->Show(true);
  m_Rwi->m_RwiBase->SetMouse(m_Mouse);

  m_Rwi->m_RenFront->AddActor(m_PolydataActor);

  vtkPolyData *polydata = vtkPolyData::SafeDownCast(((mafVME *)m_Input)->GetOutput()->GetVTKData());
 
  double bounds[6] = {0,0,0,0,0,0};
  polydata->GetBounds(bounds);

  m_Rwi->m_RenFront->ResetCamera(polydata->GetBounds());
  m_Rwi->m_RenFront->ResetCameraClippingRange(bounds);

  mafNEW(m_SelectCellInteractor);

  m_SelectCellInteractor->SetListener(this);
  m_Mouse->AddObserver(m_SelectCellInteractor, MCH_INPUT);
  
  wxPoint p = wxDefaultPosition;

  wxStaticText *brushSize  = new wxStaticText(m_Dialog,-1, "brush size: ");
  wxStaticText *foo  = new wxStaticText(m_Dialog,-1, " ");
  wxTextCtrl   *diameter = new wxTextCtrl  (m_Dialog,ID_DIAMETER, "",								 		p,wxSize(50, 16 ), wxNO_BORDER );
  wxCheckBox *unselect =   new wxCheckBox(m_Dialog, ID_DELETE,         "unselect", p, wxSize(80,20));

  wxStaticText *help  = new wxStaticText(m_Dialog,-1, "Use CTRL to select cells");

  mmgButton  *unselectAllButton =    new mmgButton(m_Dialog, ID_UNSELECT,    "unselect all", p,wxSize(80,20));
  mmgButton  *b_fit =    new mmgButton(m_Dialog, ID_FIT,    "reset camera", p,wxSize(80,20));
  mmgButton  *ok =     new mmgButton(m_Dialog, ID_OK,     "ok", p, wxSize(80,20));
  mmgButton  *cancel = new mmgButton(m_Dialog, ID_CANCEL, "cancel", p, wxSize(80,20));

  diameter->SetValidator(mmgValidator(this,ID_DIAMETER,diameter,&m_Diameter,m_MinBrushSize,m_MaxBrushMSize));
  unselect->SetValidator(mmgValidator(this, ID_DELETE, unselect, &m_UnselectCells));

  unselectAllButton->SetValidator(mmgValidator(this,ID_UNSELECT,unselectAllButton));
  b_fit->SetValidator(mmgValidator(this,ID_FIT,b_fit));
  ok->SetValidator(mmgValidator(this,ID_OK,ok));
  cancel->SetValidator(mmgValidator(this,ID_CANCEL,cancel));

  wxBoxSizer *h_sizer0 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer0->Add(help,     0,wxRIGHT);	

  wxBoxSizer *h_sizer1 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer1->Add(brushSize,     0,wxRIGHT);	
  h_sizer1->Add(diameter,     0,wxRIGHT);	
  h_sizer1->Add(foo,     0,wxRIGHT);	
  h_sizer1->Add(unselect,     0,wxRIGHT);	

  wxBoxSizer *h_sizer2 = new wxBoxSizer(wxHORIZONTAL);
  h_sizer2->Add(unselectAllButton,     0,wxRIGHT);	
  h_sizer2->Add(b_fit,     0,wxRIGHT);	
  h_sizer2->Add(ok,      0,wxRIGHT);
  h_sizer2->Add(cancel,  0,wxRIGHT);
 
  wxBoxSizer *v_sizer =  new wxBoxSizer( wxVERTICAL );
  v_sizer->Add(m_Rwi->m_RwiBase, 1,wxEXPAND);
  v_sizer->Add(h_sizer0,     0,wxEXPAND | wxALL,5);
  v_sizer->Add(h_sizer1,     0,wxEXPAND | wxALL,5);
  v_sizer->Add(h_sizer2,     0,wxEXPAND | wxALL,5);

  m_Dialog->Add(v_sizer, 1, wxEXPAND);

  int x_pos,y_pos,w,h;
  mafGetFrame()->GetPosition(&x_pos,&y_pos);
  m_Dialog->GetSize(&w,&h);
  m_Dialog->SetSize(x_pos+5,y_pos+5,w,h);

  m_Rwi->CameraUpdate();
}
//----------------------------------------------------------------------------
void mmoRemoveCells::CreateSurfacePipeline()
//----------------------------------------------------------------------------
{
  vtkPolyData *polydata = vtkPolyData::SafeDownCast(((mafVME *)m_Input)->GetOutput()->GetVTKData());

  m_rcf = vtkRemoveCellsFilter::New();
  m_rcf->SetInput(polydata);
  m_rcf->Update();

  m_PolydataMapper	= vtkPolyDataMapper::New();
  m_PolydataMapper->SetInput(m_rcf->GetOutput());
  m_PolydataMapper->ScalarVisibilityOn();

  m_PolydataActor = vtkActor::New();
  m_PolydataActor->SetMapper(m_PolydataMapper);
  
}
//----------------------------------------------------------------------------
void mmoRemoveCells::DeleteOpDialog()
//----------------------------------------------------------------------------
{
  m_Mouse->RemoveObserver(m_SelectCellInteractor);

  mafDEL(m_SelectCellInteractor);

  vtkDEL(m_PolydataMapper);
  vtkDEL(m_PolydataActor);
  vtkDEL(m_rcf);

  cppDEL(m_Rwi); 
  cppDEL(m_Dialog);
}


//----------------------------------------------------------------------------
void mmoRemoveCells::OnEvent(mafEventBase *maf_event)
//----------------------------------------------------------------------------
{
  if (mafEvent *e = mafEvent::SafeDownCast(maf_event))
  {
    switch(e->GetId())
    {	
      case ID_OK:
        m_Dialog->EndModal(wxID_OK);
      break;
   
      case ID_CANCEL:
        m_Dialog->EndModal(wxID_CANCEL);
      break;
   
      case ID_UNSELECT:
        m_rcf->UndoMarks();
        m_Rwi->m_RenderWindow->Render();
      break;

      case ID_FIT:
      {
        vtkPolyData *polydata = vtkPolyData::SafeDownCast(((mafVME *)m_Input)->GetOutput()->GetVTKData());

        double bounds[6] = {0,0,0,0,0,0};
        polydata->GetBounds(bounds);

        m_Rwi->m_RenFront->ResetCamera(polydata->GetBounds());
        m_Rwi->m_RenFront->ResetCameraClippingRange(bounds);

        m_Rwi->m_RenderWindow->Render();
      }
      break;

      case VME_PICKED:
      {
        double pos[3];
        vtkPoints *pts = NULL; 
        pts = (vtkPoints *)e->GetVtkObj();
        pts->GetPoint(0,pos);
        
        // get the picked cell and mark it as selected
        int cellID = e->GetArg();

        if (cellID == m_CellSeed)
        {
          return;
        }

        SetSeed(cellID);

        // select circle region by pick
        MarkCellsInRadius(m_Diameter/2);
        
        m_Rwi->m_RenderWindow->Render();
      }
      break;
      
      case ID_DELETE:
      {

      }     
      break ;

      default:
        mafEventMacro(*e);
      break; 
    }
  }
}

void mmoRemoveCells::TraverseMeshAndMark( double radius )
{
  
  vtkIdType cellId, ptId, numIds, idCellInWave;
  int idPoint, k;
  vtkIdType *cellPointsList, *cellsFromPoint, numCellPoints;
  vtkIdList *tmpWave;
  unsigned short ncells = 0;

  double seedCenter[3] = {0,0,0};
  FindTriangleCellCenter(m_CellSeed, seedCenter);

  while ( (numIds=m_Wave->GetNumberOfIds()) > 0 )
  {
    // for all the cells in the wave
    for ( idCellInWave=0; idCellInWave < numIds; idCellInWave++ )
    {
      cellId = m_Wave->GetId(idCellInWave);
      if ( m_VisitedCells[cellId] < 0 )
      {
        // mark it as visited
        m_VisitedCells[cellId] = m_RegionNumber;
        
        double currentCellCenter[3] = {0,0,0};
        FindTriangleCellCenter(cellId, currentCellCenter);

        // mark the cell if the distance criterion is ok
        if (vtkMath::Distance2BetweenPoints(seedCenter, currentCellCenter)
          < (m_Diameter*m_Diameter / 4))
        {
          m_UnselectCells ? m_rcf->UnmarkCell(cellId) : m_rcf->MarkCell(cellId);
        }

        // get its points
        m_Mesh->GetCellPoints(cellId, numCellPoints, cellPointsList);

        // for each cell point
        for (idPoint=0; idPoint < numCellPoints; idPoint++) 
        {
          // if the point has not been yet visited
          if ( m_VisitedPoints[ptId=cellPointsList[idPoint]] < 0 )
          {
            // mark it as visited
            m_VisitedPoints[ptId] = m_PointNumber++;
          }

          // get neighbor cells from cell point
          m_Mesh->GetPointCells(ptId,ncells,cellsFromPoint);

          // check connectivity criterion (geometric + distance)
          for (k=0; k < ncells; k++)
          {
            cellId = cellsFromPoint[k];
            
            FindTriangleCellCenter(cellId,currentCellCenter);
            if (vtkMath::Distance2BetweenPoints(seedCenter, currentCellCenter)
              < (m_Diameter*m_Diameter / 4))
            {
              // insert next cells to be visited in the other wave
              m_Wave2->InsertNextId(cellId);
            }
          }//for all cells using this point
        }//for all points of this cell
      }//if cell not yet visited
    }//for all cells in this wave

    tmpWave = m_Wave;
    m_Wave = m_Wave2;
    m_Wave2 = tmpWave;
    tmpWave->Reset();
  } //while wave is not empty
}

void mmoRemoveCells::MarkCellsInRadius(double radius){

  vtkNEW(m_NeighborCellPointIds);

  m_NeighborCellPointIds->Allocate(3);

  vtkIdType i;
  vtkIdType numPts, numCells;
  vtkPoints *inPts;
  vtkPolyData *polydata = vtkPolyData::SafeDownCast(((mafVME *)m_Input)->GetOutput()->GetVTKData());

  //  Check input/allocate storage
  //
  inPts = polydata->GetPoints();

  if (inPts == NULL)
  {
    mafLogMessage("No points!");
    return;
  }

  numPts = inPts->GetNumberOfPoints();
  numCells = polydata->GetNumberOfCells();

  if ( numPts < 1 || numCells < 1 )
  {
    mafLogMessage("No data to connect!");
    return;
  }
 
  // Initialize.  Keep track of points and cells visited.
  //

  // heuristic
  int maxVisitedCells = numCells;
  m_VisitedCells = new int[maxVisitedCells];
  for ( i=0; i < maxVisitedCells; i++ )
  {
    m_VisitedCells[i] = -1;
  }

  // heuristic
  int maxVisitedPoints = numPts;
  m_VisitedPoints = new vtkIdType[maxVisitedPoints];  
  for ( i=0; i < maxVisitedPoints; i++ )
  {
    m_VisitedPoints[i] = -1;
  }

  // Traverse all cells marking those visited. Connected region grows 
  // using a connected wave propagation.

  vtkNEW(m_Wave);
  m_Wave->Allocate(numPts);

  vtkNEW(m_Wave2);
  m_Wave2->Allocate(numPts);

  m_PointNumber = 0;

  // only one region with ID 0
  m_RegionNumber = ID_REGION;
   
  m_Wave->InsertNextId(m_CellSeed);
 
  //mark the seeded region
  TraverseMeshAndMark(m_Diameter/2);
   
  m_Wave->Reset();
  m_Wave2->Reset();

  delete [] m_VisitedCells;
  delete [] m_VisitedPoints;
  
  vtkDEL(m_NeighborCellPointIds);
  vtkDEL(m_Wave);
  vtkDEL(m_Wave2);
}
  

void mmoRemoveCells::SetSeed( vtkIdType cellSeed )
{
	m_CellSeed = cellSeed;
}

void mmoRemoveCells::FindTriangleCellCenter(vtkIdType id, double center[3])
{
  double p0[3] = {0,0,0};
  double p1[3] = {0,0,0};
  double p2[3] = {0,0,0};

  assert(m_Mesh->GetCell(id)->GetNumberOfPoints() == 3);

  vtkIdList *list = vtkIdList::New();
  list->SetNumberOfIds(3);

  m_Mesh->GetCellPoints(id, list);

  m_Mesh->GetPoint(list->GetId(0),p0);
  m_Mesh->GetPoint(list->GetId(1),p1);
  m_Mesh->GetPoint(list->GetId(2),p2);

  vtkTriangle::TriangleCenter(p0,p1,p2,center);

  list->Delete();
}

void mmoRemoveCells::InitializeMesh()
{
  // Build cell structure
  //
  vtkNEW(m_Mesh);
  vtkPolyData *polydata = vtkPolyData::SafeDownCast(((mafVME *)m_Input)->GetOutput()->GetVTKData());
  assert(polydata);

  this->m_Mesh->CopyStructure(polydata);
  this->m_Mesh->BuildLinks();
}

void mmoRemoveCells::RemoveCells()
{
  	// perform cells removing...
    if (m_TestMode == false)
    {
      wxBusyInfo("removing cells...");
    }

    m_rcf->RemoveMarkedCells();
    m_rcf->Update();

    m_ResultPolydata->DeepCopy(m_rcf->GetOutput());
}

void mmoRemoveCells::MarkCells()
{
  MarkCellsInRadius(m_Diameter/2);  
}

double mmoRemoveCells::EstimateTrianglesDimension()
{
  vtkPolyData *polydata = vtkPolyData::SafeDownCast(((mafVME *)m_Input)->GetOutput()->GetVTKData());
  
  int numTriangles = polydata->GetNumberOfCells();

  int probesNumber =  numTriangles > 100 ? 100 : numTriangles;

  double accumulator = 0;

  for (int i = 0; i < numTriangles; i++)
	{
    double length =  sqrt(polydata->GetCell(i)->GetLength2());
    accumulator += length ;
	}
  
  return (accumulator/probesNumber);

}