#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>

//convert the input into hex
std::string hex_converter(char* data, int size){

  std::stringstream	 ss;

  for(int i = size-1; i >= 0; i--)
	  ss<< std::hex << (unsigned int)(unsigned char)data[i];
  return ss.str();


}

// Read the _size_ number of bytes from the notes file
std::string data_reader(std::ifstream& file, int size){
  char		data[size];
 
  file.read (data, size);
  return hex_converter(data, size);
}

// Read null terminated string from the file
std::string string_reader(std::ifstream& file){
  char		c;
  std::string	tempstr;

  while ((c = file.get()) != '\0') {
	tempstr += c;
   } 
  return tempstr;
}

//Read LENGTH
//LENGTH is of size int32 with first byte containing 
//the number of 4-byte data that follows, followed by 
//3 Null bytes
int length_reader(std::ifstream& file){
  std::stringstream	lengthstream;
  int			length;

  lengthstream << data_reader(file, 1);
  lengthstream >> std::hex >> length;
  data_reader(file, 3);
  return length;

} 

std::string tag_reader(std::ifstream& file){
  char 	   	 	tag;
  std::stringstream	tagstream;
  data_reader(file, 2);
  tag = file.get();
  tagstream << std::hex<< data_reader(file, 1) << std::hex << tag ;
  return tagstream.str();

}
int gcno_dumper(std::string filename){
  int 			size;
  uint32_t		block_count;
  std::ifstream file (filename, std::ios::binary);
  std::ofstream dumpfile ( "gcno_dump.txt"); 
  
  size = sizeof(int);

  if (file.is_open()){
	 
	  dumpfile<<"\n===========================================================\n"
	           <<" GCNO DUMP FOR "<< filename
		   <<"\n===========================================================\n";
	  //
	  // READING MAGIC, VERSION, STAMP
	  //
          
	  //reading magic 

          dumpfile << "MAGIC    :  0x"<< data_reader (file, size) << std::endl;
 
	 //read version
	 // after reading the first 4 bytes,
	 // we know that the next int32 ( i.e 4bytes)
	 // is version
	 dumpfile <<"VERSION  :  0x"<< data_reader (file, size) <<std::endl; 
		 
	 //read stamp
	 //next 4 bytes is stamp
	 dumpfile<< "TIMESTAMP:  0x"<< data_reader(file, size) <<std::endl;

	 //
	 //READING RECORD
	 //A record has a int32: tag, int32: length, and variable amount of data
	 // 

	 //
	 //READING NOTES
	 //
         dumpfile << std::endl << "READING NOTE RECORDS \n" << std::endl; 
	 
	 //note: unit function-graph
	 //unit: header int32:checksum string:source
	 dumpfile<<"FUNCTION TAG         :  "<< tag_reader(file)<<std::endl;	 
	 dumpfile<<"FUNCTION LENGTH      :  "<< length_reader(file) <<std::endl;
	 dumpfile<<"FUNCTION IDENT       :  0x"<< data_reader(file, size)<<std::endl;
	
	 //function-graph : announce_function basic_blocks {arcs | lines}
	 //announce_function: header int32:ident int32:checksum string: name
	 //            	      string: source int32: lineno
	 dumpfile<<"LINE_NO CHECKSUM     :  0x"<< data_reader(file, size) <<std::endl;
	 dumpfile<<"CFG CHECKSUM         :  0x"<< data_reader(file, size) <<std::endl;

	 dumpfile<<"FUNCTION NAME LENGTH : "<< length_reader(file) <<std::endl;
	 dumpfile<<"FUNCTION NAME        : "<< string_reader(file) <<std::endl;
         dumpfile<<"SOURCE NAME LENGTH   : "<< length_reader(file) <<std::endl;
         dumpfile<<"SOURCE NAME          : "<< string_reader(file) <<std::endl;
	 dumpfile<<"LINE NUMBER          : "<< length_reader(file) <<std::endl;

	 //basic_block : header int32:flags*
	 //arcs : header int32: block_no arc*
	 //arc : int32: dest_block int32: flags
	 //lines: header int32: block_no line*
	 //       int32:0 string:NULL
	 //line: int32:line_no | int32:0 string:filename

	 dumpfile<<"\nBLOCK TAG            : "<< tag_reader(file) << std::endl;
	 block_count = length_reader(file);
	 dumpfile<<"NO. OF BLOCKS        : "<< block_count << std::endl;
	 for(int i=0, e = block_count; i != e; ++i){ 
	 //read block flags
	 	if ( data_reader(file, size) == "0000")
			continue;
	 	dumpfile<<"FLAGS                : "<< data_reader(file, size) << std::endl;
	 }
	 //read arcs per Basic_Block
	 for (int i = 0 ; i < block_count - 1; ++i){
	     dumpfile<< "\n\nARCS TAG             : " << tag_reader(file) << std::endl;
	     int arcs_count = length_reader(file);
	     dumpfile<< "ARCS LENGTH          : " << arcs_count << std::endl; 
	     dumpfile<< "BLOCK NO.            : " << data_reader(file, size) << std::endl;
	     dumpfile<< "DEST. BLOCK          : " << data_reader(file, size) << std::endl;
             dumpfile<< "FLAGs                : " ;
	     for (int j = 0; j < arcs_count-2; ++j)
	         dumpfile<< data_reader(file, size) << ",";
	     dumpfile<<"\n\n";
	}
	int block_number = 0;
	std::string tag;
	int length_buff;

	while( block_number  < block_count - 1){
	tag 		= tag_reader(file);
	length_buff 	= length_reader(file);
	block_number 	= length_reader(file);
	if (block_number > block_count) break;

	dumpfile<< "LINE TAG             : " << tag << std::endl;
	dumpfile<< "LINE LENGTH          : " << length_buff << std::endl;
	dumpfile<< "BLOCK NUMBER         : " << block_number << std::endl;
	dumpfile<< "LINE NO.             : " << data_reader(file, size) << std::endl;
	length_buff -= length_reader(file); //subtracting the length of string
	length_buff -= 2; //for block no. and line no.
	//dumpfile << "FILENAME LENGTH     : " << length_reader(file) << std::endl;
        dumpfile<< "FILENAME             : " << string_reader(file) << std::endl;
	length_buff -= 1; // int32: 0
	//data_reader(file, length_buff*4);
	file.seekg( length_buff*4, std::ios::cur);
	dumpfile<< "\n";
	}

        return 1;

  }

}

int main(int argc, char** argv){
    if(argc < 2)
    	std::cerr<<"NO gcno files provided\n";
    else
    	for (int i = 1 ; i < argc; i++)
	    gcno_dumper( argv[i]);

}
