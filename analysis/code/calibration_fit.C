// ----------------------- //
// DrawMCA.C -- root macro
// Author: K. Mizukoshi
// Date  : Apr. 12 2019
// Cs-137 data collected
// by MCA module
// ----------------------- //

int calibration_fit(){

	const int MCACh = 8191;
	const int header = 12;
	const int NFILES = 2;

	TH1D* hist = new TH1D("hist",";ch;Counts",MCACh,0,MCACh);

	string str;

	double sum[MCACh] = {0};
	double plot[MCACh] = {0};

	for(int ifile = 0; ifile < NFILES; ++ifile) {

		ifstream ifs(Form("/home/msgc/MCA8000D/data/20251106_calibration/per0/SN794_%d.mca",ifile));
		// cout << Form("/home/msgc/MCA8000D/data/20251104_calibration/per0_137Cs/SN794_%d.mca",ifile) << endl; 
		
		for(int ich=0; ich<header; ++ich){
		//		ifs >> sval;	
			getline(ifs,str);
			cerr << str << endl;
		}

		double val;

		for(int ich = 0; ich < MCACh; ++ich){
			ifs >> val;

			if (ich < 10) 
			cout << ich <<"\t"<< val << endl;

			sum[ich] += val;
			plot[ich] = (sum[ich] ) / (NFILES * 60);
			// plot[ich] = (sum[ich] * 1.13414) / (NFILES * 60);
		}
	}

	for(int ich = 0; ich < MCACh; ++ich) {	
		// double ienergy = (ich - 47.7106) / 1.13414;
		hist->SetBinContent(ich, plot[ich]);
	}

	hist->Draw();
	hist->SaveAs("");

	TF1 *fitFunc1 = new TF1("fitFunc_high", "[0]*exp(-0.5*((x-[1])/[2])**2) + [3]*x + [4]", 650, 840);

    double p0_amplitude_1 = 1;
    double p1_mean_1      = 750;
    double p2_sigma_1     = 60;
    double p3_slope_1     = -0.001;
    double p4_offset_1    = 1;


	fitFunc1->SetParameters(p0_amplitude_1, p1_mean_1, p2_sigma_1, p3_slope_1, p4_offset_1);

    hist->Fit(fitFunc1, "R");

 return 0;
    
 }
