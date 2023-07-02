#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TAxis.h>

void Mass_runbyrun() {

  // Create TGraphErrors object
  TGraphErrors* graph = new TGraphErrors();

  // Set coordinates and errors for 8 points
  double x[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  double y[8] = {3.075, 3.080, 3.078, 3.068, 3.076, 3.082, 3.081, 3.075};
  double ex[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  double ey[8] = {0.001, 0.003, 0.004, 0.007, 0.003, 0.002, 0.002, 0.002};

  // Set labels for x-axis
  const char* labels[8] = {"528232", "528105", "528109", "528097", "528026", "528021", "527979", "527978"};
  for (int i = 0; i < 8; i++) {
    graph->SetPoint(i, x[i], y[i]);
    graph->SetPointError(i, ex[i], ey[i]);
  }

  // Set marker size and style
  graph->SetMarkerSize(0.6);
  graph->SetMarkerStyle(8);
    
  // Create canvas and draw graph
  TCanvas* canvas = new TCanvas("canvas", "Graph Canvas", 800, 600);
  graph->GetYaxis()->SetTitleOffset(1.3);
  graph->GetXaxis()->SetLabelOffset(-999);  //put away the numbers...
  graph->Draw("AP");
  
  // Set y-axis range
  graph->GetYaxis()->SetRangeUser(2.95,3.2);

  // Set axis labels and title
  graph->GetYaxis()->SetTitle("m_{J/#psi} GeV/c^{2}");
 

  double ymin = graph->GetHistogram()->GetMinimum();
  double ymax = graph->GetHistogram()->GetMaximum();
  

  // Display x-axis labels below each point
  for (int i = 0; i < 8; i++) {
    double x, y;
    graph->GetPoint(i, x, y);
    TLatex label;
    label.SetTextAlign(22);
    label.SetTextSize(0.035);
    label.DrawLatex(x, ymin - 0.06*(ymax-ymin), labels[i]);
  }

    // Add a blue line with big size at y=3.1
  TLine* line = new TLine(graph->GetXaxis()->GetXmin(), 3.0969, graph->GetXaxis()->GetXmax(), 3.0969 );
  line->SetLineWidth(5);
  line->SetLineColor(kRed+2);
  line->Draw();

  // Create Legend
  TLegend* legend = new TLegend(0.6, 0.7, 0.9, 0.9);
  legend->AddEntry(graph, "Data", "p");
  legend->AddEntry(line, "PDG Value", "l");

  // Set legend style and draw
  legend->SetTextSize(0.04);
  legend->SetBorderSize(0);
  legend->SetFillStyle(1);
  legend->Draw();

  // Save plot as PDF
  canvas->SaveAs("mass_runbyrun.pdf");
}
