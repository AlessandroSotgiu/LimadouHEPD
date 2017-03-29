#include "LReco01Manager.hh"
#include "LSteeringManager.hh"
#include "LTrackerTools.hh"

#include <iostream>
#include <fstream>

LReco01Manager::LReco01Manager() {
  verboseFLAG=true;
  steeringLoadedFLAG=false;
}


void LReco01Manager::LoadSteering(const char* steerFileIN) {
  Reset();
  std::cout << __LRECO01MANAGER__ << "Loading the steerig file \""
	    << steerFileIN << "\"..." << std::endl;

  steerFile = steerFileIN;
  LSteeringManager steer;
  steer.Load(steerFile);
  // Load calibration
  calFileName = steer.GetParameter<std::string>("CALBFILE");
  cal.Read(calFileName.c_str());
  // Input file list
  inpFileList = steer.GetParameter<std::string>("INPLIST");
  // Output folder
  outDirectory = steer.GetParameter<std::string>("OUTFOLD");
  // Max Events
  maxEvents = steer.GetParameter<int>("MAXEVTS");
  // Max File Events
  maxFileEvents = steer.GetParameter<int>("MAXFEVTS");
  // Verbosity 
  verboseFLAG = steer.GetParameter<bool>("VERBOSE");
  std::cout << __LRECO01MANAGER__ << "done." << std::endl;
  steeringLoadedFLAG=true;

  if(!CheckLoadedSteering()) Reset();
  
  return;
}

void LReco01Manager::Reset(void) {
  steerFile="";
  calFileName="";
  inpFileList="";
  outDirectory="";
  maxEvents=0;
  maxFileEvents=0;
  verboseFLAG=true;
  steeringLoadedFLAG=false;
  cal.Reset();
  L0fname.resize(0);
  inFile=0;
  outFile=0;
  return;
}

bool LReco01Manager::CheckLoadedSteering(void) const {
  std::cout << __LRECO01MANAGER__ << "CheckLoadedSteering():" << std::endl;
  bool result=true;
  if(steeringLoadedFLAG==false) {
    std::cout << __LRECO01MANAGER__ << "steering file never loaded." << std::endl;
    result=false;
  }
  if(cal.CheckStatus()==false) result=false;
  std::ifstream istr(inpFileList, std::ifstream::in);
  if(istr.good()==false) result=false; 
  istr.close();
  if(maxEvents<0 && maxEvents!=-1) result=false;
  if(maxFileEvents<0 && maxFileEvents!=-1) result=false;
  if(result==false) {
    std::cout << __LRECO01MANAGER__ << "something is wrong or missing in the steering file" << std::endl;
  }
  else {
    std::cout << __LRECO01MANAGER__ << "steering file \"" << steerFile << "\" ok." << std::endl; 
    if(verboseFLAG==true) {
      std::cout << __LRECO01MANAGER__ << "Reco parameters:" << std::endl;
      std::cout << __LRECO01MANAGER__ << "calFileName  " << calFileName << std::endl;
      std::cout << __LRECO01MANAGER__ << "inpFileList  " << inpFileList << std::endl;
      std::cout << __LRECO01MANAGER__ << "outDirectory " << outDirectory << std::endl;
      std::cout << __LRECO01MANAGER__ << "maxEvents    " << maxEvents << std::endl;
    }
  }
  return result;
}


void LReco01Manager::Run(void) {
  if(steeringLoadedFLAG==false) {
    std::cout << __LRECO01MANAGER__ << "How to run? Steering file never loaded!" << std::endl;
    return;;
  }
  int nFiles=LoadInpFileList();
  if(nFiles==0) {
    std::cout << __LRECO01MANAGER__ << "No input file to run on." << std::endl;
    return;
  }

  // Main loop
  int totevtcounter=0;
  for(auto fin : L0fname) {
    int fevtcounter=0;
    std::cout << __LRECO01MANAGER__ << "Reading L0 file" << fin << std::endl;
    inFile = new LEvRec0File(fin.c_str());
    LEvRec0 lev0;
    inFile->SetTheEventPointer(lev0);
    int nentries=inFile->GetEntries();
    std::cout << __LRECO01MANAGER__ << "Looping on file " << fin << " (" << nentries << " entries)" << std::endl;
    for(int i0=0; i0<nentries; ++i0){
      inFile->GetEntry(i0);
      if(i0==0) NewOutFile();
      outFile->Fill(L0ToL1(lev0,cal));
      ++fevtcounter;
      if(maxFileEvents!=-1 && fevtcounter>maxFileEvents-1) break;
      ++totevtcounter;
      if(maxEvents!=-1 && totevtcounter>maxEvents-1) break;
    }
    inFile->Close();
    delete inFile;
    if(maxEvents!=-1 && totevtcounter>maxEvents-1) break;
  }
  outFile->Write();
  outFile->Close();
  delete outFile;
  
  return;
}


void LReco01Manager::NewOutFile(void) {
  if(!inFile) {
    std::cout << __LRECO01MANAGER__ << "No input file to get the output name from." << std::endl;
    return;
  }
  if(outFile!=0 && outFile->IsOpen()) {
    outFile->Write();
    outFile->Close();
    delete outFile;
  }

  // Set the new name
  std::string l1name = L0NameToL1Name();
  outFile = new LEvRec1File(l1name.c_str(),"WRITE");
  std::cout << __LRECO01MANAGER__ << "Output file \"" << l1name << "\" created." << std::endl;
  return;
}


std::string LReco01Manager::L0NameToL1Name(void) { // Naming convention needed!
  std::string l0name=inFile->GetName();
  std::string l1name=outDirectory + "/"
    + l0name.substr(l0name.find_last_of('/'),l0name.find_last_of("."))
    + "_L1.root";
  return l1name;
}


LEvRec1 LReco01Manager::L0ToL1(const LEvRec0 lev0, const LCalibration cal) {
  LEvRec1 result;
  result.tracker=GetTrackerSignal(lev0, cal);
  // ...
  //  result.FillRandom();
  return result;
}

int LReco01Manager::LoadInpFileList(void) {
  L0fname.resize(0);
  std::ifstream istr(inpFileList, std::ifstream::in);
  while(1) {
    std::string fname;
    istr >> fname;
    if(istr.eof()) break;
    if(CheckInputFile(fname)) L0fname.push_back(fname);
    else {
      std::cout << __LRECO01MANAGER__ << "File \"" << fname << "\" does not look "
		<< " a good LEvRec0File." << std::endl;
    }
  }
  int result = L0fname.size();
  std::cout << __LRECO01MANAGER__ << result << " files about to be processed." << std::endl;
  return result;
}


bool LReco01Manager::CheckInputFile(const std::string fname) const {
  bool result =true;
  LEvRec0File fin(fname.c_str());
  if(fin.GetEntries()<MINL0EVENTS) result=false;
  return result;
}

LReco01Manager& LReco01Manager::GetInstance() {
  static LReco01Manager instance; // Guaranteed to be destroyed.
                                  // Instantiated on first use.
  return instance;
}


LReco01Manager::~LReco01Manager() {
  // do not care about singleton destructor
}


