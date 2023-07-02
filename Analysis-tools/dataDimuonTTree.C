#include "TFile.h"
#include "TH1F.h"
#include "TChain.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include <string>
 
void dataDimuonTTree() {
   // Create a histogram for the values we read.
   auto h1 = new TH1F("h1","mass",100,-2,-5);
   auto h2 = new TH1F("h2","mass",100,-2,-5);
   auto hchi2_1 = new TH1F("hchi2_1", "chi2_mu1 histogram", 100, 0, 10);


   std::array<TH1F*,201> hMassChi2Cuts;
   for (int i=0;i<hMassChi2Cuts.size();i++)
   {
      hMassChi2Cuts[i] = new TH1F(Form("hMassChi2_Cut%d",i),Form("Cut%d",i),750,0,15);
   }

   TString pathToRun_528021 = "/Users/valencia/Desktop/Analisis/528021/apass3/AOD";
   TString pathToRun_528020 = "/Users/valencia/Desktop/Analisis/528021/apass3/AOD";
 
   std::array<TString,2> df_528020 = {
   "DF_2267652895436416",
   "DF_2267652906537472"};

   std::array<TString,2> df_528021 = {
   "DF_2267832931323648",
   "DF_2267832933572608"};

   TChain *tc = new TChain("O2rtdimuonall");
   for (int i = 0; i < df_528021.size(); i++)
   {
      tc->Add(pathToRun_528021 + "AO2/001/AOD.root/" + df_528021[i] + "/O2rtdimuonall");
   }
   for (int i = 0; i < df_528020.size(); i++)
   {
      tc->Add(pathToRun_528020 + "AO2/001/AOD.root/" + df_528020[i] + "/O2rtdimuonall");
   }
   // Create a TTreeReader for the tree, for instance by passing the
   // TTree's name and the TDirectory / TFile it is in.
   TTreeReader myReader(tc);
 
   // The branch "fMass" contains floats; access them as mass.
   TTreeReaderValue<Float_t> mass(myReader, "fMass");
   TTreeReaderValue<Float_t> eta1(myReader, "fEta1");
   TTreeReaderValue<Float_t> eta2(myReader, "fEta2");
   TTreeReaderValue<Float_t> chi2_mu1(myReader, "fChi2MatchMCHMFT1");
   TTreeReaderValue<Float_t> chi2_mu2(myReader, "fChi2MatchMCHMFT2");
 
   // Loop over all entries of the TTree or TChain.
   while (myReader.Next()) {
      // Just access the data as if mass was iterator (note the '*'
      // in front of it):
      hMassChi2Cuts[0]->Fill(*mass); // first fill one histo without chi2 cut
      
      if ((*eta1 > -3.6 && *eta1 < -2.5) && (*eta2 > -3.6 && *eta2 < -2.5)) // cut on eta in mft acceptance
      {
         hchi2_1->Fill(*chi2_mu1);
         //hchi2_2->Fill(*chi2_mu2);
         for (int icut = 1; icut < hMassChi2Cuts.size(); icut++) // loop over all chi2 cut histos
         {
            if ((*chi2_mu1 > 0 && *chi2_mu1 < icut) && (* chi2_mu2 > 0 && *chi2_mu2 < icut))
            {
               hMassChi2Cuts[icut]->Fill(*mass);
            } // end if chi2 cut
         }
      } // end if eta cut
   }
 
   TString pathToResFile = "/Users/valencia/Desktop/Analisis/";
   auto file = TFile::Open(pathToResFile + "mass_lhc22o_apass2_Chi2Cuts.root","RECREATE");

   for (int icut = 0; icut < hMassChi2Cuts.size(); icut++){
      hMassChi2Cuts[icut]->GetXaxis()->SetRangeUser(2,5);
      hMassChi2Cuts[icut]->Write(Form("hMassChi2is0to%d",icut));
   }
}
