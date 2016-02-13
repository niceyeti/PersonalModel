#include <list>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <wait.h>
#include <utility>
#include <cstdio>
#include <string.h>
#include <algorithm>
#include <cmath>

//using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::cin;
using std::map;
using std::list;
using std::sort;
using std::string;

#define BUFSIZE 4096
#define MAX_SENT_LEN 128

typedef struct freqTup{
  string s;
  int ct;
}FreqTup;

int findChar(const char buf[], char c, int len)
{
  int i = 0;

  if((buf == NULL) || (len <= 0)){  
    cout << "ERROR buf null or len < 0 in findChar()" << endl;
    return 0;
  }

  for(i = 0; (i < len) && (buf[i] != c) && (buf[i] != '\0'); i++);

  if(i == len)
    i--;

  return i;
}

char* getLine(int openfd, char buf[], int len)
{
  int n, pos;

  n = read(openfd,buf,len);
  if(n <= 0){
    return NULL;
  }

  /*
    Reset file position to position of first newline char
    If we read n chars, then we need to back up to 'pos' index,
    which is back from the current position by (n - pos)
  */
  pos = findChar(buf,'\n',n);
  buf[pos] = '\0';
  pos = pos - n + 1;
  printf("<pos,n>:<%d,%d> getLine() returned line: >%s<\n",pos,n,buf);


  if( pos < 0){
    lseek(openfd, pos, SEEK_CUR);
  }

/*
  cout << "getLine() returned line: " << buf << endl;
  cin >> n;
*/
  return buf;
}

bool isPhraseDelimiter(char c, const string& phraseDelims)
{
  int i;

  if(c == 34){
    return true;
  }

  for(i = 0; (phraseDelims[i] != '\0') && (i < phraseDelims.length()); i++){
    if(c == phraseDelims[i]){
      return true;
    }
  }

  return false;
}


/*
  Logically the same as strtok: replace all 'delim' chars with null, storing beginning pointers in ptrs[]
  Input string can have delimiters at any point or multiplicity

  Pre-condition: buf must be null terminated

  Testing: This used to take a len parameter, but it was redundant with null checks and made the function 
  too nasty to debug for various boundary cases, causing errors.
*/
int tokenize(char* ptrs[], char buf[BUFSIZE], char delim)
{
  int i, tokCt;

  if(buf == NULL){
    cout << "WARN buf==NULL in tokenize(). delim: " << delim << endl;
    return 0;
  }

  //consume any starting delimiters then set the first token ptr
  for(i = 0; (buf[i] == delim) && (buf[i] != '\0'); i++);
  //cout << "1. i = " << i << endl;

  if(buf[i] == '\0'){  //occurs if string is all delimiters
    cout << "NULL in tokenize: i--> " << i << "< buf: >" << buf << "< delim: >" << delim << "<" << endl;
    //cout << &buf[-32] << endl;
    ptrs[0] = NULL;
    return 0;
  }

  //at first token
  ptrs[0] = &buf[i];

  for(tokCt = 1; buf[i] != '\0'; i++){

    //advance to next delimiter
    for( ; (buf[i] != delim) && (buf[i] != '\0'); i++);
    //end loop: buf[i] == delim OR buf[i]=='\0'

    //consume extra delimiters
    for( ; (buf[i] == delim) && (buf[i] != '\0'); i++){
      buf[i] = '\0';
    } //end loop: buf[i] != delim OR buf[i]=='\0'

    //at next substring
    if(buf[i] != '\0'){
      ptrs[tokCt] = &buf[i];
      tokCt++;
    }
  } //end loop: buf[i]=='\0'

  cout << "DEBUG first/last tokens: " << ptrs[0] << "/" << ptrs[tokCt-1] << "<end>" <<  endl; 

  ptrs[tokCt] = NULL;
  //tokCt -= 1;

  return tokCt;
}



int seekLastPhraseDelimiter(const char buf[BUFSIZE], int len)
{
  int i;
  string phdel = "#";
  //start at end of string, spool back to rightmost of group of delimiters
  for(i = len-1; (i >= 0) && !isPhraseDelimiter(buf[i],phdel); i--){}
  //end loop: i points at a phrase delimiter OR i == -1

  // currently at last, rightmost delim. now spool left to get leftmost delim in this group
  for( ; (i >= 0) && isPhraseDelimiter(buf[i],phdel); i--){}
  //end loop: i==-1 OR buf[i] NOT_IN delimiters[]

  //we are off-by-one after i-- stmt in the last loop to execute, so increment by one
  if(i >= 0){
    i++;
  }

  //cout << "last delimiter is >" << buf[i] << "<" << endl;

  return i;
}

bool isDelimiter(char c, const string& delims)
{
  int i;

  for(i = 0; i < delims.length(); i++){
    if(c == delims[i]){
      return true;
    }
  }

  return false;
}

