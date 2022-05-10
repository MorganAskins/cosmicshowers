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

  DS::Run* run = new RAT::DS::Run();
  DS::PMTInfo* pmtinfo;
  DS::Root* ds = new RAT::DS::Root();

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
    if( i%decimal == 0 )
      cerr << "Completion: " << floor(i/double(entries)*1000)/10 << "%\r" << flush;
    auto mc = ds->GetMC();
    TTimeStamp timer = mc->GetUTC();
    int seconds = timer.GetSec();
    int nanoseconds = timer.GetNanoSec();
    int ntracks = mc->GetMCTrackCount();
    bool eos_inner = false;
    bool outer_tank = false;
    int mcpcount = mc->GetMCParticleCount();
    double eventTime = 0;
    if( mcpcount > 0 )
      eventTime = mc->GetMCParticle(0)->GetTime();
    // First track has creation time and energy
    for( int trk = 0; trk < ntracks; trk++ ){
      auto track = mc->GetMCTrack(trk);
      auto name = track->GetParticleName();
      auto pid = track->GetPDGCode();
      int nSteps = track->GetMCTrackStepCount();
      double energy = 0;
      double dirx = 0;
      double diry = 0;
      double dirz = 0;
      string process = "none";
      string death = "nowhere";
      double ptime = 0;
      for(int stp = 0; stp<nSteps; stp++){
        auto step = track->GetMCTrackStep(stp);
        if( stp == 0 ){
          energy = step->GetKE();
          auto tv = step->GetMomentum();
          dirx = tv.X();
          diry = tv.Y();
          dirz = tv.Z();
          process = step->GetProcess();
          //ptime = step->GetGlobalTime();
          ptime = step->GetLocalTime();
          //ptime = step->GetProperTime();
        }
        string volume = step->GetVolume();
        if( volume == "eos_inner" )
          eos_inner = true;
        if( volume == "outer_tank" )
          outer_tank = true;
        if( stp == nSteps-1 )
          death = volume;
      }
      if( (eos_inner || outer_tank) && (energy > energyThreshold)){
        cout << i << " ";
        cout << eventTime << " ";
        cout << nanoseconds << " ";
        cout << name << " ";
        cout << pid << " ";
        cout << process << " ";
        cout << energy << " ";
        cout << dirx << " ";
        cout << diry << " ";
        cout << dirz << " ";
        cout << outer_tank << " ";
        cout << eos_inner << " ";
        cout << death << endl;
      }
    }
  }
  cerr << endl;
}
