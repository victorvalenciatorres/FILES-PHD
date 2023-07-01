// \author Luca Micheletti <luca.micheletti@cern.ch>

// STL includes
#include <Riostream.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sstream>

// ROOT includes
#include <TROOT.h>
#include <TObjArray.h>
#include <TMinuit.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TF1.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TMath.h>
#include <TPad.h>
#include <TSystem.h>
#include <TGraphErrors.h>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TFitResult.h>
#include <TMatrixDSym.h>
#include <TPaveText.h>
#include <TCollection.h>
#include <TKey.h>
#include <TGaxis.h>

// RooFit includes
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooWorkspace.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
#include "RooPlot.h"
#include "RooHist.h"
#include "RooDataHist.h"
using namespace RooFit;

// My includes
#include "fit_library/FunctionLibrary.C"
#include "fit_library/UserFunctionLibrary.cxx"
#include "DQFitter.h"

ClassImp(DQFitter)

//______________________________________________________________________________
DQFitter::DQFitter(): TObject() {
  // default constructor
  fDoRooFit         = kFALSE;
  fPathToFile       = "FitResults.root";
  fNParBkg          = 2;
  fNParSig          = 3;
  fMaxFitIterations = 100;
  fMinFitRange      = 0.;
  fMaxFitRange      = 100;
  fFitMethod        = "SRL";
  fFitStatus        = "Undefined";
}
//______________________________________________________________________________
DQFitter::DQFitter(TString pathToFile): TObject() {
  // standard constructor
  fDoRooFit         = kFALSE;
  fPathToFile       = pathToFile;
  fNParBkg          = 2;
  fNParSig          = 3;
  fMaxFitIterations = 100;
  fMinFitRange      = 0.;
  fMaxFitRange      = 100;
  fFitMethod        = "SRL";
  fFitStatus        = "Undefined";
  OpenOutputFile(fPathToFile);
}
//______________________________________________________________________________
DQFitter::~DQFitter() {
  // destructor
}
//______________________________________________________________________________
void DQFitter::OpenOutputFile(TString pathToFile) {
  fPathToFile=pathToFile;
  printf("\n************ Create output file %s ************\n", fPathToFile.Data());
  fFile = new TFile(fPathToFile, "RECREATE");
}
//______________________________________________________________________________
void DQFitter::CloseOutputFile() {
  if (fFile) {
    fFile->Close();
  } else {
    printf("\n************ WARNING: no output file inizialized ************\n");
  }
}
//______________________________________________________________________________
extern "C" void DQFitter::SetHistogram(TH1F *hist) {
  fHist = hist;
  fHist->GetYaxis()->SetRangeUser(0., 2.*fHist->GetMaximum());
}
//______________________________________________________________________________
void DQFitter::SetTree(TTree *tree) {
  fTree = tree;
}
//______________________________________________________________________________
extern "C" void DQFitter::SetUserFunction(const char *nameFunc[3], Int_t nParams[3]) {
  // WARNING --> To see the function and the fit superimposed you need to load UserFunctions.cxx
  fFuncSig = new TF1("fFuncSig", nameFunc[0], -100., 100., nParams[0]);
  fNParSig = nParams[0];
  fFuncBkg = new TF1("fFuncBkg", nameFunc[1], -100., 100., nParams[1]);
  fNParBkg = nParams[1];
  fFuncTot = new TF1("fFuncTot", nameFunc[2], -100., 100., nParams[2]);
}
//______________________________________________________________________________
extern "C" void DQFitter::SetFitRange(Double_t minFitRange, Double_t maxFitRange) {
  fMinFitRange = minFitRange;
  fMaxFitRange = maxFitRange;
}
//______________________________________________________________________________
extern "C" void DQFitter::SetFitMethod(TString fitMethod) {
  fFitMethod = fitMethod;
}
//______________________________________________________________________________
extern "C" void DQFitter::InitParameters(Int_t nParams, Double_t *params, Double_t *minParamLimits, Double_t *maxParamLimits, TString *nameParams) {
  fNParams = nParams;
  fParams = params;
  fMinParamLimits = minParamLimits;
  fMaxParamLimits = maxParamLimits;
  fParamNames = nameParams;
}
//______________________________________________________________________________
extern "C" void DQFitter::BinnedFitInvMassSpectrum(TString trialName) {
  fTrialName = trialName;

  // Set/Fix the function parameters
  fFuncTot->SetNpx(10000);
  fFuncSig->SetNpx(10000);
  for (int iPar = 0;iPar < fNParams;iPar++) {
    fFuncTot->SetParName(iPar, fParamNames[iPar].Data());
    if (fMinParamLimits[iPar] == fMaxParamLimits[iPar]) {
      fFuncTot->FixParameter(iPar, fParams[iPar]);
    } else {
      fFuncTot->SetParameter(iPar, fParams[iPar]);
    }
  }

  // Init the histogram with the fit results
  fHistResults = new TH1F("histResults", "", fNParams+2, 0., fNParams+2);
  fHistResults->GetXaxis()->SetBinLabel(1, "#chi^{2} / NDF");
  fHistResults->GetXaxis()->SetBinLabel(2, "Signal");
  for (int iPar = 2;iPar < fNParams+2;iPar++) {
    fHistResults->GetXaxis()->SetBinLabel(iPar+1, fFuncTot->GetParName(iPar-2));
  }

  // Fit the background
  TH1F *histBkg = (TH1F*) fHist -> Clone("histBkg");
  Int_t fistBin = histBkg -> FindBin(2.8);
  Int_t lastBin = histBkg -> FindBin(3.8);
  for(int iBin = fistBin;iBin < lastBin;iBin++){
    histBkg -> SetBinContent(iBin,0);
    histBkg -> SetBinError(iBin,0);
  }

  fFuncBkg -> SetParameters(1.,1.,1.,1.,1.,1.,1000.);
  TFitResultPtr ptrFitBkg;
  ptrFitBkg = (TFitResultPtr) histBkg->Fit(fFuncBkg, "RS", "", fMinFitRange, fMaxFitRange);
  for (Int_t iParBkg = 0;iParBkg < fNParBkg;iParBkg++) {
    fFuncTot->SetParameter(iParBkg, fFuncBkg->GetParameter(iParBkg));
  }

  // Fit the histogram signal + background
  TFitResultPtr ptrFit;
  for(int i = 0;i < 10;i++){
    cout << "Iteration " << i << endl;
    if (i > 0) {
      fFuncTot -> SetParameters(fFuncTot -> GetParameters());
    }
    ptrFit = (TFitResultPtr) fHist->Fit(fFuncTot, fFitMethod, "", fMinFitRange, fMaxFitRange);
    if (gMinuit->fCstatu.Contains("CONVERGED")) {
      break;
    }
  }
  fFitStatus = gMinuit->fCstatu;
  //ptrFit = (TFitResultPtr) fHist->Fit(fFuncTot, fFitMethod, "", fMinFitRange, fMaxFitRange);
  TMatrixDSym covSig  = ptrFit->GetCovarianceMatrix().GetSub(fNParBkg, fNParBkg+fNParSig-1, fNParBkg, fNParBkg+fNParSig-1);

  // Set Background prameters
  for (Int_t iParBkg = 0;iParBkg < fNParBkg;iParBkg++) {
    fFuncBkg->SetParameter(iParBkg, fFuncTot->GetParameter(iParBkg));
  }

  // Set Signal parameters
  for (Int_t iParSig = 0;iParSig < fNParSig;iParSig++) {
    fFuncSig->SetParameter(iParSig, fFuncTot->GetParameter(fNParBkg + iParSig));
  }

  fChiSquareNDF      = fFuncTot->GetChisquare() / fFuncTot->GetNDF();
  fErrorChiSquareNDF = 0.;
  fSignal            = fFuncSig->Integral(fMinFitRange, fMaxFitRange) / fHist->GetBinWidth(1);
  fErrorSignal       = fFuncSig->IntegralError(fMinFitRange, fMaxFitRange, fFuncSig->GetParameters(), covSig.GetMatrixArray()) / fHist->GetBinWidth(1);

  // Default entries of the results histogram
  fHistResults->SetBinContent(fHistResults->GetXaxis()->FindBin("#chi^{2} / NDF"), fChiSquareNDF);
  fHistResults->SetBinError(fHistResults->GetXaxis()->FindBin("#chi^{2} / NDF"), fErrorChiSquareNDF);
  fHistResults->SetBinContent(fHistResults->GetXaxis()->FindBin("Signal"), fSignal);
  fHistResults->SetBinError(fHistResults->GetXaxis()->FindBin("Signal"), fErrorSignal);

  // User entries of the results histogram
  for (int iPar = 0;iPar < fNParams;iPar++) {
    fHistResults->SetBinContent(iPar+3, fFuncTot->GetParameter(iPar));
    fHistResults->SetBinError(iPar+3, fFuncTot->GetParError(iPar));
  }
  if (!fFile) {
    printf("\n************ WARNING: no output file! ************\n");
  } else {
    SaveResults();
  }
}
//______________________________________________________________________________
void DQFitter::SaveResults() {
  // Create the directory where the output will be saved
  TDirectory *trialDir = fFile->mkdir(fTrialName);
  TCanvas *canvasFit, *canvasRatio, *canvasResiduals;

  if (fDoRooFit) {
    canvasFit = new TCanvas(Form("canvasFit_%s", fTrialName.Data()), Form("canvasFit_%s", fTrialName.Data()), 600, 600);
    canvasFit->SetLeftMargin(0.15);
    gPad->SetLeftMargin(0.15);
    fRooPlot->GetYaxis()->SetTitleOffset(1.4);
    fRooPlot->Draw();

    RooHist* rooHistRatio = fRooPlot->residHist();
    RooPlot* rooPlotRatio = fRooMass.frame(Title("Residual Distribution")) ;
    rooPlotRatio->addPlotable(rooHistRatio,"P") ;
    canvasRatio = new TCanvas(Form("canvasResiduals_%s", fTrialName.Data()), Form("canvasRatio_%s", fTrialName.Data()), 600, 600);
    canvasRatio->SetLeftMargin(0.15);
    rooPlotRatio->GetYaxis()->SetTitleOffset(1.4);
    rooPlotRatio->Draw();

    RooHist* rooHistPull = fRooPlot->pullHist();
    RooPlot* rooPlotPull = fRooMass.frame(Title("Pull Distribution")) ;
    rooPlotPull->addPlotable(rooHistPull,"P") ;
    canvasResiduals = new TCanvas(Form("canvasPull_%s", fTrialName.Data()), Form("canvasResiduals_%s", fTrialName.Data()), 600, 600);
    canvasResiduals->SetLeftMargin(0.15);
    rooPlotPull->GetYaxis()->SetTitleOffset(1.4);
    rooPlotPull->Draw();
  } else {
    gStyle->SetOptStat(0);

    fHist->SetTitle("");
    fHist->GetXaxis()->SetTitle("#it{M}_{#it{l^{+}}#it{l^{-}}} GeV/#it{c^{2}}");
    fHist->GetYaxis()->SetTitle("Entries");
    fHist->SetMarkerStyle(24);
    fHist->SetMarkerSize(0.5);
    fHist->SetMarkerColor(kBlack);
    fHist->SetLineColor(kBlack);

    fFuncTot->SetRange(fMinFitRange, fMaxFitRange);
    fFuncTot->SetLineColor(kRed);
    fFuncTot->SetLineStyle(kSolid);

    fFuncBkg->SetRange(fMinFitRange, fMaxFitRange);
    fFuncBkg->SetLineColor(kGray+1);
    fFuncBkg->SetLineStyle(kDotted);

    fFuncSig->SetRange(fMinFitRange, fMaxFitRange);
    fFuncSig->SetLineColor(kBlue);
    fFuncSig->SetLineStyle(kDashed);

    fHistRatio = (TH1F*) fHist->Clone("histRatio");
    fHistRatio->Sumw2(1);
    fHistRatio->SetTitle("");
    fHistRatio->GetXaxis()->SetTitle("#it{M}_{#it{l^{+}}#it{l^{-}}} GeV/#it{c^{2}}");
    fHistRatio->GetYaxis()->SetTitle("Data / Fit");
    fHistRatio->Divide(fFuncTot);
    fHistRatio->GetYaxis()->SetRangeUser(0., 2.);
    fHistRatio->SetMarkerStyle(20);
    fHistRatio->SetMarkerSize(0.5);

    fHistResiduals = (TH1F*) fHist->Clone("histResiduals");
    fHistResiduals->SetTitle("");
    fHistResiduals->GetXaxis()->SetTitle("#it{M}_{#it{l^{+}}#it{l^{-}}} GeV/#it{c^{2}}");
    fHistResiduals->GetYaxis()->SetTitle("Residuals");
    fHistResiduals->SetMarkerStyle(20);
    fHistResiduals->SetMarkerSize(0.5);
    fHistBkg = (TH1F*) fHist->Clone("histBkg");
    for (int iBin = fHist->FindBin(fMinFitRange);iBin < fHist->FindBin(fMaxFitRange);iBin++){
      fHistBkg->SetBinContent(iBin,fFuncBkg->Eval(fHistResiduals->GetBinCenter(iBin)));
      fHistBkg->SetBinError(iBin,TMath::Sqrt(fFuncBkg->Eval(fHistResiduals->GetBinCenter(iBin))));
    }
    fHistResiduals->Add(fHistBkg, -1);
    fHistResiduals->GetYaxis()->SetRangeUser(1.5*fHistResiduals->GetMinimum(),1.5*fHistResiduals->GetMaximum());

    // Draw fit results
    TPaveText *paveText = new TPaveText(0.60,0.60,0.99,0.99,"brNDC");
    paveText->SetTextFont(42);
    paveText->SetTextSize(0.025);
    paveText->AddText(Form("#bf{STATUS = %s}", fFitStatus.Data()));
    paveText->AddText(Form("#chi^{2}/NDF = %3.2f", fChiSquareNDF));
    paveText->AddText(Form("S = %1.0f #pm %1.0f", fSignal, fErrorSignal));
    for (int iPar = 0;iPar < fNParams;iPar++) {
      paveText->AddText(Form("%s = %4.3f #pm %4.3f", fFuncTot->GetParName(iPar), fFuncTot->GetParameter(iPar), fFuncTot->GetParError(iPar)));
    }

    canvasFit = new TCanvas(Form("canvasFit_%s", fTrialName.Data()), Form("canvasFit_%s", fTrialName.Data()), 600, 600);
    canvasFit->SetLeftMargin(0.15);
    fHist->Draw("EP");
    fFuncBkg->Draw("same");
    fFuncSig->Draw("same");
    fFuncTot->Draw("same");
    paveText->Draw();

    // Draw Ratio Data / Fit
    TLine *lineUnity = new TLine(fMinFitRange, 1.,fMaxFitRange, 1.);
    lineUnity->SetLineStyle(kDashed);
    lineUnity->SetLineWidth(2);
    lineUnity->SetLineColor(kRed);

    canvasRatio = new TCanvas(Form("canvasRatio_%s", fTrialName.Data()), Form("canvasRatio_%s", fTrialName.Data()), 600, 600);
    fHistRatio->Draw("E0");
    lineUnity->Draw("same");

    // Draw Residuals Data / Fit Bkg
    TLine *lineZero = new TLine(fMinFitRange, 0.,fMaxFitRange, 0.);
    lineZero->SetLineStyle(kDashed);
    lineZero->SetLineWidth(2);
    lineZero->SetLineColor(kRed);

    canvasResiduals = new TCanvas(Form("canvasResiduals_%s", fTrialName.Data()), Form("canvasResiduals_%s", fTrialName.Data()), 600, 600);
    fHistResiduals->Draw("E0");
    lineZero->Draw("same");
  }

  // Save fHistResults and canvas into the output file
  trialDir->cd();
  canvasFit->Write();
  canvasRatio->Write();
  canvasResiduals->Write();
  fHistResults->Write();
  delete canvasFit;
  delete canvasRatio;
  delete canvasResiduals;
}
//______________________________________________________________________________
void DQFitter::SetUserPDF(const char *nameFunc[3], Int_t nParams[3]) {
  fRooMass = RooRealVar("m", "#it{M} (GeV/#it{c}^{2})", 2, 5);
  gROOT->ProcessLineSync(Form(".x fit_library/%s.cxx+", nameFunc[0]));
  gROOT->ProcessLineSync(Form(".x fit_library/%s.cxx+", nameFunc[1]));

  TString nameFuncSig = nameFunc[0];
  nameFuncSig += Form("::%s(m[2,5]", nameFunc[0]);
  for(int iPar = 0;iPar < nParams[0];iPar++){nameFuncSig += Form(",%s[%f,%f,%f]", fParamNames[iPar].Data(), fParams[iPar], fMinParamLimits[iPar], fMaxParamLimits[iPar]);}
  nameFuncSig += ")";
  fRooWorkspace.factory(nameFuncSig);

  TString nameFuncBkg = nameFunc[1];
  nameFuncBkg += Form("::%s(m[2,5]", nameFunc[1]);
  for(int iPar = nParams[0];iPar < nParams[0]+nParams[1];iPar++){nameFuncBkg += Form(",%s[%f,%f,%f]", fParamNames[iPar].Data(), fParams[iPar], fMinParamLimits[iPar], fMaxParamLimits[iPar]);}
  nameFuncBkg += ")";
  fRooWorkspace.factory(nameFuncBkg);

  TString nameFuncSigBkg = nameFunc[2];
  nameFuncSigBkg += "::sum(";
  for(int iPar = nParams[0]+nParams[1];iPar < nParams[0]+nParams[1]+nParams[2];iPar++){
    if(iPar == nParams[0]+nParams[1]){nameFuncSigBkg += Form("%s[%f,%f,%f]*%s", fParamNames[iPar].Data(), fParams[iPar], fMinParamLimits[iPar], fMaxParamLimits[iPar],nameFunc[iPar-(nParams[0]+nParams[1])]);}
    else{nameFuncSigBkg += Form(",%s[%f,%f,%f]*%s", fParamNames[iPar].Data(), fParams[iPar], fMinParamLimits[iPar], fMaxParamLimits[iPar],nameFunc[iPar-(nParams[0]+nParams[1])]);}
  }
  nameFuncSigBkg += ")";
  fRooWorkspace.factory(nameFuncSigBkg);
}
//______________________________________________________________________________
void DQFitter::UnbinnedFitInvMassSpectrum(TString trialName) {
  fDoRooFit = kTRUE;
  fTrialName = trialName;

  // Init the histogram with the fit results
  fHistResults = new TH1F("histResults", "", fNParams, 0., fNParams);
  for (int iPar = 0;iPar < fNParams;iPar++) {
    fHistResults->GetXaxis()->SetBinLabel(iPar+1, fParamNames[iPar]);
  }

  // Retrieving objects from workspace
  fRooWorkspace.Print();
  auto pdf = fRooWorkspace.pdf("sum");

  fRooPlot = fRooMass.frame(Title(Form("canvasFit_%s", fTrialName.Data())));

  RooDataSet rooDs("data", "data", RooArgSet(fRooMass), Import(*fTree));
  pdf->fitTo(rooDs);

  auto test = fRooWorkspace.var("mean");

  rooDs.plotOn(fRooPlot);
  pdf->plotOn(fRooPlot);
  pdf->paramOn(fRooPlot, Layout(0.55));

  // User entries of the results histogram
  for (int iPar = 0;iPar < fNParams;iPar++) {
    fHistResults->SetBinContent(iPar+1, fRooWorkspace.var(fParamNames[iPar])->getVal());
    fHistResults->SetBinError(iPar+1, fRooWorkspace.var(fParamNames[iPar])->getError());
  }
  if (!fFile) {
    printf("\n************ WARNING: no output file! ************\n");
  } else {
    SaveResults();
  }
}
