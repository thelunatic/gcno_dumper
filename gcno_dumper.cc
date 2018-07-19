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

int gcno_dumper(std::string filename){
  int 			size;

  std::ifstream file (filename, std::ios::binary);
  std::ofstream dumpfile ( "gcno_dump.txt"); 
  
  size = sizeof(int);

  if (file.is_open()){
	  
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
	 //TODO: tag_reader
	 data_reader(file, 3);
	 dumpfile<<"FUNCTION TAG         :  "<< data_reader(file, 1)<<std::endl;	 
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

	 //TODO: tag_reader
	// data_reader(file, 2);
	// dumpfile<<"BLOCK TAG            : "<< data_reader(file, 2) << std::endl;
	// dumpfile<<"BLOCK LENGTH         : "<< length_reader(file) << std::endl;
	 //dumpfile<<"FLAGS                : "<< data_reader(file, size) << std::endl;

	 return 1;

  }

}

int main(int argc, char** argv){
	gcno_dumper( argv[1]);

}
