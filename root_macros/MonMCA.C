// ----------------------- //
// DrawMCA.C -- root macro
// Author: K. Mizukoshi
// Date  : Apr. 12 2019
// Cs-137 data collected
// by MCA module
// ----------------------- //

//int MonMCA(){
int MonMCA(string tmp_filename){
	const int MCACh = 8191;
	const int header = 12;
	const int live_line=7;
	const int real_line=8;
	//const char tmp_filename[32]="tmp.mca";
	TCanvas *c = new TCanvas("c","",800,600);

	double val;
	string str;
	string str_live,str_real;
	//ifstream ifs("718.dat");
	//ifstream ifs("794.dat");
	while(1){
	  TH1D* hist = new TH1D("hist","hist",MCACh,0,MCACh);
	  //ifstream ifs("tmp.mca");
	  ifstream ifs(tmp_filename.c_str());
	  for(int ich=0; ich<header; ++ich){
	  //		ifs >> sval;	
	    getline(ifs,str);
	    if (ich==live_line){
	      //cerr <<"live time: "<< str<<" sec"<<endl;
	      str_live=str;
	      //cerr <<"live time: "<< str.substr(str.find_last_of(" "),6)<<" sec"<<endl;
	    }
	    if (ich==real_line){
	      str_real=str;
	      //cerr <<"real time: "<< str<<" sec"<<endl;
	    }
	    //cerr << str<<endl;
	  }
	  for(int ich=0; ich<MCACh; ++ich){
	    ifs >> val;
	    //cout<<ich<<"\t"<<val;
	    hist->SetBinContent(ich, val);
	  }
	  TText *tn = new TText(0.6*MCACh, 0.8*hist->GetMaximum(),tmp_filename.c_str());
	  TText *tl = new TText(0.6*MCACh, 0.7*hist->GetMaximum(), str_live.c_str());
	  TText *tr= new TText(0.6*MCACh, 0.6*hist->GetMaximum(), str_real.c_str());
	  //	  tl->SetTextAlign(22);
	  tn->SetTextSize(0.04);
	  tl->SetTextSize(0.04);
	  tr->SetTextSize(0.04);
	  hist->Draw();
	  tn->Draw(); 
	  tl->Draw(); 
	  tr->Draw(); 
	  c->Update();
	  ifs.close();
	  sleep(1);
	}
	return 0;
}
