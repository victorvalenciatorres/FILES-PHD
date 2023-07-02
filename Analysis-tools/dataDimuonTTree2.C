#include "TFile.h"
#include "TH1F.h"
#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include <string>
#include <array>

void dataDimuonTTree2() {

    // Create histograms for the values we read
    auto h1 = new TH1F("h1", "mass", 100, -2, -5);
    auto h2 = new TH1F("h2", "mass", 100, -2, -5);

    std::array<TH1F*, 100> hMass;
    for (int i = 0; i < hMass.size(); i++) {
        hMass[i] = new TH1F(Form("hMass%d", i), Form("Cut%d", i), 750, 0, 15);
    }

    TString pathToRun_528543 = "/Users/valencia/Desktop/Analisis/528543/apass4/";
    TString pathToRun_528537 = "/Users/valencia/Desktop/Analisis/528537/apass4/";
    
    //002
    std::array<TString,2> DF_528543 = {
    "DF_2270075478952960",
    "DF_2270075484728832"};

    std::array<TString,2> DF_528537 = {
    "DF_2270049675328128",
    "DF_2270049678050176" };

    TChain *tc = new TChain("O2rtdimuonall");
    for (int i = 0; i < DF_528543.size(); i++)
    {
        tc->Add(pathToRun_528543 + "AOD/002/AO2D.root/" + DF_528543[i] + "/O2rtdimuonall");
    }
    for (int i = 0; i < DF_528537.size(); i++)
    {
        tc->Add(pathToRun_528537 + "AOD/002/AO2D.root/" + DF_528537[i] + "/O2rtdimuonall");
    }
    // Create a TTreeReader for the tree, for instance by passing the
    // TTree's name and the TDirectory / TFile it is in.
    TTreeReader myReader(tc);

    // The branch "fMass" contains floats; access them as mass.
    TTreeReaderValue<Float_t> mass(myReader, "fMass");
    TTreeReaderValue<Float_t> eta1(myReader, "fEta1");
    TTreeReaderValue<Float_t> eta2(myReader, "fEta2");
    TTreeReaderValue<Float_t> pt1(myReader, "fPt1");
    TTreeReaderValue<Float_t> pt2(myReader, "fPt2");

    // Loop over all entries of the TTree or TChain.
    while (myReader.Next()) {
    // Just access the data as if mass was an iterator (note the '*'
    // in front of it):
    hMass[0]->Fill(*mass); // first fill one histo without c cut
        
    if ((*eta1 > -3.6 && *eta1 < -2.5) && (*eta2 > -3.6 && *eta2 < -2.5)) // cut on eta in mft acceptance
    {
        h1->Fill(*mass);
        //hchi2_2->Fill(*chi2_mu2);
        for (int PtCut = 1; PtCut < hMass.size(); PtCut++) // loop over all chi2 cut histos
        {
            // Apply PtCut cuts 
            if ((*pt1 > 0 && *pt1 < PtCut) && (*pt2 > 0 && *pt2 < PtCut))
            {
                hMass[PtCut]->Fill(*mass);
            } // end if PtCut cut
        }
    } // end if eta cut
    }


    TString pathToResFile = "/Users/valencia/Desktop/Analisis/";
    auto file = TFile::Open(pathToResFile + "tryy2.root", "RECREATE");

    for (int i = 1; i < 100; i++) {
        hMass[i]->GetXaxis()->SetRangeUser(2, 5);
        hMass[i]->Write(Form("hMassPtis0to%d", i));
    }
    
}
