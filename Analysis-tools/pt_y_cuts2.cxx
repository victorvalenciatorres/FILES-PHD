#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <TFile.h>
#include <TDirectoryFile.h>
#include <THashList.h>
#include <TString.h>
#include <TH1.h>
#include <TH2.h>

int FindBinY(TH2D* hist, double y)
{
    int bin = hist->GetYaxis()->FindBin(y);
    return bin;
}

int FindBinPt(TH2D* hist, double pt)
{
    int bin = hist->GetYaxis()->FindBin(pt);
    return bin;
}

void pt_y_cuts2()
{
    TString path = "/Users/valencia/Desktop/Analisis/";
    TString InputFileName = "AnalysisResults-ALL-LHC22o";
    
    TFile *file = TFile::Open(path + InputFileName + ".root");
    TDirectory *dir = (TDirectory*)file->Get("analysis-same-event-pairing");
    THashList *HashList = (THashList*)dir->Get("output");
    TList *list = (TList*)HashList->FindObject("PairsMuonSEPM_matchedMchMid");

    TH1D *hMass = static_cast<TH1D*>(list->FindObject("Mass"));
    TH2D *hMass_Pt = static_cast<TH2D*>(list->FindObject("Mass_Pt"));
    TH2D *hMass_y = static_cast<TH2D*>(list->FindObject("Mass_Rapidity"));

    std::array<double, 9> Pt_values = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 7.0, 10.0, 20.0};
    std::array<double, 7> y_values = {2.5, 2.75, 3.0, 3.25, 3.5, 3.75, 4.0};

    auto hMass_File = TFile::Open(path + InputFileName + "_integrated.root", "RECREATE");
    hMass->Write("mass_without_cuts");
    hMass_File->Close();

    auto Y_Bins = TFile::Open(path + InputFileName + "_Y.root", "RECREATE");
    for (int i = 0; i < y_values.size() - 1; i++)
    {
        double minY = y_values[i];
        double maxY = y_values[i + 1];
        int minY_bin = FindBinY(hMass_y, minY);
        int maxY_bin = FindBinY(hMass_y, maxY);
        
        TH1D *hMass2 = (TH1D*)hMass_y->ProjectionX(Form("_%2.f-%2.f", minY, maxY), minY_bin, maxY_bin);
        std::cout << "Entries in Y bin " << i+1 << ": " << hMass2->GetEntries() << std::endl;
        hMass2->Write(Form("y_%2.f-%2.f", minY, maxY));
        std::cout << "Y bins = " << minY << "--" << maxY << std::endl;
    }
    Y_Bins->Close();

    auto Pt_Bins = TFile::Open(path + InputFileName + "_Pt.root", "RECREATE");
    for (int i = 0; i < Pt_values.size() - 1; i++)
    {
        double minPt = Pt_values[i];
        double maxPt = Pt_values[i + 1];
        int minPt_bin = FindBinPt(hMass_Pt, minPt);
        int maxPt_bin = FindBinPt(hMass_Pt, maxPt);
        
        TH1D *hMass2 = (TH1D*)hMass_Pt->ProjectionX(Form("_%3.f-%3.f", minPt, maxPt), minPt_bin, maxPt_bin);
        std::cout << "Entries in Pt bin " << i+1 << ": " << hMass2->GetEntries() << std::endl;
        hMass2->Write(Form("pt_%3.f-%3.f", minPt, maxPt));
        std::cout << "Pt bins = " << minPt << "--" << maxPt << std::endl;
    }
    Pt_Bins->Close();
}