/*
   Finds index of the nth-to-last token in buf[]. This is necessary to adjust our file position such that we
   get a pseudo-continuous stream of ngrams from a file. For instance, the last three words in a buffer
   have only one 3-gram. We back the file position up to the second-to-last word in the buffer, so the next read
   will include the last ending two words as part of its input.
   
  Example: given "$the$dog$was###brow\0" return index of 'd' in 'dog' if nth==2 (eg, usage for a 3-gram model)

  Notes: for strings with an insufficient number of grams (words), returns -1.
  This occurs if ngrams <= nth, NOT just ngrams < nth!
*/
int seekNthLastTokenIndex(int nth, const char buf[BUFSIZE], int len)
{
  int i, nToken;
  string ph = "#";

  //first find the end boundary of this buffer (the leftmost '#' in above example)
  i = seekLastPhraseDelimiter(buf,len);
  i--;

  //begin loop: last delim points at 's' in "$the$dog$was###brow\0"
  for(nToken = 0; nToken < nth; nToken++){

    //spool to leftmost delim in this group
    for( ; (i >= 0) && isDelimiter(buf[i], "$#,;"); i--){}
    //end loop: i points at a non-delimiter OR i == -1

    //spool to next leftward delimiter
    for( ; (i >= 0) && !isDelimiter(buf[i], "$#,;"); i--){}
    //end loop: i points at any delimiter OR i == -1

  }

  //off by one after last last loop, so increment by one.
  if(i >= 0){
    i++;
  }

/*
  //finally, spool left to the beginning of this word
  for( ; (i >= 0) && !isDelimiter(buf[i]); i--){}
  //end loop: i points at any delimiter OR i == -1
  //we are off-by-one after i-- stmt in the last loop to execute, so increment by one
  if(i >= 0){
    i++;
  }
*/
  //cout << "last delimiter is >" << buf[i] << "<" << endl;

  return i;
}



/*
  Get the most likely n-Gram string, argmax(word,ngrams).
  File io version. Expect frequency file to be sorted.
  Must work for nGram models of any n.
*/
string getMaxFromFile(char* fname, string word)
{
  int i, fd, n, k = 0, pos, max, freq;
  bool bailout, found;
  string gramStr;
  char buf[2048];
  //char line[128];

  fd = open(fname, O_RDONLY);
  if(n < 0){
    cout << "ERROR file >" << fname << "< not opened in getMaxFromFile()" << endl;
    return "NOT_FOUND";
  }
  
  //we need a space as a right-anchor for word comparisons (or else strncmp("your","yourselves","your".length()) will return 0)
  if(word[word.length()-1] != ' '){
    word += " ";
  }

  //STATE 1: spool to location of line starting with first letter of word
  pos = -1;
  while((pos < 0) && (n = read(fd, buf, 2048))){
    for(i = 0; (i < (n - 2)) && (pos < 0); i++){
      if((buf[i] == '\n') && (word[0] == buf[i+1]) && (word[1] == buf[i+2])){ //matches on first two letters of word
        pos = i + 1;
        //cout << "break i==" << i << "  buf[i+1]==" << buf[i+1] << " more: >>" << &buf[pos+1] << endl;
        break;
      }
    }
  }

  //occurs if no word in file starting with first letter of word
  if(pos < 0){
    cout << "Word >" << word << "< not found in file " << fname << endl;
    close(fd);
    return "NOT_FOUND";
  }
  //seek back to first occurrence of letter
  pos = pos - n;
  lseek(fd, pos, SEEK_CUR);

  //STATE 2: call getline, matching first word of each line to word. bail if first letter of line is > word[0]
  bailout = false;  //file is sorted, so bailout when first letter of string is alphabetically beyond start letter of word
  max = -1;
  while(!bailout && getLine(fd,buf,MAX_SENT_LEN)){
    //if no more of this word
    if(buf[0] > word[0]){ //assumes file in lowercase
      bailout = true;
      break;
    }
    //word hits: save string if greater than current max
    if( strncmp(buf, word.c_str(), word.length()) == 0 ){
      cout << "Found match word/str: " << word << "/" << buf << endl;
      i = findChar(buf,'\0',MAX_SENT_LEN);
      cout << "debug i is: " << i << endl;
      cin >> n;
      buf[i] = '\0';
      i = findChar(buf,'|',MAX_SENT_LEN);
      freq = atoi(&buf[i+1]);
      if(freq > max){
        max = freq;
        gramStr = buf;
      }
    }
  }

  if(max == -1){
    gramStr = "NOT_FOUND";
  }

  close(fd);

  cout << "returning gramStr: " << gramStr << endl;
  return gramStr;
}



