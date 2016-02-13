#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <cctype>
#include <sstream>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <utility>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>

//defines max foreseeable ligetSubEnne length in the freqTable.txt database
#define MAX_LINE_LEN 256

#define NGRAM 5
#define MIN_MODEL_SIZE 100 //Minimum items sufficient to define an ngram model. This is arbitrary, for the sake of code error-checks.
#define READ_SZ 4095
#define BUFSIZE 4096
#define MAX_WORDS_PER_PHRASE 256  //these params are not very safe in updateNgramTable--possible segfaults
#define MAX_PHRASES_PER_READ 256
#define MAX_TOKENS_PER_READ 1024  //about bufsize / 4.
#define MAX_SENT_LEN 256  //not very robust. but a constraint is needed on the upperbound of sentence length, for phrase parsing data structures.
// avg sentence length is around 10-15 words, 20+ being a long sentence.
//#define PHRASE_DELIMITER '#'
//#define WORD_DELIMITER ' '
#define FILE_DELIMITER '|'
#define PERIOD_HOLDER '+'
#define ASCII_DELETE 127
#define INF_ENTROPY 9999  //constant for infinite entropy: 9999 bits is enormous (think of it as 2^9999) 
#define INF_PERPLEXITY 999999
#define NLAMBDASETS 8
#define NLAMBDAS 7




typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long int U64;

using namespace std;

struct DateTime{
  U8 hr;
  U8 day;
  U8 month;
  U8 year;
};


bool isDelimiter(char c, const string& buf)
{
  for(int i = 0; i < buf.length(); i++){
    if(c == buf[i])
      return true;
  }

  return false;
}




int tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims)
{
  int i, tokCt;
  int dummy;

  if((buf == NULL) || (buf[0] == '\0')){
    ptrs[0] = NULL;
    cout << "WARN buf==NULL in tokenize(). delims: " << delims << endl;
    return 0;
  }
  if(delims.length() == 0){
    ptrs[0] = NULL;
    cout << "WARN delim.length()==0 in tokenize()." << endl;
    return 0;
  }

  //consume any starting delimiters then set the first token ptr
  for(i = 0; isDelimiter(buf[i], delims) && (buf[i] != '\0'); i++);
  //cout << "1. i = " << i << endl;

  if(buf[i] == '\0'){  //occurs if string is all delimiters
    cout << "buf included only delimiters in tokenize(): i=" << i << "< buf: >" << buf << "< delims: >" << delims << "<" << endl;
    ptrs[0] = NULL;
    return 0;
  }

  //assign first token
  ptrs[0] = &buf[i];
  tokCt = 1;
  while(buf[i] != '\0'){

    //cout << "tok[" << tokCt-1 << "]: " << ptrs[tokCt-1] << endl;
    //cin >> dummy;
    //advance to next delimiter
    for( ; !isDelimiter(buf[i], delims) && (buf[i] != '\0'); i++);
    //end loop: buf[i] == delim OR buf[i]=='\0'

    //consume extra delimiters
    for( ; isDelimiter(buf[i], delims) && (buf[i] != '\0'); i++){
      buf[i] = '\0';
    } //end loop: buf[i] != delim OR buf[i]=='\0'

    //at next substring
    if(buf[i] != '\0'){
      ptrs[tokCt] = &buf[i];
      tokCt++;
    }
  } //end loop: buf[i]=='\0'

  //cout << "DEBUG first/last tokens: " << ptrs[0] << "/" << ptrs[tokCt-1] << "<end>" <<  endl; 

  ptrs[tokCt] = NULL;

  return tokCt;
}

/*
  Converts a datetime string "11/22/2222 22:22 PM" to a U32 value.
  byte ordering: hr|day|month|years_since_1970
*/
DateTime strToDateTime(char buf[])
{
  DateTime ret = {-1,-1,-1,-1};
  U8 temp = 0;
  string s = " ";
  int nTokens;
  char* tokens[8];
  char* date[8];
  char* time[8];

  //first split on whitespace to get tokens [11/22/1222, 05:40, PM]
  nTokens = tokenize(tokens,buf,s);
  if(nTokens != 3){
    cout << "error wrong num tokens in strToDateTime" << endl;
    return ret;
  }

  //get the date tokens
  s = "/";
  nTokens = tokenize(date,tokens[0],s);
  if(nTokens != 3){
    cout << "error wrong num date tokens in strToDateTime" << endl;
    return ret;
  }
  //get the day
  ret.day = (U8)atoi(date[1]);
  //get the month
  ret.month = (U8)atoi(date[0]);
  //get the year
  ret.year = (U8)(atoi(date[2]) - 1970);

  //get the hour token. no need to tokenize, just chop at colon :
  tokens[1][2] = '\0';
  ret.hr = atoi(tokens[1]);
  if(!strncmp(tokens[2],"PM",2)){ //add 12 hours for PM strings
    ret.hr += 12;
  }

  return ret;
}





