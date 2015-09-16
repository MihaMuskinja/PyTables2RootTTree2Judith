#include <iostream>
#include <algorithm>

#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TDirectory.h>

int PyTables2RootTTree2Judith(const char* inputName, const char* outputName, const char* plane="Plane0", const char* MODE="RECREATE", Long64_t MaxEvents = -1)
{
	// python files
	TFile* file  = new TFile(inputName);
	TTree* table = (TTree*) file->Get("Table");

    // Judith files
    TFile* j_file   = new TFile(outputName,MODE);
    TTree* event    = new TTree("Event", "Event");
    TDirectory* dir = j_file->mkdir(plane);
    dir->cd();
    TTree* hits     = new TTree("Hits", "Hits");

	int global_event_number  = 0;
    UInt_t temp_event_number = 0;

    // python arrays
	UShort_t  row[10000];
	UChar_t   column[10000];
	Long64_t  event_number[10000];
	UInt_t    trigger_time_stamp[10000];
	UChar_t   relative_BCID[10000];
	UChar_t   tot[10000];

	// Judith arrays
	Int_t    NHits = 0;
	Int_t    hitPixX[10000];
	Int_t    hitPixY[10000];
	Double_t hitPosX[10000];
	Double_t hitPosY[10000];
	Double_t hitPosZ[10000];
	Int_t    hitValue[10000];
	Int_t    hitTiming[10000];
	Int_t    hitInCluster[10000];

	ULong64_t timeStamp;
	ULong64_t frameNumber;
	Int_t     triggerOffset;
	Int_t     triggerInfo;
	Bool_t    invalid;

    // python branches
	table->SetBranchAddress("row",&row);
	table->SetBranchAddress("column",&column);
	table->SetBranchAddress("event_number",&event_number);
	table->SetBranchAddress("trigger_time_stamp",&trigger_time_stamp);
	table->SetBranchAddress("relative_BCID",&relative_BCID);
	table->SetBranchAddress("tot",&tot);

	// Judith branches
	hits->Branch("NHits"     , &NHits      , "NHits/I");
	hits->Branch("PixX"      , hitPixX     , "HitPixX[NHits]/I");
	hits->Branch("PixY"      , hitPixY     , "HitPixY[NHits]/I");
	hits->Branch("Value"     , hitValue    , "HitValue[NHits]/I");
	hits->Branch("Timing"    , hitTiming   , "HitTiming[NHits]/I");
	hits->Branch("InCluster" , hitInCluster, "HitInCluster[NHits]/I");
	hits->Branch("PosX"      , hitPosX     , "HitPosX[NHits]/D");
	hits->Branch("PosY"      , hitPosY     , "HitPosY[NHits]/D");
	hits->Branch("PosZ"      , hitPosZ     , "HitPosZ[NHits]/D");

    event->Branch("TimeStamp"    , &timeStamp    , "TimeStamp/l");
    event->Branch("FrameNumber"  , &frameNumber  , "FrameNumber/l");
    event->Branch("TriggerOffset", &triggerOffset, "TriggerOffset/I");
    event->Branch("TriggerInfo"  , &triggerInfo  , "TriggerInfo/I");
    event->Branch("Invalid"      , &invalid      , "Invalid/O");


	const Long64_t Nentries = table->GetEntries();

	for (Long64_t n = 0; n < Nentries; n++) {
		int tableEntrySize = (int)table->GetEntry(n);
		if(n%100 == 0)std::cout << "==== chunk number: " << n << " size: " << tableEntrySize << std::endl;
        
        //NHits = 0;

		for(int i = 0; i<10000; i++) {
			if(event_number[i] < temp_event_number) {
				std::cout << "we have reached the end of file at chunk index: " << i << std::endl;
				break;
			}
			else if(event_number[i] != temp_event_number) {
				temp_event_number = event_number[i];

				//std::cout << " ----- number of hits in event: " << global_event_number << " is: " << NHits << std::endl;
				hits->Fill();
				event->Fill();

				global_event_number++;
                                if(MaxEvents>0 && global_event_number==MaxEvents) {
                                  j_file->Write();
                                  j_file->Close();
                                  return global_event_number - 1;
                                }
				NHits = 0;
			}
            if(column[i]==0 || row[i]==0) {
            	NHits = 0;
			    // fill event
                timeStamp     = (ULong64_t)   trigger_time_stamp[i];
                frameNumber   = (ULong64_t) event_number[i];
                triggerOffset = 0;
                triggerInfo   = 0;
                invalid = 0;
            	continue;
            }
			hitPixX[NHits]      = column[i]-1;
			hitPixY[NHits]      = row[i]-1;
            hitValue[NHits]     = (Int_t) tot[i];
            hitTiming[NHits]    = (Int_t) relative_BCID[i];
            hitInCluster[NHits] = -1;
            hitPosX[NHits]      = 0;
            hitPosY[NHits]      = 0;
            hitPosZ[NHits]      = 0;
			NHits++;
			if(NHits == 1) {
			  // fill event
              timeStamp     = (ULong64_t)   trigger_time_stamp[i];
              frameNumber   = (ULong64_t) event_number[i];
              triggerOffset = 0;
              triggerInfo   = 0;
              invalid = 0;
			}
	        //std::cout << "       event number: " << temp_event_number << std::endl;
	        //std::cout << "global event number: " << global_event_number << std::endl;
		}
	}

  hits->Fill();
  event->Fill();
  j_file->Write();
  j_file->Close();

  return global_event_number;
}

void CombineTwoDUTS(const char* fileName1, const char* fileName2, const char* outputName, Long64_t MaxEvents=-1) 
{
  int Nevents1 = PyTables2RootTTree2Judith(fileName1, outputName, "Plane0", "RECREATE", MaxEvents);
  int Nevents2 = PyTables2RootTTree2Judith(fileName2, outputName, "Plane1", "UPDATE", MaxEvents);
  std::cout << "Nevents1: " << Nevents1 << " Nevents2: " << Nevents2 << std::endl;

  return;
}
