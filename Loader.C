#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "Loader.h"
#include "Memory.h"

#define ADDRBEGIN 2
#define ADDREND 4
#define DATABEGIN 7
#define COMMENT 28

/*
 * Loader
 * opens up the file named in argv[0] and loads the
 * contents into memory. If the file is able to be loaded,
 * then loaded is set to true.
 */
// This method is complete and does not need to be modified.
Loader::Loader(int argc, char *argv[])
{
   std::ifstream inf; // input file stream for reading from file
   int lineNumber = 1;
   lastAddress = -1;
   loaded = false;

   // if no file name given or filename badly formed, return without loading
   if (argc < 2 || badFile(argv[1]))
   {
      return;
   }
   // printf("got past for some reason\n");
   inf.open(argv[1]);

   // if file can't be opened, return without loading
   if (!inf.is_open())
      return;

   std::string line;
   while (getline(inf, line))
   {
      if (hasErrors(line))
          {
             std::cout << "Error on line " << std::dec << lineNumber
                       << ": " << line << std::endl;
             return;
          }
      if (hasAddress(line) && hasData(line))
        {loadLine(line);}
      lineNumber++;
   }
   loaded = true;
}

/*
 * hasAddress
 * returns true if the line passed in has an address on it.
 * A line that has an address has a '0' in column 0.
 * It is assumed that the address has already been checked to
 * make sure it is properly formed.
 *
 * @param line - a string containing a line of valid input from
 *               a .yo file
 * @return true, if the line has an address on it
 *         false, otherwise
 */
bool Loader::hasAddress(std::string line)
{
	if(line[0] == '0' && line[1] == 'x' && line[6] == ' ' && line[5] == ':')
	{return true;}
	else{return false;}


  // std::cout << "test1: line = " <<line<<"\n";
//   if (line[0] != '0')
  // {
	//        cout << "test2.1 line= "<<line<< "?"<<"\n";
	  //      cout << "testst2 line0= "<<line[0]<< "?"<<"\n";
 //     return false;
  // }
  // else
  // {
    //    cout << "test3 \n";
    // / return true;
  // }
}

/*
 * hasData
 * returns true if the line passed in has data on it.
 * A line that has data does not contain a space
 * at index DATABEGIN.
 * It is assumed that the data has already been checked to
 * make sure it is properly formed.
 *
 * @param line - a string containing a line of valid input from
 *               a .yo file
 * @return true, if the line has data in it
 *         false, otherwise
 */
bool Loader::hasData(std::string line)
{
  // cout<<"test4 \n";

   if (line[DATABEGIN] == ' ')
   {
       // cout<<"test4 \n";

      return false;
   }
   else
   {
       // cout << "test5 \n";

      return true;
   }
}

/*
 * hasComment
 * returns true if line is at least COMMENT in length and
 * line has a | at index COMMENT.
 *
 * @param line - a string containing a line from a .yo file
 * @return true, if the line is long enough and has a | in index COMMENT
 *         false, otherwise
 */
bool Loader::hasComment(std::string line)
{
   if (line.length() >= COMMENT && line[COMMENT] == '|')
   {
      return true;
   }
   else
   {
      return false;
   }
}

/*
 * loadLine
 * The line that is passed in contains an address and data.
 * This method loads that data into memory byte by byte
 * using the Memory::getInstance->putByte method.
 *
 * @param line - a string containing a line of valid input from
 *               a .yo file. The line contains an address and
 *               a variable number of bytes of data (at least one)
 */
void Loader::loadLine(std::string line)
{
   // Hints:
   // Use the convert method to convert the characters
   // that represent the address into a number.
   // Also, use the convert method for each byte of data.
  //` if ()
  // cout << "test6 \n";
   int32_t addressNum = convert(line, ADDRBEGIN, ADDREND-ADDRBEGIN+1);
   bool error;
   int i = DATABEGIN;
   int8_t b;
   Memory * inst = Memory::getInstance();

   while (line[i] != ' ' && line[i] != '|' )
   {
      b = convert(line, i, 2);
     // Memory::getInstance()->putByte(b, addressNum, error);
      inst->putByte(b, addressNum, error);
      i = i + 2;
      Loader::setLastAddress(addressNum);
      addressNum++;
   }
   
}

/*
 * convert
 * takes "len" characters from the line starting at character "start"
 * and converts them to a number, assuming they represent hex characters.
 * For example, if len is 2 and line[start] is '1' and
 * line[start + 1] is 'a' then this function returns 26.
 * This function assumes that the line is long enough to hold the desired
 * characters and that the characters represent hex values.
 *
 * @param line - string of characters
 * @param start - starting index in line
 * @param len - represents the number of characters to retrieve
 */
int32_t Loader::convert(std::string line, int32_t start, int32_t len)
{
   // Hint: you need something to convert a string to an int such as strtoil
   //  int32_t addressNum = convert(line, ADDRBEGIN, 3);
    //    cout << "test7 \n";
   char str[4] = {0,0,0,0};
   int32_t s = start;
   for (int i = 0; i < len; i++)
   {
      str[i] = line[s ];
      s++;
   }

   long answer = strtol(str, NULL, 16);
   return answer;
}

/*
 * hasErrors
 * Returns true if the line file has errors in it and false
 * otherwise.
 *
 * @param line - a string that contains a line from a .yo file
 * @return true, if the line has errors
 *         false, otherwise
 */