/*
  Search a buffer for a datetime.
  Returns 0 if none found in this buf, else return a U32 date-time encoding.
*/
DateTime searchDateTime(char buf[MAX_LINE_LEN])
{
  bool match;
  int span, start, end, i, j, k;
  DateTime ret = {-1,-1,-1,-1};
  char pattern[] = "dd/dd/dddd dd:dd aa\0";
  int endcmp = MAX_LINE_LEN - strnlen(pattern,64); //point of no return, after which this buf could not hold pattern

  //look for forward slash
  begin:
  for(i = 0; i < endcmp && buf[i] && buf[i] != '/'; i++){
    //cout << "searching for slash... endcmp=" << endcmp << "  buf[" << i << "]=" << buf[i] << endl;
  }

  if(i >= endcmp || !buf[i]){ //ret if at end of string
    return ret;
  }

  if(buf[i+3] != '/' || buf[i+11] != ':'){ //check to be somewhat sure this is a hit
    i++;
    //cout << "ret begin" << endl;
    goto begin;
  }

  //found slash. now do pattern match, and get start/end indices of pattern
  i -= 2;
  if(!isdigit((int)buf[i])){  //dates can have single digit months, this compensates
    i++;
    k = 1;
  }
  else{
    k = 0;
  }

  span = strnlen(pattern,64);
  match = true;
  for(j = i; match && j < span+i; j++, k++){
    //cout << "spooling... " << endl;
    switch(pattern[k]){
      case 'd':
          match = isdigit((int)buf[j]);
        break;
      case '/':  //skip punct
          match = '/' == buf[j]; 
        break;
      case ' ':
          match = ' ' == buf[j];
        break;
      case ':':
          match = ':' == buf[j];
        break;
      case 'a':
          match = (buf[j] == 'P') || (buf[j] == 'A') || (buf[j] == 'M');
        break;
      default:
        break;
    }
  }

  if(match){
    start = i;
    end = i + span;
    cout << "HIT start=" << start << "  end=" << end << endl;
    buf[end] = '\0';
    cout << "datetime is: >" << &buf[start] << "<" << endl;
    ret = strToDateTime(&buf[start]); //extract datetime as our U32
  }
  else{
    //cout << "hkjhkjhk" << endl;
    goto begin;
  }

  //strToDateTime();

  return ret;
}


int main(void)
{
  char test[] = "kjhsdkjhkjfhkhah dh vkjhdfkjh 44/22/2004 01:45 PM fkhfjh fkjh";
  char nodt[] = "lkfjh fhew vhkh kh234 k3h kj54ht kj45h kj4h rjk 2hr3kjrh 23jkrh ";
  char test2[] = "kjhsdkjhkjfhkhah dh vkjhdfkjh 4/22/2004 01:45 AM fkhfjh fkjh";
  char test3[] = "h 1/12/1999 04:56 PM kjhkjh";
  DateTime res;
  string s;


/*
  cout << "t1" << endl;
  res = searchDateTime(test);
  cout << "result: hr=" << (int)res.hr << " day=" << (int)res.day << " month=" << (int)res.month << " year=" << (int)res.year << endl;
  cout << "t2" << endl;
  res = searchDateTime(nodt);
  cout << "result: hr=" << (int)res.hr << " day=" << (int)res.day << " month=" << (int)res.month << " year=" << (int)res.year << endl;
  cout << "t3" << endl;
  res = searchDateTime(test2);
  cout << "result: hr=" << (int)res.hr << " day=" << (int)res.day << " month=" << (int)res.month << " year=" << (int)res.year << endl;
  cout << "t4" << endl;
  res = searchDateTime(test3);
  cout << "result: hr=" << (int)res.hr << " day=" << (int)res.day << " month=" << (int)res.month << " year=" << (int)res.year << endl;
*/

  s.reserve(1024);
  cout << "size is: " << s.size() << endl;
  s = "hello";
  cout << "size is: " << s.size() << endl;
  s.clear();
  cout << "size is: " << s.size() << endl;
  s = "world";
  cout << "size is: " << s.size() << endl;
  s[0] = '\0';
  cout << s << " size is: " << s.size() << endl;
  s += "adios";
  cout << s << "size is: " << s.size() << endl;







  return 0;
}








