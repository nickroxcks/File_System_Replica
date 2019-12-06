#include "FileSystem.h"
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;
#include <map>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <bits/stdc++.h>
#include <iterator>
char buffer[1024];
bool command_complete = 0;
bool is_mounted = 0;
string real_disk_name;
vector<int> process_list;  //vector of all the child processes that have been created
string current_working_dir_OS;
uint8_t current_working_dir;  //try to keep MSB to 0
Super_block real_Super_block;
uint8_t original_current_dir;
bool recurse_flag =0;

/**
 * @brief Tokenize a string 
 * 
 * @param str - The string to tokenize
 * @param delim - The string containing delimiter character(s)
 * @return std::vector<std::string> - The list of tokenized strings. Can be empty
 */
std::vector<std::string> tokenize(const std::string &str, const char *delim) {
  char* cstr = new char[str.size() + 1];
  std::strcpy(cstr, str.c_str());

  char* tokenized_string = strtok(cstr, delim);

  std::vector<std::string> tokens;
  while (tokenized_string != NULL)
  {
    tokens.push_back(std::string(tokenized_string));
    tokenized_string = strtok(NULL, delim);
  }
  delete[] cstr;

  return tokens;
}


/**
 * @brief Make a Super_block given an array of bytes
 * 
 * @param block_data - 1024 bytes of the Super_block
 * @return Super_block
 */
Super_block make_super_block(char block_data[1024] ){
    Super_block temp_block;
    //make the free list
    for(int i=0;i<16;i++){
        temp_block.free_block_list[i] = block_data[i];
    }
    int current_position = 16;
    //make all the inodes for the super block
    for(int i=0;i<126;i++){
        
        for(int j=0; j<5 ;j++){  //the name
            temp_block.inode[i].name[j] = block_data[current_position+j];
        }
        current_position = current_position + 5;
        temp_block.inode[i].used_size = block_data[current_position];
        temp_block.inode[i].start_block = block_data[current_position+1];
        temp_block.inode[i].dir_parent = block_data[current_position+2];
        current_position = current_position+3;
    }

    return temp_block;
}

