#if !defined(__CINT__) || defined(__MAKECINT__)

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

#endif

void pt_y_cuts()
{
    TString path = "/Users/valencia/Desktop/Analisis/";
    TString InputFileName = "AnalysisResults-LUCA.root";
    TString OutputFileName = "pt_y_cutss";

    TFile *file = TFile::Open(path + InputFileName);
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

    auto Pt_y_Bins = TFile::Open(path + InputFileName + "_Pt_y.root", "RECREATE");
    for (int i = 0; i < Pt_values.size() - 1; i++)
    {
        double minPt = Pt_values[i];
        double maxPt = Pt_values[i + 1];

        TH1D *hMass2 = (TH1D*)hMass_Pt->ProjectionX(Form("_%1.f-%1.f", minPt, maxPt), minPt, maxPt);
        std::cout << hMass2->GetEntries() << std::endl;
        hMass2->Write(Form("pt_%1.f-%1.f", minPt, maxPt));
        std::cout << "Pt bins = " << minPt << "--" << maxPt << std::endl;
    }

    for (int i = 0; i < y_values.size() - 1; i++)
    {
        double minY = y_values[i];
        double maxY = y_values[i + 1];

        TH1D *hMass2 = (TH1D*)hMass_y->ProjectionX(Form("_%2.f-%2.f", minY, maxY), minY, maxY);
        std::cout << hMass2->GetEntries() << std::endl;
        hMass2->Write(Form("y_%2.f-%2.f", minY, maxY));
        std::cout << "Y bins = " << minY << "--" << maxY << std::endl;
    }

    hMass->Write("mass_with_cuts");
    Pt_y_Bins->Close();
}
