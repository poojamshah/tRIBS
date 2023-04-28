/***************************************************************************
**
**  		     tRIBS Distributed Hydrologic Model
**
**              TIN-based Real-time Integrated Basin Simulator
**		         Ralph M. Parsons Laboratory
**  		    Massachusetts Institute of Technology
**  
**
**  tInvariant.cpp:   Functions for tInvariant classes (see tInvariant.h)
**
**
***************************************************************************/

#include "src/Headers/globalIO.h"
#include "src/tRasTin/tInvariant.h"

//=========================================================================
//
//
//                  Section 1: tInvariant GenericSoilData Functions
//
//
//=========================================================================

/***************************************************************************
**
**  GenericSoilData(tInputFile *)
**
**  Constructor and Destructor for GenericSoilData Class
**
***************************************************************************/
GenericSoilData::GenericSoilData(tMesh<tCNode> *mesh, 
								 tInputFile *input, tResample *resamp)
{  
	int np, id;
	double *sc;
    
    // Changes added by Giuseppe in August 2016 to allow reading soil grids
    int stdgrid_opt = 100; // Assign a random value
    stdgrid_opt = input->ReadItem(stdgrid_opt, "OPTSOILTYPE" );
  		
	// The last one calls tResample functions
	input->ReadItem(soilTable, "SOILTABLENAME"); // input table
	input->ReadItem(soilGrid,  "SOILMAPNAME");   // reads input file
	double *tmp = resamp->doIt(soilGrid, 2);     // resamples grid
	
	tCNode *cn;
	tMeshListIter <tCNode> niter ( mesh->getNodeList() );
	id = 0;
	for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
		cn->setSoilID((int)tmp[id]);  //sets soil ID to tCNode 
		id++;
	}
	
	ifstream Inp0(soilTable);
	if (!Inp0) {
		cout <<"\nFile "<<soilTable<<" not found!"<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(2);
	}
	
	// Reads # of soil classes and # parameters
	Inp0 >> numClass >> np; 
	
	assert(numClass > 0 && np > 0);
	SoilClass = new SoilType* [numClass];
	assert(SoilClass != 0);
	
	sc = new double [np];
	assert(sc != 0);
	
	// Reads in the soil parameters 
	for (int i=0; i < numClass; i++) {
		for (int j=0; j < np; j++)
			Inp0 >> sc[j];
		SoilClass[i] = new SoilType(sc, np);
		assert(SoilClass[i] != 0);
	}
	
	Inp0.close();
	delete [] sc;
    
    // Giuseppe 2016 - Assign the soil properties to the nodes via the set functions
    id = 0;
    for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
        setSoilPtr( cn->getSoilID() );
        
        cn->setKs(getSoilProp(1)); // Surface hydraulic conductivity
        cn->setThetaS(getSoilProp(2)); // Saturation moisture content
        cn->setThetaR(getSoilProp(3)); // Residual moisture content
        cn->setPoreSize(getSoilProp(4)); // Pore-size distribution index
        cn->setAirEBubPres(getSoilProp(5)); // Air entry bubbling pressure
        cn->setDecayF(getSoilProp(6)); // Decay parameter in the exp
        cn->setSatAnRatio(getSoilProp(7)); // Anisotropy ratio (saturated)
        cn->setUnsatAnRatio(getSoilProp(8)); // Anisotropy ratio (unsaturated)
        cn->setPorosity(getSoilProp(9)); // Porosity
        cn->setVolHeatCond(getSoilProp(10)); // Volumetric Heat Conductivity
        cn->setSoilHeatCap(getSoilProp(11)); // Soil Heat Capacity
        
        id++;
    }
    
    //}                                 // Comment out later Giuseppe 2016
    //else if (stdgrid_opt == 1)        // Comment out later Giuseppe 2016
    if (stdgrid_opt == 1)
    {
        
        int sID_array = 0;
        for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
            sID_array++;
        }
        int Asize = sID_array;
        
        // Read the path to the soil grids from the gdf file
        input->ReadItem(scfile, "SCGRID");
        
        int sID = 0;
        int numParameters;
        double SCgridlat;
        double SCgridlong;
        double SCgridgmt;
        
        cout<<"\nReading Soil Cover Data Grid File: ";
        cout<< scfile<<"..."<<endl<<flush;
        
        ifstream readFile(scfile);
        if (!readFile) {
            cout << "\nFile "<<scfile<<" not found!" << endl;
            cout << "Exiting Program...\n\n"<<endl;
            exit(1);}
        
        readFile >> numParameters; //Reads first line of the *.gdf file (11 parameters)
        
        //Read default parameters from the second line of the *.gdf file
        readFile >> SCgridlat;
        readFile >> SCgridlong;
        readFile >> SCgridgmt;
        
        // Initialize variable with the names of the soil parameters and corresponding files
        SCgridBaseNames = new char*[numParameters];
        SCgridParamNames = new char*[numParameters];
        SCgridExtNames = new char*[numParameters];
        SCgridName = new char*[numParameters];
        
        for (int ct=0;ct<numParameters;ct++) {
            
            SCgridParamNames[ct] = new char[10];// Giuseppe: This a char, while previously it was defined as pointer!!!
            SCgridBaseNames[ct] = new char[kName];
            SCgridExtNames[ct] = new char[10];
            SCgridName[ct] = new char[100];
            readFile >> SCgridParamNames[ct];
            
            if ( (strcmp(SCgridParamNames[ct],"KS")!=0) &&
                (strcmp(SCgridParamNames[ct],"TS")!=0) &&
                (strcmp(SCgridParamNames[ct],"TR")!=0) &&
                (strcmp(SCgridParamNames[ct],"PI")!=0) &&
                (strcmp(SCgridParamNames[ct],"PB")!=0) &&
                (strcmp(SCgridParamNames[ct],"FD")!=0) &&
                (strcmp(SCgridParamNames[ct],"AR")!=0) &&
                (strcmp(SCgridParamNames[ct],"UA")!=0) &&
                (strcmp(SCgridParamNames[ct],"PO")!=0) &&
                (strcmp(SCgridParamNames[ct],"VH")!=0) &&
                (strcmp(SCgridParamNames[ct],"SH")!=0) ) {
                
                cout << "\nA soil cover parameter name in the SC gdf file is an unexpected one."<<endl;
                cout << "\nExpected variables: KS,TS,TR,PI,PB,FD,AR,UA,PO,VH or SH" << endl;
                cout << "\tCheck and re-run the program" << endl;
                cout << "\nExiting Program..."<<endl<<endl;
                exit(1);
                
            }
            
            readFile >> SCgridBaseNames[ct];
            
            if (strcmp(SCgridBaseNames[ct],"NO_DATA")==0) {
                Cout << "\nCannot use NO_DATA for SC Grids"<<endl;
                Cout << "\nExiting Program..."<<endl<<endl;
                exit(1);
            }
            readFile >> SCgridExtNames[ct];
            
            //char result[100];   // array to hold the result.
            
            strcpy(SCgridName[ct],SCgridBaseNames[ct]); // copy the first string into SCgridName[ct]
            strcat(SCgridName[ct],"."); // append "."
            strcat(SCgridName[ct],SCgridExtNames[ct]); // append SCgridExtNames[ct]
            
            if (strcmp(SCgridParamNames[ct],"KS")==0)
            {
                // Resample Ks grid
                cout << "\nResampling Ks grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setKs(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"TS")==0)
            {
                // Resample ThetaS grid
                cout << "\nResampling ThetaS grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setThetaS(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"TR")==0)
            {
                // Resample ThetaR grid
                cout << "\nResampling ThetaR grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setThetaR(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"PI")==0)
            {
                // Resample Pore Index grid
                cout << "\nResampling Pore Index grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setPoreSize(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"PB")==0)
            {
                // Resample Air E. Bubbling P grid
                cout << "\nResampling Air Entry Bubbling Pressure grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setAirEBubPres(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"FD")==0)
            {
                // Resample Decay grid
                cout << "\nResampling Decay Exponent grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setDecayF(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"AR")==0)
            {
                // Resample Decay grid
                cout << "\nResampling Saturated Anisotropy Ratio grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setSatAnRatio(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"UA")==0)
            {
                // Resample Decay grid
                cout << "\nResampling Unsaturated Anisotropy Ratio grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setUnsatAnRatio(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"PO")==0)
            {
                // Resample Porosity grid
                cout << "\nResampling Porosity grid..." << endl;;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setPorosity(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"VH")==0)
            {
                // Resample Volumetric Heat grid
                cout << "\nResampling Volumetric Heat Conductivity grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setVolHeatCond(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
            if (strcmp(SCgridParamNames[ct],"SH")==0) 
            {
                // Resample Soil Heat grid
                cout << "\nResampling Soil Heat Capacity grid..." << endl;
                double *tmp = resamp->doIt(SCgridName[ct], 1);     // resamples grid // Bug fix to replace hardcoded indexing CJC 2022
                tCNode *cn;
                tMeshListIter <tCNode> niter ( mesh->getNodeList() );
                id = 0;
                for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP()) {
                    cn->setSoilHeatCap(tmp[id]);  //sets soil ID to tCNode
                    id++;
                }
            }
            
        }
        
    }
    
    
}

GenericSoilData::~GenericSoilData() {   
	for (int iprop=0;iprop < numClass;iprop++)
		delete SoilClass[iprop];
	delete [] SoilClass;
}

/***************************************************************************
**
**  GenericSoilData:: Get and Set Soil Ptr
**
**  SetSoilParameters: To set (reset) soil parameter values
**
***************************************************************************/
void GenericSoilData::SetSoilParameters(tMesh<tCNode> *mesh,
										tResample *resamp, tInputFile &infile, int option)
{
	int np, nt;
	int id;
	double troyan;
	
	if ( option ) {     // Needs a new soil resampling 
		Cout<<"\nResampling Soils......"<<endl<<flush;
		infile.ReadItem(soilGrid,  "SOILMAPNAME"); //Reads input file
		double *tmp = resamp->doIt(soilGrid, 2);   //Resamples grid
		
		tCNode *cn;
		tMeshListIter <tCNode> niter ( mesh->getNodeList() );
		id = 0;
		for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP())  {
			cn->setSoilID((int)tmp[id]);  //Sets soil ID to tCNode 
			id++;
		}
	}
	
	infile.ReadItem(soilTable, "SOILTABLENAME"); // input table
	ifstream Inp0(soilTable);
	if (!Inp0) {
		cout <<"File "<<soilTable<<" not found!"<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(2);
	}
	
	Inp0>>nt>>np;           // Reads # of soil classes and # parameters
	
	if ( option ) {         // Needs a new soil resampling 
		for (int iprop=0;iprop < numClass;iprop++)
			delete SoilClass[iprop];
		delete [] SoilClass;
		
		numClass = nt;
		assert(numClass > 0 && np > 0);
		
		// Create 'numClass' of 'SoilType' pointers
		SoilClass = new SoilType* [numClass];
		assert(SoilClass != 0);
		
		double *sc;
		sc = new double [np];
		assert(sc != 0);
		
		// Create each 'SoilType' object
		for (int i=0; i < numClass; i++)    {
			for (int j=0; j < np; j++)
				Inp0 >> sc[j];
			SoilClass[i] = new SoilType(sc, np);
			assert(SoilClass[i] != 0);
		}
		Inp0.close();
		delete [] sc;
	}
	else if ( !option ) {
		if (nt != numClass) {
			cout<<"\nWarning! Number of classes does not correspond to previous";
			cout <<"\nProceeding with latter number of classes"<<endl<<flush; 
		}
		for (int i=0; i < numClass; i++) {
			for (int j=0; j < np; j++) {
				Inp0 >> troyan;
				SoilClass[i]->setProperty( j, troyan );
			}
		}
		Inp0.close();
	}
	return;
}

void GenericSoilData::printSoilPars() 
{
	cerr<<numClass<<" "<<SoilClass[0]->numProps<<endl;
	for (int i=0; i < numClass; i++) {
		for (int j=0; j < SoilClass[i]->numProps; j++)
			cerr<<SoilClass[i]->sProperty[j]<<" ";
		cerr<<endl;
	}
	return;
}

void GenericSoilData::setSoilPtr(int soilID) 
{
	if (soilID < 0 || soilID > numClass) { 
		cout <<"\nError: In setSoilPtr: soilclass > numClass or < 0"<<endl;
		cout <<"SoilClass = "<<soilID<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(10);
	}
	currClass = soilID - 1;   // '-1' In order to make the corresponding 
							  //      indices start from '0'
	return;
}

double GenericSoilData::getSoilProp(int propID) 
{
	if (propID < 0 || propID > SoilClass[currClass]->numProps) { 
		cout <<"\nError: In getSoilProp: propID > numProps or < 0"<<endl;
		cout <<"PropID = "<<propID<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(10);
	}
	return(SoilClass[currClass]->getProperty(propID)); 
}

//=========================================================================
//
//
//                  Section 2: tInvariant SoilType Functions
//
//
//=========================================================================

/***************************************************************************
**
**  SoilType:: Constructor and Destructor
**
***************************************************************************/
SoilType::SoilType(double *a, int n) 
{
	numProps = n;
	assert(n > 0);
	sProperty = new double [n];
	assert(sProperty != 0);
	for (int i=0; i < numProps; i++)
		sProperty[i] = a[i];
}

SoilType::SoilType() 
{
	sProperty = NULL;
	numProps  = -999;
}

SoilType::~SoilType() 
{
	if (sProperty) 
		delete [] sProperty;
	numProps = 0;
}

/***************************************************************************
**
**  SoilType:: get and set Property
**
***************************************************************************/

double SoilType::getProperty(int id) {
	return (sProperty[id]);
}

void SoilType::setProperty(int id, double param) {
	sProperty[id] = param;
}


//=========================================================================
//
//
//                  Section 3: tInvariant GenericLandData Functions
//
//
//=========================================================================

/***************************************************************************
**
**  GenericLandData(tInputFile *)
**
**  Constructor and Destructor for GenericLandData Class
**
***************************************************************************/
GenericLandData::GenericLandData(tMesh<tCNode> *mesh, 
								 tInputFile *input, tResample *resamp)
{  
	int np, id;
	double *lc;
	
	input->ReadItem(landTable, "LANDTABLENAME");   // input table
	input->ReadItem(landGrid,  "LANDMAPNAME");     // reads input file
	double *tmp = resamp->doIt(landGrid, 2);       // resamples grid
	
	tCNode *cn;
	tMeshListIter <tCNode> niter ( mesh->getNodeList() );
	id = 0;
	for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP())  {
		cn->setLandUse((int)tmp[id]);  //<- sets land use ID to tCNode 
		id++;
	}
	
	ifstream Inp0(landTable);
	if (!Inp0) {
		cout <<"\nFile "<<landTable<<" not found!"<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(2);
	}
	
	// Reads # of land classes and # parameters
	Inp0 >> numClass >> np; 
	
	assert(numClass > 0 && np > 0);
	LandClass = new LandType* [numClass];
	assert(LandClass != 0);
	
	lc = new double [np];
	assert(lc != 0);
	
	// Reads in the land parameters 
	for (int i=0; i < numClass; i++) {
		for (int j=0; j < np; j++) {
			Inp0 >> lc[j];}
		LandClass[i] = new LandType(lc, np);
		assert(LandClass[i] != 0);
	}
	
	Inp0.close();
	delete [] lc;
}

GenericLandData::~GenericLandData() {   
	for (int iprop=0;iprop < numClass;iprop++)
		delete LandClass[iprop];
	delete [] LandClass;
}

/***************************************************************************
**
**  GenericLandData:: Get and Set Land Ptr
**
**  SetLandParameters: To set (reset) land parameter values
**
***************************************************************************/
void GenericLandData::SetLtypeParameters(tMesh<tCNode> *mesh,
										 tResample *resamp, tInputFile &infile, int option)
{
	int np, nt;
	int id;
	double troyan;
	
	if ( option ) {              // Needs a new soil resampling 
		Cout<<"\nResampling Landuse......"<<endl<<flush;
		infile.ReadItem(landGrid,  "LANDMAPNAME");  // Reads input file
		double *tmp = resamp->doIt(landGrid, 2);    // Resamples grid
		
		tCNode *cn;
		tMeshListIter <tCNode> niter ( mesh->getNodeList() );
		id = 0;
		for (cn=niter.FirstP(); niter.IsActive(); cn=niter.NextP())  {
			cn->setLandUse((int)tmp[id]);  // Sets land use ID to tCNode 
			id++;
		}
	}
	
	infile.ReadItem(landTable, "LANDTABLENAME"); // Input table
	ifstream Inp0(landTable);
	if (!Inp0) {
		cout <<"File "<<landTable<<" not found!!!"<<endl;
		cout<<", tInvariant Error"<<endl;
		exit(2);
	}
	
	Inp0>>nt>>np; // Reads # of landuse classes and # parameters
	
	if ( option ) { // Needs a new landuse resampling 
		for (int iprop=0;iprop < numClass;iprop++)
			delete LandClass[iprop];
		delete [] LandClass;
		
		numClass = nt;
		assert(numClass > 0 && np > 0);
		LandClass = new LandType* [numClass];
		assert(LandClass != 0);
		
		double *lc;
		lc = new double [np];
		assert(lc != 0);
		
		for (int i=0; i < numClass; i++) {
			for (int j=0; j < np; j++)
				Inp0 >> lc[j];
			LandClass[i] = new LandType(lc, np);
			assert(LandClass[i] != 0);
		}
		Inp0.close();
		delete [] lc;
	}
	else if ( !option ) {
		if (nt != numClass) {
			cout<<"\nWarning! Number of classes does not correspond to previous";
			cout <<"\nProceeding with latter number of classes"<<endl<<flush; 
		}
		
		for (int i=0; i < numClass; i++) {
			for (int j=0; j < np; j++) {
				Inp0 >> troyan;
				LandClass[i]->setProperty( j, troyan );
			}
		}
		Inp0.close();
	}
	return;
}

void GenericLandData::setLandPtr(int landID) 
{
	if (landID < 0 || landID > numClass) { 
		cout <<"\nError: In setLandlPtr: landclass > numClass or < 0"<<endl;
		cout <<"LandClass = "<<landID<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(10);
	}
	currClass = landID - 1;  // '-1' In order to make the corresponding 
							 //      indices start from '0'
	return;
}

double GenericLandData::getLandProp(int propID) {
	//if (propID < 0 || propID > LandClass[currClass]->numProps) { 
	if (propID < 0 || propID >= LandClass[currClass]->numProps) { // GMnSKY2008MLE
		cout <<"\nError: In getLandProp: propID > numProps or < 0"<<endl;
		cout <<"PropID = "<<propID<<endl;
		cout <<"Exiting Program...\n\n"<<endl;
		exit(10);
	}
	return(LandClass[currClass]->getProperty(propID)); 
}

//=========================================================================
//
//
//                  Section 4: tInvariant LandType Functions
//
//
//=========================================================================

/***************************************************************************
**
**  LandType:: Constructor and Destructor
**
***************************************************************************/
LandType::LandType(double *a, int n) 
{
	numProps  = n;
	assert(n > 0);
	lProperty = new double [n];
	assert(lProperty != 0);
	
	for (int i=0; i < numProps; i++)
		lProperty[i] = a[i];
}

LandType::LandType() 
{
	lProperty = NULL;
	numProps  = -999;
}

LandType::~LandType() 
{
	if (lProperty) 
		delete [] lProperty;
	numProps = 0;
}

/***************************************************************************
**
**  LandType:: Set and Get Property
**
***************************************************************************/

double LandType::getProperty(int id) {
	if ((id < 0) || ( id >= numProps)) {
	cout << "LandType getProperty call warning: id is" << id <<endl;
	}
	return (lProperty[id]);
}

void LandType::setProperty(int id, double param) {
	lProperty[id] = param;
}

//=========================================================================
//
//
//                      End of tInvariant.cpp
//
//
//=========================================================================