bool find_run_command(vector<string> command_vec,string buff_str){
    string command = command_vec.at(0);  //the command to be run
    if(command.compare("M")==0){
        fs_mount( (char *) command_vec.at(1).c_str() );//convert to cstring
        return 1;
    }
    else if(command.compare("C")==0){
        if(is_mounted){
            fs_create( command_vec.at(1),atoi(command_vec.at(2).c_str()));
            return 1;
        }
        else{
           fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("D")==0){
        if(is_mounted){
            original_current_dir = current_working_dir;
            fs_delete(command_vec.at(1));
            if(recurse_flag){
                current_working_dir = original_current_dir;
            }
            return 1;
        }
        else{
           fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("R")==0){
        if(is_mounted){
            fs_read(command_vec.at(1),atoi(command_vec.at(2).c_str()));
            return 1;
        }
        else{
            fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("W")==0){
        if(is_mounted){
            fs_write(command_vec.at(1),atoi(command_vec.at(2).c_str()));
            return 1;
        }
        else{
            fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("B")==0){
        if(is_mounted){
            uint8_t buff[1024];
            for(int i =0;i<1024;i++){
                buff[i] = buff_str.at(i);
            }
            fs_buff(buff);
            return 1;
        }
        else{
            fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("L")==0){
        if(is_mounted){
            fs_ls();
            return 1;
        }
        else{
            fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("E")==0){
        if(is_mounted){
            fs_resize(command_vec.at(1),atoi(command_vec.at(2).c_str()));
            return 1;
        }
        else{
            fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("O")==0){
        if(is_mounted&&command_vec.size()==1){
            fs_defrag();
            return 1;
        }
        else{
            fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    else if(command.compare("Y")==0){
        if(is_mounted){
            fs_cd(command_vec.at(1));
            return 1;
        }
        else{
           fprintf(stderr,"Error: No file system is mounted");
        }
        //let ta know u changed the parameter for this
        //fs_create()
    }
    return 0;
}
void command_handling(string file_in){

    int line_num = 0;
    std::ifstream in(file_in.c_str());
    if(!in) {
        cout << "Cannot open input file. Exiting program\n";
        return;
    }
    std::string str;
    string buff_str;
    while (std::getline(in, str)) {
        // output the line
        //std::cout << str << std::endl;
        line_num = line_num + 1;
        buff_str = str;  //used in some cases when we care about spaces in commands
        vector<string> command_vec = tokenize(str," ");  //removes space from the current line
        if(find_run_command(command_vec,buff_str)){
            continue;
        }
        else{
            fprintf(stderr, "Command Error: %s, %d\n",file_in.c_str(),line_num);
        }
    }

}

void fs_mount(char *new_disk_name){
    //check if virtual disk is in directory
    bool val_disk = 0;
    string str_disk_name = string(new_disk_name);
    DIR *dir;
    struct dirent *dir_struct;
    if ((dir = opendir(current_working_dir_OS.c_str())) != NULL){
        while ((dir_struct = readdir(dir)) != NULL) {
            //printf ("For reference, this file in curdir is: %s\n", dir_struct->d_name);
            if(str_disk_name.compare(dir_struct->d_name) == 0){
                //cout<<"found the disk"<<endl;
                val_disk=1;
            }
        }
        closedir(dir);
    } 
    else{
        cout<<"fatal error: failed to read from current directory."<<endl;
        return;
    }
    if(!val_disk){
        fprintf(stderr, "Error: Cannot find disk %s",str_disk_name.c_str());
        return;
    }
    //found virtual disk, so now check for potential errors
    std::ifstream infile(str_disk_name.c_str());

    int super_size = 1024;
    char temp_buffer[super_size];   //dont do anything to the real buffer just yet

    //read block 0 of disk(the super block)
    infile.read(temp_buffer, super_size);
    Super_block temp_block = make_super_block(temp_buffer);    
    /*
    for(int i =0;i<super_size;i++){
        size_t temp = temp_buffer[i];
        cout<<temp<<endl;
    }
    */
    cout<<"about to start consistency check"<<endl;

    //Consistency Check 1
    string test_str;
    for(int i=0;i<16;i++){  //make the binary stream of free_block_list
        test_str+= bitset<8>(temp_block.free_block_list[i]).to_string();
        //string s(1,testing.free_block_list[i]);
        //test_str = test_str + s;
    }
    //cout<<"Before: "<<test_str<<endl;
    //cout<<"does it like strings?"<<endl;
    bitset<128> bset(test_str);
    //cout<<"yes it does nani?"<<endl;
    for(int i=0;i<127;i++){  //TODO: RECHECK THIS. we dont care about 127th position as its block0
        //loop through all blocks in free list, make sure they are matching free list description
        int block_index = 127-i;
        //from right to left of the bit stream bset
        if(bset[i]==0){  // This means block shouldnt be used for anything
            for(int j =0;j<126;j++){  //loop through inodes (j), see if active, and block_index is within file range
                
                uint8_t inode_used_size = temp_block.inode[j].used_size;
                uint8_t inode_start_block = temp_block.inode[j].start_block;
                uint8_t manipulate = 127;  //ensure that bitwise operation is happening with 8 bits
                uint8_t num_blocks = inode_used_size & manipulate;  //grabs the 7 bits representing size(blocks)
                bitset<8> bsetsize(inode_used_size); //most significant bit indicates if it is active
                
                if(bsetsize[7]==1 && ((inode_start_block<= block_index) && (block_index<=(inode_start_block+num_blocks-1)))){
                    //if we reach here, that means the block we are searching is being used for a file
                    fprintf(stderr,"Error: File system in %s is inconsistent (error code: 1)",str_disk_name.c_str());
                    return;
                }
            }
        }
        
        else if(bset[i]==1){  //This means block should be used for one file or directory
            
            int numfiles = 0;
            
            for(int j =0;j<126;j++){  //loop through inodes (j), see if active, and if block_index is a part of the file size
                uint8_t inode_used_size = temp_block.inode[j].used_size;
                uint8_t inode_start_block = temp_block.inode[j].start_block;
                uint8_t manipulate = 127;  //ensure that bitwise operation is happening with 8 bits
                uint8_t num_blocks = inode_used_size & manipulate;  //grabs the last 7 bits
                bitset<8> bsetsize(inode_used_size); //most significant bit indicates if it is active
                if(bsetsize[7]==1 && ((inode_start_block<= block_index) && (block_index<=(inode_start_block+num_blocks-1)))){
                    numfiles = numfiles + 1;
                    if(numfiles>1){
                        fprintf(stderr,"Error: File system in %s is inconsistent (error code: 1)",str_disk_name.c_str());
                        return;
                    }
                }
            }
            if(numfiles<1){  //means block isnt allocated to a file/directory, even though it should be
                fprintf(stderr,"Error: File system in %s is inconsistent (error code: 1)",str_disk_name.c_str());
                return;
            }
        }

    }
    cout<<"passed 1"<<endl;
    //Consistency Check 2 The name of every file/directory must be unique in each directory. check if active maybe?
    multimap<string,uint8_t> hash_table;
    typedef std::multimap<string, uint8_t>::iterator MMAPIterator;
    for(int i =0;i<126;i++){
        string str_inode_name;
        for(int k=0;k<5;k++){
            string s(1,temp_block.inode[i].name[k]);
            str_inode_name = str_inode_name+ s;
        }
        uint8_t inode_used_size = temp_block.inode[i].used_size;
        uint8_t inode_dir_parent = temp_block.inode[i].dir_parent;
        uint8_t manipulate = 127;  //ensure that bitwise operation is happening with 8 bits
        uint8_t inode_parent_index = inode_dir_parent & manipulate;
        bitset<8> busedsize(inode_used_size); //MSB indicates if active or not
        if(busedsize[7]==1){
            pair<MMAPIterator, MMAPIterator> result = hash_table.equal_range(str_inode_name);
            for (MMAPIterator it = result.first; it != result.second; it++){
		        //std::cout << it->second << std::endl;
                uint8_t hash_value = it->second;
                string hash_key = it->first;
                if(hash_value==inode_parent_index && hash_key.compare(str_inode_name)==0){
                    fprintf(stderr,"Error: File system in %s is inconsistent (error code: 2)",str_disk_name.c_str());
                    return;
                }
            }
            hash_table.insert({str_inode_name,inode_parent_index});
        }

    }
    cout<<"passed 2"<<endl;

    //Consistency Check 3
    for(int i =0;i<126;i++){
        uint8_t inode_used_size = temp_block.inode[i].used_size;
        bitset<8> bsetsize(inode_used_size); //most significant bit indicates if it is active
        
        string test_str;
        for(int k=0;k<5;k++){
            test_str+= bitset<8>(temp_block.inode[i].name[k]).to_string();
        }
        bitset<40> bname(test_str);
        //bitset<40> bname(string(temp_block.inode[i].name));
        
        
        if(bsetsize[7]==0){  //inode is free
            for(int j=0;j<bname.size();j++){
                
                if(bname[j]==0){
                    continue;
                }
                else{
                    fprintf(stderr,"Error: File system in %s is inconsistent (error code: 3)",str_disk_name.c_str());
                    return;
                }
            }
        }
        else{  //inode is not free
            bool non_zero = 0;
            for(int j=0;j<40;j++){
                if(bname[j]==0){
                    continue;
                }
                else{
                    non_zero = 1;
                    break;
                }
            }
            if(!non_zero){
                fprintf(stderr,"Error: File system in %s is inconsistent (error code: 3)",str_disk_name.c_str());
                return;
            }
        }
    }
        
    cout<<"passed 3"<<endl;

    //Consistency Check 4
    for(int i =0;i<126;i++){  //checking all inodes
        uint8_t inode_start_block = temp_block.inode[i].start_block;
        if(inode_start_block!=0){  //check if it is a file
            if(1<=inode_start_block&&inode_start_block<=127){
                continue;
            }
            else{
                fprintf(stderr,"Error: File system in %s is inconsistent (error code: 4)",str_disk_name.c_str());
                return;
            }
        }
    }
    
    cout<<"passed 4"<<endl;

    //Consistency Check 5  should i check if the inode is active?
    for(int i =0;i<126;i++){
        uint8_t inode_start_block = temp_block.inode[i].start_block;
        uint8_t inode_used_size = temp_block.inode[i].used_size;
        uint8_t manipulate = 127;  //ensure that bitwise operation is happening with 8 bits
        uint8_t num_blocks = inode_used_size & manipulate;  //grabs the last 7 bits
        uint8_t inode_dir_parent = temp_block.inode[i].dir_parent;
        bitset<8> bdirparent(inode_dir_parent);
        bitset<8> busedsize(inode_used_size);
        if(busedsize[7]==1 && bdirparent[7]==1){  //indicates we are looking at a directory
            if(num_blocks==0 && inode_start_block==0){
                continue;
            }
            else{
                fprintf(stderr,"Error: File system in %s is inconsistent (error code: 5)",str_disk_name.c_str());
                return;
            }
        }
        else{
            continue;
        }

    }
    
    cout<<"passed 5"<<endl;

    //Consistency Check 6
    for(int i =0;i<126;i++){
        uint8_t inode_dir_parent = temp_block.inode[i].dir_parent;
        uint8_t inode_used_size = temp_block.inode[i].used_size;
        uint8_t manipulate = 127;  //ensure that bitwise operation is happening with 8 bits
        uint8_t parent_index = inode_dir_parent & manipulate;
        bitset<8> bparent(inode_dir_parent);
        bitset<8> bsetsize(inode_used_size); //most significant bit indicates if it is active

        if(parent_index == 126){
            fprintf(stderr,"Error: File system in %s is inconsistent (error code: 6)",str_disk_name.c_str());
            return;
        }
        if(0<=parent_index && parent_index <=125){
            uint8_t parent_used_size = temp_block.inode[parent_index].used_size;
            bitset<8> bparent_used_size(parent_used_size);
            if(bparent_used_size[7]==1 && bparent[7]==1){
                continue;
            }
            else{
                fprintf(stderr,"Error: File system in %s is inconsistent (error code: 6)",str_disk_name.c_str());   
                return;          
            }
        }

    }
    
    cout<<"all cases passed"<<endl;

    //at this point, we have passed all the tests. 
    is_mounted = 1;
    real_Super_block = temp_block;
    current_working_dir = 127;  //set current directory to root
    real_disk_name = str_disk_name;

}

void fs_create(string name, int size){
    int first_available_inode = -1;
    //at this point, we can assume no dupes in a directory on disk. make hash of all directories
    multimap<string,uint8_t> hash_table;  //multimap<inode_name,parent_index>
    typedef std::multimap<string, uint8_t>::iterator MMAPIterator;
    for(int i =0;i<126;i++){
        string str_inode_name;
        for(int k=0;k<5;k++){
            string s(1,real_Super_block.inode[i].name[k]);
            str_inode_name = str_inode_name+ s;
        }
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        uint8_t manipulate = 127;  //ensure that bitwise operation is happening with 8 bits
        uint8_t inode_parent_index = inode_dir_parent & manipulate;
        bitset<8> busedsize(inode_used_size); //MSB indicates if active or not
        if(busedsize[7]==1){
            pair<MMAPIterator, MMAPIterator> result = hash_table.equal_range(str_inode_name);
            hash_table.insert({str_inode_name,inode_parent_index});
        }
        else{
            if(first_available_inode<0){
                first_available_inode = i;
            }
        }

    }
    //check if space in super block
    if(first_available_inode<0){
        fprintf(stderr,"Error: Superblock in disk %s is full, cannot create %s",real_disk_name.c_str(),name.c_str());
    }
    uint8_t temp127 = 127;
    //now check if name is already in the current directory
    pair<MMAPIterator, MMAPIterator> result = hash_table.equal_range(name);
    for (MMAPIterator it = result.first; it != result.second; it++){
        //std::cout << it->second << std::endl;
        uint8_t hash_value = it->second;
        string hash_key = it->first;
        if(hash_value==(current_working_dir & temp127) && hash_key.compare(name)==0){
            fprintf(stderr,"Error: File or directory %s already exists",name.c_str());
            return;
        }
    }
    //make sure the name is not . or ..
    if(name.compare(".") == 0 || name.compare("..")==0){
        fprintf(stderr,"Error: File or directory %s already exists",name.c_str());
    }

    //determine if file or directory
    if(size==0){//means we have a directory
        if(name.size()==5){
            for(int i =0;i<5;i++){  //not null terminated
                real_Super_block.inode[first_available_inode].name[i] = name.at(i);
            }
        }
        else{  //check this
            int str_size = name.size();
            for(int i =0;i<5;i++){
                if(i<str_size){
                    real_Super_block.inode[first_available_inode].name[i] = name.at(i);
                }
                else{
                    real_Super_block.inode[first_available_inode].name[i] = '\0';
                }
            }
        }
        //update all other atributes of inode in struct
        uint8_t inode_new_used_size = 128; //10000000 in binary
        real_Super_block.inode[first_available_inode].used_size = inode_new_used_size;
        uint8_t inode_new_start_block = 0;
        real_Super_block.inode[first_available_inode].start_block = inode_new_start_block;
        uint8_t inode_dir_parent = current_working_dir | inode_new_used_size; //Yxxxxxxx | 10000000
        real_Super_block.inode[first_available_inode].dir_parent = inode_dir_parent;
        
        //write to the disk
        char temp_buffer[1024];  //this is just here for writing
        for(int i=0;i<5;i++){
            temp_buffer[i] = real_Super_block.inode[first_available_inode].name[i];
        }
        temp_buffer[5] = inode_new_used_size;
        temp_buffer[6] = inode_new_start_block;
        temp_buffer[7] = inode_dir_parent;
        streampos dir_disk_location = 16 + 8*first_available_inode;
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        myFile.seekp (dir_disk_location);  //set location in file to write
        myFile.write (temp_buffer, 8);
        return;
    }
    else{  //means we have a file
        if(size<0){
            fprintf(stderr,"  Error: Cannot allocate %d on %s",size,real_disk_name.c_str());
        }
        //find the first available block and make sure there is enough contiguous blocks
        string bit_str;
        int num_continuous = 0;
        int block_start_placement = 1;
        for(int i=0;i<16;i++){  //make the binary stream of free_block_list
            bit_str+= bitset<8>(real_Super_block.free_block_list[i]).to_string();
        }
        bitset<128> freebits(bit_str);  //update the bits for what is being deleted
        for(int i=1;i<128;i++){
            if(freebits[127-i]==0){
                num_continuous = num_continuous +1;
                if(num_continuous == size){
                    for(int j=0;j<size;j++){  //update bits for allocated file size
                        freebits[(127-i) +j] = 1;
                    }
                    block_start_placement = i - size +1;
                    break;
                }
            }
            else{
                num_continuous = 0;
            }
        }
        if(num_continuous==0){
            fprintf(stderr,"  Error: Cannot allocate %d on %s",size,real_disk_name.c_str());
            return;
        }
        string free_space_str = freebits.to_string();
        
        //at this point, we can now safely write to disk
        //first update structs
        if(name.size()==5){
            for(int i =0;i<5;i++){  //not null terminated
                real_Super_block.inode[first_available_inode].name[i] = name.at(i);
            }
        }
        else{  //check this
            int str_size = name.size();
            for(int i =0;i<5;i++){
                if(i<str_size){
                    real_Super_block.inode[first_available_inode].name[i] = name.at(i);
                }
                else{
                    real_Super_block.inode[first_available_inode].name[i] = '\0';
                }
            }
        }
        //update all other atributes of file inode
        uint8_t temp128 = 128; //10000000 in binary
        uint8_t temp127 = 127; //01111111 in binary
        uint8_t continuous_block = (uint8_t) num_continuous;
        real_Super_block.inode[first_available_inode].used_size = continuous_block | temp128;
        real_Super_block.inode[first_available_inode].start_block = (uint8_t) block_start_placement;
        uint8_t inode_dir_parent = current_working_dir & temp127; //Yxxxxxxx & 01111111
        real_Super_block.inode[first_available_inode].dir_parent = inode_dir_parent;

        //update freespace list
        for(int i=0;i<16;i++){
            real_Super_block.free_block_list[i] = free_space_str.at(i);
        }

        //Now write to the actual disk. update free space, and the free inode
        char temp_buffer[1024];
        for(int i=0;i<16;i++){
            temp_buffer[i] = real_Super_block.free_block_list[i];
        }
        streampos free_list_position = 0;
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        myFile.seekp (free_list_position);
        myFile.write (temp_buffer, 16);  //write the new free space list to disk
        if(name.size()==5){
            for(int i =0;i<5;i++){  //not null terminated
                temp_buffer[i] = name.at(i);
            }
        }
        else{  //check this
            int str_size = name.size();
            for(int i =0;i<5;i++){
                if(i<str_size){
                    temp_buffer[i] = name.at(i);
                }
                else{
                    temp_buffer[i] = '\0';
                }
            }
        }
        
        temp_buffer[5] = continuous_block | temp128;
        temp_buffer[6] = (uint8_t) block_start_placement;
        temp_buffer[7] = inode_dir_parent;
        streampos inode_disk_position = 16 + first_available_inode*8;
        myFile.seekp(inode_disk_position);
        myFile.write(temp_buffer,8);
    }

}


void fs_delete(std::string name){
    int first_available_inode = -1;
    //at this point, we can assume no dupes in a directory on disk. make hash of all directories
    /*Comments op map: the parent index is the entire dir_parent in the inode
    */
    multimap<string,uint8_t> hash_table;  //multimap<inode_name,parent_index>
    typedef std::multimap<string, uint8_t>::iterator MMAPIterator;
    for(int i =0;i<126;i++){
        string str_inode_name;
        for(int k=0;k<5;k++){
            string s(1,real_Super_block.inode[i].name[k]);
            str_inode_name = str_inode_name+ s;
        }
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        bitset<8> busedsize(inode_used_size); //MSB indicates if active or not
        if(busedsize[7]==1){
            pair<MMAPIterator, MMAPIterator> result = hash_table.equal_range(str_inode_name);
            hash_table.insert({str_inode_name,inode_dir_parent});
        }
        else{
            if(first_available_inode<0){
                first_available_inode = i;
            }
        }

    }
    //check if file/directory exists in the current directory, and obtain relevant info
    int inode_index;
    string file_dir_name;
    uint8_t file_dir_used_size;
    uint8_t file_dir_start_block;
    uint8_t file_dir_dir_parent;
    bool found_file_dir = 0;
    pair<MMAPIterator, MMAPIterator> result = hash_table.equal_range(name);
    for (MMAPIterator it = result.first; it != result.second; it++){
        //std::cout << it->second << std::endl;
        uint8_t hash_value = it->second;
        uint8_t manipulate = 127;  //01111111
        string hash_key = it->first;
        if((hash_value & manipulate)==(current_working_dir & manipulate) && hash_key.compare(name)==0){
            for(int j=0;j<126;j++){  //here we confirmed its in the current directory
                string str_inode_name;  //obtain its relevent info
                for(int k=0;k<5;k++){
                    string s(1,real_Super_block.inode[j].name[k]);
                    str_inode_name = str_inode_name+ s;
                }
                uint8_t temp_used_size = real_Super_block.inode[j].used_size;
                bitset<8> btemp(temp_used_size);
                if(str_inode_name.compare(hash_key)==0 && hash_value==(real_Super_block.inode[j].dir_parent)
                && btemp[7]==1){
                    file_dir_name = hash_key;
                    file_dir_used_size = real_Super_block.inode[j].used_size;
                    file_dir_start_block = real_Super_block.inode[j].start_block;
                    file_dir_dir_parent = real_Super_block.inode[j].dir_parent;
                    inode_index = j;
                    found_file_dir=1;
                }
            }
            //fprintf(stderr,"Error: File or directory %s already exists",name.c_str());
        }
    }
    if(!found_file_dir){
        fprintf(stderr,"Error: File or directory %s does not exist",name.c_str());
        return;
    }
    //now check if file or directory, and update disk accordingly
    bitset<8> bdirparent(file_dir_dir_parent);
    if(file_dir_start_block!=0){//indicates a file

        string bit_str;
        uint8_t temp127 = 127;
        for(int i=0;i<16;i++){  //make the binary stream of free_block_list
            bit_str+= bitset<8>(real_Super_block.free_block_list[i]).to_string();
        }
        bitset<128> freebits(bit_str);  //update the bits for what is being deleted
        for(int i=1;i<128;i++){//verified
            if((file_dir_start_block)<=i && i <= (file_dir_start_block+(file_dir_used_size&temp127)-1)){
                freebits[127-i]= 0;
            }
        }
        string free_space_str = freebits.to_string();
        //begin deleting to structs
        for(int i=0;i<5;i++){
            real_Super_block.inode[inode_index].name[i]=='\0';
        }
        real_Super_block.inode[inode_index].used_size = 0;
        real_Super_block.inode[inode_index].start_block = 0;
        real_Super_block.inode[inode_index].dir_parent = 0;
        for(int i=0;i<16;i++){
            real_Super_block.free_block_list[i] = free_space_str.at(i);
        }
        //begin deleting to disk
        char temp_buffer[1024];
        for(int i=0;i<16;i++){
            temp_buffer[i] = free_space_str.at(i);
        }
        //first write the free list on disk
        streampos free_list_position = 0;
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        myFile.seekp (free_list_position);
        myFile.write (temp_buffer, 16);
        //now update the inode in superblock on disk
        for(int i=0;i<8;i++){
            temp_buffer[i] = 0;  //TODO: check this maybe
        }
        streampos disk_inode_position = 16 + inode_index*8;
        myFile.seekp(disk_inode_position);
        myFile.write(temp_buffer,8);
        //now delete block data
        streampos disk_block_position = file_dir_start_block*1024;
        long total_size_bytes = (file_dir_used_size & temp127)*1024;
        char max_buffer[total_size_bytes];  //worst case, file takes 1024*127=130048 bytes
        for(int i=0;i<total_size_bytes;i++){  //TODO:this loop might not be neccesary. check later
            max_buffer[i] = '\0';
        }
        myFile.seekp(disk_block_position);
        myFile.write(max_buffer,total_size_bytes);
        myFile.close();
        return;
    }
    else{  //this means name is a directory
        //begin by finding everything inside this directory and deleting them
        for(int i = 0; i<126;i++){
            uint8_t recurse_parent = real_Super_block.inode[i].dir_parent;
            uint8_t temp127 = 127;
            uint8_t recurse_used_size = real_Super_block.inode[i].dir_parent;
            uint8_t recurse_start_block = real_Super_block.inode[i].start_block;
            bitset<8> busedsize(recurse_used_size);
            if((recurse_parent & temp127) == (file_dir_dir_parent & temp127)&&(busedsize[7]==1)){
                string temp_name;
                for(int k=0;k<5;k++){
                    string s(1,real_Super_block.inode[i].name[k]);
                    temp_name = temp_name+ s;
                }
                if(recurse_start_block==0){
                current_working_dir = inode_index;  //this needs to be done the not in current directory error
                fs_delete(temp_name);  //deleting a directory
                }
                else{
                fs_delete(temp_name);  //deleting a file
                }
            }         
        }
        //begin deleting to structs
        for(int i=0;i<5;i++){
            real_Super_block.inode[inode_index].name[i]=='\0';
        }
        real_Super_block.inode[inode_index].used_size = 0;
        real_Super_block.inode[inode_index].start_block = 0;
        real_Super_block.inode[inode_index].dir_parent = 0;
        //begin deleting inode superblock data from disk
        char temp_buffer[1024];
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        for(int i=0;i<8;i++){
            temp_buffer[i] = 0;
        }
        streampos disk_inode_position = 16 + inode_index*8;
        myFile.seekp(disk_inode_position);
        myFile.write(temp_buffer,8);  //TODO: might be error with deleting becuase
        recurse_flag = 1;//structs are not properly updated after recursion. maybe mount again
        return;
    }


}
void fs_read(std::string name, int block_num){
    uint8_t temp127 = 127;
    int inode_index=-1;
    for(int i=0;i<126;i++){
        string temp_name;
        for(int k=0;k<5;k++){
            string s(1,real_Super_block.inode[i].name[k]);
            temp_name = temp_name+ s;
        }
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        bitset<8> busedsize(inode_used_size);
        bitset<8> bdirparent(inode_dir_parent);
        if(busedsize[7] == 1 && temp_name.compare(name)==0 && 
        (inode_dir_parent&temp127) == (current_working_dir&temp127) && bdirparent[7]==0){
            inode_index = i;//maybe double check if bdirparent[7]=0 is the right thing to do
            break;
        }
    }
    if(inode_index<0){
        fprintf(stderr, "Error: File %s does not exist",name.c_str());
        return;
    }
    uint8_t inode_used_size = real_Super_block.inode[inode_index].used_size;
    uint8_t inode_start_block = real_Super_block.inode[inode_index].start_block;
    inode_used_size = inode_used_size&temp127;
    int size = (int) inode_used_size;
    if(0<=block_num && block_num<=(size-1)){
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        long block_size = block_num;
        long block_start = inode_start_block;
        streampos disk_block_position = (block_start+ block_size - 1)*1024;
        myFile.seekg(disk_block_position);
        myFile.read(buffer,1024);
        return;
    }
    else{
       fprintf(stderr, "Error: %s does not have block %d",name.c_str(),block_num);
    }
}
void fs_write(std::string name, int block_num){
    uint8_t temp127 = 127;
    int inode_index=-1;
    for(int i=0;i<126;i++){
        string temp_name;
        for(int k=0;k<5;k++){
            string s(1,real_Super_block.inode[i].name[k]);
            temp_name = temp_name+ s;
        }
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        bitset<8> busedsize(inode_used_size);
        bitset<8> bdirparent(inode_dir_parent);
        if(busedsize[7] == 1 && temp_name.compare(name)==0 && 
        (inode_dir_parent&temp127) == (current_working_dir&temp127) && bdirparent[7]==0){
            inode_index = i;
            break;
        }
    }
    if(inode_index<0){
        fprintf(stderr, "Error: File %s does not exist",name.c_str());
        return;
    }
    uint8_t inode_used_size = real_Super_block.inode[inode_index].used_size;
    uint8_t inode_start_block = real_Super_block.inode[inode_index].start_block;
    inode_used_size = inode_used_size&temp127;
    int size = (int) inode_used_size;
    if(0<=block_num && block_num<=(size-1)){
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        long block_size = block_num;
        long block_start = inode_start_block;
        streampos disk_block_position = (block_start+ block_size - 1)*1024;
        myFile.seekp(disk_block_position);
        myFile.write(buffer,1024);
        return;
    }
    else{
       fprintf(stderr, "Error: %s does not have block %d",name.c_str(),block_num);
    }   
}

void fs_buff(uint8_t buff[1024]){
    for(int i =0;i<1024;i++){
        buffer[i] = 0;
    }
    for(int i=0;i<1024;i++){
        buffer[i] = buff[i];
    }
}

void fs_ls(void){
    uint8_t temp127 = 127;
    uint8_t current_parent_dir = real_Super_block.inode[(current_working_dir&temp127)].dir_parent;
    current_parent_dir = current_parent_dir&temp127;  //inode index of parrent directory
    int num_children_parent = 2;
    int num_children_current = 2;
    for(int i=0;i<126;i++){  //find # files/dir in current directory
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        bitset<8> busedsize(inode_used_size);
        inode_dir_parent = inode_dir_parent&temp127;
        if((inode_dir_parent == (current_working_dir&temp127))&&(busedsize[7]==1)){
            num_children_current = num_children_current + 1;
        }
    }
    for(int i=0;i<126;i++){  ////find # files/dir in parent directory
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        bitset<8> busedsize(inode_used_size);
        inode_dir_parent = inode_dir_parent&temp127;
        if((inode_dir_parent == (current_parent_dir))&&(busedsize[7]==1)){
            num_children_parent = num_children_parent + 1;
        }
    }
    printf(". %3d\n", num_children_current);
    printf(".. %3d\n", num_children_parent);
    for(int i=0;i<126;i++){  //find the files and directories in current directory
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        bitset<8> busedsize(inode_used_size);
        inode_dir_parent = inode_dir_parent&temp127;
        if(inode_dir_parent == (current_working_dir&temp127) && busedsize[7]==1){  //we found something in current dir
            string inode_name;
            for(int k=0;k<5;k++){  //grabbing name of the inode
                string s(1,real_Super_block.inode[i].name[k]);
                inode_name = inode_name + inode_name;
            }
            if(real_Super_block.inode[i].start_block==0){  //we have directory
                int inode_num_children = 2;
                for(int j=0;j<126;j++){
                    if(((real_Super_block.inode[j].dir_parent) & temp127)==i){
                        inode_num_children = inode_num_children +1;
                    }
                }
                printf("%-5s %3d\n", inode_name.c_str(), inode_num_children);
            }
            else{//we have file
                int size = (int) ((real_Super_block.inode[i].used_size) & temp127);
                printf("%-5s %3d KB\n", inode_name.c_str(), size);
            }             
        }
    }

}

void fs_resize(std::string name, int new_size){
    uint8_t temp127 = 127;
    int inode_index=-1;
    for(int i=0;i<126;i++){
        string temp_name;
        for(int k=0;k<5;k++){
            string s(1,real_Super_block.inode[i].name[k]);
            temp_name = temp_name+ s;
        }
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        bitset<8> busedsize(inode_used_size);
        bitset<8> bdirparent(inode_dir_parent);
        if(busedsize[7] == 1 && temp_name.compare(name)==0 && 
        (inode_dir_parent & temp127) == (current_working_dir & temp127) && bdirparent[7]==0){
            inode_index = i;
            break;
        }
    }
    if(inode_index<0){
        fprintf(stderr, "Error: File %s does not exist",name.c_str());
        return;
    }
    int original_num_blocks = (int) ((real_Super_block.inode[inode_index].used_size) & temp127);
    if(new_size==original_num_blocks){
        return;
    }
    else if(new_size>original_num_blocks){
        string bit_str;
        uint8_t temp127 = 127;
        uint8_t original_inode_start_block = real_Super_block.inode[inode_index].start_block;

        for(int i=0;i<16;i++){  //make the binary stream of free_block_list
            bit_str+= bitset<8>(real_Super_block.free_block_list[i]).to_string();
        }
        bitset<128> freebits(bit_str);
        int more_available_contiguous = 0;  //remember, with bitsets, msb is indexed the highest
        for(int i=(127-(original_inode_start_block +original_num_blocks));i>=0;i--){  //look for more contiguous space
            if(freebits[i]==0){
                more_available_contiguous = more_available_contiguous +1;
            }
            else{
                break;
            }
        }
        if(more_available_contiguous>=(new_size-original_num_blocks)){  //this means their is indeed more contiguous room
            for(int i=(127-(original_inode_start_block +original_num_blocks));i>=0;i--){  //update bits in free space list
                if(freebits[i]==0){
                    freebits[i]=1;
                }
                else{
                    break;
                }
            }
            string free_space_str = freebits.to_string();
            //update to structs
            for(int i=0;i<16;i++){
                real_Super_block.free_block_list[i] = free_space_str.at(i);
            }
            uint8_t updated_size = (((uint8_t) new_size) | 128);
            real_Super_block.inode[inode_index].used_size = updated_size;
            //update to disk
            char temp_buffer[1024];
            for(int i=0;i<16;i++){
                temp_buffer[i] = real_Super_block.free_block_list[i];
            }
            fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
            streampos free_list_position = 0;
            myFile.seekp(free_list_position);
            myFile.write(temp_buffer,16);
            //now update inode on disk in superblock
            streampos added_block_position = 16 + inode_index*8 + 5;
            temp_buffer[0] = updated_size;
            myFile.seekp(added_block_position);
            myFile.write(temp_buffer,1);
            return;
        }
        else{  //check if there is any other available space to fit new_size
            int num_consec_blocks = 0;  //TODO:: FIX SPECIAL CASE
            int new_block_index = -1;  //potential new starting block 
            for(int i=126;i>=0;i--){  //look for more contiguous space
                if(freebits[i]==0){
                    num_consec_blocks = num_consec_blocks + 1;
                    if(num_consec_blocks == new_size){  //found enough space
                        new_block_index = 127-i-num_consec_blocks+1;
                        for(int j=0;j<original_num_blocks;j++){  //zero out old free list bits
                            freebits[127-original_inode_start_block-j]=0;
                        }
                        for(int j=0;j<new_size;j++){  //update free list for the new size
                            freebits[i+j] = 1;
                        }
                        break;
                    }
                }
                else{
                    num_consec_blocks = 0;
                    continue;
                }
            }
            if(new_block_index<0){
                fprintf(stderr, "Error: File %s cannot expand to size %d",name.c_str(),new_size);
                return;
            }
            //if we reach this point, there is room but in another place
            //we have too: update free list, update inode in superblock, 
            //copy original data from block, delete this data from disk,
            //paste data into new block location
            uint8_t temp128 = 128;
            string free_space_str = freebits.to_string();
            //update structs
            for(int i=0;i<16;i++){
                real_Super_block.free_block_list[i] = free_space_str.at(i);
            }
            uint8_t new_used_size = ((uint8_t) new_size) | temp128;
            uint8_t new_start_block = (uint8_t) new_block_index;
            real_Super_block.inode[inode_index].used_size = new_used_size;
            real_Super_block.inode[inode_index].start_block = new_start_block;
            //update disk
            char temp_buffer[1024];
            for(int i=0;i<16;i++){  //put free list data into the buffer
                temp_buffer[i] = free_space_str.at(i);
            }
            fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
            streampos free_list_position = 0;
            myFile.seekp(free_list_position);
            myFile.write(temp_buffer,16);  //write freelist data to disk
            streampos disk_used_size_position = 16 + inode_index*8 + 5;  //next byte after is start_blocl
            temp_buffer[0] = new_used_size;
            temp_buffer[1] = new_start_block;
            myFile.seekp(disk_used_size_position);
            myFile.write(temp_buffer,2);  //write the new inode data to superblock
            char tempbuffer2[original_num_blocks*1024];  //used to copy/paste old data
            char tempbuffer3[1024*new_size];  //used for deleting
            streampos old_disk_data_position = original_inode_start_block*1024;
            myFile.seekg(old_disk_data_position);
            myFile.read(tempbuffer2,original_num_blocks*1024);//pass old data into buffer
            myFile.seekp(old_disk_data_position);
            for(long i=0;i<(1024*original_num_blocks);i++){//not sure if this is necesary, but just in case
                tempbuffer3[i] = '\0';
            }
            myFile.write(tempbuffer3,original_num_blocks*1024);//delete data from old location
            streampos new_disk_data_position = new_start_block *1024;
            myFile.seekp(new_disk_data_position);
            myFile.write(tempbuffer2,1024*original_num_blocks);  //write the old data to the new location
            return;
        }
        //string free_space_str = freebits.to_string();
    }
    else if(new_size<original_num_blocks){
        string bit_str;
        uint8_t temp127 = 127;
        uint8_t original_inode_start_block = real_Super_block.inode[inode_index].start_block;

        for(int i=0;i<16;i++){  //make the binary stream of free_block_list
            bit_str+= bitset<8>(real_Super_block.free_block_list[i]).to_string();
        }
        bitset<128> freebits(bit_str);
        //update free list and 0 out bits that are no longer needed
        //update the used size to the new used size
        //delete unused data from the blocks
        for(int i=(127-(original_inode_start_block +original_num_blocks-1));
        i<(127-(original_inode_start_block+new_size-1));i++){  //update bits in free space list
            freebits[i] = 0;  
        }
        string free_space_str = freebits.to_string();
        //update structs
        for(int i=0;i<16;i++){
            real_Super_block.free_block_list[i] = free_space_str.at(i);
        }
        uint8_t temp128 = 128;
        uint8_t new_used_size = ((uint8_t) new_size) | temp128;
        real_Super_block.inode[inode_index].used_size = new_used_size;
        //update disk
        char temp_buffer[1024];
        for(int i=0;i<16;i++){  //put free list data into the buffer
            temp_buffer[i] = free_space_str.at(i);
        }
        fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
        streampos free_list_position = 0;
        myFile.seekp(free_list_position);
        myFile.write(temp_buffer,16);  //write freelist data to disk
        temp_buffer[0] = new_used_size;
        streampos disk_used_size_position = 16 + inode_index*8 + 5;  //next byte after is start_blocl
        myFile.seekp(disk_used_size_position);
        myFile.write(temp_buffer,1);//update the used size on disk
        streampos disk_delete_start = (original_inode_start_block+new_size)*1024;
        long num_bytes_deleted = original_num_blocks - new_size;
        char tempbuffer2[num_bytes_deleted];
        for(long i=0;i<num_bytes_deleted;i++){
            tempbuffer2[i] = '\0';
        }
        myFile.seekp(disk_delete_start);
        myFile.write(tempbuffer2,num_bytes_deleted);
        return;

    }
    else{
        cout<<"unknown error"<<endl;
    }

}
void fs_defrag(void){
    map<uint8_t, int> mp; 
  
    //find order in which to fix inodes
    for(int i=0;i<126;i++){
        uint8_t inode_star_block = real_Super_block.inode[i].start_block;
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        bitset<8> bset(inode_used_size);
        if((inode_star_block!=0)&&(bset[7]==1)){
            mp.insert({inode_star_block,i});
        }
    }

    string bit_str;
    for(int i=0;i<16;i++){  //make the binary stream of free_block_list
        bit_str+= bitset<8>(real_Super_block.free_block_list[i]).to_string();
    }
    bitset<128> freebits(bit_str);
    uint8_t temp127 = 127;
    for(map<uint8_t, int>::const_iterator it = mp.begin();  //TODO:: verify this iterates properly
    it != mp.end(); ++it) { //move blocks over, one at a time
        uint8_t inode_start_block =  it->first;
        int inode_index =  it->second;
        int num_blocks = ((real_Super_block.inode[inode_index].used_size) & temp127);
        int new_block_index = -1;  //potential new starting block
        int num_blocks_over = 0; 
        for(int i=(127-inode_start_block+1);i<=127;i++){ //update bits in freebits
            if(freebits[i]==0 && i!=127){
                num_blocks_over = num_blocks_over + 1;
            }
            else{
                if(num_blocks_over>0){
                    new_block_index = 127-i+1;
                    for(int k=0;k<num_blocks_over;k++){
                        freebits[127-new_block_index-k] = 1;
                    }
                    for(int k=127-(inode_start_block+num_blocks-1);
                    k<127-(new_block_index+num_blocks-1);k++){
                        freebits[k]=0;
                    }
                }
            }
        }
        if(new_block_index>0){
            //update the new start_block of the inode, copy data from old, put it in new location
            //update structs
            real_Super_block.inode[inode_index].start_block = (uint8_t) new_block_index;
            //update inode on disk
            uint8_t new_start_block = (uint8_t) new_block_index;
            char tempbuffer[1024];
            tempbuffer[0] = new_start_block;
            fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
            streampos disk_start_block_position= 16 + inode_index*8 + 6;  //next byte after is start_blocl
            myFile.seekp(disk_start_block_position);
            myFile.write(tempbuffer,1);  //write new start_block to inode on disk

            char tempbuffer2[num_blocks*1024];  //used to copy/paste old data
            char tempbuffer3[1024*num_blocks];  //used for deleting
            for(int i=0;i<(1024*num_blocks);i++){
                tempbuffer3[i] = '\0';
            }
            streampos old_disk_data_position = inode_start_block*1024;
            myFile.seekg(old_disk_data_position);
            myFile.read(tempbuffer2,num_blocks*1024);//pass old data into buffer
            myFile.seekp(old_disk_data_position);
            myFile.write(tempbuffer3,1024*num_blocks);  //delete the old data
            streampos new_disk_data_position = new_block_index*1024;
            myFile.write(tempbuffer2,1024*num_blocks);

        }

    }
    //finally, update the free list
    string free_space_str = freebits.to_string();
    char tempbuffer[16];
    for(int i=0;i<16;i++){
        real_Super_block.free_block_list[i] = free_space_str.at(i);
        tempbuffer[i] = free_space_str.at(i);;
    }
    //on disk
    fstream myFile (real_disk_name.c_str(), ios::in | ios::out | ios::binary);
    streampos free_list_position = 0;
    myFile.seekp(free_list_position);
    myFile.write(tempbuffer,16);  //write freelist data to disk
}
void fs_cd(std::string name){
    if(name.compare(".")==0){
        return;
    }
    uint8_t temp127 = 127;
    uint8_t current_dir_parent_index = real_Super_block.inode[current_working_dir & temp127].dir_parent;
    if(name.compare("..")==0){
        current_working_dir = current_dir_parent_index & temp127;
        return;
    }
    for(int i=0;i<126;i++){
        string temp_name;
        for(int k=0;k<5;k++){
            string s(1,real_Super_block.inode[i].name[k]);
            temp_name = temp_name+ s;
        }
        uint8_t inode_dir_parent = real_Super_block.inode[i].dir_parent;
        uint8_t inode_used_size = real_Super_block.inode[i].used_size;
        uint8_t inode_start_block = real_Super_block.inode[i].start_block;
        bitset<8> busedsize(inode_used_size);  //look for directory inside current directory
        if(busedsize[7]==1 && (inode_start_block==0) && (temp_name.compare(name)==0)
        && ((inode_dir_parent&temp127)==(current_working_dir&temp127))){
            current_working_dir = inode_dir_parent&temp127;
            return;
        }
    }
    fprintf(stderr,"Error: Directory %s does not exist",name.c_str());
}


int main(int argc, char *argv[]) {

    
    string file_name = argv[1];
    char wdir[1000];
    current_working_dir_OS = getcwd(wdir,1000);
    //file_name should be a file name if all was entered correctly
    command_handling(file_name);
    

   /*
   //THE GOLDEN CODE
   char idk[1024];
    idk[0] = 'g';
    idk[1] = 'h';
    idk[2] = 'i';
    streampos wtf = 3;
    fstream myFile ("new_write_file", ios::in | ios::out | ios::binary);
    myFile.seekp (wtf);
    myFile.write (idk, 3);
    */
   /*
   //debugging code 3 FOR WRITING
    std::ifstream infile("blockfile.txt");
    char temp_buffer[1024];   //dont do anything to the real buffer just yet
    //read block 0 of disk(the super block)
    infile.read(temp_buffer, 1024);
    //cout<<temp_buffer[1023]<<endl;
    Super_block testing = make_super_block(temp_buffer);
    infile.close();
    std::ifstream infile2("new_write_file");
    char temp_buffer2[1024];   //dont do anything to the real buffer just yet
    //read block 0 of disk(the super block)
    infile2.read(temp_buffer2, 1024);
    //cout<<temp_buffer[1023]<<endl;
    infile2.close();

    string test_str;
    std:ofstream outfile;
    outfile.open("new_write_file");
    int k = 0;
    int first_available_inode = 1;
    int bytelocation = 16 + first_available_inode*8;
    while(!outfile.eof()){
        if(k<16){
        outfile << testing.free_block_list[k];
        k = k+1;
        }
        else if(k == bytelocation){
            outfile << testing.inode[first_available_inode].name[0];
            outfile << testing.inode[first_available_inode].name[1];
            outfile << testing.inode[first_available_inode].name[2];
            outfile << testing.inode[first_available_inode].name[3];
            outfile << testing.inode[first_available_inode].name[4];
            outfile << testing.inode[first_available_inode].used_size;
            outfile << testing.inode[first_available_inode].start_block;
            outfile << testing.inode[first_available_inode].dir_parent;
            k=k+8;
        }
        else{
            outfile<<temp_buffer2[k];
            k = k+1;
        }
        if(k==1024){
            break;
        }
    }    
    outfile.close();
    */
   /*
   //Debugging code 2
    std::ifstream infile(real_disk_name.c_str());
    while(infile.read(buffer,2)){
            
    }
    char another_buff[1023];
    std::ifstream infile("blockfile.txt");
    int k = 0;
    while(!infile.eof()){
        //infile.read(another_buff, 2);
        infile >> another_buff[k];
        k = k+1;
    }
    char temp = another_buff[0];
    char temp2 = another_buff[1023];
    char temp3[5] = {'a','b','c','\0','\0'};
    cout<<temp<<temp2<<endl;
    string test_str;
    for(int i=0;i<5;i++){  //make the binary stream of free_block_list
        test_str+= bitset<8>(temp3[i]).to_string();
    }
    bitset<40> testing(test_str);
    
    for(int i=39;i>=0;i--){
        cout<<testing[i];
    }
    */
    /*
    //debugging code to make sure make_super_block is working
    std::ifstream infile("blockfile.txt");

    char temp_buffer[1024];   //dont do anything to the real buffer just yet
    //read block 0 of disk(the super block)
    infile.read(temp_buffer, 1024);

    //cout<<temp_buffer[1023]<<endl;


    Super_block testing = make_super_block(temp_buffer);
    //cout<<testing.inode[125].name[3]<<endl;
    for(int i=0;i<16;i++){
        cout<< testing.free_block_list[i]<<endl;
    }
    //cout<<testing.free_block_list[16]<<endl;
    string test_str;
    for(int i=0;i<16;i++){
        test_str+= bitset<8>(testing.free_block_list[i]).to_string();
        //string s(1,testing.free_block_list[i]);
        //test_str = test_str + s;
    }
    cout<<"Before: "<<test_str<<endl;
    cout<<"does it like strings?"<<endl;
    bitset<128> bset(test_str);  //TODO MAKE THIS PART WORK
    cout<<"yes it does nani?"<<endl;
    cout<<bset[127]<<bset[126]<<bset[125]<<bset[124]<<bset[123]<<bset[122]<<endl;
    cout<<"last 4 bits are: "<<bset[3]<< bset[2]<<bset[1]<<bset[0]<<endl;
    cout<<testing.inode[1].used_size <<endl;
    bitset<8> plz_work(testing.inode[1].used_size);
    cout<<plz_work[7]<<plz_work[6]<<plz_work[5]<<plz_work[4]<<plz_work[3]<<plz_work[2]<<plz_work[1]<<plz_work[0]<<endl;
    //end of debudding code
    */
    return 0;

}
