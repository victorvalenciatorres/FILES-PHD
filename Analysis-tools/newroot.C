int newroot()
{
          
    
    TFile* file = TFile::Open("/Users/valencia/Desktop/analysishoy/AnalysisResults-17.root");
    TDirectory* dir =  (TDirectory*) file -> Get("analysis-same-event-pairing");
    THashList* HashList = (THashList*) dir->Get("output");
    TList* list = (TList*) HashList->FindObject("PairsMuonSEPM_muonQualityCuts");
    list->ls();
    
    TFile *file2  = new TFile("LHC22m.root", "RECREATE");
    TH1F* Mass= (TH1F*)list->FindObject("Mass");

    file2->WriteObject(Mass, "Mass_22m_pass3_muonQualityCuts");
   
    file2->Close();



    return 0;   
}

