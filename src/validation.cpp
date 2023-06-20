#include <iostream>
#include <vector>
#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TTimeStamp.h>
#include <TVector3.h>

#include <RAT/DS/PMT.hh>
#include <RAT/DS/PMTInfo.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
#include <RAT/DS/MC.hh>
#include <RAT/DS/MCParticle.hh>

using namespace std;
using namespace RAT;

int main(int argc, char** argv){
  // Perform a few checks to be sure that the cosmic ray generator is giving
  // the output we expect.
  // 1. Check both particle timing and event timing.
  // 2. Angular distribution of particles.
  // 3. Flux.

  DS::Run* run = new RAT::DS::Run();
  DS::PMTInfo* pmtinfo;
  DS::Root* ds = new RAT::DS::Root();

  // Load file.
  string filename = argv[1];
  TFile* tfile = new TFile(filename.c_str());
  cerr << "opened " << tfile->GetName() << endl;
  TTree* runT = (TTree*)tfile->Get("runT");
  runT->SetBranchAddress("run", &run);
  cerr << " entries " << runT->GetEntries() << endl;
  runT->GetEntry(0);

  pmtinfo = run->GetPMTInfo();

  TTree* T = (TTree*)tfile->Get("T");
  int entries = T->GetEntries();
  T->SetBranchAddress("ds", &ds);
  // What would we like to know? CSV with:
  // Volumes of interest in eos are inner_vessel and outer_tank
  // Time EventID PID energy momentum eos_inner? outer_tank?
  double energyThreshold = 1.0; // MeV
  int decimal = static_cast<int>(entries/1000);
  for( int i=0; i<entries; i++ ){
    T->GetEvent(i);
    auto mc = ds->GetMC();
    TTimeStamp timer = mc->GetUTC();
    int seconds = timer.GetSec();
    int nanoseconds = timer.GetNanoSec();
    int ntracks = mc->GetMCTrackCount();
    int mcpcount = mc->GetMCParticleCount();
    double eventTime = 0;
    if( mcpcount > 0 )
    {
      eventTime = mc->GetMCParticle(0)->GetTime();
      cout << i << " ";
      cout << eventTime*1e9 << " ";
      cout << seconds << ":" << nanoseconds << " ";
      cout << endl;
      // First track has creation time and energy
    }
  }
}
