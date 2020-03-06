#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

static const int BUCKET_SIZE = 600;					//set bucket size for all program count

ifstream file_read;									//BWT file read stream
ifstream idx_read;									//index file read stream
vector<int*> bucket_L;								//declear bucket for store L
bool big_file = false;								//big file flag
int BUCKET_NUMBER;									//declear for store number of bucket
int *F_List = new int[127];							//declear for store F
int *C_List = new int[127];							//declear for store C

/**************************************************************
/*function set mode
/*  get file size according size set different mode for run
/*  according size comput bucket number
/*************************************************************/
void Set_Mode(){  

    file_read.seekg(0, ifstream::end);				//set read from end of file
    int file_size = file_read.tellg();				//get file size
    if(file_size > 8 * 1024 * 1024)big_file = true;	//set mode flag				
    BUCKET_NUMBER = ceil(file_size / (float)BUCKET_SIZE);

    return;
}  

/**************************************************************
/*function file scan
/*  for small file store in memory get F[]
/*  according F[] comput C[]
/*	clear file read to EOF for next read
/*************************************************************/
void Scan_file(){

	int file_char_code;
	int count = 0, i = 0, n = 0;

	file_read.seekg(0, ifstream::beg);							//set read from file beginning

	while((file_char_code = file_read.get()) != EOF){			//scan whole file until file end

		if(i != 0 && i % BUCKET_SIZE == 0){
			
			n = i / BUCKET_SIZE - 1;
			memcpy(bucket_L[n], F_List, 127 * sizeof(int));		//store into memory
		}

		F_List[file_char_code]++;
		i++;
	}

	file_read.clear();											//clear read to EOF for next read stream

	for(i = 0; i < 127; i++){									//comput C[] according F[]

		C_List[i] = count;
		count += F_List[i];
	}

	return;
}

/**************************************************************
/*function file scan for big file
/*  for big file store in index get F[]
/*  two mode here 1. no index exist make an index and get F[] C[]
/*				  2. exist index and just get F[] C[]
/*	clear file read EOF for next read
/*	open index read for big file index read from index 
/*************************************************************/
void Scan_file_4_IDX(char *idx_path){

	int file_char_code;
	int count = 0, i = 0;
	
	file_read.seekg(0, ifstream::beg);							//set read from file beginning

	idx_read.open(idx_path, ifstream::in | ifstream::binary);	//try to open index file

	if(!idx_read){												//mode 1: no index exist

		idx_read.close();

		ofstream idx_write(idx_path, ifstream::out | ifstream::binary | ifstream::app);

		while((file_char_code = file_read.get()) != EOF){		//loop for save index finish

			if(i != 0 && i % BUCKET_SIZE == 0)for(int j = 0; j < 127; j++)idx_write.write((char*)&F_List[j], sizeof(int));		
			F_List[file_char_code]++;
			i++;
		}

		file_read.clear();											//clear read to EOF for next read stream
		
		for(i = 0; i < 127; i++){								//comput C[] according F[]

			C_List[i] = count;
			count += F_List[i];
		}

		for(int j = 0; j < 127; j++)idx_write.write((char*)&C_List[j], sizeof(int));		//save C[] to index

		idx_write.close();											//close file write
		idx_read.open(idx_path, ifstream::in | ifstream::binary);	//open index read for next index read stream
	}
	else{														//mode 2: exist index

		idx_read.seekg((BUCKET_NUMBER - 1) * 127 * sizeof(int));
		for(int j = 0; j < 127; j++)idx_read.read((char*)&C_List[j], sizeof(int));			//get c[]
	}
	
	return;
}

/**************************************************************
/*function comput Occ
/*  get data from memory
/*  read BWT for complete data and return
/*************************************************************/
int Get_Occ(int letter, int end_bound){

	int bucket = end_bound / BUCKET_SIZE;
	int start_bound = bucket * BUCKET_SIZE;
	int count = bucket != 0 ? bucket_L[bucket - 1][letter] : 0;	//get front bucket record

	file_read.seekg(start_bound);								//set file read from the point

	for(int i = start_bound; i < end_bound; i++){				//add locate bucket number of letter to front bucket record

		int file_char_code = file_read.get();
		if(file_char_code == letter)count++;
	}

	return count;
}

/**************************************************************
/*function comput Occ for big file
/*  get data from index record data
/*  read BWT for complete data and return
/*************************************************************/
int Get_Occ_4_IDX(int letter, int end_bound){

	int bucket = end_bound / BUCKET_SIZE;
	int start_bound = bucket * BUCKET_SIZE;
	int count = 0;

	if(bucket != 0){											//read index from different point to get front bucket record

		idx_read.seekg((bucket - 1) * 127 * sizeof(int) + letter * sizeof(int));
		idx_read.read((char*)&count, sizeof(int));
	}

	file_read.seekg(start_bound);								//set file read from the point

	for(int i = start_bound; i < end_bound; i++){				//add locate bucket number of letter to front bucket record
	
		int file_char_code = file_read.get();
		if(file_char_code == letter)count++;
	}

	return count;
}

