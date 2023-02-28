/***************************************************************************
**
**  		     tRIBS Distributed Hydrologic Model
**
**              TIN-based Real-time Integrated Basin Simulator
**		         Ralph M. Parsons Laboratory
**  		    Massachusetts Institute of Technology
**
**
**  tReservoir.h: Header for class tReservoir (see tReservoir.cpp) which inherets
**              tFlowNet, tKinemat and implements Level Pool Routing.
**
***************************************************************************/

#ifndef TRESERVOIR_H
#define TRESERVOIR_H

//=========================================================================
//
//
//                  Section 1: tReservoir Include Statements
//
//
//=========================================================================


#include "tFlowNet/tResData.h"
#include "Headers/Inclusions.h"

using namespace std;

//=========================================================================
//
//
//                  Section 3: tReservoir Class Definitions
//
//
//=========================================================================

class tReservoir
{
 public:
  tReservoir();
  tReservoir(tInputFile &);
  ~tReservoir();
  void RunLevelPoolRouting(double);
  void ComputeInflow(double);
  void ComputeInitialSTQ();
  void ComputeSTQnext();
  void ComputeSTQ();
  void ComputeResQ();

  void SetResVariables(tInputFile &);
  void SetResNodes(tInputFile &);
  void readResNodeFile(char *);
  void readReservoirFile(char *);

  void setCurrResNode(int);
  int  getCurrResNode();

  void setCurrResType(int);
  int  getCurrResType();

  void setNReservoirs(int);
  int  getNReservoirs();

  void setResDischargeOut(double);
  double getResDischargeOut();

  void setResElevOut(double);
  double getResElevOut();

  void setModelTimeStep(double);
  double getModelTimeStep();

  int getNodetKinemat(int);
  int getTypetKinemat(int);

  void setResArraySize(int);
  int  getResArraySize();

protected:

  char resfile[kName];
  char resNodeFile[kName];

  tResData * reservoirNodes;
  tResData * reservoirTypes;
  int rType;
  int rNode;

  double resInflow; // Inflow = Ij + Ij+1
  double ResQinflow, ResQinflow2;

  // Values obtained through linear interpolation from data files:
  double STQ_0, initialH, initialS, initialQ;

  // Values with a "2" indicate they are from current time step
  // Values without a "2" are from previous time step:
  double resQ, resQ2, resH, resH2, resS, resS2, EDSdata, EDSdata2;

  double elevData;
  int interNum, interNum2;
  int lengthH; //Length of the data table provided for the reservoir
  double STQnext; // Storage-discharge function for the next time step
  double STQ, STQprev;
  double Q_0, H_0; // Outflow and elev from the Reservoir
  int RStep;
  int NumRes;
  double ResTimeDt;
  int ResTyp, ResNID, ArraySize;
  double ResOutflow, ResOutElev;

  int nReservoirs;

};

#endif // TRESERVOIR_H

//=========================================================================
//
//
//                          End tReservoir.h
//
//
//=========================================================================
