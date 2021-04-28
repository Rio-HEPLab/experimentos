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

void MakeTree(TString const& input, TString const& output, TString const& output_root = "TreeTDC.root"){
	
	FILE *data;
	
        data = fopen(input,"rt");
	if(data == NULL){
		printf("Arquivo input não encontrado");
		exit(0);
	}
	
	char urlNull[]="Null.txt";

	FILE *arqNull;
	arqNull = fopen(urlNull,"w");
	if(arqNull == NULL){
		printf("Não foi possivel abrir o arquivo NULL\n");
		exit(0);
	}	

	FILE *arq;
        arq = fopen(output,"w"); 
	if(arq == NULL){
		printf("Não foi possivel abrir o arquivo output\n");
		exit(0);
	}
	
	TFile *file = new TFile(output_root,"recreate");
   	TTree *t1 = new TTree("t1","Tree com dados TDC");
	
	bool use_all_channels = false;
	
        // char channels[3] = { 0, 1, 2 };
        char channels[3] = { 1, 0, 2 };

	std::vector<TString> channel_names;
	channel_names.push_back( "TDC1" );
	channel_names.push_back( "TRIG" );

	int event, prev_event, eventN, channel0, channel1, channel2, tdc0, tdc1, tdc2, tdc;
        eventN = 0; 
        const int NSEARCHMAX = 100;
        event = -1;
        prev_event = -1;
        bool valid = true;
        fpos_t pos;

	t1->Branch("event",&event,"event/I");
   	t1->Branch("tdc",&tdc,"tdc/I");
	
	std::vector<int*> tdc_ptr_;
        tdc_ptr_.push_back( &tdc0 );
        tdc_ptr_.push_back( &tdc1 );
        if( use_all_channels ) tdc_ptr_.push_back( &tdc2 );

	TString header_ = "EVENT\t";
	for( auto begin_ = channel_names.begin(), end_ = channel_names.end(), it_ = begin_; it_ != end_; ++it_ ){
	   auto const& name_ = *it_;

   	   t1->Branch( name_, tdc_ptr_[ it_ - begin_ ], name_ + "/I" );

	   header_ += name_;
           if( it_ < ( end_ - 1 ) ) header_ += "\t";
           else                     header_ += "\n";
	} 
	fprintf( arq, header_ );

	while( !feof(data) ){

                int ret = -1;
                
                if ( !valid ) {
                   cout << "Find correct start line." << endl;
                   for(int i=0; i < NSEARCHMAX; ++i) {
                      cout << "  Iteration " << (i+1) << endl;
                      cout << "  Prev. event " << prev_event << endl;
                      ret = fscanf(data,"%d", &event);
                      if ( event == (prev_event + 1) ) { valid = true; break; }
                   }
                   if ( !valid ) continue;
                } else{
                   prev_event = event;
                   fgetpos( data, &pos ); 
   		   ret = fscanf(data,"%d", &event);
                   if( feof(data) ) break;

                   if( event == 0 ) prev_event = 0;
                   else if ( event != (prev_event + 1) ) { valid = false; fsetpos( data, &pos ); continue; }
                }

                fgetpos( data, &pos );
		ret = fscanf(data,"%d %d", &channel0, &tdc0);
                if ( channel0 != channels[0] ) { valid = false; fsetpos( data, &pos ); prev_event = event; continue; }

                fgetpos( data, &pos ); 
		ret = fscanf(data,"%d %d", &channel1, &tdc1);
                if ( channel1 != channels[1] && channel1 != channels[2] ) { valid = false; fsetpos( data, &pos ); prev_event = event; continue; }

                if( use_all_channels ){
                   fgetpos( data, &pos ); 
		   ret = fscanf(data,"%d %d", &channel2, &tdc2);
                   if ( channel2 != 1 && channel2 != 2 ) { valid = false; fsetpos( data, &pos ); prev_event = event; continue; }
                }

		tdc = ( use_all_channels ) ? abs( (tdc2-tdc1)*25 ) : (tdc1-tdc0)*25;
		
		if( tdc0 == 0 || tdc1 == 0 || ( (use_all_channels) && tdc2 == 0 ) ){
			(eventN++);
			fprintf(arqNull, "Evento Nulo: %d Event: %d TDC0: %d TDC1: %d\n",eventN,event,tdc0,tdc1);
			 }
		 else{
			// fprintf(arq,"Evento: %d TDC: %d \n", event, tdc);
			if( use_all_channels )
			   fprintf( arq,"%d\t%d\t%d\t%d\n", event, tdc0, tdc1, tdc2);
			else
			   fprintf( arq,"%d\t%d\t%d\n", event, tdc0, tdc1);

			t1->Fill();
	        }

        }
	t1->Write();
   	t1->Print();
   	file->Close();

        fclose(data);
	fclose(arq);
	fclose(arqNull);
	return ;

}
