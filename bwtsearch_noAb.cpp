#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include <algorithm>

using namespace std;

static const int BUCKET_SIZE = 1024;

ifstream file_read;
ifstream idx_read;
vector<int*> bucket_L;
bool big_file = false;
int BUCKET_NUMBER;
int *F_List = new int[127];
int *C_List = new int[127];

void Set_Mode(){  

    file_read.seekg(0, ifstream::end);
    int file_size = file_read.tellg();
    if(file_size > 5 * 1024 * 1024)
    	big_file = true;
    BUCKET_NUMBER = ceil(file_size / (float)BUCKET_SIZE);

    return;
}  

void Scan_file(){

	int file_char_code;
	int count = 0, i = 0, n = 0;

	file_read.seekg(0, ifstream::beg);

	while((file_char_code = file_read.get()) != EOF){

		if(i != 0 && i % BUCKET_SIZE == 0){
			
			n = i / BUCKET_SIZE - 1;
			memcpy(bucket_L[n], F_List, 127 * sizeof(int));
		}

		F_List[file_char_code]++;
		i++;
	}

	file_read.clear();

	for(i = 0; i < 127; i++){

		C_List[i] = count;
		count += F_List[i];
	}

	return;
}

void Scan_file_4_IDX(char *idx_path){

	int file_char_code;
	int count = 0, i = 0;
	
	file_read.seekg(0, ifstream::beg);

	idx_read.open(idx_path, ifstream::in | ifstream::binary);

	if(!idx_read){

		idx_read.close();

		ofstream idx_write(idx_path, ifstream::out | ifstream::binary | ifstream::app);

		while((file_char_code = file_read.get()) != EOF){

			if(i != 0 && i % BUCKET_SIZE == 0)for(int j = 0; j < 127; j++)idx_write.write((char*)&F_List[j], sizeof(int));		
			F_List[file_char_code]++;
			i++;
		}

		file_read.clear();
		idx_write.close();
		idx_read.open(idx_path, ifstream::in | ifstream::binary);

		for(i = 0; i < 127; i++){

			C_List[i] = count;
			count += F_List[i];
		}
	}
	else{

		while((file_char_code = file_read.get()) != EOF){	

			F_List[file_char_code]++;
			i++;
		}

		file_read.clear();

		for(i = 0; i < 127; i++){
			
			C_List[i] = count;
			count += F_List[i];	
		}
	}
	
	return;
}

int Get_Occ(int letter, int end_bound){

	int bucket = end_bound / BUCKET_SIZE;
	int start_bound = bucket * BUCKET_SIZE;
	int count = bucket != 0 ? bucket_L[bucket - 1][letter] : 0;

	file_read.seekg(start_bound);

	for(int i = start_bound; i < end_bound; i++){

		int file_char_code = file_read.get();
		if(file_char_code == letter)count++;
	}

	return count;
}

int Get_Occ_4_IDX(int letter, int end_bound){

	int bucket = end_bound / BUCKET_SIZE;
	int start_bound = bucket * BUCKET_SIZE;
	int count = 0;

	if(bucket != 0){

		idx_read.seekg((bucket - 1) * 127 * sizeof(int) + letter * sizeof(int));
		idx_read.read((char*)&count, sizeof(int));
	}

	file_read.seekg(start_bound);

	for(int i = start_bound; i < end_bound; i++){
	
		int file_char_code = file_read.get();
		if(file_char_code == letter)count++;
	}

	return count;
}

int main(int argc, char *argv[]){

	if(argc != 5)return 0;

	char *mode_type = argv[1];
	char *Path_BWTcode = argv[2];
	char *Path_IDX = argv[3];
	char *Search_query = argv[4];

	file_read.open(Path_BWTcode, ifstream::in);

	memset(F_List, 0, 127 * sizeof(int));
	memset(C_List, 0, 127 * sizeof(int));
	
	Set_Mode();

	if(big_file)Scan_file_4_IDX(Path_IDX);
	else {

		for(int i = 0; i < BUCKET_NUMBER; i++){
			
			int *bucket_node = new int[127];
			memset(bucket_node, 0, 127 * sizeof(int));
			bucket_L.push_back(bucket_node);
		}

		Scan_file();
	}

	int i = strlen(Search_query);
	int temp_char_code = (int)Search_query[i - 1];
	int First = C_List[temp_char_code] + 1;
	int Last = C_List[temp_char_code];
	
	for(int temp_location = temp_char_code; temp_location < 127; temp_location++){
		
		if(Last != C_List[temp_location]){
		
			Last = C_List[temp_location];
			break;
		}
	}

	if(big_file){
		
		while((First <= Last) && (i > 1)){
		
			temp_char_code = Search_query[i - 2];
			First = C_List[temp_char_code] + Get_Occ_4_IDX(temp_char_code, First - 1) + 1;
			Last = C_List[temp_char_code] + Get_Occ_4_IDX(temp_char_code, Last);
			i--;		
		}	
	}
	else{
	
		while((First <= Last) && (i > 1)){
	
			temp_char_code = Search_query[i - 2];
			First = C_List[temp_char_code] + Get_Occ(temp_char_code, First - 1) + 1;
			Last = C_List[temp_char_code] + Get_Occ(temp_char_code, Last);
			i--;
		}
	}

	First--;
	Last--;

	if(mode_type[1] != 'n'){
		
		vector<unsigned int> offset_L;
		unsigned int temp_offset;
		char temp_bit_offset;

		for(int i = First; i <= Last; i++){

			file_read.seekg(i);

			string offset_number = "";
			bool find_flag = false;

			int letter_position = i;

			while(true){
				
				int letter = file_read.get();

				if(letter == ']')find_flag = true;	
				else if(letter == '['){

					stringstream change2int;
					change2int << offset_number;
					change2int >> temp_offset;
					offset_L.push_back(temp_offset);
					break;		
				}
				else if(find_flag){
			
					temp_bit_offset = (char)letter;
					offset_number = temp_bit_offset + offset_number;
				}

				if(big_file)letter_position = C_List[letter] + Get_Occ_4_IDX(letter, letter_position);
				else letter_position = C_List[letter] + Get_Occ(letter, letter_position);

				if((letter_position >= First) && (letter_position <= Last))break;

				file_read.seekg(letter_position);
			}
		
		}

		if(mode_type[1] == 'r')cout<<offset_L.size()<<endl;
		else if (mode_type[1] == 'a'){
			
			sort(offset_L.begin(), offset_L.end());
			for(unsigned int i = 0; i < offset_L.size(); i++)cout<<"["<<offset_L[i]<<"]"<<endl;
		}
	}
	else cout<<(Last - First + 1)<<endl;

	if(big_file)idx_read.close();
	file_read.close();

	return 0;
}