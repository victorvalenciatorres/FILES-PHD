from telnetlib import DO
import ROOT
from ROOT import TCanvas, TFile, TH1F, TPaveText, TLatex, RooRealVar, RooDataSet, RooWorkspace, RooDataHist, RooArgSet
from ROOT import gPad, gROOT, gStyle, kRed, kBlue, kGreen
from utils.plot_library import DoResidualPlot, DoPullPlot, DoCorrMatPlot, DoAlicePlot, LoadStyle

class DQFitter:
    def __init__(self, fInName, fInputName, fOutPath):
        self.fPdfDict          = {}
        self.fOutPath          = fOutPath
        self.fFileOut          = TFile("{}{}.root".format(fOutPath, fInputName), "RECREATE")
        self.fFileIn           = TFile.Open(fInName)
        self.fInputName        = fInputName
        self.fInput            = 0
        self.fRooWorkspace     = RooWorkspace('w','workspace')
        self.fParNames         = []
        self.fFitMethod        = "likelyhood"
        self.fFitRangeMin      = []
        self.fFitRangeMax      = []
        self.fTrialName        = ""
        self.fRooMass          = RooRealVar("m", "#it{M} (GeV/#it{c}^{2})", 2, 5)
        self.fDoResidualPlot   = True
        self.fDoPullPlot       = True
        self.fDoCorrMatPlot    = False

    def SetFitConfig(self, pdfDict):
        '''
        Method set the configuration of the fit
        '''
        self.fPdfDict = pdfDict
        self.fInput = self.fFileIn.Get(self.fInputName)
        self.fInput.Sumw2()
        self.fFitMethod = pdfDict["fitMethod"]
        self.fFitRangeMin = pdfDict["fitRangeMin"]
        self.fFitRangeMax = pdfDict["fitRangeMax"]
        self.fDoResidualPlot = pdfDict["doResidualPlot"]
        self.fDoPullPlot = pdfDict["doPullPlot"]
        self.fDoCorrMatPlot = pdfDict["doCorrMatPlot"]
        pdfList = []
        for pdf in self.fPdfDict["pdf"][:-1]:
            self.fTrialName = self.fTrialName + pdf + "_"
        
        for i in range(0, len(self.fPdfDict["pdf"])):
            if not self.fPdfDict["pdf"][i] == "SUM":
                gROOT.ProcessLineSync(".x ../fit_library/{}Pdf.cxx+".format(self.fPdfDict["pdf"][i]))
        
        for i in range(0, len(self.fPdfDict["pdf"])):
            parVal = self.fPdfDict["parVal"][i]
            parLimMin = self.fPdfDict["parLimMin"][i]
            parLimMax = self.fPdfDict["parLimMax"][i]
            parName = self.fPdfDict["parName"][i]
            parFix = self.fPdfDict["parFix"][i]

            if not self.fPdfDict["pdf"][i] == "SUM":
                # Filling parameter list
                for j in range(0, len(parVal)):
                    if ("sum" in parName[j]) or ("prod" in parName[j]):
                        self.fRooWorkspace.factory("{}".format(parName[j]))
                        # Replace the exression of the parameter with the name of the parameter
                        r1 = parName[j].find("::") + 2
                        r2 = parName[j].find("(", r1)
                        parName[j] = parName[j][r1:r2]
                        if (parFix[j] == True):
                            self.fRooWorkspace.factory("{}[{}]".format(parName[j], parVal[j]))
                    else:
                        if (parFix[j] == True):
                            self.fRooWorkspace.factory("{}[{}]".format(parName[j], parVal[j]))
                        else:
                            self.fRooWorkspace.factory("{}[{},{},{}]".format(parName[j], parVal[j], parLimMin[j], parLimMax[j]))

                        self.fParNames.append(parName[j]) # only free parameters will be reported in the histogram of results

                # Define the pdf associating the parametes previously defined
                nameFunc = self.fPdfDict["pdf"][i]
                nameFunc += "Pdf::{}Pdf(m[{},{}]".format(self.fPdfDict["pdfName"][i],2,5)
                pdfList.append(self.fPdfDict["pdfName"][i])
                for j in range(0, len(parVal)):
                    nameFunc += ",{}".format(parName[j])
                nameFunc += ")"
                self.fRooWorkspace.factory(nameFunc)
            else:
                nameFunc = self.fPdfDict["pdf"][i]
                nameFunc += "::sum("
                for j in range(0, len(pdfList)):
                    nameFunc += "{}[{},{},{}]*{}Pdf".format(parName[j], parVal[j], parLimMin[j], parLimMax[j], pdfList[j])
                    self.fParNames.append(parName[j])
                    if not j == len(pdfList) - 1:
                        nameFunc += ","
                nameFunc += ")"
                self.fRooWorkspace.factory(nameFunc)

        #self.fRooWorkspace.Print()

    def FitInvMassSpectrum(self, fitMethod, fitRangeMin, fitRangeMax):
        '''
        Method to perform the fit to the invariant mass spectrum
        '''
        LoadStyle()
        trialName = self.fTrialName + "_" + str(fitRangeMin) + "_" + str(fitRangeMax)
        self.fRooWorkspace.Print()
        pdf = self.fRooWorkspace.pdf("sum")
        self.fRooMass.setRange("range", fitRangeMin, fitRangeMax)
        fRooPlot = self.fRooMass.frame(ROOT.RooFit.Title(trialName), ROOT.RooFit.Range("range"))
        #fRooPlot = self.fRooMass.frame(ROOT.RooFit.Title(trialName))

        print(">>>>>>>>>>>>>>>>>>>> ~~~~~~~~~Number of bins: ", fRooPlot.GetNbinsX(), " for fit range : ",fitRangeMin,fitRangeMax)

        fRooPlotOff = self.fRooMass.frame(ROOT.RooFit.Title(trialName))
        if "TTree" in self.fInput.ClassName():
            print("########### Perform unbinned fit ###########")
            rooDs = RooDataSet("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))
            
        else:
            print("########### Perform binned fit ###########")
            rooDs = RooDataHist("data", "data", RooArgSet(self.fRooMass), ROOT.RooFit.Import(self.fInput))
            print(">>>>>dataset<<<<<")
            rooDs.Print("V")
            print("Weight:", rooDs.isWeighted())
            print("WeightPois:", rooDs.isNonPoissonWeighted())
            print("weights: ", rooDs.weight())


        # Select the fit method
        if fitMethod == "likelyhood":
            print("########### Perform likelyhood fit ###########")
            #rooFitRes = ROOT.RooFitResult(pdf.fitTo(rooDs, ROOT.RooFit.Extended(ROOT.kTRUE), ROOT.RooFit.Save()))
            rooFitRes = ROOT.RooFitResult(pdf.fitTo(rooDs, ROOT.RooFit.Range(fitRangeMin,fitRangeMax),ROOT.RooFit.PrintLevel(-1), ROOT.RooFit.Save()))
        if fitMethod == "chi2":
            print("########### Perform X2 fit ###########")
            rooFitRes = ROOT.RooFitResult(pdf.chi2FitTo(rooDs, ROOT.RooFit.Save()))

        index = 1
        histResults = TH1F("fit_results_{}".format(trialName), "fit_results_{}".format(trialName), len(self.fParNames), 0., len(self.fParNames))
        for parName in self.fParNames:
            histResults.GetXaxis().SetBinLabel(index, parName)
            histResults.SetBinContent(index, self.fRooWorkspace.var(parName).getVal())
            histResults.SetBinError(index, self.fRooWorkspace.var(parName).getError())
            index += 1

      
        rooDs.plotOn(fRooPlot, ROOT.RooFit.MarkerStyle(20), ROOT.RooFit.MarkerSize(0.6), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        pdf.plotOn(fRooPlot, ROOT.RooFit.VisualizeError(rooFitRes, 1), ROOT.RooFit.FillColor(ROOT.kRed-10), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        rooDs.plotOn(fRooPlot, ROOT.RooFit.MarkerStyle(20), ROOT.RooFit.MarkerSize(0.6), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        pdf.plotOn(fRooPlot, ROOT.RooFit.LineColor(ROOT.kRed+1), ROOT.RooFit.LineWidth(2), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        for i in range(0, len(self.fPdfDict["pdf"])):
            if not self.fPdfDict["pdfName"][i] == "SUM":
                pdf.plotOn(fRooPlot, ROOT.RooFit.Components("{}Pdf".format(self.fPdfDict["pdfName"][i])), ROOT.RooFit.LineColor(self.fPdfDict["pdfColor"][i]), ROOT.RooFit.LineStyle(self.fPdfDict["pdfStyle"][i]), ROOT.RooFit.LineWidth(2), ROOT.RooFit.Range(fitRangeMin, fitRangeMax))
        
        extraText = [] # extra text for "propaganda" plots

        paveText = TPaveText(0.60, 0.45, 0.99, 0.94, "brNDC")
        paveText.SetTextFont(42)
        paveText.SetTextSize(0.025)
        paveText.SetFillColor(ROOT.kWhite)
        for parName in self.fParNames:
            paveText.AddText("{} = {:.4f} #pm {:.4f}".format(parName, self.fRooWorkspace.var(parName).getVal(), self.fRooWorkspace.var(parName).getError()))
            if self.fPdfDict["parForAlicePlot"].count(parName) > 0:
                text = self.fPdfDict["parNameForAlicePlot"][self.fPdfDict["parForAlicePlot"].index(parName)]
                if "sig" in parName:
                    extraText.append("{} = {:.0f} #pm {:.0f}".format(text, self.fRooWorkspace.var(parName).getVal(), self.fRooWorkspace.var(parName).getError()))
                else:
                    extraText.append("{} = {:.3f} #pm {:.3f}".format(text, self.fRooWorkspace.var(parName).getVal(), self.fRooWorkspace.var(parName).getError()))
            for i in range(0, len(self.fPdfDict["pdfName"])):
                if self.fPdfDict["pdfName"][i] in parName:
                    (paveText.GetListOfLines().Last()).SetTextColor(self.fPdfDict["pdfColor"][i])

        
        #n = self.fRooWorkspace.var("sig_Jpsi").getVal()
        #fRooPlot.Print("V")  




        #Fit with RooChi2Var
        nbinsperGev = rooDs.numEntries() / (self.fPdfDict["fitRangeMax"][0] - self.fPdfDict["fitRangeMin"][0])
        # Print the number of bins
        print("Number of bins per Gev:",nbinsperGev )
        Nbinss = (fitRangeMax - fitRangeMin)*nbinsperGev
        chi2 = ROOT.RooChi2Var("chi2", "chi2", pdf, rooDs)
        ndof = Nbinss - rooFitRes.floatParsFinal().getSize()
        reduced_chi2 = chi2.getVal() / ndof
        print("number of bin1:",fRooPlot.GetXaxis().FindBin(fitRangeMax) - fRooPlot.GetXaxis().FindBin(fitRangeMin))
        print("number of binss:",Nbinss)
        # Print the chi-squared value
        print("Chi-squared value:", chi2.getVal())
        print("Reduced chi-squared value:", reduced_chi2)

       
        # Print the chiSquare value
        nPars = rooFitRes.floatParsFinal().getSize()
        nBins = fRooPlot.GetXaxis().FindBin(fitRangeMax) - fRooPlot.GetXaxis().FindBin(fitRangeMin)

        
        print("parameters: ",nPars," nBins: ",Nbinss)
        paveText.AddText("n Par = %3.2f" % (nPars)) 
        paveText.AddText("n Bins = %3.2f" % (nBins))
        #paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % (fRooPl.ot.chiSquare()/(nBins - nPars)))
        #paveText.AddText("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare("SUM","fit_results_{}".format(trialName),nPars)))

        paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % reduced_chi2)
        #paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % (fRooPlot.chiSquare("{}_Norm[m]_Range[{:.6f}_{:.6f}]_NormRange[]".format(pdf.GetName(),fitRangeMin,fitRangeMax),"h_{}".format(rooDs.GetName()),nPars))) #likelyhood
        #paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % (fRooPlot.chiSquare("sum_Norm[m]_Range[2.200000_4.800000]_NormRange[]",rooDs.GetName(),nPars))) #likelyhood
        #paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % (fRooPlot.chiSquare("sum_Norm[m]_Range[2.100000_4.900000]_NormRange[]","h_data",nPars))) #likelyhood
        #paveText.AddText("#bf{#chi^{2}/dof = %3.2f}" % (fRooPlot.chiSquare("sum_Norm[m]_Range[2.000000_5.000000]","h_data",nPars))) . #chi2
        
        fRooPlot.addObject(paveText)
        #extraText.append("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare(nPars)))
        #extraText.append("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare()/(nBins - nPars)))

        extraText.append("#chi^{2}/dof = %3.2f" % reduced_chi2)
        #extraText.append("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare("{}_Norm[m]_Range[{:.6f}_{:.6f}]_NormRange[]".format(pdf.GetName(),fitRangeMin,fitRangeMax),"h_{}".format(rooDs.GetName()),nPars))) #likelyhood
        #extraText.append("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare("sum_Norm[m]_Range[2.200000_4.800000]_NormRange[]","h_data",nPars))) #likelyhood
        #extraText.append("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare("sum_Norm[m]_Range[2.100000_4.900000]_NormRange[]","h_data",nPars))) #likelyhood
        #extraText.append("#chi^{2}/dof = %3.2f" % (fRooPlot.chiSquare("sum_Norm[m]_Range[2.000000_5.000000]","h_data",nPars)))



        # Fit plot
        canvasFit = TCanvas("fit_plot_{}".format(trialName), "fit_plot_{}".format(trialName), 800, 600)
        canvasFit.SetLeftMargin(0.15)
        gPad.SetLeftMargin(0.15)
        fRooPlot.GetYaxis().SetTitleOffset(1.4)
        fRooPlot.Draw()
        fRooPlot.Print("V")
        


# Create the chi2
        chi2 = pdf.createChi2(rooDs,ROOT.RooFit.Extended(ROOT.kTRUE))
        # Print the fit result
        rooFitRes.Print()
        
        # Official fit plot
        if self.fPdfDict["doAlicePlot"]:
            DoAlicePlot(rooDs, pdf, fRooPlotOff, self.fPdfDict, self.fInputName, trialName, self.fOutPath, extraText)

        # Save results
        self.fFileOut.cd()
        histResults.Write()
        canvasFit.Write()

        # Ratio plot
        if self.fDoResidualPlot:
            canvasRatio = DoResidualPlot(fRooPlot, self.fRooMass, trialName)
            canvasRatio.Write()

        # Pull plot
        if self.fDoPullPlot:
            canvasPull = DoPullPlot(fRooPlot, self.fRooMass, trialName)
            canvasPull.Write()

        # Correlation matrix plot
        if self.fDoCorrMatPlot:
            canvasCorrMat = DoCorrMatPlot(rooFitRes, trialName)
            canvasCorrMat.Write()

        rooFitRes.Write("info_fit_results_{}".format(trialName))
    
        print("Name of the data Hist: ", rooDs.GetName())
        print("Name - ", "{}_Norm[m]_Range[{:.6f}_{:.6f}]_NormRange[]".format(pdf.GetName(),fitRangeMin,fitRangeMax))
    def MultiTrial(self):
        '''
        Method to perform a multi-trial fit
        '''
        #self.FitInvMassSpectrum(self.fFitMethod, self.fFitRangeMin[0], self.fFitRangeMax[0])
        for iRange in range(0, len(self.fFitRangeMin)):
            self.FitInvMassSpectrum(self.fFitMethod, self.fFitRangeMin[iRange], self.fFitRangeMax[iRange])
        self.fFileOut.Close()