int main(void)
{
  int ofd, fd, i, j, n, k;
  char buf[1024];
  char* strs[16];
  char str1[64];
  //char test[8];
  string test = "AA$BB$CC#DD";
  string delims = "$#,;'";
  map<string,freqTup> model;

  cout << "max size is: " << model.max_size() / 1000000 << " M items" << endl;

  cin >> j;
  cout << "got:: " << j << endl;

  cin.ignore();
  fflush(stdin);

  getline(cin,test);
  cout << "got: " << test << endl;




/*
  cout << test << "<< test string: " << endl;
  for(int i  = 0; i < test.length(); i++){
    cout << i;
  }
  cout << endl;
  i = seekNthLastTokenIndex(1, test.c_str(), test.length());
  cout << "1st to last: i==" << i << endl;
  i = seekNthLastTokenIndex(2, test.c_str(), test.length());
  cout << "2st to last: i==" << i << endl;
  i = seekNthLastTokenIndex(3, test.c_str(), test.length());
  cout << "3st to last: i==" << i << endl;
  i = seekNthLastTokenIndex(4, test.c_str(), test.length());
  cout << "4st to last: i==" << i << endl;
*/





/*
  //tokenize() testing
  cout << "CASE 1>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"#######abc def#hjhj##kjkhjkh####kjkj");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE 2>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"#abc def#hjhj##kjkhjkh####kjkj");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE 3>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"abc def#hjhj##kjkhjkh####kjkj");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE 4>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"#######abc def#hjhj##kjkhjkh####kjkj#");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE 5>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"#######abc def#hjhj##kjkhjkh####kjkj#####");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE 6>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"#######abc def#hjhj##kjkh\0jkh####kjkj");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE 7>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"#####\0##abc def#hjhj##kjkh\0jkh####kjkj");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  } 
  cout << "CASE 8>>>>>>>>>>>>>>>>>>>>>>" << endl;
  buf[0] = '\0';
  buf[1] = '\0';
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  }
  cout << "CASE len == buf null term test>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"abc#efg\0");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  } 
  cout << "CASE len == buf delim term test>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"abc#efg#");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  } 
  cout << "CASE len < buf delimi term test>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"abc#efg#");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  } 
  cout << "CASE len < buf null term test>>>>>>>>>>>>>>>>>>>>>>" << endl;
  strcpy(buf,"abc#efg\0");
  k = tokenize(strs,buf,'#');
  cout << "k=" << k << " tokens:" << endl;
  for(i = 0 ; strs[i] != NULL; i++){
    cout << i << ": " << strs[i] << "<<" << endl;
  } 
*/

  


  /*
  while( getLine(fd,buf,1024) != NULL){
    cout << "line: " << buf << endl;

    cin >> i;
    if(i == 3){
      goto end_prog;
    }
  } 
  */

/*
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  printf("got line: %s", getLine(fd,buf,1024));
  j = tokenize(strs,buf,'|',1024);  
  printf("j: %d\ntok1: %s\ntok2: %s\n",j,strs[0],strs[1]);
*/

/*
  strcpy(str1,"blah blah|23");
  printf("input str: %s\n",str1);
  j = tokenize(strs,str1,'|',64);  
  printf("j: %d\n",j);
  for(i = 0; strs[i] != NULL; i++){ printf("tok%d: %s\n",i,strs[i]); }

  strcpy(str1,"no tokens");
  printf("input str: %s\n",str1);
  j = tokenize(strs,str1,'|',64);  
  printf("j: %d\n",j);
  for(i = 0; strs[i] != NULL; i++){ printf("tok%d: %s\n",i,strs[i]); }

  strcpy(str1,"three tokens|23|jhkjh");
  printf("input str: %s\n",str1);
  j = tokenize(strs,str1,'|',64);  
  printf("j: %d\n",j);
  for(i = 0; strs[i] != NULL; i++){ printf("tok%d: %s\n",i,strs[i]); }

  strcpy(str1,"many tokens|23|jhkjh|||||sjlksjh");
  printf("input str: %s\n",str1);
  j = tokenize(strs,str1,'|',64);  
  printf("j: %d\n",j);
  for(i = 0; strs[i] != NULL; i++){ printf("tok%d: %s\n",i,strs[i]); }
*/

/*
  string word = "you";
  word = getMaxFromFile("5gram.ng",word);
  cout << "result: " << word << endl;
*/
/*
  fd = open("2gram.ng", O_RDONLY);
  if(fd < 0){
    cout << "bombed out...";
    return 0;
  }

  for(i = 0; i < 1024; i++){
    buf[i] = '!';
  }
  i = read(fd,buf,1024);
  for(k = 0; k < 5; k++){
    putchar(buf[k]);
    printf("%d\n",buf[k]);
  }
*/

/*
  fd = open("../HuckleberryFinn.txt", O_RDONLY);
  if(fd < 0){
    cout << "bombed out 1...";
    return 0;
  }
  ofd = open("../noReturns", O_WRONLY);
  if(fd < 0){
    cout << "bombed out 2...";
    return 0;
  }

  while(n = read(fd,buf,1024)){
    for(i = 0; i < 1024; i++){
      if((buf[i] == '\r') || (buf[i])){
        buf[i] = ' ';
      }
    }
    write(ofd,buf,n);
  }
*/


  end_prog:
  //close(fd);
  //close(ofd);

  return 0;
}