bool Loader::hasErrors(std::string line)
{
   // checking for errors in a particular order can significantly
   // simplify your code
   // 1) line is at least COMMENT characters long and contains a '|' in
   //    column COMMENT. If not, return true
   //    Hint: use hasComment
   //
   // 2) check whether line has an address.  If it doesn't,
   //    return result of isSpaces (line must be all spaces up
   //    to the | character)
   //    Hint: use hasAddress and isSpaces
   //
   // 3) return true if the address is invalid
   //    Hint: use errorAddress
   //
   // 4) check whether the line has data. If it doesn't
   //    return result of isSpaces (line must be all spaces from
   //    after the address up to the | character)
   //    Hint: use hasData and isSpaces
   //
   // 5) if you get past 4), line has an address and data. Check to
   //    make sure the data is valid using errorData
   //    Hint: use errorData
   //
   // 6) if you get past 5), line has a valid address and valid data.
   //    Make sure that the address on this line is > the last address
   //    stored to (lastAddress is a private data member)
   //    Hint: use convert to convert address to a number and compare
   //    to lastAddress
   //
   // 7) Make sure that the last address of the data to be stored
   //    by this line doesn't exceed the memory size
   //    Hint: use numDBytes as set by errorData, MEMSIZE in Memory.h,
   //          and addr returned by convert

   // if control reaches here, no errors found
   int32_t val = 0;
   if (!hasComment(line)) { return true; }
   if (!hasAddress(line)) { return !isSpaces(line,0,COMMENT -1); }
   if (errorAddr(line)) { return true; }
   //if (!hasData(line)) { return !isSpaces(line, 5, COMMENT - 1); }
   if (errorData(line, val)) { return true; }
   int32_t ans = convert(line, ADDRBEGIN, ADDREND-ADDRBEGIN+1);
   //std::cout << "last add is: " << lastAddress << "\n";
   if (ans <= lastAddress) {return true; }
   if (ans + val > MEMSIZE) { return true; }
   return false;


   /*
   if (hasComment(line) == true) {
      if (hasAddress(line) == true) {
         if (errorAddr(line) == false) {
            if (hasData(line) == true) {}
            else { return isSpaces(line, 5, COMMENT - 1);            }
         }
         else { return true;}
      }
      else {
         return !isSpaces(line,0,COMMENT -1);
        // return isSpaces(line, 0, COMMENT - 1);
      }
   }
   else {  return true;   }
   return false;
   */
}

/*
 * errorData
 * Called when the line contains data. It returns true if the data
 * in the line is invalid.
 *
 * Valid data consists of characters in the range
 * '0' .. '9','a' ... 'f', and 'A' .. 'F' (valid hex digits).
 * The data digits start at index DATABEGIN.
 * The hex digits come in pairs, thus there must be an even number of them.
 * In addition, the characters after the last hex digit up to the
 * '|' character at index COMMENT must be spaces.
 * If these conditions are met, errorData returns false, else errorData
 * returns true.
 *
 * @param line - input line from the .yo file
 * @return numDBytes is set to the number of data bytes on the line
 */
bool Loader::errorData(std::string line, int32_t &numDBytes)
{
   // Hint: use isxdigit and isSpaces
   //return true if wrong
   numDBytes = 0;
   int j = DATABEGIN;
   for (int i = DATABEGIN; i < COMMENT && line[i] != ' '; i++) {
      if ((isxdigit(line.at(i)) != 0 ) && (isxdigit(line.at(i + 1)) != 0)) {
         numDBytes++;
      }
      else { return true; }
      i++;
      j = i + 1;
   }

   if (isSpaces(line, j, COMMENT - 1)) { return false; }
   else { return true; }

}

/*
 * errorAddr
 * This function is called when the line contains an address in order
 * to check whether the address is properly formed.  An address must be of
 * this format: 0xHHH: where HHH are valid hex digits.
 *
 * @param line - input line from a .yo input file
 * @return true if the address is not properly formed and false otherwise
 */
bool Loader::errorAddr(std::string line)
{
   // Hint: use isxdigit
   if (hasAddress(line) == true) {
      if ((isxdigit(line.at(ADDRBEGIN)) != 0) && (isxdigit(line.at(ADDRBEGIN + 1)) != 0) && (isxdigit(line.at(ADDRBEGIN + 2)) != 0)){
         return false;
      }
      else {
         return true;
      }
   }
   else {
      return true;
   }
}

/*
 * isSpaces
 * This function checks that characters in the line starting at
 * index start and ending at index end are all spaces.
 * This can be used to check for errors
 *
 * @param line - string containing a line from a .yo file
 * @param start - starting index
 * @param end - ending index
 * @return true, if the characters in index from start to end are spaces
 *         false, otherwise
 */
bool Loader::isSpaces(std::string line, int32_t start, int32_t end)
{
   while (line[start] == ' ' && start != end + 1)
   {
      start++;
   }
   if (start == end + 1)
   {
      return true;
   }
   else
   {
      return false;
   }
}

/*
 * isLoaded
 * getter for the private loaded data member
 */
bool Loader::isLoaded()
{
   return loaded;
}

/*
 * badFile
 * returns true if the name of the file passed in is an improperly
 * formed .yo filename. A properly formed .yo file name is at least
 * four characters in length and ends with a .yo extension.
 *
 * @return true - if the filename is improperly formed
 *         false - otherwise
 */
bool Loader::badFile(std::string filename)
{
   // Hint: use std::string length method and C strcmp (or std::string find
   //       or std::string at or ...)

   // The path is 42 chars

   std::string str(".yo");
   if ((filename.find(str) != std::string::npos) && (filename.length() >= 4))
   {
      //std::cout << "good file" << "\n";
      return false;
   }
   else
   {
      //std::cout << "bad file"<< "\n";

      return true;
   }
}