/**************************************************************
/*function main run function
/*  input 4 parameters as order -mode BWT_path index_path string
/*  output according mode n r a give different answers
/*************************************************************/
int main(int argc, char *argv[]){

	if(argc != 5)return 0;

	char *mode_type = argv[1];									//mode parameter
	char *Path_BWTcode = argv[2];								//BWT path parameter
	char *Path_IDX = argv[3];									//index path parameter
	char *Search_query = argv[4];								//search string parameter

	file_read.open(Path_BWTcode, ifstream::in);					//open file for read stream

	memset(F_List, 0, 127 * sizeof(int));						//initialize F[]
	memset(C_List, 0, 127 * sizeof(int));						//initialize C[]
	
	Set_Mode();													//function set mode

	if(big_file)Scan_file_4_IDX(Path_IDX);						//big file scan function
	else {														//small file scan function

		for(int i = 0; i < BUCKET_NUMBER; i++){					//initialize bucket L for save data
			
			int *bucket_node = new int[127];
			memset(bucket_node, 0, 127 * sizeof(int));
			bucket_L.push_back(bucket_node);
		}

		Scan_file();											//small file scan function
	}

	int i = strlen(Search_query);								//number of search location
	int temp_char_code = (int)Search_query[i - 1];				//first search code
	int First = C_List[temp_char_code] + 1;						//initialize First by first letter search
	int Last = C_List[temp_char_code];							//initialize Last by first letter search
	
	for(int temp_location = temp_char_code; temp_location < 127; temp_location++){	//get real Last data
		
		if(Last != C_List[temp_location]){
		
			Last = C_List[temp_location];
			break;
		}
	}

	if(big_file){												//big file search function
		
		while((First <= Last) && (i > 1)){
		
			temp_char_code = Search_query[i - 2];
			First = C_List[temp_char_code] + Get_Occ_4_IDX(temp_char_code, First - 1) + 1;	//Occ index search mode
			Last = C_List[temp_char_code] + Get_Occ_4_IDX(temp_char_code, Last);
			i--;		
		}	
	}
	else{														//small file search function
	
		while((First <= Last) && (i > 1)){
	
			temp_char_code = Search_query[i - 2];
			First = C_List[temp_char_code] + Get_Occ(temp_char_code, First - 1) + 1;		//Occ memory search mode
			Last = C_List[temp_char_code] + Get_Occ(temp_char_code, Last);
			i--;
		}
	}

	First--;													//unify BWT algorithm First and real First
	Last--;														//unify BWT algorithm Last and real First

	if(mode_type[1] != 'n'){									//mode r or a
		
		vector<unsigned int> offset_L;
		unsigned int temp_offset;
		char temp_bit_offset;

		for(int i = First; i <= Last; i++){						//search to begin get offset

			file_read.seekg(i);									

			string offset_number = "";
			bool find_flag = false;

			int letter_position = i;

			while(true){										//search until get offset or reach repeat offset
				
				int letter = file_read.get();

				if(letter == ']')find_flag = true;				//start record offset number
				else if(letter == '['){

					stringstream change2int;					//change type
					change2int << offset_number;
					change2int >> temp_offset;
					offset_L.push_back(temp_offset);			//push offset number to list
					break;										//get offset break
				}
				else if(find_flag){
			
					temp_bit_offset = (char)letter;
					offset_number = temp_bit_offset + offset_number;	//splice offset from string type
				}

				if(big_file)letter_position = C_List[letter] + Get_Occ_4_IDX(letter, letter_position);	//big file Occ function from index
				else letter_position = C_List[letter] + Get_Occ(letter, letter_position);				//small file Occ function from memory

				if((letter_position >= First) && (letter_position <= Last))break;	//reach repeat offset break

				file_read.seekg(letter_position);				//for next file read jump start point
			}
		
		}

		if(mode_type[1] == 'r')cout<<offset_L.size()<<endl;		//mode r
		else if (mode_type[1] == 'a'){							//mode a
			
			sort(offset_L.begin(), offset_L.end());
			for(unsigned int i = 0; i < offset_L.size(); i++)cout<<"["<<offset_L[i]<<"]"<<endl;	//export offset
		}
	}
	else cout<<(Last - First + 1)<<endl;						//mode n

	if(big_file)idx_read.close();								//index file read close
	file_read.close();											//BWT file read close

	return 0;
}