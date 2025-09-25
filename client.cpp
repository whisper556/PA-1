/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Arul Dsena
	UIN: 733006903
	Date: 09/22/2025
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <chrono>


using namespace std;



int main (int argc, char *argv[]) {

	int opt;
	int p = -1;
	double t = -1.0;
	int e = -1;
	int m = MAX_MESSAGE;
	bool new_chan = false;
	vector<FIFORequestChannel*> channels;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi (optarg);
				break;
			case 'c':
				new_chan = true;
				break;
		}
	}

	
    pid_t pid = fork();
    if (pid == 0) {
        // child process -> run the server
        char *args[] = {(char*)"./server", NULL};
        execvp(args[0], args);
        cerr << "Error executing server" << endl;
        return 1;
    } else if (pid < 0) {
        cerr << "Failed to fork server process" << endl;
        return 1;
    }

	FIFORequestChannel cont_chan("control", FIFORequestChannel::CLIENT_SIDE);
	channels.push_back(&cont_chan);

	FIFORequestChannel chan = *channels.back();

	if(new_chan){
		//create a variable to hold the name 
		// cread response from the server 
		// call the FIFORequestChannel constructor with the name from the server
		// Push the new channel into the vector  


		MESSAGE_TYPE nc = NEWCHANNEL_MSG;
		cont_chan.cwrite(&nc, sizeof(NEWCHANNEL_MSG));

		char buf[256];
		cont_chan.cread(buf, sizeof(buf));
		string new_name(buf);

		FIFORequestChannel* new_channel_ptr = new FIFORequestChannel(new_name , FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(new_channel_ptr);
		chan = *new_channel_ptr;
		
	}

	

	if(p != -1 && e != -1 && t != -1.0 ){
		datamsg x(p,t,e);
		chan.cwrite(&x, sizeof(datamsg));

		double reply;
		chan.cread(&reply, sizeof(double));

		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;


	}
	else if (p != -1) {
		// 2) First 1000 data points for ECG1 and ECG2 -> x1.csv
		ofstream fout("received/x1.csv", ios::binary);
		if (!fout) { cerr << "Cannot create x1.csv\n"; return 1; }

		for (int i = 0; i < 1000; i++) {
			double time = i * 0.004;
			double ecg1, ecg2;

			datamsg msg1(p, time, 1);
			chan.cwrite(&msg1, sizeof(datamsg));
			chan.cread(&ecg1, sizeof(double));

			datamsg msg2(p, time, 2);
			chan.cwrite(&msg2, sizeof(datamsg));
			chan.cread(&ecg2, sizeof(double));

			fout << time <<","<< ecg1 << "," << ecg2 << "\n";
		}

		fout.close();
		cout << "x1.csv generated for patient " << p << endl;
   } 
	else if(!filename.empty()){
		filemsg fm(0,0);
		int len = sizeof(filemsg) + filename.size() + 1;
		char *buf2 = new char[len];
	    memcpy(buf2, &fm, sizeof(filemsg));
        strcpy(buf2 + sizeof(filemsg), filename.c_str());
        chan.cwrite(buf2, len);

        __int64_t file_size;
        chan.cread(&file_size, sizeof(__int64_t));

		char* buf3 = new char[m]; //create buffer size of buffer capacity (m)


        cout << "File " << filename << " has " << file_size << " bytes" << endl;

		ofstream outfile(("received/" + filename).c_str(), ios::binary);
		if (!outfile){
			cerr << "Error opening output file";
		}

		__int64_t offset = 0;
		while(offset < file_size){
			int chunk_size = min(m, static_cast<int>(file_size - offset));
			filemsg* file_req = (filemsg*)buf2;
		    file_req->offset = offset; //set offset in file
		    file_req->length = chunk_size; //set the length be careful of the last segment

			chan.cwrite(buf2,len);
			chan.cread(buf3, chunk_size);

			outfile.write(buf3, chunk_size);
			offset += chunk_size;
		}

		outfile.close();
		delete[] buf2;
		delete[] buf3;
	}




    
	
	/* // example data point request
    char buf[MAX_MESSAGE]; // 256
    datamsg x(p, t, e);
	
	memcpy(buf, &x, sizeof(datamsg));
	chan.cwrite(buf, sizeof(datamsg)); // question
	double reply;
	chan.cread(&reply, sizeof(double)); //answer
	cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	
    // sending a non-sense message, you need to change this
	filemsg fm(0, 0);
	string fname = "teslkansdlkjflasjdf.dat";
	
	int len = sizeof(filemsg) + (fname.size() + 1);
	char* buf2 = new char[len];
	memcpy(buf2, &fm, sizeof(filemsg));
	strcpy(buf2 + sizeof(filemsg), fname.c_str());
	chan.cwrite(buf2, len);  // I want the file length;

	delete[] buf2;
	delete[] buf3;
	 */


	// closing the channels
	for (FIFORequestChannel* chan_ptr : channels){
		 MESSAGE_TYPE m = QUIT_MSG;
         chan_ptr->cwrite(&m, sizeof(MESSAGE_TYPE));

	}   
   
	int status;
	wait(&status);

	for(size_t i = 1; i<channels.size(); i++ ){
		delete channels[i];
	}

	return 0;
}
