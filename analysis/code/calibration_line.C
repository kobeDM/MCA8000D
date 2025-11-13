#include <TCanvas.h>
#include <TGraph.h>
#include <TApplication.h>
#include <vector>
#include <utility> 

void calibration_line() {

    std::vector<std::pair<double, double>> points = {
        {662, 740.828}, 
        {1170, 1281.53}, 
        {1330, 1450.47}, 
        {511, 577.2}, 
        {1274.5, 1383.94}
    };

    int n = points.size();
    std::vector<double> x(n), y(n);

    for (int i = 0; i < n; ++i) {
        x[i] = points[i].first;
        y[i] = points[i].second;
    };

    TGraph *gr = new TGraph(n, x.data(), y.data());
    gr->SetTitle("calibration_NaI;Energy (keV);ADC");
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(1.2);
    gr->SetMarkerColor(kBlue + 1);
    
    TCanvas *c = new TCanvas("c1", "Graph Example", 800, 600);
    gr->GetXaxis()->SetLimits(0, 1500);  
    gr->SetMinimum(0);
    gr->Draw("AP");

    TF1 *f1 = new TF1("fi", "[0]*x + [1]", 0, 1500);
    f1->SetLineColor(kRed);

    double slope_guess = y.back() / x.back();
    f1->SetParameter(0, slope_guess);
    f1->SetParameter(1, 1);

    gr->Fit(f1, "R");

    f1->Draw("same");


    c->SaveAs("calibration_NaI.png");


}