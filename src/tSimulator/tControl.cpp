/***************************************************************************
**
**  		     tRIBS Distributed Hydrologic Model
**
**              TIN-based Real-time Integrated Basin Simulator
**		         Ralph M. Parsons Laboratory
**  		    Massachusetts Institute of Technology
**  
**
**  tControl.cpp: Functions for class SimulationControl 
**              (see tControl.h)
**
***************************************************************************/

#include "src/tSimulator/tControl.h"
#include "src/Headers/globalIO.h"

#ifdef PARALLEL_TRIBS
#include "src/tParallel/tParallel.h"
#endif

//=========================================================================
//
//
//                  Section 1: SimulationControl Constructors/Destructors
//
//
//=========================================================================

SimulationControl::SimulationControl(int argc, char **argv)
{  
	Cout<<"\n-----------------------------------------------------------------"
	<<"-------";
	Cout<<"\n\n\t\t tRIBS Distributed Hydrologic Model";
	Cout<<"\n\t\t TIN-based Real-time Integrated Basin Simulator";
	Cout<<"\n\t\t Ralph M. Parsons Laboratory";
	Cout<<"\n\t\t Massachusetts Institute of Technology \n\n";
	Cout<<"-----------------------------------------------------------------"
		<<"-------"<<endl;
	
	char VERSION[50] = "3.0 Revision 33, June 2008";
	
	Cout <<"\n\ntRIBS Version "<< VERSION <<endl<<endl;
	
	static char usage[]=
		"Usage : %s [-R] [-G] [-A] [-V NodeID] [-O] [-K] [-W] [-M] [-H] [-F]\n";
	
	mode = STD_INPUT;       //Default: If file doesn't exist, assume zero rainfall
	fore_rain_label  = 'N';   
	GW_model_label   = 'N';   
	inter_results    = 'N';
	hydrog_results   = 'N';
	Verbose_label    = 'N';
	Check_label      = 'Y';    //Default is yes
	mod_is_on        = 'N';    //Single run is default
	hydro_visual     = 'N';
	Header_label     = 'Y';
	smooth_weather   = 'N';
   debug            = 'N';
	num_simul = 0;
	VerbID = -999;
	
#ifdef PARALLEL_TRIBS
  // Command line args are broadcast to all nodes
  tParallel::inputArgs(argc, argv);
#endif

	if( argc < 2 ){
		Cout<<"\n\nUsage: " << argv[0] <<" <input file>  [options]"<<endl<<endl;
		Cout<<"Options: "<<endl;
		Cout<<"\t-A    Automatic listing of rainfall files (zero if missing)"<<endl;
		Cout<<"\t-G    Run groundwater model"<<endl;
		Cout<<"\t-R    Write intermediate states (spatial output)"<<endl;
		Cout<<"\t-H    Write intermediate hydrographs (.mrf)"<<endl;
		Cout<<"\t-M    Do NOT Write headers in pixel/hydrograph/voronoi output files"<<endl;
		Cout<<"\t-F    Measured and forecasted rainfall"<<endl;
		Cout<<"\t-O    On after simulation completion, awaiting user's input"<<endl;
		Cout<<"\t-K    Check output"<<endl;
		Cout<<"\t-V [NodeID] Verbose mode (output run-time information)"<<endl<<endl;
		Cout<<"Provide name of an input file. Exiting program...\n"<<endl;
		exit(1);
	}
	
	infile = argv[1];  		//Name of input file 
	
	for (int i=2; i < argc; i++)  //Start from the 3rd argument 
    { 
		if (*argv[i] != '-'){ 
			if (i > 2) {
				if ((*(argv[i-1]+1)) == 'V') {
					sscanf(argv[i],"%d",&VerbID);
					cout<<"\nVerbose Node ID = "<<VerbID<<endl;
				}
				else {
					printf("Simulation control: Syntax incorrect %s\n", argv[i]);
					printf(usage, argv[0]);
					exit(1);
				}
			}
			else {
				printf("Simulation control: Syntax incorrect %s\n", argv[i]);
				printf(usage, argv[0]);
				exit(1);
			}
		}
		
		switch(*(argv[i]+1)){ 
			case 'A': 		//Automatic listing
			{ 
				mode=AUTO_INPUT;      // if file does not exist -> assumes '0'-s!
				break; 
			}
			case 'R': 		//Write intermediate results
			{ 
				inter_results = 'Y';
				break;
			}
			case 'F': 		//Measured and forecasted rainfall
			{ 
				fore_rain_label = 'Y';
				inter_results = 'Y';
				break;
			}
			case 'G': 		//Run Groundwater model
			{ 
				GW_model_label = 'Y';
				break;
			}
			case 'V':                 //Verbose Output
			{
				Verbose_label = 'Y';
				break;
			}
			case 'K':                 //Check Output
			{
				Check_label = 'N';
				break;
			}
			case 'O':                 //On, awaiting user input
			{ 
				mod_is_on = 'Y';
				break;
			}
			case 'W':                 //Hydrograph visualization (SGI only)
			{
				hydro_visual = 'Y';
				break;
			}
			case 'H':                 //Write intermediate hydrographs (.mrf)
			{
				hydrog_results = 'Y';
				break;
			}
			case 'M':                 //Headers in pix, hydrogr and voron output
			{
				Header_label = 'N';
				break;
			}
			case 'U':                 //Special option: no randomness in climate 
			{
				smooth_weather = 'Y';
				break;
			}
         case 'D':                        //For tGraph debugging output
         {
            debug = 'Y';
            break;
         }
      }
   }
}

SimulationControl::~SimulationControl() 
{  
	Cout<<"SimulationControl Object has been destroyed..."<<endl<<flush;
	
	Cout<<"\n-----------------------------------------------------------------"
		<<"-------";
	Cout<<"\n\n\t\t tRIBS Distributed Hydrologic Model";
	Cout<<"\n\t\t TIN-based Real-time Integrated Basin Simulator";
	Cout<<"\n\t\t Ralph M. Parsons Laboratory";
	Cout<<"\n\t\t Massachusetts Institute of Technology \n\n";
	Cout<<"-----------------------------------------------------------------"
		<<"-------"<<endl;
}

//=========================================================================
//
//
//                          End of tControl.cpp
//
//
//=========================================================================