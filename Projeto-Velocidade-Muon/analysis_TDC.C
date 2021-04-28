#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TF1.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using std::cout;
using std::endl;

void analyiss_TDC(TString const& filename){
	

	TFile *file = new TFile(filename,"read");
   	TTree* t1 = (TTree*)file->Get("t1");

	TBranch *b_event;
   	TBranch *b_tdc;

        int event, tdc;
	t1->SetBranchAddress("event", &event, &b_event);
  	t1->SetBranchAddress("tdc", &tdc, &b_tdc);

        double tdc_min = 2000.;
        double tdc_max = 8000.;

	Long64_t nentries = t1->GetEntriesFast();

        int n_events = 0;
        double ave = 0.;
	for (Long64_t i=0; i < nentries; ++i) {
    	   t1->GetEntry(i);
           if( tdc < tdc_min) continue;   
           if( tdc > tdc_max) continue;   
           if(tdc) ave += tdc;
           ++n_events;
        }
        ave /= n_events;
        cout << "Average TDC: " << ave << endl;
 
        double res = 100.; // ps
        double hist_xmin = ave - 2000.;
        double hist_xmax = ave + 2000.;
        //double hist_xmin = 2000.;
        //double hist_xmax = 8000.;
        int n_bins = ( hist_xmax - hist_xmin )/res;
	TH1F* hist = new TH1F("histTDC", "", n_bins, hist_xmin, hist_xmax);
        hist->Sumw2(true);

	for (Long64_t i=0; i < nentries; ++i) {
    	   t1->GetEntry(i);
           hist->Fill(tdc);
        }

        int n_sigmas = 2;
        double mean = hist->GetMean();
        double sigma = hist->GetStdDev();
        double x_min = mean - n_sigmas*sigma; 
        double x_max = mean + n_sigmas*sigma;
 
        TF1* fit_function0 = new TF1("fit_function","gaus",x_min,x_max);
	hist->Fit(fit_function0,"R","");
	
        double chi2 = fit_function0->GetChisquare(); 
        int ndf = fit_function0->GetNDF();
        double par0 = fit_function0->GetParameter(0);
        double err_par0 = fit_function0->GetParError(0);
        double par1 = fit_function0->GetParameter(1);
        double err_par1 = fit_function0->GetParError(1);
        double par2 = fit_function0->GetParameter(2);
        double err_par2 = fit_function0->GetParError(2);
 	
 	hist->GetYaxis()->SetTitle("Events");
   	hist->GetXaxis()->SetTitle("#Delta t (ps)");
   	hist->Draw(); 
         
}

