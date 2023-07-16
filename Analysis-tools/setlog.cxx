int setlog()
{
    TFile* file = TFile::Open("/Users/valencia/Desktop/analisis/AnalysisResults-ALL-LHC22o.root");
    TDirectory* dir = (TDirectory*)file->Get("analysis-same-event-pairing");
    THashList* HashList = (THashList*)dir->Get("output");
    TList* list = (TList*)HashList->FindObject("PairsMuonSEPM_matchedMchMid");
    list->ls();
    
    TFile* file2 = new TFile("AnalysisResults-ALL-LHC22o-logyScale.root", "RECREATE");
    TH1F* Mass = (TH1F*)list->FindObject("Mass");

    TCanvas* canvas = new TCanvas("ALL-LHC22o-logyScale", "ALL-LHC22o-logyScale"); // Set the desired name for the canvas

    canvas->SetLogy();

    Mass->Draw();

    canvas->Write(); // Write the canvas with the desired name to the file

    file->Close();
    file2->Close();

    return 0;   
}
