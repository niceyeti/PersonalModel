/*

  *corpus input must be in ASCII

  huck finn - based n-gram model. It's not such a bad personal model,
  since most of Huck's speech is highly personalized, using lots of slang
  and other informal structures.

  Model: only uses phrase structures (delimited by {.?!;--} ). This should provide slightly better
  context-sensitivity.

  LOL: 
    Huck Finn 3-gram max:  <by and by||85>
              4-gram max:  <and by and by||24>
              2-gram max:  <in the||432>
              5-gram max:  <the king and the duke||15>
              6-gram max:  <i made up my mind i||4>
              7-gram max:  <i made up my mind i wouldn't||3>
              8-gram max:  <and i see it warn't no use for||2>
             10-gram max:  <and i see it warn't no use for me to||2>

  Issues:

  -Foul language, etc?
  -phrase delimiters are finding their way into keys somehow. see output, grep for "huck#" [RESOLVED]
  -dropping and stemming: tense, plurality, etc. [DEFER]
  -improve efficiency of parsing; pipe-filter structure is equivalent to nested loops, and there is other nesting as well. [RESOLVED]
   this might help with very, very large data sets.
  -there are many cases when periods do not end a sentence that need to be handled ("Mr." "Mrs." "Ms." "Co." etc.). Only relevant to contextual parsing. [OPEN]
  -getline() len parameter is screwy. using findchar() makes no guarantee an entire line will be read properly

  COCA: I suspect these models were built without sentence context. terminal sequences like "briefed on the matter" (which usually
  occurs at the end of the sentence), yields a lot of paths like "briefed on the matter the" "briefed on the matter a" which most likely
  were parsed from input such as "briefed on the matter. A..." and "briefed on the matter. The..." But note these also indicate
  normal grammatical continuations: "briefed on the matter, the report stated..." etc.
  This shows that significant gains with Markov-based english modeling may come from basing them on sentences, rather than raw word streams.
  Or, possible to use <start> and <end> tags as "words", giving "briefed on the matter <end>" as some sense of sentence termination.
  Speech is not a raw stream, which shows one serious flaw with the COCA models.

  TODO:
  clean and designate numerical data types. I'm using unsigned long in many places; I'd assumed these were 64 bit, but probably are 32.
  online notes indicate multimap.clear() invalidates all pointers to table; however, does this apply to pointers to the model itself?
  Parsing has tons of "stateful" related methods, which is required but also sucks. Be mindful of reducing this as much as possible;
  there is a conflict between catching every single parsing exception vs. trying to keep things stateless.

  When testing various model queries, be sure to feed it queries with the highest branching factor, such as queries ending with "the"
  "to" "as" "a" and other very common words. These common words can be found by sorting through the one-gram table.

  TODO: optimize the prediction (bool,real,dynamic,static) methods. They... do a lot of unnecessary work. For one, the list/filter/sort
  stuff incurs heavy query-time overhead.

  TODO: replace tokenize() function calls with Boost::split() if it suffices the same requirements (returning num tokens, etc).

  TODO: replace underlying data n-gram data structure with something more efficient, compact, like a trie. But be
  mindful that using this will alter data model access, and needs to be evaluated. For instance, A two layer map structure
  (with one for each n-gram model) allows storing references to items using KeyIt and SubkeyIt, which is very nice and simple.
  For multiply nested maps (eg, to simulate a trie), storing references to items would require some sort of level parameter.
  To the point, you would need different iterators for each level in the trie: map<string,double>::iterator for leaf nodes,
  and something like map<map<map<map<map<...>>>>>::iterator to access the upper levels.


  FYI: Don't write contractions tables, translation stuff, etc. Apache SOLR has all this stuff. Download Apache SOLR
  and drill into a "conf" directory, where there are a bunch of these sorts of text parsing resources already built.

  Spell-checking, collation, etc: see Levenshtein-distance algorithm (wikipedia). Also Norvig "Spell-Checker".


  VERY IMPORTANT!! Using n-gram model lookups, it is possible to have zero results if the user inputs a word sequence where
  the last word is unknown to our models. For these, we would need to backoff somehow to the previous sequence, to generate
  possible continuations. This behavior can be observed by running generate() and noticing where/when it terminates when the proc
  encounters "<UNK>" as a result.

  If by any chance we end up going the mmap route or similar routes, look up the Boyer-Moore and KMP algorithms for efficient string matching,
  searching some buffer for the start of a subset/key.
  For multicore architecture could also use multithreading or other tricks, eg, using multiple buffers: read into one, while processing
  the other, swap, continue, etc. or similar map-style methods of delegating work and so on.
  Most promising direction (in terms of generality) may be to create a hash-map of keys in the file, each of which points to two
  offsets: begin and end, where begin is the file byte-offset of the first occurrence of the key, and end is the offset of the last occurrence (or end of it).
  We would then index this key into our map, which would return begin and end. Then, seek to begin, and read (end - begin) bytes (basically).
  Further gains could be acquired by splitting the COCA-ngram files by letter: this partitions the query subset even further, although
  simply lseek'ing over a large file probably doesn't cost more (it may even cost less! its only a single operation, whereas the system
  calls and other methods for splitting the files may cost more). Such a method needs to be able to update itself, at least as needed.
  For instance, we may want to sparsify our ngram models by simply removing the least likely ngram from each subset (tail smoothing) as
  described in Schutze, to give us a better probability distribution of only the most significant n-grams. Note this implies a lot of
  exception handling and other validation, since this is a purely numerical method. Also, the general pattern here would be lseek+read,
  for which pread should be used instead.

  std::multimap has no reserve() function to allow reserving chunks of memory. We may get around this using allocators instead,
  since the n-gram tables are so huge, and likewise, their size known in advance from previous runs. Though I'm a bigger fan
  of either memory mapped files, or premaking a hash-file of subset file offsets.

  All of the predictWord functions can probably be optimized by various grammatical and numerical observation. I suspect
  that Bayes rule, redundant normal constants, or other observations may allow certain calculations to be removed or improved.


  neural perceptron use batch approach: enumerate all errors, then adjust, per test data run (?)
  lookup simple nlg

  Need to determine how to pre-process the text.
  -First-person speech in this book is way too loose for an n-gram model. Might ignore all
  phrases enclosed by double quotes. If used it will require heavy parsing.
  -preserve contractions. Ideally could map these into their formal forms, but often recognizing
  contractions as unique from the words they contract seems reasonable...


  Possible classes:
    phraseStructure<length, order>
    corpus<phraseDelim, wordDelim, phrases, etc ???>


  Standard:
    HACK: tag bad code with HACK so they can be evaluated later. With language processing there are a lot
    of little hacky thing needed to parse text, so tracking them is necessary. whats hacky? anything not generalizable to the most texts
    DEBUG: tag important debug points with DEBUG
    ALGORITHM: all-encompassing tag for issues of patterns, efficiency, or overall structure.
 
  Text extraction/normalization belongs in its own class. Also add a preprocess step, which will be used to look
  in text file for certain regular structures, and replace them with whatev (for instance, "CHAPTER XVI" in Huck Finn)


  To-do: rewrite map data structures according to conditional probability. We're currently using them like lists. This may
  be improved by using std::map::find() instead of iterating through items using iterators and comparing. These may be equivalent,
  but look up i the documentation to verify; find() searches in log(n) time, whereas using iterators may (?) use n-time.
  They should be structured as key := query of length (n-1) + value = tuple(word-n,frequency)
  Like, "the dog was|12" must be queried as "the dog", which will return ("was",12). 

  adding elements to multimap can be sped up with hints. multimap has some other interesting methods such as lower_bound and upper_bound.
  
  huck Finn has around 110,66[7/8] words, other src says 110,223.


  



*/

#include "nGram.h"

//this global comparison only works for n-grams of the same start word, since frequencies are given by marginal probabilities
bool compare(freqTuple t1, freqTuple t2)
{
  return t1.freq > t2.freq;
}

//global needed for sorting result sets by probability (final step, after all calculations, etc)
//this foo will need to be made into a static class member, in beautiful c++ syntax
bool byScore(const pair<SubkeyIt,long double>& first, const pair<SubkeyIt,long double>& second)
{
  return first.second > second.second;
}

bool byNextWord(const pair<SubkeyIt,long double>& first, const pair<SubkeyIt,long double>& second)
{
  return first.first->first > second.first->first;
}

bool isUniqueResultPair(const pair<SubkeyIt,long double>& first, const pair<SubkeyIt,long double>& second)
{
  //cout << (first.first->first == second.first->first ? "TRUE" : "FALSE") << "   " << first.first->first << "," << second.first->first << endl;
  return first.first->first == second.first->first;
}



/*
  Possible constructor parameters:

    Model-based: n-Grams, semantic delimiters (words, phrases, etc)
    Future: multimodels (backoff) and other advanced

*/
nGram::nGram()
{
  int i, j;
  //U64 my_u64;
  char buf[64];

  cout << "in ctor" << endl;

  fileDelimiter = "|";
  phraseDelimiters = "\".?!#;:)(";  // octothorpe is user defined
  rawDelimiters = "\"?!#;:)(, "; //all but period
  wordDelimiters = ", ";
  delimiters = phraseDelimiters;  //other special chars? could be useful for technical texts, eg, financial reports
  delimiters += wordDelimiters;
  //delimiters += "'";

  wordDelimiter = ' ';
  phraseDelimiter = '#';
  wordCt = -1;
  phraseCt = -1;
  rawParse = true;  //this could be a constructor parameter.
  contextualParse = false;
  isCocaBuilt = false;
  normalized = false;
  
  //TODO: build these
  //buildContractionModel();
  //buildAbbreviationModel();

  //init model stats
  for(i = 0; i <= 5; i++){
    //word model
    wordModelStats[i].totalEntropy = -1;
    wordModelStats[i].expectedSubEntropy = -1;
    wordModelStats[i].meanSubEntropy = -1;
    wordModelStats[i].totalPerplexity = -1;
    wordModelStats[i].expectedSubPerplexity = -1;

    //part of speech model
    posModelStats[i].totalEntropy = -1;
    posModelStats[i].expectedSubEntropy = -1;
    posModelStats[i].meanSubEntropy = -1;
    posModelStats[i].totalPerplexity = -1;
    posModelStats[i].expectedSubPerplexity = -1;
  }
  
  //initialize modelLambdas to give equal weight to all models (see Jurafsky p.104). Initial weights don't matter.
  for(i = 0; i < NLAMBDASETS; i++){
     for(j = 0; j < NLAMBDAS; j++){
       modelLambdas[i].l[j] = 0.2;
     }
    modelLambdas[i].boolHitRate = 0.0; 
    modelLambdas[i].realHitRate = 0.0; 
  }

  //build the coca-ngram filenames
  cocaNgramFiles.resize(16); //later we may use the other coca files, for now using the fully pos-tagged case-sensitive ones
  for(i = 2; i <= 5; i++){
    sprintf(buf,"../../../corpii/coca_ngrams/w%dc.txt",i);  //TODO: this file structure needs to be reorganized if using coca ngrams hardens
    string s = buf;
    cocaNgramFiles[i] = s;
  }
  //cout << "phrase delims: " << phraseDelimiters << endl;
}

nGram::~nGram()
{
  cout << "destroying models" << endl;
  phraseDelimiters.clear();
  wordDelimiters.clear();
  delimiters.clear();
  cout << "1" << endl;
  cocaNgramFiles.clear();
  cout << "2" << endl;
  oneGramTable.clear();
  twoGramTable.clear();
  threeGramTable.clear();
  fourGramTable.clear();
  fiveGramTable.clear();
  sixGramTable.clear();
  cout << "3" << endl;
  oneGramPosTable.clear();
  twoGramPosTable.clear();
  threeGramPosTable.clear();
  fourGramPosTable.clear();
  fiveGramPosTable.clear();
  cout << "4" << endl;
  abbreviationModel.clear();
  contractionModel.clear();
  cout << "5" << endl;
}

void nGram::unitTest(void)
{
  //FreqTablePointer table = getTablePointer(3);
  //FreqTablePointer table = &(this->twoGramTable); //getTablePointer(3);
  MapPair p;
  int dummy;

  p.first = "the";
  p.second.nextWord = "cat";
  p.second.freq = 67;

  cout << "begin unit test" << endl;

  cout << "phrasedelims: " << phraseDelimiters << "  rawDelims: " << rawDelimiters << "  wordelims: " << wordDelimiters << endl;
  cin >> dummy;



/*
  cout << "t2: " << table->empty() << endl;
  cout << "kjhlkh" << endl;
  table->clear();
  cout << "hyerre" << endl;
  table->insert(p);


  cout << twoGramTable.empty();
  cout << "before" << endl;
  twoGramTable.insert(p);
*/

  cout << "model sizes (1-5): " << this->oneGramTable.size() << "," << this->twoGramTable.size()
                                << "," << this->threeGramTable.size() << "," << this->fourGramTable.size()
                                << "," << this->fiveGramTable.size() << endl;

/*

  cout << "one" << endl;
  oneGramTable.clear();
  cout << "two" << endl;
  twoGramTable.clear();
  cout << "three" << endl;
  threeGramTable.clear();
  cout << "four" << endl;
  fourGramTable.clear();
  cout << "five" << endl;
  fiveGramTable.clear();
  cout << "six" << endl;
  sixGramTable.clear();
  cout << "aus" << endl;


  oneGramPosTable.clear();
  twoGramPosTable.clear();
  threeGramPosTable.clear();
  fourGramPosTable.clear();
  fiveGramPosTable.clear();
  cout << "ho" << endl;


  //buildCocaModel(3);


  cout << "hyah!" << endl;
  cout << "t2: "  << endl;
*/
  //cout << "t3: " << table->empty() << endl;
  //cout << "t4: " << table->empty() << endl;

/*
  for(int i = 0; i < 65534; i++){
    cout << i << endl;
    i %= 64000;
  }
*/

}


/*
  Hyphens are ambiguous since they can represent nested phrases or compound words:
    1) "Mary went to the park--ignoring the weather--last Saturday."
    2) "John found a turquoise-green penny."
  Process string by checking for hyphens. Double hyphens represent nested
  phrases, so will be changed to phrase delimiter. Single hyphens will
  be changed to a word-delimiter (a space: ' ').

  Notes: this function snubs context. Huck Finn contains a lot of hyphens, without
  much regular structure (except double vs. single). More advanced parsing will be needed
  to preserve nested context: <phrase><hyphen-phrase><phrase>. Here the first and second <phrase>
  are severed contextually if we just shove in a delimiter. Re-organizing the string would be nice,
  but also difficult for parses like <phrase><hyphen-phrase><hyphen-phrase><hyphen-phrase><phrase> 
  or <phrase><hyphen-phrase><phrase><hyphen-phrase><phrase> which occur often in Huck Finn.
*/
void nGram::scrubHyphens(string& istr)
{
  int i;

  for(i = 0; istr[i] != '\0'; i++){
    if((istr[i] == '-') && (istr[i+1] == '-')){  //this snubs nested context
      istr[i+1] = istr[i] = phraseDelimiter;
    }
    else if(istr[i] == '-'){   //this could also use more context sensitivity: eg, "n-gram" should not necessarily resolve to "n" and "gram" since n is not a word
      istr[i] = wordDelimiter;
    }
  }
}

void nGram::nullifyString(string buf)
{
  for(int i = 0; i < buf.length(); i++){
    buf[i] = '\0';
  }
}

void nGram::nullifyString(char buf[], int len)
{
  for(int i = 0; i < len; i++){
    buf[i] = '\0';
  }
}

bool nGram::isWordDelimiter(char c)
{
  int i;

  for(i = 0; (wordDelimiters[i] != '\0') && (i < wordDelimiters.length()); i++){
    if(c == wordDelimiters[i]){
      return true;
    }
  }

  return false;
}

bool nGram::isPhraseDelimiter(char c)
{
  int i;

  /*
  if(c == 34){
    return true;
  }
  */

  for(i = 0; (phraseDelimiters[i] != '\0') && (i < phraseDelimiters.length()); i++){
    if(c == phraseDelimiters[i]){
      return true;
    }
  }

  return false;
}

void nGram::toLower(string& myStr)
{
  for(int i = 0; i < myStr.length(); i++){
    if((myStr[i] >= 'A') && (myStr[i] <= 'Z')){
      myStr[i] += 32;
    }
  }
}

//standardize input by converting to lowercase
void nGram::toLower(char buf[BUFSIZE])
{
  int i;

  for(i = 0; buf[i] != '\0'; i++){
    if((buf[i] >= 'A') && (buf[i] <= 'Z')){
      buf[i] += 32;
      //cout << buf[i] << " from " << (buf[i] -32) << endl;
    }
  }
}

/*
  This is the most general is-delim check:
  Detects if char is ANY of our delimiters (phrase, word, or other/user-defined.)
*/
bool nGram::isDelimiter(const char c, const string& delims)
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
  Convert various temp tags back to their natural equivalents.
  For now, just converts "Mr+" back to the abbreviation "Mr."
*/
void nGram::finalPass(string& buf)
{
  for(int i = 0; i < buf.length(); i++){
    if(buf[i] == PERIOD_HOLDER){
      buf[i] = '.';
    }
  }
}


/*
  Replaces delimiter chars with either phrase (#) or word (@) delimiters.
  This is brutish, so input string must already be preprocessed.
  Output can be used to tokenize phrase structures and words.


  Notes: This function could be made more advanced. Currently it makes direct replacement
  of phrase/word delimiters with our delimiters (without regard to context, etc)
*/
void nGram::delimitText(string& istr)
{
  int i, k;

  for(i = 0; istr[i] != '\0'; i++){
    if(isPhraseDelimiter(istr[i])){  //chop phrase structures
      istr[i] = phraseDelimiter;
      this->phraseCt++;
      
      //consume white space and any other delimiters (both phrase and words delims):  "to the park .  Today" --becomes--> "to the park####Today"
      k = i+1;
      while((istr[k] != '\0') && isDelimiter(istr[k],this->delimiters)){
        istr[k] = phraseDelimiter;
        k++;
      }
    }
    else if(isWordDelimiter(istr[i])){
      istr[i] = wordDelimiter;

      //consume right delimiters
      k = i+1;
      while((istr[k] != '\0') && isWordDelimiter(istr[k])){
        istr[k] = wordDelimiter;
        k++;
      }
    }
  }
}

/* 
   Find and return index of leftmost, last delimiter. This tells us where
   the last regular structure (sentence, phrase, etc) ended. This
   will be used to chop the input string in a logical manner between
   read() operations.

  Examples:
     "abc###" - returns  3
     "abcde#" - returns  5
     "abcde"  - returns -1

  Returns index of leftmost delimiter last delimiter (-1 if none found).
*/
int nGram::seekLastPhraseDelimiter(const char buf[BUFSIZE], int len)
{
  int i;

  //starting at end of string, spool left to rightmost of group of phrase delimiters
  for(i = len-1; (i >= 0) && !isPhraseDelimiter(buf[i]); i--){}
  //end loop: i points at a phrase delimiter OR i == -1

  // currently at last/rightmost delim. now spool to leftmost phrase delim in this group
  for( ; (i >= 0) && isPhraseDelimiter(buf[i]); i--){}
  //end loop: i==-1 OR buf[i] NOT_IN delimiters[]

  //we are off-by-one after i-- stmt in the last loop to execute, so increment by one
  if(i > 0){
    i++;
  }

  //cout << "last delimiter is >" << buf[i] << "<" << endl;

  return i;
}

/*
  Raw char transformer. currently just replaces any newlines or tabs with spaces. And erases "CHAPTER" headings.
  changes commas to wordDelimiter
*/
void nGram::rawPass(string& istr)
{
  int i;

  for(i = 0; istr[i] != '\0'; i++){
    if((istr[i] < 32) || (istr[i] > 122)){ // convert whitespace chars and extended range chars to spaces
    //if((istr[i] == '\n') || (istr[i] == '\t') || (istr[i] == '\r') || (istr[i] == ',')){
      istr[i] = wordDelimiter;
    }
    else if(istr[i] == ','){   //HACK
      istr[i] = wordDelimiter;
    }

    /*
    //HACK erase "CHAPTER n." from input, a common header in Hucklberry Finn
    // left two checks short-circuit to only allow call strncmp if next two letters are "CH" (ie, high prob. of "CHAPTER")
    if(((i + 16) < len) && (istr[i] == 'C') && (istr[i+1] == 'H') && !strncmp("CHAPTER",&istr[i],7)){
      j = i + 8;
      for( ; (i < j) && (i < len); i++){
        istr[i] = phraseDelimiter;
      }
      //now we point at 'X' in "CHAPTER X", so consume chapter numerals until we hit the period
      for( ; (istr[i] != '.') && (i < len); i++){
        istr[i] = phraseDelimiter;
      }
      if(i == len){
        cout << "ERROR istrfer misalignment in rawPass()" << endl;
      }
      else{
        istr[i++] = phraseDelimiter;  
      }
    }
    */
  }
}

/*
  Builds a simple abbreviation model in memory.

  Abbreviations are not case-sensitive, so for ambiguous abbreviations, I've just
  used what I judge to be the most common target abbreviation.  This is bad for
  "C." and "c.": Celsius and circa.  As well as others.  The input file was just from
  a simple online list of abbreviations.
*/
bool nGram::buildAbbreviationModel(void)
{
  int fd, ntoks;
  char buf[MAX_LINE_LEN];
  char* tokens[16];
  string delim = "|";

  fd = open("../abbreviations.csv", O_RDONLY);
  if(fd < 0){
    cout << "ERROR ../abbreviations.csv could not be opened(), result=" << fd << endl;
    return false;
  }
  
  abbreviationModel.clear();

  while(getLine(fd, buf, MAX_LINE_LEN)){

    //cout << "got buf: >" << buf << "< and delim is: >" << delim << "<" << endl;
    //cin >> ntoks;
    ntoks = tokenize(tokens, buf, delim);
    //cout << "HERE ntoks=" << ntoks << endl;

    if(ntoks > 1){
      abbreviationModel[tokens[0]] = tokens[1]; //set this contraction to most likely expansion
    }
    else{
      cout << "ERROR ntoks==" << ntoks << " < 1 in buildAbbreviationModel()" << endl;
    }
  }

  close(fd);

  return true;
}


/*
  Builds contraction model/table in memory, by reading each line
  in contractionModel.csv, splitting lines, and associating each
  contraction key string with an expansion string value.

  This function could be made to encompass more context and other contraction
  features, since a single contraction can sometimes have multiple expansions.
  Right now we're just assuming the simplest model, which is problematic for cases
  like "aint" in  "I ain't" and "She aint" which respectively map to "I am not" and
  "She is not", which are different values for the same key.

  --Mapping ambiguous contractions requires a state machine, and context; to keep this function
  simple, perhaps we should do such context-driven mappings before calling this function.

*/
bool nGram::buildContractionModel(void)
{
  int fd, ntoks;
  char buf[MAX_LINE_LEN];
  char* tokens[16];
  string delim = ",";

  fd = open("../contractionModel.csv", O_RDONLY);
  if(fd < 0){
    cout << "ERROR ../contractionModel.csv could not be opened(), result=" << fd << endl;
    return false;
  }
  
  contractionModel.clear();

  while(getLine(fd, buf, MAX_LINE_LEN)){
    
    ntoks = tokenize(tokens, buf, delim);

    if(ntoks > 1){
      contractionModel[tokens[0]] = tokens[1]; //set this contraction to most likely expansion
    }
    else{
      cout << "ERROR ntoks==" << ntoks << " < 1 in buildContractionModel()" << endl;
    }
  }

  close(fd);

  return true;
}


/*
  Text normalization function for mapping contractions in some text to a single
  expansion:
     don't  -> do not
     ain't  -> are not
     aren't -> are not
     he'd   -> he would, he had  
     you'd  -> you would, you had

  Notice associations are n:n, where one expansion may relate to multiple contractions,
  and one contraction may relate to multiple expansions.
  Also lengths may vary, and are not simply 2:1, such as:
    couldn't    -> could not
    couldn't've -> could not have

  This is a corner case anyway. For now assume the simplest cases, roughly (one contraction -> one expansion)

  OBSOLETE: we will perform contraction mapping at analysis-time, rather than during preprocessing

void nGram::mapContractions(string& ibuf, string& obuf)
{
  char* phrases[MAX_PHRASES_PER_READ];
  char* words[MAX_PHRASES_PER_READ];

  //first tokenize the entire input buffer
  
  if(buildContractionModel()){


    ntoks = tokenize(tokens, ibuf.c_str(), this->delims);

    //run through copying tokens to output string
    for(int i = 0; i < ntoks; i++){
      
    }



    contractionModel.clear();
  }
  else{
    cout << "ERROR failed to build contractionModel in mapContractions()" << endl;
  }

}
*/

//see above
void nGram::mapContractions(string& ibuf, string& obuf)
{
  //feature
}

bool nGram::abbreviationTableHasKey(const string& key)
{
  if(abbreviationModel.find(key) == abbreviationModel.end()){
    return false;
  }
  return true;
}


/*
  Gets the inclusive left and right indices for some word boundary

  Ex: returns true if both anchors found, false otherwise, and stores
  inclusive right/left indices of some words. So for "ah blah bla",
  left = 3, and right = 6 (note len is: right - left + 1)

  Note: analysis starts at buf[i-1] and buf[i+1]

  This function shall return bounds for multiply-perioded abbrevs like "A.D."
*/
bool nGram::getWordBounds(const string& ibuf, int i, string wordDelims, int* left, int* right)
{
  int j;
  
  //find left anchor
  for(j = i-1; (j >= 0) && !isDelimiter(ibuf[j],wordDelims); j--){};
  if(j >= 0){
    *left = j + 1;
  }
  else{
    return false;
  }

  //find right anchor
  for(j = i+1; (j >= ibuf.length()-1) && !isDelimiter(ibuf[j],wordDelims); j++){};
  if(j <= ibuf.length()-1){
    *right = j - 1;
  }
  else{
    return false;
  }

  return true;
}


/* 
  Periods don't always designate a sentence boundary, for instance, due to common abbreviations.
  So attempt to map periods to common such abbrevs. If an author uses abbreviations, it is assumed
  they will prefer them. So we preserve this usage by running through and identifying abbreviations
  such as "Mr." and change them (temporarily) to "Mr+", so the period is not used as a phrase-break
  when we called delimitText().

  Process is:
    -call mapAbbreviations(): "Mr." converted to "Mr+"
    -later, call delimitText(): all periods marked as phrase breaks
    -call restorePeriods(): convert "Mr+" back to "Mr."

  Only works if abbreviation is delimited by any of our delimiters (periods, spaces, commas, quotes, colons, etc)

  Notes: only handles abbreviations with a single period, not ones like "b.c." "a.d." or "f.b.i."
*/
void nGram::mapAbbreviations(string& ibuf, string& obuf)
{
  int i, left, right; //inclusive boundary indices of the abbreviation
  //string abbrevDelims = "\"',; "
  string word;

  for(i = 0; i < ibuf.length(); i++){

    if(ibuf[i]=='.'){
      if( getWordBounds(ibuf,i,this->rawDelimiters,&left,&right) ){  //extract word from string
        word = ibuf.substr(left,right-left+1);
        toLower(word);
        if( abbreviationTableHasKey(word) ){
          ibuf[i] = PERIOD_HOLDER;
          //cout << "set buf[i] to PERIOD_HOLDER for word: " << word << endl;
        }
      }
      else{
       cout << "WARN: word bounds too narrow in mapAbbrevs()" << endl;
      }
    }
  }
}



/*
  Do our best to clean the sample. We try to preserve as much of the author's style as possible,
  so for instance, we don't expand contractions, viewing them instead as synonyms. "Aren't" and
  "are not" thus mean two different things.

  ASAP change params to both string
*/
void nGram::normalizeText(char ibuf[BUFSIZE], string& ostr)
{
  string istr = ibuf; //many of these are redundant
  //string ostr;

  /*
  cout << len << " (first 120ch) length input sample looks like: (chct=" << len << "," << strlen(buf) << "):" << endl;
  for(i = 0; i < 120; i++){
    putchar(buf[i]);
  }
  cout << "<end>" << endl;
  */

  //non-context free transformer pipeline
  //TODO: map abbreviations.
  //mapAbbreviations(istr,ostr);  //demark Mr. Ms., other common abbreviations before rawPass() uses periods as phrase delimiters
  //post: "Mr." will instead be "Mr+". We'll use '+' to convert back to "Mr." after calling delimitText() 
  //mapContractions(obuf.c_str(), obuf2);
  //dumb for now. obuf will be used later for more context-driven text preprocessing

  //filters and context-free transformers
  rawPass(istr);
  //cout << "1: " << buf << endl;
  toLower(istr);
  //cout << "2: " << buf << endl;
  scrubHyphens(istr);
  //cout << "3: " << buf << endl;
  delimitText(istr);
  //cout << "4: " << buf << endl;
  finalPass(istr);

  ostr = istr;

  //cout << "here: " << buf[0] <<  endl;
  /*
  cout << "120ch output sample looks like: " << endl;
  for(i = 0; i < 120; i++){
    putchar(buf[i]);
  }
  cout << "<end>" << endl;
  cin >> i;
  */
}




/*
  Logically the same as strtok: replace all 'delim' chars with null, storing beginning pointers in ptrs[]
  Input string can have delimiters at any point or multiplicity

  Pre-condition: This function continues tokenizing until it encounters '\0'. So buf must be null terminated,
  so be sure to bound each phrase with null char.

  Testing: This used to take a len parameter, but it was redundant with null checks and made the function 
  too nasty to debug for various boundary cases, causing errors.
*/
int nGram::tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims)
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

//add or update a key+val pair in the table. 
SubkeyIt nGram::updateKeyVal(MapPair& keyValPair, FreqTablePointer table)
{
  MapSubset subset;
  KeyIt it;
  SubkeyIt ret;

  //cout << "in updateKeyVal with kp.first: >" << keyValPair.first << "<" << endl;
  it = table->find(keyValPair.first);
  //cout << "there" << endl;
  if(it == table->end()){
    //key not in map, so insert key and freqtuple
    ret = (*table)[keyValPair.first].insert( std::pair<string,long double>(keyValPair.second.nextWord, keyValPair.second.freq) ).first;
  }
  //key found in key table, but not subkey table, so init it
  else if( (*table)[keyValPair.first].find(keyValPair.second.nextWord) == (*table)[keyValPair.first].end()){
    //same stmt as prior if block
    ret = (*table)[keyValPair.first].insert( std::pair<string,long double> (keyValPair.second.nextWord, keyValPair.second.freq)).first;
  }
  //key in key table, subkey in subkey table, so increment the current value
  else{
    ret = (*table)[keyValPair.first].find(keyValPair.second.nextWord);
    ret->second += keyValPair.second.freq;
  }

  return ret;
}


/*
  Increments some gramStr frequency in our lookup, where (key=gramStr, val=count)
  If gramStr is not already in table, its value is initialized to 1, else it is incremented.

  This is a driver for training an n-gram model. It creates key/subkey pair, then calls
  updateKeyVal to either 1) create a new key/subkey and init it to one, or 2) increment an existing key/subkey.

  TODO: factor this to return iterator to inserted item, as hint to next ordered insertion.
*/
SubkeyIt nGram::incrementKey(string gramStr, int nModel)
{
  MapPair keyValPair;
  MapSubset subset;
  SubkeyIt ret;
  KeyPair kp;
  FreqTablePointer table;

  table = getTablePointer(nModel);
  if(table == NULL){
    cout << "testsample_raw(): table ptr not found" << endl;
    return ret;  //HACK TODO: this returns an uninitialized iterator! No way for caller to detect.
  }

  //make the key, and then the insertion pair
  makeKeyPair(gramStr,&kp);

  keyValPair.first = kp.primaryKey;
  keyValPair.second.nextWord = kp.subKey;
  keyValPair.second.freq = 1;

  ret = updateKeyVal(keyValPair,table);

  return ret;
}

/*
  Given a parsed string, tokenize each word and add each n-gram to our model.

  *Unlike contextual updateNgramTable() function, this function views input as a raw list of grams delimited by any delimiter (word or phrase).
  Note that switching between either one requires updating the input file reading algorithm. If we view input as raw grams, then we must not
  lose ngrams that span the end of one read() operation and the beginning of the next one. OTOH the contextual updateNgramTablee function
  requires making sure the phrases within each read() operation are complete and bounded.
  
  Notes: maybe use dynamic vector of char* pointers instead of a static array.
*/
int nGram::updateNgramTable(char buf[BUFSIZE], int len, int nModel, const string& delims)
{
  int nTokens, nSubStrs, i, j;
  char* grams[MAX_TOKENS_PER_READ] = {NULL};
  string gramBuf;

  //nullifyTokens(grams,MAX_TOKENS_PER_READ);

  //for each bounded-sample, get the words within it
  nTokens = tokenize(grams, buf, delims);
  if(nTokens > MAX_TOKENS_PER_READ){
    cout << "WARNING: MAX_TOKENS_PER_READ exceeded in updateNgramTable(), sample ignored" << endl;
    return 0;
  }
  if(nTokens < 2){
    cout << "WARN anomalous nPhrases >" << nTokens << "< in updateFreqFile. phrases[0]: >" << grams[0] << "<" << endl;
    //cin >> i;
  }

  this->wordCt += nTokens;
  //phraseCt += nPhrases;

/*
  cout << "DEBUG got words:" << endl;
  cout << grams[0] << endl;
  cout << grams[1] << endl;
  cout << grams[2] << endl;
  cout << grams[3] << endl;
  cout << grams[4] << endl;
  cout << grams[5] << endl;
  cin >> i;
*/

/*
  //DEBUG
    cout << "DEBUG got words:" << endl;
    for(int w = 0; grams[w] != NULL; w++){
      cout << w << ">>" << grams[w] << endl;
    }
    cin >> i;
*/

    //check enough tokens in this buffer for this n-gram model
    if(nTokens >= nModel){

      //count number of n-gram strings in this buffer, then hash each n-gram substring to frequencyTable / map.
      nSubStrs = nTokens - nModel + 1;
      for(i = 0; i < nSubStrs; i++){

        //build this gram string
        gramBuf.clear();
        gramBuf = grams[j];
        for(j = 1; j < nModel; j++){
          gramBuf += " ";
          gramBuf += grams[j + i];
        }

        //cout << "[j:"<< j << "][k:" << k <<"] gramBuf is: " << gramBuf << endl;
        //cout << "adding key: " << gramBuf << endl;
        incrementKey(gramBuf,nModel); // increment this n-gram string in our table (init key+val to 1 if key does not yet exist)
      }
    }
    else{
      cout << "too few words (" << nTokens << ") in this phrase for " << nModel << "-grams. sample discarded, continuing..."  << endl;
    }

  return nSubStrs;
}



/*
  Given the corpus has been parsed and delimited, read "phrases" (delimited by phraseDelimiter)

  Updates frequencyTable based on some new, parsed values.

  nModel will factor to class member value.

  Frequency table can be 1-gram, 2-gram, 3-gram, etc., words separated by a space

  Returns gCt, the running number of n-gram substrings added to map (DEBUG).

  //proceed left to right: foreach phrase, tokenize phrase into words, hash n-grams into map.
  
  NOTES: tokenize() is null-driven, so be sure buf[] is correctly null-terminated.

  This version of the updateNgramTable takes some consideration of sentence context by parsing through sentences
  (previously delimited by parseCorpus), instead of running through ngrams as a raw string of grams. This was supposed
  to build our ngram model from more from context than from the raw transitions between grams. char buf[] contains a set
  of delimited sentences, and within these sentences words are delimited by word delimiter. So we split the buf into sentences,
  then we split each sentence into words and build our model from that. So for a three-gram model, the sentence "I went to the
  store" would only give n-grams "I went to", "went to the", and "to the store". But note the information loss. If the fuller context is
  "... nice day. I went to the store. There was a sale ..." then note we lose transitions such as "day I went" and "the store there".
  
  Despite this loss, we can compensate since our previous models (1-gram, 2-gram, up to n-1 gram) will capture this information.
  Likewise, this function may still be useful for capturing complete n-gram structures in context, as well as supporting other statistical
  data, such as the position of certain elements within a sentence, perhaps such as incorporating information about the actual probability
  distribution of some word's location within a sentence, eg, being more likely at word-3 than at word-last.
  
  Precondition: buf is both word and phrase delimited (eg, "had$a$nice$time###Then$the$dog$got$out$but$it$wasn't$the$first$time###" (not necesarily these delims #$)
*/
int nGram::updateNgramTable_Contextual(char buf[BUFSIZE], int len, int nModel)
{
  int nPhrases, nWords, nSubStrs, i, j, k, gCt, ct;
  char* phrases[MAX_PHRASES_PER_READ];
  char* words[MAX_WORDS_PER_PHRASE];
  string gramBuf;
  
  //char debug[1024];
  //strncpy(buf,debug,1024);
  nullifyTokens(phrases,MAX_PHRASES_PER_READ);

  //for each bounded-sample, get the phrases within it
  nPhrases = tokenize(phrases, buf, this->phraseDelimiters);
  if(nPhrases > MAX_PHRASES_PER_READ){
    cout << "WARNING: MAX_PHRASES_PER_READ exceeded in updateNgramTable(), sampled ignored" << endl;
    return 0;
  }
  if(nPhrases < 2){
    cout << "WARN anomalous nPhrases >" << nPhrases << "< in updateFreqFile. phrases[0]: >" << phrases[0] << "<" << endl;
    //cin >> i;
  }

  phraseCt += nPhrases;

  /*
  cout << "phrases: " << endl;
  for(int q = 0; q < nPhrases; q++){
    cout << "ph" << q << ": " << phrases[q] << endl;
  }
  */

  //iterate over phrases
  for(i = 0, gCt = 0; i < nPhrases; i++){

    //cout << "tokenizing phrase[" << i << "]>>" << phrases[i] << endl;
    nullifyTokens(words,MAX_WORDS_PER_PHRASE);
    //for each phrase, get all words
    nWords = tokenize(words, phrases[i], this->wordDelimiters);
    if(nWords > MAX_WORDS_PER_PHRASE){
      cout << "WARNING: MAX_WORDS_PER_PHRASE exceeded in updateNgramTable(), sample skipped" << endl;
      continue;
    }
    wordCt += nWords;

/*  //DEBUG
    cout << "DEBUG got words:" << endl;
    for(int w = 0; words[w] != NULL; w++){
      cout << w << "|" << nWords << "|" << nPhrases << ">>" << words[w] << endl;
    }
    cin >> dummy;
*/

    if(nWords >= nModel){

      //count number of n-gram strings in this phrase, then hash each n-gram substring to frequencyTable / map.
      nSubStrs = nWords - nModel + 1;
      for(j = 0; j < nSubStrs; j++){

        //build this gram string
        gramBuf.clear();
        gramBuf = words[j];
        for(k = 1; k < nModel; k++){
          gramBuf += " ";
          gramBuf += words[k + j];
        }

        //DEBUG: I was getting '#' at end of last token sometimes, but this was fixed when I went to null-terminal based parsing.
        if(gramBuf[ gramBuf.length() - 1 ] == phraseDelimiter){
          for(int end = gramBuf.length() - 1; (end >= 0) && (gramBuf[end] == phraseDelimiter); end--){
            gramBuf[ end ] = '\0';
            ct++;
          }
          cout << "HACK1 replaced >" << ct << " octothorpes" << endl;
        }
        ct = 0;
        if(gramBuf[ gramBuf.length() - 2 ] == phraseDelimiter){
          for(int end = gramBuf.length() - 2; (end >= 0) && (gramBuf[end] == phraseDelimiter); end--){
            gramBuf[ end ] = '\0';
            ct++;
          }
          cout << "HACK2 replaced >" << ct << " octothorpes" << endl;
        }
        //END DEBUG

        //cout << "[j:"<< j << "][k:" << k <<"] gramBuf is: " << gramBuf << endl;
        //cout << "adding key: " << gramBuf << endl;
        incrementKey(gramBuf,nModel); // increment this n-gram string in our table (init key::val to 1 if key does not yet exist)
      }
      gCt += nSubStrs;
    }
    else{
      //cout << "too few words (" << nWords << ") in this phrase for " << nModel << "-grams, continuing..."  << endl;
    }
  }

  return gCt;
}

/*
  Dumps the ngram frequency table to an output file.
  File is formatted as follows with grams separated by spaces:
    <l>nModel,frequency</l>
    the dog|23
    the cat|45
    ...

  Returns: number of lines written, less than zero for failure.

unsigned long long nGram::tableToFile(const char* outfile, int nModel)
{
  int fd, dummy, r;
  unsigned long long n;
  KeyIt it;
  FreqTablePointer tablePtr;
  string s;
  char buf[16];

  if(outfile == NULL){
    cout << "ERROR null outfile* passed to tableToFile()" << endl;
    return -1;
  }

  fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC);
  if(fd < 0){
    cout << "ERROR " << outfile << "could not be opened(), result=" << fd << endl;
    return fd;
  }
  fchmod(fd, S_IRUSR | S_IWUSR |S_IRGRP | S_IWGRP | S_IROTH );

  tablePtr = getTablePointer(nModel);
  if(tablePtr == NULL){
    cout << "ERROR NULL tablePtr in tableToFile()" << endl;
    return -1;
  }

  for(n = 0, r = 1, it = tablePtr->begin(); (it != tablePtr->end()) && (r >= 0); ++it, n++){

    //build the record string: "gram1 gram2 gram3|34"
    s += it->first;
    //for the one-gram model, since there is no n-1 gram string, we duplicate the 1gram in both the key and in the subkey.
    //so skip these duplicates, for the 1-gram model
    if(nModel > 1){ 
      s += " ";
      s += it->second.nextWord;
    }
    sprintf(buf,"|%lu\n",it->second.freq);
    s += buf;
    
    
    if(nModel > 1){
    cout << "writing: (newline expected)>" << s << endl;
    cin >> dummy;
    }
    

    //ALGORITHM: this is calling write to often, though we dont flush to file that often. could be improved by building a large buffer string of lines before flushing
    r = write(fd, s.c_str(), s.length());
    if(r < 0){
      cout << "ERROR write() aborting. ret val: " << r << endl; 
    }
    s.clear();
  }

  close(fd);

  return n;
}
*/

/*
  Small utility for finding the position of a char in some input buffer.
  Used when we need to lseek() back to some position in a file to verify we're
  reading delimiter-length segments.

  Returns: position of char c, or -1 if not in string.
*/
int nGram::findChar(const char buf[], char c, int len)
{
  int i;

  if((buf == NULL) || (len <= 0)){  
    cout << "ERROR buf null or len < 0 in findChar()" << endl;
    return 0;
  }

  for(i = 0; buf[i] && (i < len) && (buf[i] != c); i++){ /*empty*/ }

  if((i == len) || ((buf[i] == '\0') && (c != '\0'))){
    i = -1;
  }

  return i;
}

/*
  Routine for getting line from frequency file. Chops buf at newline,
  and sets file position to the next line that is also not a newline.
  
  Note this function may be slow if a file contains many empty lines (repeated \n sequences).
*/
char* nGram::getLine(int openfd, char buf[], int maxlen)
{
  int n, i;

  n = read(openfd,buf,maxlen);
  if(n <= 0){
    //cout << "WARN read() returned >" << n << "< in getLine(). Returning NULL." << endl;
    return NULL;
  }

  /*
    Reset file position to position of first newline char
    If we read n chars, then we need to back up to 'pos' index,
    which is back from the current position by (n - pos)
  */
  i = findChar(buf,'\n',n);
  if(i == -1){
    cout << "ERROR findChar(\\n) returned -1 in getLine(). Returning NULL." << endl;
    buf[0] = '\0';
    return NULL;
  }
  buf[i] = '\0';

  //cout << "pos/n = <" << pos << "|" << n << ">" << endl;
  i = i - n + 1;  //position of first newline + 1

  if(i < 0){
    lseek(openfd, i, SEEK_CUR);
  }

  /*
  cout << "getLine() returned line: " << buf << endl;
  cin >> n;
  */

  return buf;
}

void nGram::clearStrTokens(char* toks[], int len)
{
  for(int i = 0; i < len; i++){
    toks[i] = NULL;
  }
}

/*
  Dumb function for removing all special chars (those less than ascii 32), EXCEPT
  tab chars, which are the token delimiters in the coca files.
*/
void nGram::filterCocaLine(char buf[])
{
  for(int i = 0; buf[i] != '\0'; i++){
    if((buf[i] != '\t') && (buf[i] < 32)){
      buf[i] = '\0';
    }
  }
}

/* poor version of a rtrim function. 
   buf[] must be a null-terminated line with \r or \n at the end,
   or this function will put null chars throughout string
*/
void nGram::rtrim(char buf[])
{
  for(int i = 0; buf[i] != '\0'; i++){
    if((buf[i] == '\r') || (buf[i] == '\n')){
      buf[i] = '\0';
    }
  }
}


/*
  Build the freqPosTuple for some key, based on coca-parsing methods.

   toks: an arrray of string pointer parsed from coca files in coca format
   nModel: model we wish to generate a freqtuple for
  
  
*/
void nGram::buildPosFreqTuple(FreqTuple* tup, char* toks[], int nModel)
{
  //TODO: error check that toks[0] is all numeric before this call
  sscanf(toks[0],"%Lf",&(tup->freq));
  
  switch(nModel){
    case 1:
        cout << "CASE 1 not complete" << endl;
        tup->nextWord = toks[4];
      break;
     case 2:
        tup->nextWord = toks[4];
      break;
     case 3:
        tup->nextWord = toks[6];
      break;
     case 4:
        tup->nextWord = toks[8];
      break;
     case 5:
        tup->nextWord = toks[10];
      break;
    default:
      cout << "Unknown nModel passed to buildKey: " << nModel << endl;
  }
}


/*
  Build the freqTuple for some key, based on coca-parsing methods.

   toks: an arrray of string pointer parsed from coca files in coca format
   nModel: model we wish to generate a freqtuple for
  
  
*/
void nGram::buildFreqTuple(FreqTuple* tup, char* toks[], int nModel)
{
  //TODO: error check that toks[0] is all numeric before this call
  sscanf(toks[0],"%Lf",&(tup->freq));
  tup->nextWord = toks[nModel];
}

/*
  Here is where we build a primary key based off coca ngrams.
  This function requires the coca ngram file format of frequency,words,parts-of-speech.
  
  This and build-val() are where we can capture parts of speech, if we want to do these.
  It just depends where/when we want to partition things based on pos tags.
  If we store the words as word+part-of-speech, then we have already partitioned. However,
  if we do not store them here, but instead use the 1-gram table as a pos lookup it might be more
  useful, but we've probably lost some context since the same word may be bound to different pos tags.
  
  The problem is whether or not we can get a hold of CRAWLs tagger for testing, since we need to be able
  to tag training/test corpii in order to use the word+pos key approach.
  
  For now, assume no parts of speech in keys or vals.
*/
string nGram::buildKey(char* toks[], int nModel)
{
  bool fail = false;
  char buffer[150];
  string key;
  
  switch(nModel){
    case 1:
        cout << "CASE 1 not complete" << endl;
        sprintf(buffer,"%s",toks[1]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
     case 2:
         //sprintf(buffer,"%s<%s>",toks[1],toks[3]); //build prior: "cat<np1>[" where '[' signals start of list
         sprintf(buffer,"%s",toks[1]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
     case 3:
         sprintf(buffer,"%s %s",toks[1],toks[2]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
     case 4:
         sprintf(buffer,"%s %s %s",toks[1],toks[2],toks[3]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
     case 5:
         sprintf(buffer,"%s %s %s %s",toks[1],toks[2], toks[3], toks[4]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
    default:
      cout << "Unknown nModel passed to buildKey: " << nModel << endl;
      fail = true;
  }
  
  if(!fail){
    key = buffer;
  }
  else{
    key = "<UNK>";
  }

  return key;
}

//see previous header. this function also requires COCA format
string nGram::buildPosKey(char* toks[], int nModel)
{
  bool fail = false;
  char buffer[150];
  string key;
  
  switch(nModel){
    case 1:
        cout << "CASE 1 not complete in build posKey(). must build 1-gram table first, then parse it with this to get pos frequencies" << endl;
        sprintf(buffer,"%s",toks[3]); //build prior: "cat<np1>[" where '[' signals start of list
      break;
     case 2:
         //sprintf(buffer,"%s<%s>",toks[1],toks[3]); //build prior: "cat<np1>[" where '[' signals start of list
         sprintf(buffer,"%s",toks[3]); //build prior: "cat<np1>[" where '[' signals start of list
      break;
     case 3:
         sprintf(buffer,"%s %s",toks[4],toks[5]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
     case 4:
         sprintf(buffer,"%s %s %s",toks[5],toks[6],toks[7]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
     case 5:
         sprintf(buffer,"%s %s %s %s",toks[6],toks[7],toks[8],toks[9]); //build prior: "cat<np1>[" where '[' signals start of list
       break;
    default:
      cout << "Unknown nModel passed to buildKey: " << nModel << endl;
      fail = true;
  }
  
  if(!fail){
    key = buffer;
  }
  else{
    key = "<UNK>";
  }

  return key;
}



/*
  Build the part of speech table for some nModel based on the COCA parts of speech in one of their n-gram files.

  This function requires that the COCA files be formatted as they are for freq+n-gram+pos.
  Also note that parts of speech frequencies will be much higher, so we need larger data types.
  
  This function could be merged with other COCA file parsing, depending on the direction we plan to go with the COCA
  files. This function currently duplicate file i/o work done by buildCocaModel() with which it could be merged.
*/
bool nGram::buildCocaPosTable(const string& fname, int nModel)
{
  int ifile, ntoks;
  char buf[256];
  char* toks[32];
  FreqTablePointer table;
  MapPair item;
  
  if(nModel == 1){
    cout << "buildCocaPosTable not yet written for nModel == 1" << endl;
    return false;
  }
  ifile = open(fname.c_str(), O_RDONLY);
  if(ifile < 0){
    cout << "ERROR file failed to open in buildCocaPosTable(). retval: " << ifile << endl;
    return false;
  }
  table = getPosTablePointer(nModel);
  if(table == NULL){
    cout << "ERROR table ptr null in buildCocaPosTable()" << endl;
    return false;
  }
  
  while(getLine(ifile,buf,256) != NULL){
    //TODO: these all need error checks based on their return vals
    filterCocaLine(buf);
    ntoks = tokenize(toks,buf,"\t");
    if(ntoks != (nModel * 2 + 1) ){
      cout << "ERROR Read incorrect num toks from COCA file in buildCocaPosTable : " << ntoks << endl;
    }
    item.first = buildPosKey(toks,nModel);
    buildPosFreqTuple(&(item.second),toks,nModel);
    cout << "line skipped, updateKeyVal needs to be rewritten or factored to support pos tags from coca" << endl;
    //updateKeyVal(&item, nModel);
  }
  
  close(ifile);

  return true;
}

//for list<string>.unique( uniqueStrings )
bool uniqueStrings(const string& left, const string& right)
{
  return left == right;
}

/*
  OBSOLETE. This was necessary for iterating through multimap values. Iterating over the unique
  keys in the map model (instead of multimap) is simply a matter of iterating the keys in the outer map<>,
  so there is no need for a uniqueKeys() function.

  Gets all the unique keys in a table. This is useful for doing subset analyses.
  The list of keys is in reverse of their order in the map. This may not be efficiently
  used by external algorithms (if iterating over all keys it shouldn't matter).

  Returns: num unique keys

U64 nGram::getUniqueKeys(list<string>& uniqueKeys, FreqTablePointer table)
{
  //int dummy;
  U64 ct;
  KeyIt it;

  if(table == NULL){
    cout << "ERROR table ptr NULL in getUniqueKeys()" << endl;
    return 0;
  }

  //first clear the list
  uniqueKeys.clear();

  //get all unique keys in table so we can iterate over the subsets (per key). this is meh, but its the only way to get only the unique keys in the table
  it = table->begin();
  uniqueKeys.push_front( it->first );
  it++;

//this insertion algorithm relies on the keys in the multimap being in order, but this is guaranteed by multimap (keys are ordered by cmp, vals are ordered by insertion)
//for an order-agnostic version, build entire list of keys, then call list.unique(compare)
  for(ct = 1; it != table->end(); ++it){
    if(it->first != *(uniqueKeys.begin())){
      //cout << "model, pushed: " << it->first << " after comp with: " << *(uniqueKeys.begin()) <<  endl;
      uniqueKeys.push_front( it->first );
      ct++;
    }

    else{
      cout << "skipped dupe: " << it->first << endl;
    }

  }

  //this shouldnt be necessary, given above algorithm; but algo has not been tested enough to be sure...
  uniqueKeys.unique( uniqueStrings );



  cin >> dummy;
  //debug
  for(list<string>::iterator it = uniqueKeys.begin(); it != uniqueKeys.end(); it++){
    cout << "key: " << *it << endl;
  }
  cout << "^^^----key dump, verify sorted, no dupes" << endl;
  cin >> dummy;

  return ct;
}
*/


/*
  Builds a one-gram model from an input table (assumed to be two-gram!) by marginalizing the
  one-gram probabilities from it.
  
  Keys must be mapped to subkeys, but subkeys aren't needed for one-grams. So I'm storing
  these as key = subkey = word -> freq. So to retrieve a freq: oneGramTable[key][key]. The
  word is stored redundantly.
*/
bool nGram::buildCocaOneGramModel(FreqTablePointer twoGramTablePtr, FreqTablePointer oneGramTablePtr)
{
  long double sumFreq;
  MapPair tablePair;
  MapSubset subset;
  KeyIt outer;
  SubkeyIt inner;

  if(twoGramTablePtr == NULL || oneGramTablePtr == NULL){
    cout << "ERROR table ptr null in buildCocaOneGramModel()" << endl;
    return false;
  }
  if(twoGramTablePtr->empty()){
    cout << "ERROR 2-gram table empty (size = " << (int)((twoGramTablePtr->size() * sizeof(MapPair)) >> 10) << " kb) in buildCocaOneGramModel(). Could not build 1-gram model" << endl;
    return false;
  }

  if(!oneGramTablePtr->empty()){
    oneGramTablePtr->clear();
  }
 
  //iterate over the subsets within the two-gram table. IOW, sum the frequency of each subset for a given
  //two-gram key (a one gram). By definition this gives the one-gram frequency for that one-gram.
  for(outer = twoGramTablePtr->begin(); outer != twoGramTablePtr->end(); ++outer){
    //sum the frequency of this key
    for(sumFreq = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      sumFreq += inner->second;
    }

    //key not yet in table
    if(oneGramTablePtr->find(outer->first) == oneGramTablePtr->end()){
      (*oneGramTablePtr)[outer->first][outer->first] = sumFreq;   
    }
    //key already in table, so increment instead of init
    else{
      (*oneGramTablePtr)[outer->first][outer->first] += sumFreq;
    }
  }
  
  return true;
}



/*
  Designed for mapping a COCA n-gram+pos file into a multimap data structure.
  There is a lot of redundant data in the COCA files, since each prefix string is repeated:
    <32> <the cat> <was> <pos pos pos>  (>,< inserted for emphasis)
    <44> <the cat> <had> <pos pos pos>
    ...
    Note the repetition of "the cat", the prior string for each possible word to follow "the cat".
 
  This function consumes the tab-delimited <frequency prior_string nextWord pos pos ...> format.
  Note that to the prior_string is not itself the key, since parts of speech may vary for the same prior string.
  Thus, the prior_string combined with respective parts of speech gives a table key, eg, "the cat" would probably
  be "the<pp> cat<np1>", since every word in our model is treated as word+pos.
  
  Precondition: File must be tab-delimited and must have the correct columnar correspondence of part-of-speech to words.

  Note: This function DESTROYS old ngram model in multimap. It does not merge models.

  Insertion hint *strongly* assumes coca model file is in order by pkey, which isn't strictly true for case-sensitive and pos COCA files.

*/
bool nGram::buildCocaModel(int nModel)
{
  int ifile, ntoks, dummy;
  struct stat fstats;
  long double fpos, fsize;
  char buf[MAX_LINE_LEN];
  char* toks[32];
  MapPair wordTablePair, posTablePair;
  FreqTablePointer wordTable, posTable;

  if(nModel == 1){
    cout << "CANNOT BUILD 1-Gram COCA model. Use marginalization on 2-gram" << endl;
    return false;
  }
  ifile = open(cocaNgramFiles[nModel].c_str(), O_RDONLY);
  if(ifile < 0){
    cout << "ERROR file failed to open in cocaFileToTable(). retval: " << ifile << endl;
    return false;
  }
  wordTable = getTablePointer(nModel);
  if(wordTable == NULL){
    cout << "ERROR word table ptr null in cocaFileToTable()" << endl;
    return false;
  }
  posTable = getPosTablePointer(nModel);
  if(posTable == NULL){
    cout << "ERROR pos table ptr null in cocaFileToTable()" << endl;
    return false;
  }

  //Stat the file to get its size. We'll only use this to keep track of progress, since these are very large files.
  fpos = 0.0; fsize = 0.0; dummy = 0;
  fstat(ifile, &fstats);
  fsize = (long double)fstats.st_size;

  //DESTROY old models
  if(!wordTable->empty()){
    wordTable->clear();
  }
  if(!posTable->empty()){
    posTable->clear();
  }
  cout << "Processing COCA " << nModel << "-gram file to map<string,map<string,long double> >: " << cocaNgramFiles[nModel].c_str() <<  endl;
  //mondo slow, but mostly due to size. could be sped up by reading chunks of file instead of lines. getline is likely very inefficient due to kernel calls
  while(getLine(ifile,buf,MAX_LINE_LEN) != NULL){
    //TODO: these all need error checks based on their return vals; right now I'm assuming validity
    //cout << "unbuffered: " << buf << endl;
    filterCocaLine(buf);
    ntoks = tokenize(toks,buf,"\t");
    if(ntoks != (nModel * 2 + 1) ){
      cout << "ERROR Read incorrect num toks from COCA file in buildCocaPosTable. ntoks=" << ntoks << endl;
    }

    //build and insert word pair into word table
    wordTablePair.first = buildKey(toks,nModel); //cur = buffer;
    buildFreqTuple(&(wordTablePair.second),toks,nModel);

    /*
    if(!strncmp(wordTablePair.second.nextWord,"zygote",8)){
      cout << "HIT" << endl;
    }*/

    // key not in key table
    if( wordTable->find(wordTablePair.first) == wordTable->end()){
      (*wordTable)[wordTablePair.first][wordTablePair.second.nextWord] = wordTablePair.second.freq;
    }
    // subkey not in subkey table
    else if( (*wordTable)[wordTablePair.first].find( wordTablePair.second.nextWord ) == (*wordTable)[wordTablePair.first].end()){
      (*wordTable)[wordTablePair.first][wordTablePair.second.nextWord] = wordTablePair.second.freq;
    }
    // both already in table, so increment current count
    else{
      (*wordTable)[wordTablePair.first][wordTablePair.second.nextWord] += wordTablePair.second.freq;
    }

    //ui output to track progress, like a progress bar
    dummy++;
    if((dummy %= 3000) == 0){
      fpos = (long double)lseek(ifile,0,SEEK_CUR); //this is just to get the current file position, to find out how far we are in processing the file
      cout << "\r...  " << (int)(101 * (fpos / fsize)) << "% complete " << flush; // " cur key: " << wordTablePair.first << "             " << flush;
      //<< "-gram Word-Map size (kb): " 
      //<< ((testMap.size() * sizeof(MapPair)) >> 10)
      //<< "  POS Map size (kb): " << ((posTable->size() * sizeof(MapPair)) >> 10)  << flush;
    }
  }

  close(ifile);

  return true;
}

/*
  I couldn't think of a better name for this function. However, when a table is built from the coca
  files, the key/subkeys are all raw integer frequencies:
    the-> cat | 3
    the-> dog | 5
    the->fish | 10   //a two-gram coca format

  This is nice for being able to marginalize out certain probabilities, but were not doing that yet.
  This function goes through and replaces each word count with its conditional probability:
    the-> cat | (3.0 / (3.0 + 5.0 + 10.0))

  TODO: In the future, it would be much better to instead store the normalization constant (just the 3+5+10 sum above)
  in the parent/key node. That way, we don't lose information. The maps would instead be of type map<string, tuple(sumFreq,map<string,int>)
  It would likely be much smaller memory footprint than using long doubles. Getting conditional probability would work like:
    sumFreq  = map[key];
    wordFreq = map[key][subkey];
    prob = wordFreq / sumFreq;
  Clearly, the recursive definition would be preferable, leading to a more trie-like data object.

*/
void nGram::normalizeTable(FreqTablePointer table)
{
  KeyIt outer;
  SubkeyIt inner;
  long double sumFreq;
  //int n;

  if(table == NULL){
    cout << "ERROR table ptr NULL in normalizeTable(). returning -1.0" << endl;
    return;
  }

  for(outer = table->begin(); outer != table->end(); ++outer){
    //get sumfrequency of this subset
    for(sumFreq = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      sumFreq += inner->second;
    }

    //calculate and store the conditional probability of each word in subset
    for(inner = outer->second.begin(); (sumFreq > 0.0) && (inner != outer->second.end()); ++inner){
      inner->second /= sumFreq;
      //cout << "this condprob: " << outer->first << "," << inner->first << "|" << inner->second << endl;
    }
  }
}

//a rather dumb one-off case of the above function, for the one-gram model, which must be iterated differently.
void nGram::normalizeOneGramTable(FreqTablePointer table)
{
  KeyIt outer;
  SubkeyIt inner;
  long double sumFreq;
  //int n;

  if(table == NULL){
    cout << "ERROR table ptr NULL in normalizeTable(). returning -1.0" << endl;
    return;
  }

  sumFreq = getSumFrequency(table);
  //cout << "one-g sumFreq is " << sumFreq << endl;

  for(outer = table->begin(); outer != table->end(); ++outer){
    //calculate and store the conditional probability of each word in subset
    for(inner = outer->second.begin(); (sumFreq > 0.0) && (inner != outer->second.end()); ++inner){
      inner->second /= sumFreq;
      //cout << "this condprob: " << outer->first << "," << inner->first << "|" << inner->second << endl;
    }
  }
}


/*Outputs a compressed version of the COCA-based table. This will only be useful for cutting down on the size
  of the COCA files.
  The primary key in an ngram data base (ignoring pos tags for now) is just the prior string: the n-1 word before the
  next predicted word. The COCA files repeat primary keys over and over for every next word, which is redundant.
  We can compress the COCA format by only storing the primary key with a csv list of associated next words and their frequency.
*/
bool nGram::tableToCompressedCocaFile(int nModel)
{
  //TODO: code, but only after determining if data schema will include pos tags in words or not, since this changes methods significantly
  return false;
}


/*
  Converts an nGram frequency file into a std::map consisting of <key:=nModeltring, value:=frequency_int>.
  File is expected to be formatted as output by tableToFile():
    <n-gramString,count>\n
    <n-gramString,count>\n
    <n-gramString,count>\n
    etc.

  File integrity is assumed, not verified.

  IGNORING THIS FUNCTION FOR NOW. This kind of i/o is more for the user-app, to come after research.

int nGram::fileToTable(const char* fname, int nModel)
{
  int fd, tokCt, pos, n;
  U64 freq;
  char buf[MAX_LINE_LEN];
  string gramString;
  char* strTokens[8];
  KeyPair kp;
  FreqTablePointer tablePtr = getTablePointer(nModel);
  MapPair keyValPair;

  if(tablePtr == NULL){
    cout << "ERROR null table ptr in fileToTable()" << endl;
    return -1;
  }
  if((fname == NULL) || (*fname == '\0')){
    cout << "ERROR null file name passed to fileToTable()" << endl;
    return -1;
  }

  fd = open(fname, O_RDONLY);
  if(fd < 0){
    cout << "ERROR fileToTable() could not open file: " << fname << endl;
    return fd;
  }

  tablePtr->clear();

  while( getLine(fd, buf, MAX_LINE_LEN) ){
    //cout << "got line: " << buf << endl;

    clearStrTokens(strTokens,8);
    n = tokenize(strTokens, buf, this->fileDelimiter);   //use tokenize, since files may end up with more statistical data in each record, more delimiters
    if(n < 2){
      cout << "ERROR nToks < 2 in fileToTable(): " << n << " delim: " << this->fileDelimiter << endl;
      cout << "tok1>>" << strTokens[0] << "\ntok2>>" << strTokens[1] << endl;
    }
    else{
      //read in the frequency of this ngram str
      sscanf(strTokens[1],"%lu",&freq);

      gramString = strTokens[0];
      makeKeyPair(gramString,&kp);
      keyValPair.first = kp.primaryKey;
      keyValPair.second.nextWord = kp.subKey;
      keyValPair.second.freq = freq;

      //assume no checks to see if key already exists; file should have all unique/consolidated strs
      tablePtr->insert( keyValPair );
    }
  }

  close(fd);

  return 0;
}
*/

void nGram::makeKeyPair(string& gramString, KeyPair* keys)
{

  int lastSpace = gramString.find_last_of(this->wordDelimiter,gramString.npos);

  if(lastSpace == gramString.npos){  //HACK: this is bad, but if there is no space in gramStr, we assume we're looking in 1-gram model (risky if query itself is bad)
    keys->primaryKey = gramString;
    keys->subKey = gramString;
  }
  else{
    keys->primaryKey = gramString.substr(0, lastSpace);
    keys->subKey = gramString.substr(lastSpace+1, gramString.length());
  }

  //cout << "in makeKey, verify key/subkey from gramstr>" << gramString << "< : " << keys->primaryKey << "/" << keys->subKey << endl;
  //cin >> lastSpace;
} 

//check if table has some primary key (a key of n-1 grams)
bool nGram::gramTableHasKey(string gramString, FreqTablePointer tablePtr)
{
  KeyPair kp;

  makeKeyPair(gramString, &kp);

  return tablePtr->find(kp.primaryKey) == tablePtr->end();
}

//gets the frequency of a full gramStr
/*
int nGram::getFrequency(string gramString, map<string,int>* tablePtr)
{
  if(gramTableHasKey(gramString,tablePtr)){
    return (*tablePtr)[gramString];
  }
  return 0;
}
*/

void nGram::testGetGramsFromTable(void)
{
  list<freqTuple> grams;
  string testString = "your";  
  string nGramFile = "2gram.ng";
  //nGram nModel;
  
  FreqTablePointer table = getTablePointer(2);

  if(table == NULL){
    cout << "ERROR nModel table not found in getMaxFromTable" << endl;
    return;
  }

  //TODO: fix these
  //fileToTable(nGramFile.c_str(),2);
  //queryTable(testString, table, 2, 10, grams);
}


/*
  Returns the ith index of a char. This is useful for analyzing subsequences of words in some string,
  delimited by ' '.  Typical use-case is comparing the first n-words of some query to another
  string of words.

  Returns: ith index, or -1 if not found.
*/
int nGram::getIthIndexOf(char c, int ith, const string str)
{
  int i, ct;

  for(i = 0, ct = 0; (i < str.length()) && (str[i] != FILE_DELIMITER); i++){
    if(str[i] == c){
      ct++;
      if(ct == ith){
        return i;
      }
    }
  }

  return -1;
}

/*
  Given a query of length (n-1), return a sorted list of all grams for that word
  Table must be in memory!

  *This function has important usage characteristics:

    --Remember, n-gram models estimate the most likely next word, given a string of
    n-1 words. So if we want to query a 3-gram model, our query must be length (3-1) = 2 words. 

    --Then notice if we want to query a 1-gram model, then our query length is n-1 = zero

    The algorithm for storing only the first nPrediction results could probably be improved. Or better, the multimap
    subsets could be stored in sorted order, but this is unlikely to be possible without building the multimaps,
    pulling and sorting all their subelements, then rebuilding the multimap (though it is possible to suffice the ordered-subset
    precondition, which could prove useful elsewhere).

  Returns: number of grams added to list

  Post-condition: all query result gramTuples are added to gramList, sorted by decreasing frequency.

  Notes: not sure if this should have both an nModel parameter and a table* parameter or not. Function 
  needs the nModel parameters only to split logic on the 1-gram model case. Whether to have both params or
  not depends on the data model definition we develop later. Kept as-is for now.

  TODO: This needs re-write when re-building trained n-gram models. Get rid of dual list filter design.

*/
int nGram::queryTrainingTable(const string& query, FreqTablePointer table, int nModel, int nPredictions, list<freqTuple>& resultList)
{
  int ct, i;
  MapSubset subset;
  KeyIt outer;
  SubkeyIt inner;
  FreqTuple tup;
  list<freqTuple> tempList;
  list<freqTuple>::iterator lit;

  if(table == NULL){
    cout << "ERROR nModel table not found in getMaxFromTable" << endl;
    return 0;
  }

  //if querying the 1-gram model, just return the freqTuple of the query string
  if(nModel == 1){
    if((outer = table->find(query)) != table->end()){
      tup.freq = (*table)[query][query];
      tup.nextWord = query;
      resultList.push_front( tup );
      ct = 1;
    }
    else{
      ct = 0;
    }
  }
  //else, query n-gram model by getting range of items
  else{
    subset = equalRange(table,query);
    if(subset.first == subset.second){
      cout << "No items found for query <" << query << ">" << endl;
      ct = 0;
    }
    else{
      //get all results, then sort them by frequency since the multimap subsets are not frequency-sorted
      for(ct = 0, inner = subset.first; inner != subset.second; ++inner){
        tup.nextWord = inner->first;
        tup.freq = inner->second;
        tempList.push_front(tup);
        ct++;
      }
      tempList.sort(compare);
      //append the first nPrediction results to the output list
      for(i = 0, lit = tempList.begin(); (i < nPredictions) && (lit != tempList.end()); i++, ++lit){
        resultList.push_back(*lit);
      }
      tempList.clear();  //probly not necessary, since list should be deallocated on function exit
    }
  }
  
  return ct;
}


void nGram::nullifyTokens(char* toks[], int ntoks)
{
  for(int i = 0; i < ntoks; i++){
    toks[i] = NULL;
  }
}



/*
  Get the most likely n-Gram string, argmax(word,nModel).
  File io version. Expect frequency file to be sorted.
  Must work for nGram models of any n.
*/
string nGram::getMaxFromFile(char* fname, string word)
{
  int i, fd, n, k = 0, pos, max;
  U64 freq;
  bool bailout, found;
  string gramStr;
  char buf[2048];
  //char line[128];

  fd = open(fname, O_RDONLY);
  if(fd < 0){
    cout << "ERROR file >" << fname << "< not opened in getMaxFromFile()" << endl;
    return "NOT_FOUND";
  }
  
  //we need a space as a right-anchor for word comparisons (or else strncmp("your","yourselves","your".length()) will return MATCH)
  if(word[word.length()-1] != ' '){
    word += " ";
  }

  //STATE 1: spool to location of line starting with first letter of word (binary search)
  pos = -1;
  while((pos < 0) && (n = read(fd, buf, 2048))){
    for(i = 0; (i < (n - 2)) && (pos < 0); i++){
      if((buf[i] == '\n') && (word[0] == buf[i+1]) && (word[1] == buf[i+2])){ //match on first two letters of word, using newline as left-anchor
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

/*
  DEPRECATED

  Read in a file containing natural language, and parse it by n-Grams.

  To-do: factor out the text normalization into its own function. That way we call
  parseCorpus(), then call buildModel(2-gram), buildModel(3-gram), etc. on parsed files.
  
    -post-condition of parseCorpus should be a parsed corpus
    -post-condition of buildModel() is a model is loaded into memory or output to file, or both


bool nGram::parseCorpus(string ifile, string ofile, int nModel)
{
  int n, lastDelim;
  U64 gCt = 0;
  int inFile, outFile;
  char buf[BUFSIZE];

  buf[BUFSIZE-1] = '\0';

  //build the model from some corpus
  inFile = open(ifile.c_str(), O_RDONLY);
  if(inFile < 0){
    cout << "could not open input file >" << corpus << "< parseCorpus() aborted" << endl;
    return false;
  }

  outFile = open(ofile, O_CREAT | O_WRONLY | O_TRUNC);
  //freqTable = open("freqTable.txt", O_CREAT | O_RDWR);
  if(outFile < 0){
    cout << "could not open model file >parsedCorpus.txt< parseCorpus() aborted" << endl;
    return false;
  }

  //regular structures in huckleberry finn are period-delimited sentences, dash-delimited phrases, and double-quote-delimited quotations

  //read 1024 bytes, then spool file position back to location of last delimiter
  while(n = read(inFile, buf, READ_SZ)){

    buf[n] = '\0';
    //return file position to index of last delimiter (the end of the last regular structure), to get a bounded sample.
    lastDelim = seekLastPhraseDelimiter(buf,n);
    if(lastDelim != -1){

      
      //cout << "found last delimiter: " << buf[lastDelim] << "buf[lastdelim + 1]=" << buf[lastDelim+1] << endl;
      //buf[lastDelim + 5] = '\0';
      //cout << "&buf[lastDelim]" << &buf[lastDelim+1] << endl;
      //buf[lastDelim] = '\0';
      
      buf[lastDelim] = '\0';

      //prevents infinite loop at end of file: we will continuously wind back to last delim if not checked.
      if(n == READ_SZ){
        //cout << "sought" << endl;
        lseek(inFile, lastDelim - n + 1, SEEK_CUR);
      }

      //DEBUG
      //read(inFile,buf,8);
      //buf[8] = '\0';
      //cout << "verify equals last delimiter above: >" << buf << "<" << endl;
      //cin >> dummy;
      //lseek(inFile, -1, SEEK_CUR);
      

      //cout << "here. last delim=" << lastDelim << " and n=" << n << endl; cin >> dummy;
      //a series of pipes/filters to clean up sample, each of which will be defined for different corpii to fit different language styles.
      normalizeText(buf);  //preprocess strings to fit our (simple) language model (stem, drop, etc.)

      //DEBUG: output samples to file to verify correct
      write(outFile, buf, lastDelim);    

      //build n-gram dict
      gCt += updateNgramTable(buf, lastDelim, nModel);

      //nullifyString(buf,BUFSIZE);
      buf[BUFSIZE-1] = '\0';
    }
    else{
      cout << "WARN no phrase delimiter found in buf, input discarded: >" << buf << "<" << endl;
    }
  }

  //verify: frequencyTable.count() == gCt == analyzeModelFile().gCt ???

  //analyzeModelFile(); //could get min/max/avg words per sentence/phrase, etc., other stats, such as gCt, for verification
  cout << "*********" << nModel << "-gram model counts*******" << endl;
  cout << "gram-string count:  " << gCt << endl;
  cout << "word count:         " << wordCt << endl;
  cout << "phrase count:       " << phraseCt << endl;
  cout << "***********************************" << endl;

  gCt = wordCt = phraseCt = 0;

  close(inFile);
  close(outFile);

  return true;
}
*/

/*
   Finds index of the nth-to-last token in buf[]. This is necessary to adjust our file position such that we
   get a pseudo-continuous stream of ngrams from a file. For instance, the last three words in a buffer
   have only one 3-gram. We back the file position up to the second-to-last word in the buffer, so the next read
   will include the last ending two words as part of its input.
   
  Example: given "$the$dog$was###brow\0" return index of 'd' in 'dog' if nth==2 (eg, usage for a 3-gram model)

  Notes: for strings with an insufficient number of grams (words), returns -1.
  This occurs if ngrams <= nth, NOT just ngrams < nth!

  Precondition: len must be the right index of the buffer boundary, which is a phraseDelimiter ('#') index, since
  we currently bound samples by phrase delimiters. Thus, buf[len] should be the leftmost delimiter in the
  rightmost group of delimiters (eg, the first '#' in the buffer "went to the park###The next day w\0"
*/
int nGram::seekNthLastTokenIndex(int nth, const char buf[BUFSIZE], int len)
{
  int i, nToken;

  i = len - 1;

  //begin loop: i points at 's' in "$the$dog$was###brow\0"
  for(nToken = 0; nToken < nth; nToken++){

    //spool to leftmost delim in this group
    for( ; (i >= 0) && isDelimiter(buf[i], this->delimiters); i--){}
    //end loop: i points at a non-delimiter OR i == -1

    //spool to next leftward delimiter
    for( ; (i >= 0) && !isDelimiter(buf[i], this->delimiters); i--){}
    //end loop: i points at any delimiter OR i == -1

  }

  //off by one after last last loop, so increment by one.
  if(i >= 0){
    i++;
  }

  //cout << nth << "_th-last token index is >" << buf[i] << "< as str: >" << &buf[i] << "<" << endl;

  return i;
}

/*
  Build an n-gram model in memory based on parsed input file.
  
  Note the flags rawParse and contextualParse. This function maps different i/o logic for each,
  such that we get samples bounded by the requirements of each type of parsing (contextual
  vs. continuous).
  
  Pre-condition: input file must be parsed/normalized.
  Post-condition: all of the n-length substrings in the parsedFile are added
  to our frequency table/map data structure. Word counts and phrase counts are stored
  in class memory, but not reliably, since we do little analysis on them.
*/
bool nGram::buildModel(string parsedFile, int nModel)
{
  int ifile, filePos, gCt, n, lastDelim, nthLastToken;
  char buf[BUFSIZE];

  //build the model from some parsed corpus
  ifile = open(parsedFile.c_str(), O_RDONLY);
  if(ifile < 0){
    cout << "could not open input file >" << parsedFile << "< buildModel() aborted" << endl;
    return false;
  }

  gCt = wordCt = phraseCt = 0;
  //read READ_SZ bytes, then spool file position back to location of last delimiter
  while(n = read(ifile, buf, READ_SZ)){

    lastDelim = seekLastPhraseDelimiter(buf,n);    
    buf[lastDelim] = '\0';
    //cout << "buf: " << &buf[lastDelim - 60] << endl;

    //return file position to index put bounds on the samples (the end of the last regular structure).
    if(this->contextualParse){  //finds terminal delimiter of last contiguous phrase: "had$a$nice$day##Then$I$$wen\0" returns index of first '#'
      nthLastToken = 99;  //dummy value so we pass error check below
    }
    if(this->rawParse){  //finds the last of any delimiters: "had$a$nice$day##Then$I$$wen\0" returns the leftmost '$' in the rightmost group of '$' delimiters
      //cout << "here " << endl;
      //this saves the position of the starting position of the last n-1 gram. "the cat was\0" returns the index of 'c', the start of the next
      nthLastToken = seekNthLastTokenIndex(nModel,buf,lastDelim);

      /*cout << "lastdelim/nthLastToken: " << lastDelim << "|" << nthLastToken << " <str>" << &buf[nthLastToken] << endl;
      cout << "next buf: <str>";
      for(int p = 0; p < 60; p++){
        cout << buf[p];
      }
      cout << endl;
      */
    }
    

    if(lastDelim >= BUFSIZE){
      cout << "ERROR lastDelim >= BUFSIZE in buildModel()" << endl;
    }
    if(nthLastToken >= BUFSIZE){
      cout << "ERROR nthLastToken >= BUFSIZE in buildModel()" << endl;
    }

    if((lastDelim > 0) && (nthLastToken > 0)){
      /*
      cout << "found last delimiter: " << buf[lastDelim] << "buf[lastdelim + 1]=" << buf[lastDelim+1] << endl;
      buf[lastDelim + 5] = '\0';
      cout << "&buf[lastDelim]" << &buf[lastDelim+1] << endl;
      buf[lastDelim] = '\0';
      */

      //prevents infinite loop at end of file: we will continuously wind back to last delim if not checked.
      if(n == READ_SZ){

          //for contextual parsing, we adjust file pos to last delim
          if(this->contextualParse){
            filePos = lastDelim - n + 1;
          }
          //for raw parsing, we adjust to make the ngrams continuous between reads
          if(this->rawParse){
            filePos = nthLastToken - n;
/*
            //debug
            cout << "nth last token==" << nthLastToken << ". confirm buf[nthlasttoken]==file[file.pos]: " << buf[nthLastToken];
            read(ifile,buf,5);
            cout << "==" << buf[0] << endl;
            cin >> n;
*/
          }
          //cout << "new file pos: " << filePos << endl;
          lseek(ifile, filePos, SEEK_CUR);
      }

      //build n-gram dict
      gCt += updateNgramTable(buf, lastDelim, nModel, this->delimiters);

      //nullifyString(buf,BUFSIZE);
      //buf[BUFSIZE-1] = '\0';
    }
    else{
      cout << "WARN lastDelimiter==-1 in parseCorpus" << endl;
    }
  }
  
  //analyzeModelFile(); //could get min/max/avg words per sentence/phrase, etc., other stats, such as gCt, for verification
  cout << "***********" << nModel << "-gram model counts*********" << endl;
  cout << "gram-string count:  " << gCt << endl;
  cout << "word count:         " << wordCt << endl;
  cout << "phrase count:       " << phraseCt << endl;
  cout << "***************************************" << endl;

  close(ifile);
  
  return true;
}

/*
  Read in a file containing natural language, and preprocess it so it is consumable by our models.
  Output to ofile should be standardized, such that this function should never need to be customized for
  different parsing schemes by other functions (for instance, contextutal vs. non-contextual parsing).
  This will keep parsing and reading/interpreting functionality separate.
*/
bool nGram::parseCorpus(string ifile, string ofile)
{
  int n, lastDelim;
  U64 gCt = 0;
  int inFile, outFile;
  char ibuf[BUFSIZE];
  string obuf;

  obuf.reserve(BUFSIZE*2);

  ibuf[BUFSIZE-1] = '\0';

  //open input corpus
  inFile = open(ifile.c_str(), O_RDONLY);
  if(inFile < 0){
    cout << "could not open input file >" << ifile << "< parseCorpus() aborted" << endl;
    return false;
  }

  outFile = open(ofile.c_str(), O_CREAT | O_WRONLY | O_TRUNC);
  //freqTable = open("freqTable.txt", O_CREAT | O_RDWR);
  if(outFile < 0){
    cout << "could not open model file >" << ofile << "< parseCorpus() aborted" << endl;
    return false;
  }

  //read BUFSIZE bytes, then spool file position back to location of last delimiter
  while(n = read(inFile, ibuf, READ_SZ)){
    //cout << "parsing..." << endl;
    ibuf[n] = '\0';
    //return file position to index of last delimiter (the end of the last regular structure), to get a bounded sample.
    lastDelim = seekLastPhraseDelimiter(ibuf,n);
    if(lastDelim != -1){
 
      /*
      cout << "found last delimiter: " << buf[lastDelim] << "buf[lastdelim + 1]=" << buf[lastDelim+1] << endl;
      buf[lastDelim + 5] = '\0';
      cout << "&buf[lastDelim]" << &buf[lastDelim+1] << endl;
      buf[lastDelim] = '\0';
      */
      ibuf[lastDelim] = '\0';

      //prevents infinite loop at end of file: we will continuously wind back to last delim if not checked.
      if(n == READ_SZ){
        lseek(inFile, lastDelim - n + 1, SEEK_CUR);
      }

      /* //DEBUG
      read(inFile,buf,8);
      buf[8] = '\0';
      cout << "verify equals last delimiter above: >" << buf << "<" << endl;
      cin >> dummy;
      lseek(inFile, -1, SEEK_CUR);
      */

      //cout << "here. last delim=" << lastDelim << " and n=" << n << endl; cin >> dummy;
      //a series of pipes/filters to clean up sample, each of which will be defined for different corpii to fit different language styles.
      normalizeText(ibuf,obuf);  //preprocess strings to fit our (simple) language model (stem, drop, delimit, etc.)

      //DEBUG: output samples to file to verify correct
      write(outFile, obuf.c_str(), lastDelim);

      //nullifyString(buf,BUFSIZE);
      ibuf[BUFSIZE-1] = '\0';
      obuf.clear();
    }
    else{
      cout << "WARN no phrase delimiter found in buf, input discarded: >" << ibuf << "<" << endl;
    }
  }

  close(inFile);
  close(outFile);

  return true;
}


/*
  Given a query of n-1 grams (ie, a 2-word query for a 3-gram model),
  return the argmax next most likely word.

  Returns: the next most likely word, given an n-1 gram-length query string, or <UNK> for no result

string nGram::getMaxFromTable(const string query, int nModel)
{
  string ret = "<UNK>";
  KeyIt it;
  FreqTablePointer tablePtr = getTablePointer(nModel);

  if(tablePtr == NULL){
    cout << "ERROR nModel table not found in getMaxFromTable" << endl;
    return ret;
  }
  //is more query validation needed??
  if(countChar(query.c_str(),' ') >= (nModel - 1)){
    cout << "ERROR table query >" << query << "< contains >= grams: >" << countChar(query.c_str(), ' ') << ">= " << nModel << "-gram model" << endl;
    return ret;
  }

  it = tablePtr->find(query);
  if(it != tablePtr->end()){
    ret = it->second.nextWord;
  }

  return ret;
}
*/

//void nGram::testDriver(void)
//{
//  list<int> myList;
//  int child = 0, pid = 0, i = 0, status = 0, index = 0, n, lastDelim, dummy;
//  unsigned long long lines;
//  U64 gCt = 0;
//  int inFile, modelFile;
//  char buf[BUFSIZE];
//  //char* phrases[PHRASES_PER_READ];
//  map<string,int>::iterator it;
//
//
//  //build the model from some corpus
//  inFile = open("../huckleberryFinn.txt", O_RDONLY);
//  //inFile = open("huckTest.txt", O_RDONLY);
//  modelFile = open("../parsedCorpus.txt", O_CREAT | O_WRONLY | O_TRUNC);
//  //freqTable = open("freqTable.txt", O_CREAT | O_RDWR | O_TRUNC);
//
//  if(inFile < 0){
//    cout << "could not open input file >huckleberryFinn.txt< run aborted!" << endl;
//    return;
//  }
//  if(modelFile < 0){
//    cout << "could not open model file >modelFile.txt< run aborted!" << endl;
//    return;
//  }
//
//  //regular structures in huckleberry finn are period-delimited sentences, dash-delimited phrases, and double-quote-delimited quotations
//  //ignore: "CHAPTER n" and hidden whitespace chars (newlines, etc)
//
//  //read 1024 bytes, then spool file position back to location of last delimiter
//  while(n = read(inFile, buf, READ_SZ)){
//
//    //(n == READ_SZ) ? (buf[READ_SZ+1] = '\0') : (buf[n] = '\0');
//    buf[n] = '\0';
//
//    //return file position to index of last delimiter (the end of the last regular structure), to get a bounded sample.
//    lastDelim = seekLastPhraseDelimiter(buf,n);
//    if(lastDelim != -1){
//      //buf[lastDelim] = phraseDelimiter;
//      //buf[lastDelim+1] = '\0';
//      buf[lastDelim] = '\0';
//
//      //prevents infinite loop at end of file: we will continuously wind back to last delim if not checked.
//      if(n == READ_SZ){
//        lseek(inFile, lastDelim - n, SEEK_CUR);
//      }
//
//      //cout << "here. last delim=" << lastDelim << " and n=" << n << endl; cin >> dummy;
//      //a series of pipes/filters to clean up sample, each of which will be defined for different corpii to fit different language styles.
//      normalizeText(buf);  //preprocess strings to fit our (simple) language model (stem, drop, etc.)
//
//      //DEBUG: output samples to file to verify correct
//      write(modelFile, buf, lastDelim);    
//
//      //build n-gram dict
//      gCt += updateNgramTable(buf, lastDelim, NGRAM);
//    }
//    else{
//      cout << "WARN no phrase delimiter found in buf, input discarded in testDriver" << endl;
//    }
//  }
//
//  //verify: frequencyTable.count() == gCt == analyzeModelFile().gCt ???
//
//  //analyzeModelFile(); //could get min/max/avg words per sentence/phrase, etc., other stats, such as gCt, for verification
//  cout << "gCt=" << gCt << endl;
//  cout << "wordCt=" << wordCt << endl;
//  cout << "phraseCt=" << phraseCt << endl;
//
//  /*
//  it = frequencyTable.begin();
//  for(i = 0; i < 20; i++){
//    cout << "key,val: <" << it->first << "," << it->second << ">" << endl;
//    it++;
//  }
//  */
///*
//  int max = 0;
//  string key;
//  for(it = frequencyTable.begin(); it != frequencyTable.end(); ++it){
//    //cout << "<" << it->first << "||" << it->second << ">" << endl;
//    if(max < it->second){
//      max = it->second;
//      key = it->first;
//    }
//  }
//  cout << "max:  <" << key << "||" << max << ">" << endl;
//*/
//  lines = tableToFile(freqFile.c_str(), NGRAM);
//
//  fileToTable(freqFile.c_str(), NGRAM);
//
//  close(inFile);
//  close(modelFile);
//}


//OBSOLETE, unless summing probs in a subset. TODO: rename or re-write this function.
long double nGram::getSumFrequency(FreqTablePointer table)
{
  KeyIt outer;
  SubkeyIt inner;
  long double ret;

  if(table == NULL){
    cout << "ERROR table ptr NULL in getSumFrequency(). returning -1.0" << endl;
    return -1.0;
  }

  for(ret = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      ret += inner->second;
    }
  }

  return ret;
}


void nGram::buildCocaModels(void)
{
  int i, j;
  FreqTablePointer tablePtr, twoGramTablePtr; //oneGramPosTablePtr, twoGramPosTablePtr;
  //debug
  struct timespec begin, end;

  //build 2-5 gram models from COCA files
  for(i = 2; i <= 5; i++){
    cout << "begin... file: " << cocaNgramFiles[i] << endl;
    clock_gettime(CLOCK_MONOTONIC,&begin);
    buildCocaModel(i);
    clock_gettime(CLOCK_MONOTONIC,&end);
    cout << "\nbuild time (s): " << (end.tv_sec - begin.tv_sec) << endl;

    //debug
    tablePtr = getTablePointer(i);
    cout << "  built coca model[" << i << "] " << tablePtr->size() << " keys   size is (kb): " << (sizeof(*tablePtr) >> 10) << endl;
    /*
    for(j = 0, it = tablePtr->begin(); j < 100; j++, ++it){
      cout << "key: " << it->first << "|(" << it->second.nextWord << "," << it->second.freq << ")" << endl;
    }
    */
  }

  //cout << "2-gram table size: " << getTablePointer(2)->size() << " keys" << endl;

  //now build the one-gram model from data in the 2-gram table. This is done after 2-gram table since the 1-g table requires 2-g data.
  tablePtr = getTablePointer(1);
  twoGramTablePtr = getTablePointer(2);
  buildCocaOneGramModel(twoGramTablePtr, tablePtr);

  this->isCocaBuilt = true;

  //calculate the models stats for each table, BEFORE normalizing the tables
  calculateModelStats();
  printModelStats();
  initLambdas();

  //now normalize the tables
  cout << "normalizing one-gram table..." << endl;
  normalizeOneGramTable(tablePtr);
  for(i = 2; i <= 5; i++){
    normalizeTable( getTablePointer(i) );
  }
  this->normalized = true;

  cout << "1-gram table size: " << getTablePointer(1)->size() << " items" << endl;
  cout << "2-gram table size: " << getTablePointer(2)->size() << " items" << endl;
  cout << "3-gram table size: " << getTablePointer(3)->size() << " items" << endl;
  cout << "4-gram table size: " << getTablePointer(4)->size() << " items" << endl;
  cout << "5-gram table size: " << getTablePointer(5)->size() << " items" << endl;
  //build the one gram pos table
  //buildCocaOneGramModel(twoGramPosTable, oneGramPosTable);

/*  //building pos models is now done w/in buildCocaModel()
  for(i = 2; i <=5; i++){
    buildCocaPosTable(cocaNgramFiles[i], i);

    //debug
    table = getPosTablePointer(i);

    cout << "built coca model[" << i << "]  size is (bytes): " << (sizeof(MapPair) * (U64)(table->size())) << endl;  
  }
*/  

/*
  //DEBUG
  cout << "sample of 5-gram pos table: " << endl;
  table = getPosTablePointer(5);
  for(KeyIt it = table->begin(); it != table->end() && i < 20; ++it, i++ ){
    cout << it->first << " " << it->second.nextWord << "|" << it->second.freq << endl;
  }
  cin >> i;
  
  cout << "sample of 5-gram table: " << endl;
  table = getTablePointer(5);
  for(KeyIt it = table->begin(); it != table->end() && i < 20; ++it, i++ ){
    cout << it->first << " " << it->second.nextWord << "|" << it->second.freq << endl;
  }
  cin >> i;
*/

}

void nGram::buildModels(string corpus)
{
  string parsedFile = "../parsedCorpus.txt";

  //preprocess corpus
  if( !parseCorpus(corpus, parsedFile) ){
    cout << "parseCorpus(" << corpus << ", " << parsedFile << ") failed in build_N_Models(" << corpus << ", " << parsedFile << ", " << ")" << endl;
  }

  if(!build_N_Models(parsedFile,5)){
    cout << "ERROR occurred in buildModels." << endl;
  }

}


/*
  Builds 1-N n-gram models from some input corpus. The corpus is first parsed and written to a parsed output file,
  then we build each model from the parsed file.
*/
bool nGram::build_N_Models(string parsedFile, int nModels)
{
  int i, dummy;
  char buf[4];
  string nModelFile;

  if(nModels > 6){
    cout << "cannot build over 6-gram models, buildModels() building up to 5-gram" << endl;
    nModels = 5;
  }

  for(i = 1; i < nModels+1; i++, nModelFile.clear()){
    if(buildModel(parsedFile,i)){
      sprintf(buf,"%d",i);
      nModelFile = buf;
      nModelFile += "gram.ng\0";
      //tableToFile(nModelFile.c_str(), i); //TODO: rebuild this functionality
    }
    else{
      cout << "buildModel(" << parsedFile << ", " << i << ") failed in build_N_Models(" << ", " << parsedFile << ", " << nModels << ")" << endl;
    }
  }

  calculateModelStats();
  printModelStats();

  return true;
}

/*
  Description: calculates model stats for each model, up to 5-grams.
  
  Lazy is better. I index into the model stats vector with the actual number of the n-gram
  model. This means I'm wasting the zeroth index to make accessing the vector more accessible
  by indexing it using the nModel parameter (used in many functions). The subEntropy indices
  for the 1-gram model are also wasted, since they are not meaningful for a 1-gram model.

  COCA output:

              Raw H(X)  Expct. H(x)   Avg Expct. H(x)   Perplexity   Avg Expct. Perp.
    ---------------------------------------------------------------------------------
      1-gram  10.8969   NA            NA                   1906.79
      2-gram  17.8304   8.2849        2.1655             233073.63      311.88
      3-gram  19.6260   5.2445        1.0829             809141.73       37.91
      4-gram  20.2141   3.1517        0.7219            1216291.92        8.89
      5-gram  18.8104   1.9567        0.4885             459735.64        3.88
      1g POS  -1.0000   NA            NA                     -1.00
      2g POS  -1.0000  -1.0000       -1.0000                 -1.00       -1.00
      3g POS  -1.0000  -1.0000       -1.0000                 -1.00       -1.00
      4g POS  -1.0000  -1.0000       -1.0000                 -1.00       -1.00
      5g POS  -1.0000  -1.0000       -1.0000                 -1.00       -1.00
    **********************************************************************************


*/
void nGram::calculateModelStats(void)
{
  int i;
  FreqTablePointer table;

  //there is no [0] index, so [0] is just wasted space. but its nicer for indexing the array by model
  wordModelStats[0].totalEntropy = -1;
  wordModelStats[0].expectedSubEntropy = -1;
  wordModelStats[0].meanSubEntropy = -1;
  wordModelStats[0].totalPerplexity = -1;

  //build one gram word model stats. its data model is different, so calculating its values is an exception route.
  table = getTablePointer(1);
  wordModelStats[1].sumFrequency = getSumFrequency(table);
  //wordModelStats[1].totalEntropy = getTotalEntropy_OneGram(table);
  wordModelStats[1].totalEntropy = getTotalEntropy(table);
  wordModelStats[1].expectedSubEntropy = -1;
  wordModelStats[1].meanSubEntropy = -1;
  if(wordModelStats[1].totalEntropy > 0){
    wordModelStats[1].totalPerplexity = pow(2.0, wordModelStats[1].totalEntropy);
  }

  for(i = 2; i <= 5; i++){

    table = getTablePointer(i);
    if(table != NULL){
      wordModelStats[i].sumFrequency = getSumFrequency(table);
      wordModelStats[i].totalEntropy = getTotalEntropy(table);
      wordModelStats[i].expectedSubEntropy = getExpectedSubEntropy(table);
      wordModelStats[i].meanSubEntropy = getMeanSubEntropy(table);
      wordModelStats[i].totalPerplexity = getTotalPerplexity(table, wordModelStats[i].totalEntropy);
      wordModelStats[i].expectedSubPerplexity = getExpectedSubPerplexity(table, wordModelStats[i].expectedSubEntropy);
    }
    else{
      cout << "table ptr NULL in calculateWordModelStats()" << endl;
    }
  }

  /*build POS model stats
  table = getPosTablePointer(1);
  posModelStats[1].sumFrequency = getSumFrequency(table);
  posModelStats[1].totalEntropy = getTotalEntropy(table);
  posModelStats[1].expectedSubEntropy = -1;
  posModelStats[1].meanSubEntropy = -1;
  posModelStats[1].totalPerplexity = getTotalPerplexity(table, posModelStats[1].totalEntropy);  
  
  for(i = 2; i <= 5; i++){
 
    table = getPosTablePointer(i);
    if(table != NULL){
      posModelStats[i].sumFrequency = getSumFrequency(table);
      posModelStats[i].totalEntropy = getTotalEntropy(table);
      posModelStats[i].expectedSubEntropy = getExpectedSubEntropy(table);
      posModelStats[i].meanSubEntropy = getMeanSubEntropy(table);
      posModelStats[i].totalPerplexity = getTotalPerplexity(table, posModelStats[i].totalEntropy);
      posModelStats[i].expectedSubPerplexity = getExpectedSubPerplexity(table, posModelStats[i].expectedSubEntropy);
    }
    else{
      cout << "table ptr NULL in calculateWordModelStats()" << endl;
    }
  }  
  */
  
}

/*
  get the perplexity of some individual subset of items
  check that it is valid to use sumFreq as the argument to power (should it be sumFreq, or nWords in model?)
  Returns: long double perplexity value if query is in model; else returns -1.0
*/
long double nGram::getExpectedSubPerplexity(FreqTablePointer table, long double expectedSubEntropy)
{
  long double ret;

  if(expectedSubEntropy <= 0.0){
    cout << "Computing subEntropy to get subPerplexity..." << endl;
    ret = getExpectedSubEntropy(table);
  }

  //check on INF_ENTROPY, which will cause overflow for perplexity, since perplexity is 2^entropy
  if(expectedSubEntropy > 0 && expectedSubEntropy < 64){
    ret = pow(2.0, expectedSubEntropy);
  }
  else{
    cout << "WARN subPerplexity too large to compute based on subEntropy. Set to -1." << endl;
    ret = -1;
  }

  return ret;
}

/*
  Get the complete perplexity of a model. (see Jurafsky, p.96, perplexity calculations)
  See wikipedia. Perplexity is simply 2^ H(x) where H(x) is some entropy function (total entropy, subentropy, etc).
  So here all we do is get our previous total entropy value, or update it, then use it to get perplexity.
*/
long double nGram::getTotalPerplexity(FreqTablePointer table, long double totalEntropy)
{
  long double ret;

  if(totalEntropy < 0.0){
    cout << "WARNING getTotalPerplexity() called prior to totalEntropy being defined. Should not hit this line" << endl;
    ret = getTotalEntropy(table);
  }
  else if(totalEntropy > 0.0 && totalEntropy < 64.0){
    ret = pow(2.0, totalEntropy);
  }
  else{
    cout << "WARN perplexity too large to compute based on (total) entropy. Set to -1." << endl;
    ret = -1.0;
  }

  return ret;
}


void nGram::printModelStats(void)
{
  char numbuf[128];
  int i;
  
  cout << "********************************Model Statistics*********************************" << endl
       << "  H(X) is total/raw entropy over all n-grams." << endl
       << "  Expct. H(x) is the expected value of entropy computed for each n-1 gram subset.\n"
       << "  Perplexity is simply pow(2,entropy(x)) or rather 2^(H(x)).\n" << endl
       << "         Raw H(X)  Expct. H(x)   Avg Expct. H(x)   Perplexity   Avg Expct. Perp." << endl
       << "---------------------------------------------------------------------------------" << endl;
  sprintf(numbuf,"%4.4Lf   NA            NA                %10.2Lf",wordModelStats[1].totalEntropy,wordModelStats[1].totalPerplexity);
  cout << " 1-gram  " << numbuf << endl;
  for(i = 2; i <= 5; i++){
    sprintf(numbuf,"%4.4Lf   %4.4Lf        %4.4Lf            %10.2Lf  %10.2Lf",wordModelStats[i].totalEntropy, wordModelStats[i].expectedSubEntropy, wordModelStats[i].meanSubEntropy,wordModelStats[i].totalPerplexity,wordModelStats[i].expectedSubPerplexity);
    cout << " " << i << "-gram  " << numbuf << endl;
  }
  sprintf(numbuf,"%4.4Lf   NA            NA                %10.2Lf",posModelStats[1].totalEntropy,posModelStats[1].totalPerplexity);
  cout << " 1g POS  " << numbuf << endl;
  for(i = 2; i <= 5; i++){
    sprintf(numbuf,"%4.4Lf  %4.4Lf       %4.4Lf            %10.2Lf  %10.2Lf",posModelStats[i].totalEntropy, posModelStats[i].expectedSubEntropy, posModelStats[i].meanSubEntropy,posModelStats[i].totalPerplexity,posModelStats[i].expectedSubPerplexity);
    cout << " " << i << "g POS  " << numbuf << endl;
  }
  cout << "**********************************************************************************" << endl;
}

void nGram::userQueryMenu(void)
{
  cout << "nGram models can be queried by word. For instance, querying \"the\" may\n"
       << "return\"the dog was|5\" from the 3-gram model, where \'5\' denotes then\n"
       << "number of times that gram string occurs. To query, enter the (int) nGram\n"
       << " model, followed query string. Finally, enter the number of results to\n"
       << " return. Enjoy!" << endl;
}

//general version for when we are going to query all models, so we dont need an ngram parameter.
void nGram::getQueryParameters(string& queryStr, int& nPredictions)
{
  bool valid;
  string prompt;

  userQueryMenu();

  valid = false;
  while(!valid){
    queryStr.clear();
    cin.clear();
    cout << "*queries include n-1 words separated by single spaces, where 'n' is the n-gram model\nEnter query: ";
    getline(std::cin, queryStr);
    valid = isValidQuery(queryStr,5);
  }

  prompt = "Enter number of predictions (top k n-gram strings, where k = 1-5): ";
  nPredictions = userGetInt(prompt);
}

/*
  Checks if number of words is equal to nModel-1.
  Queries to nGram models must include n-1 words,
  separated by a single space (if n>1).
    1-gram query==nospaces / one word
    2-gram query==nospaces / one word
    3-gram query==one space / two words
    4-gram query==two spaces / three words
    ...etc
*/
bool nGram::isValidQuery(const string& query, int nModel)
{
  int i, nspaces;

  for(i = 0, nspaces = 0; i < query.length(); i++){
    if(query[i] == ' '){
      nspaces++;
    }
  }

  if((nModel == 1) && (nspaces > 0)){
    cout << "invalid query. 1-gram queries may include only one word." << endl;
    return false;
  }
  if((nModel == 1) && (nspaces == 0)){
    return true;
  }

  nspaces++;  //increment by two to compare with nModel valu (ex: query "king and" valid for 3-gram model, contains only one space)

  if(nspaces != (nModel-1)){
    cout << "invalid query. queries may include only n-1=" << nModel-1 << " words, separated by single spaces" << endl;
    //DEBUG
    cout << "nspaces=" << nspaces << " nModel=" << nModel << "  query=" << query << endl;
    return false;
  }

  return true;
}

/*
  Get user search query parameters: word, ngram model to search within, and numpredictions to return from model.
*/
void nGram::getQueryParameters(string& queryStr, int& nModel, int& nPredictions)
{
  bool valid;
  string prompt;
  //string buf;

  userQueryMenu();

/*
  cout << "got word: " << word << endl;
  cin >> i;
*/

  prompt = "Enter number of n-gram model to query (range = 1-5): ";
  nModel = userGetInt(prompt);

  queryStr = "NIL";
  //querying the 1-gram table finds the word with the max unconditional probability, so no query string is used
  if(nModel > 1){
    valid = false;
    while(!valid){
      queryStr.clear();
      cout << "*queries include n-1 words separated by single spaces, where 'n' is the n-gram model\nEnter query: ";
      getline(std::cin, queryStr);
      valid = isValidQuery(queryStr,nModel);
    }
  }

  prompt = "Enter number of predictions (top k n-gram strings, where k = 1-10): ";
  nPredictions = userGetInt(prompt);
}

bool nGram::isNumericStr(const string& s)
{
  for(int i = 0; i < s.length(); i++){
    if(!isdigit(s[i]) && (s[i] != '-') && (s[i] != '.')){
      return false;
    }
  }
  return true;
}


//gets an int in range 0-63, w/ modest input validation
int nGram::userGetInt(string prompt)
{
  bool valid;
  int n;
  string buf;

  valid = false;
  while(!valid){
    cout << prompt;
    cin.clear();
    getline(std::cin, buf);
    if(isNumericStr(buf)){
      n = atoi(buf.c_str());
      valid = true;
    }
    else{
      cout << "Invalid input. Retry." << endl;
    }
  }

  return n; 
}

//maps some n-gram number to a table.
FreqTablePointer nGram::getPosTablePointer(int i)
{
  FreqTablePointer tp = NULL;

  switch(i){
    case 1:
        tp = &(oneGramPosTable);
      break;
    case 2:
        tp = &(twoGramPosTable);
      break;
    case 3:
        tp = &(threeGramPosTable);
      break;
    case 4:
        tp = &(fourGramPosTable);
      break;
    case 5:
        tp = &(fiveGramPosTable);
      break;
    default:
        cout << "ERROR No table found for index: " << i << endl;
      break;
  }

  return tp;
}

//maps some n-gram number to a table.
FreqTablePointer nGram::getTablePointer(int i)
{
  FreqTablePointer tp = NULL;

  switch(i){
    case 1:
        tp = &(oneGramTable);
      break;
    case 2:
        tp = &(twoGramTable);
      break;
    case 3:
        tp = &(threeGramTable);
      break;
    case 4:
        tp = &(fourGramTable);
      break;
    case 5:
        tp = &(fiveGramTable);
      break;
    case 6:
        tp = &(sixGramTable);
      break;
    default:
        cout << "ERROR No table found for index: " << i << endl;
  }

  return tp;
}

/*
  Query a single model for top n results (where n = nPredictions)
  If nModel == 1-gram, then we do not use _query_ to perform lookups, since 1-gram queries just look for most frequent word.

*/
void nGram::queryModel(const string& query, int nModel, int nPredictions, list<freqTuple>& resultList)
{
  /*
  cout << "DEBUG word/nModel/nPredictions: " << query << "/" << nModel << "/" << nPredictions << endl;
  cin >> i;
  cin.clear();
  */
  
  FreqTablePointer table = getTablePointer(nModel);
  if(table == NULL){
    cout << "ERROR nModel table not found in getMaxFromTable" << endl;
    return;
  }

  cout << "ERROR queryModel() todo" << endl;
  //queryTable(query, table, nModel, nPredictions, resultList);
  /*
  for(i = 0, it = tempList.begin(); (i < nPredictions) && (it != tempList.end()); ++it, i++){
    resultList.push_back(*it);
  }
  */
}

void nGram::queryAllModels(const string& query, int nPredictions, list<freqTuple>& resultList)
{
  list<freqTuple> tempList;
  list<freqTuple>::iterator it;

  resultList.clear();
  for(int i = 1; i <= 5; i++){ //query up to the 5-gram model
    queryModel(query, i, nPredictions, tempList);
    for(it = tempList.begin(); it != tempList.end(); ++it){
      resultList.push_back(*it);
    }
    tempList.clear();
  }
}

void nGram::userMainMenu(void)
{
  cout << "Example query:\n  3-gram query: \"the cats\"   numpredictions: 3\n\n  Results:\n    1) the cats pajamas<15>\n    2) the cats meow<7>\n    3) the cats KC<5>" << endl
       << "\nUser Menu:\n  1) Query word by n-gram model (1-5) and numpredictions (1-5)\n"
       << "  2) Query word in all models (all 1-5 ngram models) by numpredictions\n  3) Seed (awesome)\n  4) quit\n\n" << endl;
}

void nGram::printFreqTuple(const string& prefix, freqTuple& tup)
{
    cout << prefix + " " + tup.nextWord << "|" << tup.freq << endl;
}

void nGram::printGramList(const string& prefix, list<freqTuple>& gramList)
{
  list<freqTuple>::iterator it;
  int i;

  for(i = 0, it = gramList.begin(); it != gramList.end(); ++it, i++){
    cout << i << ": ";
    printFreqTuple(prefix, *it);
  }
}

void nGram::userDriver(void)
{
  bool quit;
  int nModel, nSelection, nPredictions;
  string prompt, query;
  string inputFile = "../trainingText.txt";
  string parsedFile = "../parsedCorpus.txt";
  list<freqTuple> resultList;

  //buildModels("../statsTestText.txt",5);
  
  quit = false;
  while(quit == false){

    userMainMenu();
    prompt = "Enter selection: \0";
    nSelection = userGetInt(prompt);

    switch(nSelection){

      case 1:
          getQueryParameters(query,nModel,nPredictions);
          queryModel(query, nModel, nPredictions, resultList);
          cout << "Results:" << endl;
          printGramList(query, resultList);
       break;

      case 2:
          getQueryParameters(query,nPredictions);
          queryAllModels(query, nPredictions, resultList); //higher level driver than queryTable
          printGramList(query, resultList); 
       break;

      case 3:
         induceLanguage();
       break;
      
      case 4:
          cout << "goodbye!" << endl;
          quit = true;
          break;
      
      default:
          cout << "Selection >" << nSelection << "< not recognized" << endl;
        break;
    }
 
    resultList.clear();
  }
}


/*
  This does hardly any validation except to verify there are only four words in the seed string.
*/
bool nGram::isValidSeed(const string& seed)
{
  int nwords;
  char buf[128];
  char* toks[32];
  KeyIt it;
  FreqTablePointer tablePtr;
  
  if(seed.length() == 0){
    cout << "Cannot seed empty string, retry" << endl;
    return false;
  }
  
  strncpy(buf,seed.c_str(),128);
  nwords = tokenize(toks,buf," ");
  if(nwords != 4){
    cout << "Invalid seed. See must include 4 words only, separated by spaces. Retry" << endl;
    return false;
  }
  
  return true;
}

/*
  For fun.
*/
void nGram::induceLanguage(void)
{
  string seed;
  bool isValid;
  int nModel;


  cout << "This demonstrates generating language from our language models based on a seed string (or none).\n"
          << "A seed-string will be used as a starting point for finding the most probable path through the n-gram graph,\n"
          << "up to some number of predictions (or until we are trapped by a zero probability). Have fun!" << endl;

  nModel = 0;
  while(nModel == 0){
  nModel = userGetInt("Enter n-gram model (2-5) or 0 to quit:");

    //get the seed from the user  
    isValid = false;
    while(isValid == false){
      cout << "Enter a seed string of 4 words, or NIL to allow the process to seed itself based on the most probable strings: " << endl;
      getline(std::cin, seed);
      isValid = isValidSeed(seed);
    }

    /*write recursive search procedure: look in highest gram model first, then n-1 model, n-2 model, and so on. select path based on statistically best path (also with fewest dead-ends)
    not this is very much a graph search procedure. We could search through this space using a heuristic or other lookahead methods to check our coherence with various factors (grammars, deadends, etc)
    for instance, could weigh a path based not only on its likelihood wrt other models (measured by a combo of entropy and probability), but also based on the sort of "promise" of the path, such as the
    branching factor ahead (which is likely a good thing) and the value of those paths themselves.... this is starting to sound a lot like Bayesian state transition models.
    
    if(nModel >= 2 && nModel <= 5){
      generateFromTrainingModel(seed, nModel); // out of date!
    }
    */
  }
}

void nGram::testPredictWord_RealStaticLinear(void)
{
  int i;
  string t5 = "the car to a";
  string t4 = "car to a";
  string t3 = "to a";
  string t2 = "a";
  list<pair<SubkeyIt,long double> > results;
  list<pair<SubkeyIt,long double> >::iterator it;

  initLambdas();

  cout << "\nin testPredictWord_RealStaticLinear(void) testing lambdaset 0" << endl;
  predictWord_RealStaticLinear(t5,0,results);
  cout << "result count: " << results.size() << endl;
  for(i = 0, it = results.begin(); i < 100 && it != results.end(); i++, ++it){
    cout << t5 << "|" << it->first->first << "|" << it->second << endl;
  }
  cout << "above results of query for lambdaset 0>" << t5 << "<" << endl;
  cin >> i;
  
  results.clear();
  cout << "\nin testPredictWord_RealStaticLinear(void) testing lambdaset 1" << endl;
  predictWord_RealStaticLinear(t5,1,results);
  cout << "result count: " << results.size() << endl;
  for(i = 0, it = results.begin(); i < 100 && it != results.end(); i++, ++it){
    cout << t5 << "->" << it->first->first << "|" << it->second << endl;
  }
  cout << "above results of query for lambdaset 1>" << t5 << "<" << endl;
  cin >> i;

  results.clear();
  cout << "\nin testPredictWord_RealStaticLinear(void) testing lambdaset 2" << endl;
  predictWord_RealStaticLinear(t5,2,results);
  cout << "result count: " << results.size() << endl;
  for(i = 0, it = results.begin(); i < 100 && it != results.end(); i++, ++it){
    cout << t5 << "->" << it->first->first << "|" << it->second << endl;
  }
  cout << "above results of query for lambdaset 2>" << t5 << "<" << endl;
  cin >> i;
  
  results.clear();
  cout << "\nin testPredictWord_RealStaticLinear(void) testing lambdaset 3" << endl;
  predictWord_RealStaticLinear(t5,3,results);
  cout << "result count: " << results.size() << endl;
  for(i = 0, it = results.begin(); i < 100 && it != results.end(); i++, ++it){
    cout << t5 << "->" << it->first->first << "|" << it->second << endl;
  }  
  cout << "above results of query for lambdaset 3>" << t5 << "<" << endl;
  cin >> i;
  
  results.clear();

/*  TODO ?? test these asap
  predictWord_RealStaticLinear(t4,0,results);
  cout << "result count: " << results.size() << endl;
  for(it = results.begin(); it != results.end(); ++it){
    cout << it->first->first << " " << it->first->second.nextWord << "|" << it->second << endl;
  }

  predictWord_RealStaticLinear(t3,0,results);
  cout << "result count: " << results.size() << endl;
  for(it = results.begin(); it != results.end(); ++it){
    cout << it->first->first << " " << it->first->second.nextWord << "|" << it->second << endl;
  }
*/


}

void nGram::testPredictWord_RealDynamicLinear(void)
{
  int i;
  string t5 = "the car to a";
  string t4 = "car to a";
  string t3 = "to a";
  string t2 = "a";
  list< pair<SubkeyIt,long double> > results;
  list< pair<SubkeyIt,long double> >::iterator it;

  cout << "in testPredictWord_RealDynamicLinear(void)" << endl;
  predictWord_RealDynamicLinear(t5,results);
  cout << "result count: " << results.size() << endl;
  cin >> i;
  for(i = 0, it = results.begin(); i < 100 && it != results.end(); i++, ++it){
    cout << it->first->first << " " << it->first->second << "|" << it->second << endl;
  }
  results.clear();
  
/*  test these asap
  predictWord_RealStaticLinear(t4,0,results);
  cout << "result count: " << results.size() << endl;
  for(it = results.begin(); it != results.end(); ++it){
    cout << it->first->first << " " << it->first->second.nextWord << "|" << it->second << endl;
  }

  predictWord_RealStaticLinear(t3,0,results);
  cout << "result count: " << results.size() << endl;
  for(it = results.begin(); it != results.end(); ++it){
    cout << it->first->first << " " << it->first->second.nextWord << "|" << it->second << endl;
  }
*/


}


/*
  Description: Given a string of previous words <n-i...n-3,n-2,n-1>, predict the most likely next word
  based on our statistical analysis of each model. Each n-model provides a "most likely" next word prediction
  given some string of n-1 grams. However, due to the sparsity of our training data, we must have some way to
  weight the predictions of each model accordingly.  For instance, sub-entropy decreases as we increase n, meaning
  our n-model predictions become more certain for greater n. However, these models also become more sparse as
  n increases, meaning this "certainty" is informed by a smaller proportion of the data.  Thus, we must weight the
  predictions of each model according to factors such as total entropy, sub-entropy, and similar factors.

  This is our "smart" linear-interpolation method. It might be better to have an outer driver predictWord() that takes some
  prediction method as a parameter, since we will want to test many different methods.
  
  This is a search problem. Is the search space the set of all 5-gram results for some query, or the set of all results given by each
  subquery within some query?  Also, it would be best to implement a backoff method, since there will invitably be 5-gram queries
  with zero/few results. Since our ngram models 1-5 (induced or COCA) all come from the same source, this means that every
  n+1 model query will have at least one result in an n model, since that substring was stored there as well. However, a 5-gram
  query may have a miss on some test data, so we need to backoff to lower-n models until we get a hit (recursion).
  
  TODO: This is a very high complexity method. Find ways to optimize it.
  There are many algorithmic and design questions about this function. Should the query be pos-tagged? Should the models
  themselves include tagged keys instead of splitting the word model from the pos model? There's also a ton of combinatoric 
  complexity: given some query, there are many pos-parses (about exp(30, query_length) where 30 is the number of possible pos tags).
  Yet another question is whether we should fit words to pos-tags, or fit pos-tags to words. For instance, should we generate
  a set of possible strings based on words, and then re-sort by some pos-probability parameter, or generate a set of pos-tag
  parses (ranked) and then re-sort these by mapping them to possible word paths? This is a classic problem of statistical model merging.
  see Schutze.
  
  One thing is that with the COCA models, it is generally, though not always, true that some 4-gram model string is a substring
  in some 5-gram entry. It is also true that some pos-path through a given 5-gram string will map to a specific 5-gram pos string,
  since the pos model was based off the tagged COCA word model; however, since we split our models, it is difficult to re-unify
  some given word string with its corresponding pos-string. It may be necessary to do this unification for statistical stability, or
  we end up just guessing the pos-string for a given word string, which is needlessly complex.

  Is this written to accept any query length, not just 5-gram queries?

  TODO: this functions calculates the prob of all subqueries w/in a query (if this doesn't make sense, see the output of a
  result list). This means the result list often contains duplicates. But these duplicates are a subset of previous predictions,
  since the duplicates are simply missing one more sum-probability term. Thus, the dupes can be removed, since the first (non-duplicate)
  foremost in a list contains the most information per that prediction.

*/
string nGram::predictWord_BoolStaticLinear(const string& query, int lambdaSet)
{
  bool hit;
  int ngrams, i, j;
  char buf[128];
  char* toks[16];
  long double pi[6];
  string q[6];
  MapSubset subset;
  //unordered_set<string> SeenWords;
  SubkeyIt it;
  FreqTablePointer wordTable[6]; //yes, there are only up to 5-gram tables. But indices start at 0, so alloc'ing 6 pointers allows us to index by nModel: eg, to index to 2-gram model, use table[2]->foo()
  FreqTablePointer posTable[6];
  //the result set for all possible subqueries in the query: the cat had, the cat was, the cat, the dog, the goldfish, the
  pair<SubkeyIt,long double> maxPair;

  if(query.length() > 128){
    cout << "ERROR query string too long in predictWord, returning <UNK>, query discarded: " << query << endl;
    return "<UNK>";
  }

  //split the query string by spaces
  strncpy(buf,query.c_str(),query.length());
  buf[ query.length() ] = '\0';
  //get the n-grams in the prior (w_0, w_1, w_2, ... w_i-1)
  ngrams = tokenize(toks,buf," ");
  if(ngrams > 4){
    cout << "More than 4 grams found in query. ngrams reset to 4 (5-gram max lookup) in predictWord()" << endl;
    ngrams = 4;
  }

  //get the wordTable ptrs
  for(i = 1, wordTable[0] = NULL; i <= ngrams+1; i++){
    wordTable[i] = getTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(wordTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning <UNK>" << endl;
      return "<UNK>";
    }
  }
  //get the posTable ptrs
  for(i = 1, posTable[0] = NULL; i <= ngrams+1; i++){
    posTable[i] = getPosTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(posTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning <UNK>" << endl;
      return "<UNK>";
    }
  }

  //build the queries. q1 is the nextWord given by each item in some subset. q5 is just the 4-gram query parameter as passed to this function
  q[2] = toks[ngrams - 1];
  q[3] = toks[ngrams - 2]; q[3] += " "; q[3] += q[2];
  q[4] = toks[ngrams - 3]; q[4] += " "; q[4] += q[3];
  q[5] = query;
  //cout << "DEBUG q2 q3 q4 q5 => " << q[2] << "," << q[3] << "," << q[4] << "," << q[5] << endl;

  //Most basic argmax/boolean version of this function
  //get each subset.
  //iterate over each subset, appending to list ordered by probability
  //1) return most likely word (running max, so no need for list) 2) adjusted likelihood of item in list
  for(hit = false, maxPair.second = -9999, i = 2; i <= 5; i++){

    subset = equalRange(wordTable[i], q[i]);
    if(subset.first != subset.second){ //flag if there is a hit. we need this to verify we can access iterator in return stmt
      hit = true;
    }

    //for each possible next word from the n-gram model, calculate the linear interpolation of probabilities across that string
    for(it = subset.first; it != subset.second; ++it){

      //filter duplicate word analyses. The overhead of this datastructure is offset by skipping duplicate calculations
      //if( SeenWords.find(it->first) == SeenWords.end() ){ //word not in set (unordered_set.find() is O(1) in average case)
      //  SeenWords.insert(it->first);
 
        //get and store each probability in prob vector
        //pi[1] = getOneGramProbability(it->first,wordTable[1]); //obsolete function
        for(j = 1; j <= i; j++){
          pi[j] = getConditionalProbability(q[j], it->first, wordTable[j]);
        }

        //get the interpolated probability of this prediction in subset. static lambdas.
        for(pi[0] = 0.0, j = 1; j <= i; j++){
          pi[0] += modelLambdas[lambdaSet].l[j] * pi[j];
        }

        //store the max: string argmax(q,lambdas) version of word prediction
        if(maxPair.second < pi[0]){
          maxPair.second = pi[0];
          maxPair.first = it;
        }
      //}
    }
  }

  if(!hit){
    return "<UNK>";
  }
  
  //cout << "prediction: " << maxPair.first->first << endl;

  return maxPair.first->first;
}

/*
  Version: stores the results in a list. This way we can query for if the actual next word
  is even in our result set, and give it some weighted score, despite not always being the
  predictedNextWord / not being the most likely next word in the subset.

  return: i dunno, return nresults... which is a property of the list anyway.

  Notes: I decided not to clear the input list. It is the caller's responsibility to clear() the list. This
  may allow us to generate accumulating lists, which may be useful.

  After building a result list, we sort it, then call unique() to filter out duplicates. The duplicates occur
  since we query each model multiple times for subsets of the same query (is there a way to check for this
  when building list results? a mathematical way to get only unique coverage to begin with?)

*/
int nGram::predictWord_RealStaticLinear(const string& query, int lambdaSet, list<pair<SubkeyIt,long double> >& resultList)
{
  int ngrams, i, j, nres;
  char buf[128];
  char* toks[16];
  long double pi[6];  // this is a vector of conditional probabilities. Not "Pi" but rather "P of Xi" given some evidence
  string q[6];
  //unordered_set<string> SeenWords;
  MapSubset subset;
  SubkeyIt it;
  pair<SubkeyIt,long double> listPair;
  FreqTablePointer wordTable[6]; //Ya, there are only up to 5-gram tables. But indices start at 0, so alloc'ing 6 pointers allows us to index by nModel/model number: eg, to index to 2-gram model, use table[2]->foo()
  //FreqTablePointer posTable[6];
  //the result set for all possible subqueries in the query: the cat had, the cat was, the cat, the dog, the goldfish, the

  if(query.length() > 128){
    cout << "ERROR query string too long in predictWord, returning 0 results, query discarded: " << query << endl;
    return 0;
  }

  //TODO We pass and tokenize queries redundantly. Currently due to the dualing use-cases of research and the user-application.
  strncpy(buf,query.c_str(),query.length());
  buf[ query.length() ] = '\0';
  //get the n-grams in the prior (w_0, w_1, w_2, ... w_i-1)
  ngrams = tokenize(toks,buf," ");
  if(ngrams > 4){
    cout << "More than 4 grams found in query. ngrams reset to 4 (5-gram max lookup) in predictWord()" << endl;
    ngrams = 4;
  }

  //get the wordTable ptrs
  for(i = 1, wordTable[0] = NULL; i <= ngrams+1; i++){
    wordTable[i] = getTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(wordTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning 0 results" << endl;
      return 0;
    }
  }
  /*get the posTable ptrs
  for(i = 1, posTable[0] = NULL; i <= ngrams+1; i++){
    posTable[i] = getPosTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(posTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning <UNK>" << endl;
      return "<UNK>";
    }
  }
  */

  //build the queries. q1 is the nextWord given by each item in some subset. q5 is just the 4-gram query parameter as passed to this function
  q[2] = toks[ngrams - 1];
  q[3] = toks[ngrams - 2]; q[3] += " "; q[3] += q[2];
  q[4] = toks[ngrams - 3]; q[4] += " "; q[4] += q[3];
  q[5] = query;
//  cout << "DEBUG q2 q3 q4 q5 => " << q[2] << "," << q[3] << "," << q[4] << "," << q[5] << endl;
//  cout << "lambdas set in predictWord_realStaticLinear: " << modelLambdas[lambdaSet].l[1] << "," << modelLambdas[lambdaSet].l[2] << ","
//       << modelLambdas[lambdaSet].l[3] << "," << modelLambdas[lambdaSet].l[4] << "," << modelLambdas[lambdaSet].l[5] << endl;

  //Real version: build result list from each subquery in query, then sort and return.
  for(i = 2, nres = 0; i <= 5; i++){

    //DEBUG
    //cout << "  testing q[" << i << "] >" << q[i] << "< of wordtable[" << i << "]" << endl;
    subset = equalRange(wordTable[i],q[i]);
    /* //DEBUG
    if(subset.first == subset.second){
        cout << ">>> DEBUG warning: query >" << q[i] << "< not in model[" << i << "]" << endl;
    }
    */

    /*
    if(subset.first == subset.second){
      cout << "  -> query not in table" << endl;
    }
    */

    //for each possible next word from the n-gram model, calculate the linear interpolation of probabilities across that string
    for(it = subset.first; it != subset.second; ++it){

      //filter duplicate word analyses. The overhead of this datastructure is offset by skipping duplicate calculations
      //if( SeenWords.find(it->first) == SeenWords.end() ){ //unordered_set.find() is O(1) in average case
      //  SeenWords.insert(it->first);

        for(j = 1; j <= i; j++){
          pi[j] = getConditionalProbability(q[j], it->first, wordTable[j]);
          //cout << "p(" << it->second.nextWord << "|" << q[j] << ") = " << pi[j] << endl;
        }

        //get the interpolated probability of this prediction in subset. static lambdas.
        for(pi[0] = 0.0, j = 1; j <= i; j++){
          if(pi[j] != 0.0){ //tiny optimization so we don't do calcs for zero-probability events
            pi[0] += modelLambdas[lambdaSet].l[j] * pi[j];
          }
        }
        //cout << "resultant prob sum: p[0] = " << pi[0] << endl;
        listPair.second = pi[0];
        listPair.first = it;
        resultList.push_front( listPair );  //insert in no particular order, then sort. more efficient than ordered insertion cost.
        nres++;
      }
    //}
  }

  //hopefully some of this can be eliminated: filter duplicates, then sort by score
  if(resultList.size() > 0){
    /*filter non-unique results by sorting on nextword, then removing non-unique adjacent nextwords in list
    resultList.sort( byNextWord );
    resultList.unique(isUniqueResultPair); //filter duplicates from the sorted list (see function header)
    */

    //re-sort by score
    resultList.sort( byScore ); //this may be unnecessary: if we run analytics over the entire list, there is no need for order
  }

/*
  //DEBUG
  list<pair<SubkeyIt,long double> >::iterator listIt = resultList.begin();
  for(j = 0; j < 25 && listIt != resultList.end(); j++, listIt++){
    cout << listIt->first->first  << "|" << listIt->second << endl;
  }
  cout << "list size is: " << resultList.size() << "  Confirm first " << j  << " results from predict word---^ " << endl;
  cin >> j;
  cout << "prediction: query=" << query << "->" << resultList.begin()->first->first << endl;
*/

  //cout << "nres=" <
  return nres;
}

/*
  Version: stores the results in a list. This way we can query for if the actual next word
  is even in our result set, and give it some weighted score, despite not always being the
  predictedNextWord / not being the most likely next word in the subset.

  return: i dunno, return nresults... which is a property of list anyway.

  POS information not being leveraged in any way right now. POS info should be built into the conditional probs themselves anyway... ?
  See Carlberger 1999 for analysis.

  This attempts to predict the dynamically-generated interpolation of a string based on the subentropy
  value of each generated set. Effectively our lambdas are 1/subentropy(q[i]). But notice that since the
  q[i] values are fixed, these values are fixed and do not change! It is like multiplying each prediction
  by a more or less arbitrary constant, albeit a slightly contextual one. So I need to evaluate if this is
  even valid, or what its implications are. As an aside, this sort of analysis could be very useful when looking
  beyond the next predicted word, to assigned value to that predicted word based on the subsequent states
  to which it gives rise. For instance, if "he went to the" is our query, and "park" is one possible next word
  prediction, then query "went to the park" to discover the value of states beyond "park" and use these to also score
  the value of the prediction "park".
*/
int nGram::predictWord_RealDynamicLinear(const string& query, list<pair<SubkeyIt,long double> >& resultList)
{
  int ngrams, i, j, nres;
  char buf[128];
  char* toks[16];
  long double pi[6], subsetEntropy[6];
  string q[6];
  //unordered_set<string> SeenWords;
  MapSubset subset;
  SubkeyIt it;
  KeyIt kit;
  FreqTablePointer wordTable[6]; //Ya, there are only up to 5-gram tables. But indices start at 0, so alloc'ing 6 pointers allows us to index by nModel: eg, to index to 2-gram model, use table[2]->foo()
  //FreqTablePointer posTable[6];  //also, these should be class members init'ed at start--iff we verify the pointer won't change!!
  //the result set for all possible subqueries in the query: the cat had, the cat was, the cat, the dog, the goldfish, the
  pair<SubkeyIt,long double> listPair;

  if(query.length() > 128){
    cout << "ERROR query string too long in predictWord, returning 0 results, query discarded: " << query << endl;
    return 0;
  }

  // TODO THIS WHOLE SECTION IS BAD. Most of it should be factored out (like copying the buffer, building string queries, etc) once we settle on a data model.
  //split the query string by spaces
  strncpy(buf,query.c_str(),query.length());
  buf[ query.length() ] = '\0';
  //get the n-grams in the prior (w_0, w_1, w_2, ... w_i-1)
  ngrams = tokenize(toks,buf," ");
  if(ngrams > 4){
    cout << "More than 4 grams found in query. ngrams reset to 4 (5-gram max lookup) in predictWord()" << endl;
    ngrams = 4;
  }

  //get the wordTable ptrs
  for(i = 1, wordTable[0] = NULL; i <= ngrams+1; i++){
    wordTable[i] = getTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(wordTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning 0 results" << endl;
      return 0;
    }
  }
  /*get the posTable ptrs
  for(i = 1, posTable[0] = NULL; i <= ngrams+1; i++){
    posTable[i] = getPosTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(posTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning <UNK>" << endl;
      return "<UNK>";
    }
  }
  */

  //build the queries. q1 is the nextWord given by each item in some subset. q5 is just the 4-gram query parameter as passed to this function
  q[2] = toks[ngrams - 1];
  q[3] = toks[ngrams - 2]; q[3] += " "; q[3] += q[2];
  q[4] = toks[ngrams - 3]; q[4] += " "; q[4] += q[3];
  q[5] = query;
  //  cout << "DEBUG q2 q3 q4 q5 => " << q[2] << "," << q[3] << "," << q[4] << "," << q[5] << endl;
  //// END BAD SECTION ///////////////////////////////////////////////////////

//Real version with dynamic lambdas: build result list, then find next word in list and give it a weighted success score. Good for performance measurements.
  //get each subset.
//PROBABLY dont need to normalize lambdas (dynamic or static), but I need to think about this some more.
  //iterate over each subset, appending to list ordered by probability
  //1) return most likely word in list (running max, so no need for list) 2) adjusted likelihood of item in list
  subsetEntropy[1] = wordModelStats[1].totalEntropy; //1-gram model entropy is simply that of the entire model
  //for this member of this subset, get the subentropy 
  //be mindful that subentropy values are fixed for a given query, only change "dynamically" for new queries
  //this could also be precomputed node metadata in some tree/trie (where the parent contains entropy info for its children), instead of being recomputed for each query
  for(i = 2; i <= 5; i++){
    subsetEntropy[i] = 0.0;
    kit = wordTable[i]->find(q[i]);
    if(kit != wordTable[i]->end()){
      subsetEntropy[i] = -1.0 * getSubEntropy(kit,this->normalized);
      if(subsetEntropy[i] < 0.0){
        subsetEntropy[i] = 0.0;
      }
    }
    //cout << "subsetEntropy[" << i << "]= "<< subsetEntropy[i] << endl;
    //cin >> ngrams;
  }
//  cout << "subentropy vals for these queries (1-5): " << subsetEntropy[1] << "," << subsetEntropy[2] << "," << subsetEntropy[3] << "," << subsetEntropy[4] << "," << subsetEntropy[5] << endl;
//  cin >> i;

  //iterates over the subsets, summing the linear probability for each. notice this gives us duplicates in our result list.
  // for instance: go to the park today is summed over the 5-1 gram queries it contains, but we then do the same for "to the park today".
  // the list is sorted after being built, and experiment shows these repeat queries are closely clustered, with the 5-gram prevailing.
  // this captures results not in the 5-gram query, but remember, there are dupes in the resultList!
  for(i = 2, nres = 0; i <= 5; i++){

    //cout << "getting subset..." << endl;
    subset = equalRange(wordTable[i], q[i]);
    /*
    if(subset.first == subset.second){
      cout << "  DEBUG warning: query >" << q[i] << "< not in model[" << i << "]" << endl;
    }
    */
    //cout << "got subset..." << endl;

    //for each possible next word from the n-gram models, calculate the linear interpolation of probabilities across/within that string
    for(it = subset.first; it != subset.second; ++it){
      //filter duplicate word analyses. The overhead of this datastructure is offset by skipping duplicate calculations
      //if( SeenWords.find(it->first) == SeenWords.end() ){ //unordered_set.find() is O(1) in average case
      //  SeenWords.insert(it->first);

        for(j = 1; j <= i; j++){
          pi[j] = getConditionalProbability(q[j], it->first, wordTable[j]);
          //cout << "p(" << it->second.nextWord << "|" << q[j] << ") = " << pi[j] << endl;
          //cout << "getting cond-prob..." << endl;
        }
        //get the interpolated probability of this prediction in subset using dynamic lambdas
        for(pi[0] = 0.0, j = 1; j <= i; j++){
          if(subsetEntropy[j] <= 0){
            cout << "subsetEntropy[" << j << "] =" << subsetEntropy[j] << " <= 0  in predictWord_RealDynamicLinear() for query=" << q[j] << endl;
          }
          else if(pi[j] > 0.0){  //if stmt here is just a small optimization. aborts computation of zero-conditional-probability events
            pi[0] += (pi[j] / subsetEntropy[j]);  //heck, don't even use the static lambdas for now. Let the dynamic lambdas dominate.
          }
          //cout << "interp..." << endl;
        }
        //cout << "resultant prob sum: p[0] = " << pi[0] << endl;
         
        listPair.second = pi[0];
        listPair.first = it;
        resultList.push_front( listPair );
        nres++;
      //}
    }
  }

  //hopefully some of this can be eliminated
  if(resultList.size() > 0){
    /*
    //filter by sorting on nextword, then calling unique to get rid of non-unique adjacent next words in result list
    resultList.sort( byNextWord );
    resultList.unique(isUniqueResultPair); //filter duplicates from the sorted list (see function header)
    */
    //sort descending by score, for output
    resultList.sort( byScore ); //this may be unnecessary: if we run analytics over the entire list, there is no need for order
  }

/* //DEBUG
  cout << "list size is: " << resultList.size() << "  Confirm first 30 results from predict word: " << endl;
  j = 0;
  for(list<pair<KeyIt,long double> >::iterator listit = resultList.begin(); listit != resultList.end() && j < 30; ++listit, j++){
    cout << listit->first->first << "|" << listit->first->second.nextWord  << "|" << listit->second << endl;
  }
  cin >> j;
*/
  return nres;
}

/*
  Version: stores the results in a list. This way we can query for if the actual next word
  is even in our result set, and give it some weighted score, despite not always being the
  predictedNextWord / not being the most likely next word in the subset.

  return: i dunno, return nresults... which is a property of list anyway.

  POS information not being leveraged in any way right now. POS info should be built into the conditional probs themselves anyway... ?
*/
string nGram::predictWord_BoolDynamicLinear(const string& query)
{
  int ngrams, i, j;
  char buf[128];
  char* toks[16];
  long double pi[6], subsetEntropy[6];
  string q[6];
  MapSubset subset;
  //unordered_set<string> SeenWords;
  pair<SubkeyIt,long double> maxPair;
  KeyIt kit;
  SubkeyIt it;
  FreqTablePointer wordTable[6]; //Ya, there are only up to 5-gram tables. But indices start at 0, so alloc'ing 6 pointers allows us to index by nModel: eg, to index to 2-gram model, use table[2]->foo()
  //FreqTablePointer posTable[6];
  //the result set for all possible subqueries in the query: the cat had, the cat was, the cat, the dog, the goldfish, the

  if(query.length() > 128){
    cout << "ERROR query string too long in predictWord, returning <UNK>, query discarded: " << query << endl;
    string s = "<UNK>";
    return s;
  }

  // TODO THIS WHOLE SECTION IS BAD. Most of it should be factored out (redundantly copying the buffer, building string queries, etc) once we settle on a data model.
  //split the query string by spaces
  strncpy(buf,query.c_str(),query.length());
  buf[ query.length() ] = '\0';
  //get the n-grams in the prior (w_0, w_1, w_2, ... w_i-1)
  ngrams = tokenize(toks,buf," ");
  if(ngrams > 4){
    cout << "More than 4 grams found in query. ngrams reset to 4 (5-gram max lookup) in predictWord()" << endl;
    ngrams = 4;
  }

  //get the wordTable ptrs
  for(i = 1, wordTable[0] = NULL; i <= ngrams+1; i++){
    wordTable[i] = getTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(wordTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning <UNK>" << endl;
      string s = "<UNK>";
      return s;
    }
  }
  /*get the posTable ptrs
  for(i = 1, posTable[0] = NULL; i <= ngrams+1; i++){
    posTable[i] = getPosTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(posTable[i] == NULL){
      cout << "wordTable ptr NULL in predictWord, returning <UNK>" << endl;
      return "<UNK>";
    }
  }
  */

  //build the queries. q1 is the nextWord given by each item in some subset. q5 is just the 4-gram query parameter as passed to this function
  q[2] = toks[ngrams - 1];
  q[3] = toks[ngrams - 2]; q[3] += " "; q[3] += q[2];
  q[4] = toks[ngrams - 3]; q[4] += " "; q[4] += q[3];
  q[5] = query;
  //  cout << "DEBUG q2 q3 q4 q5 => " << q[2] << "," << q[3] << "," << q[4] << "," << q[5] << endl;
  //// END BAD SECTION ///////////////////////////////////////////////////////
  //cout << "here" << endl;
  subsetEntropy[1] = wordModelStats[1].totalEntropy; //1-gram model entropy is simply that of the entire model
  for(i = 2; i <= 5; i++){
    subsetEntropy[i] = 0.0;
    kit = wordTable[i]->find(q[i]);
    if(kit != wordTable[i]->end()){
      subsetEntropy[i] = -1.0 * getSubEntropy(kit,this->normalized);
      if(subsetEntropy[i] < 0.0){
        subsetEntropy[i] = 0.0;
      }
    }
    //cout << "subsetEntropy[" << i << "]= "<< subsetEntropy[i] << endl;
    //cin >> ngrams;
  }

//Real version with dynamic lambdas: build result list, then find next word in list and give it a weighted success score. Good for performance measurements.
  //get each subset.
//PROBABLY dont need to normalize lambdas (dynamic or static), but I need to think about this some more.
  //iterate over each subset, appending to list ordered by probability
  //1) return most likely word in list (running max, so no need for list) 2) adjusted likelihood of item in list
  for(maxPair.second = -9999, i = 2; i <= 5; i++){

    subset = equalRange(wordTable[i], q[i]);
    //for each possible next word from the n-gram models, calculate the linear interpolation of probabilities across/within that string
    for(it = subset.first; it != subset.second; ++it){

      //filter duplicate word analyses. The overhead of this datastructure is offset by skipping duplicate calculations
      //if( SeenWords.find(it->first) == SeenWords.end() ){ //unordered_set.find() is O(1) in average case
      //  SeenWords.insert(it->first);

        for(j = 1; j <= i; j++){
          pi[j] = getConditionalProbability(q[j], it->first, wordTable[j]);
        }

        //get the interpolated probability of this prediction in subset using dynamic lambdas
        for(pi[0] = 0.0, j = 1; j <= i; j++){
          pi[0] += subsetEntropy[j] * pi[j] ;  //hell, don't even use the static lambdas for now. Let the dynamic lambdas dominate.
        }

        //cout << "pi[0]=" << pi[0] << " max=" << maxPair.second << endl;
        //store the max: string argmax(q,lambdas) version of word prediction
        if(maxPair.second < pi[0]){
          maxPair.second = pi[0];
          maxPair.first = it;
          //cout << "max: " << maxPair.first->second.nextWord << "|" << maxPair.second << endl;
        }
      //}
    }
  }

  if(maxPair.second <= 0){
    cout << "no results found for query" << endl;
    string s = "<UNK>";
    return s;
  }

  return maxPair.first->first;
}

/*
  Simulates multimap.equal_range() for getting a range of subkeys in a table, by key

  Note the use of begin() and end() here.
*/
MapSubset nGram::equalRange(FreqTablePointer table, const string& q)
{
  MapSubset ret;
  KeyIt begin = table->find(q);

  //item found
  if(begin != table->end()){
    ret.first  = begin->second.begin();
    ret.second = begin->second.end();
  }
  //item not found in table, so return two equal values as range
  else{
    ret.second = ret.first = table->begin()->second.end();
  }

  return ret;
}


/* 

  small experiment to see if map< sKey,map<sSubkey,int> > outperforms the multimap model

  Output: 
    Processing COCA 2-gram file to map<string,map<string,U64> >: ../../../corpii/coca_ngrams/w2c.txt
    ...  100% complete  cur key: your          map<string,map<string,U64> > build time (s): 21.553
    testMap item count: 403636  ~size=10494536
    testMap.size()=53396 keys
    unique keys: 53396
    q=the zucchini 2-gram lookup time (s): 2.4359e-05
      conditional freq=111
    q=the abalone 2-gram lookup time (s): 1.2224e-05
      conditional freq=39
    q=the middle 2-gram lookup time (s): 8.967e-06
      conditional freq=34798
    subEntropy calculation time (s): 0.707467
      subEntropy=nan

  Results: using nested map<map> instead of multimaps gives insanely fast lookups!
    The 2gram model lookup time, per above, is 2.009e-05 seconds, whereas
    the multimap gave 0.00532439 seconds lookup time, nearly 265 times faster!!
    But note that the 5-gram multimap model also gives fast lookup times, around 1.0e-05.
    The 5-gram model is far more sparse (fewer subkeys per key)


  IT appears there is something wrong with the subsets of multimaps, perhaps they
  are doing many tree


*/
void nGram::testMapMapQueryTimes(void)
{
  //see if this outperforms multimap<string, freqTuple> lookup times. may do better, iff subkey is hashed.
  map<string,map<string,U64> > testMap;
  struct timespec start, end;

  int ifile, ntoks, dummy, nModel = 2;
  struct stat fstats;
  long double fpos, fsize;
  char buf[MAX_LINE_LEN];
  char* toks[32];
  MapPair wordTablePair, posTablePair;
  KeyIt hintIt;
  map<string,bool> keys;

  ifile = open(cocaNgramFiles[nModel].c_str(), O_RDONLY);
  if(ifile < 0){
    cout << "ERROR file failed to open in cocaFileToTable(). retval: " << ifile << endl;
    return;
  }

  //Stat the file to get its size. We'll only use this to keep track of progress, since these are very large files.
  fpos = 0.0; fsize = 0.0; dummy = 0;
  fstat(ifile, &fstats);
  fsize = (long double)fstats.st_size;

  cout << "-------------begin testMapMap-------------------" << endl;
  clock_gettime(CLOCK_MONOTONIC,&start);

  cout << "Processing COCA " << nModel << "-gram file to map<string,map<string,U64> >: " << cocaNgramFiles[nModel].c_str() <<  endl;
  //mondo slow, but mostly due to size. could be sped up by reading chunks of file instead of lines. getline is likely very inefficient due to kernel calls
  while(getLine(ifile,buf,MAX_LINE_LEN) != NULL){
    //TODO: these all need error checks based on their return vals; right now I'm assuming validity
    //cout << "unbuffered: " << buf << endl;
    filterCocaLine(buf);
    ntoks = tokenize(toks,buf,"\t");
    if(ntoks != (nModel * 2 + 1) ){
      cout << "ERROR Read incorrect num toks from COCA file in buildCocaPosTable. ntoks=" << ntoks << endl;
    }

    //build and insert word pair into word table
    wordTablePair.first = buildKey(toks,nModel); //cur = buffer;
    buildFreqTuple(&(wordTablePair.second),toks,nModel);

    //a lazy way to count unique keys
    keys[wordTablePair.first] = 1;

    /*
    if(!strncmp(wordTablePair.second.nextWord,"zygote",8)){
      cout << "HIT" << endl;
    }*/

    // key not in key table
    if( testMap.find(wordTablePair.first) == testMap.end()){
      testMap[wordTablePair.first][wordTablePair.second.nextWord] = wordTablePair.second.freq;
    }
    // subkey not in subkey table
    else if( testMap[wordTablePair.first].find( wordTablePair.second.nextWord ) == testMap[wordTablePair.first].end()){
      testMap[wordTablePair.first][wordTablePair.second.nextWord] = wordTablePair.second.freq;
    }
    // both already in table, so increment current count
    else{
      testMap[wordTablePair.first][wordTablePair.second.nextWord] += wordTablePair.second.freq;
    }

    //ui output to track progress, like a progress bar
    dummy++;
    if((dummy %= 3000) == 0){
      fpos = (long double)lseek(ifile,0,SEEK_CUR); //this is just to get the current file position, to find out how far we are in processing the file
      cout << "\r...  " << (int)(101 * (fpos / fsize)) << "% complete " << flush;
      //<< "-gram Word-Map size (kb): " 
      //<< ((testMap.size() * sizeof(MapPair)) >> 10)
      //<< "  POS Map size (kb): " << ((posTable->size() * sizeof(MapPair)) >> 10)  << flush;
    }
  }

  close(ifile);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "map<string,map<string,U64> > build time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  
  map<string,map<string,U64>>::iterator outer;
  map<string,U64>::iterator inner;
  U64 ct = 0, sz = 0;
  for(outer = testMap.begin(); outer != testMap.end(); ++outer){
    ct += outer->first.size();
    sz += outer->first.size() * (sizeof(string) + sizeof(U64) + 10);
/*    for(inner = outer->first.begin(); inner != outer->first.end(); ++inner){
      ct++;
    }
*/
  }
  cout << "testMap item count: " << ct << "  ~size=" << sz << endl;
  cout << "testMap.size()=" << testMap.size() << " keys" << endl;
  cout << "unique keys: " << keys.size() << endl;


  //test lookup time of a difficult lookup
  clock_gettime(CLOCK_MONOTONIC,&start);
  U64 res = testMap["the"]["zucchini"];  //a difficult query for the multimap model
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=the zucchini 2-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "  conditional freq=" << res << endl;

  //test lookup time of a simple lookup
  clock_gettime(CLOCK_MONOTONIC,&start);
  res = testMap["the"]["abalone"];  //a simple query for the multimap model
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=the abalone 2-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "  conditional freq=" << res << endl;

  //test lookup time of a simple lookup
  clock_gettime(CLOCK_MONOTONIC,&start);
  res = testMap["the"]["middle"];  //a mid-difficulty query for the multimap model
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=the middle 2-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "  conditional freq=" << res << endl;

  clock_gettime(CLOCK_MONOTONIC,&start);
  //calculate expected sub-entropy
  long double totalFreq = 0.0;
  for(outer = testMap.begin(); outer != testMap.end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      totalFreq += (long double)inner->second;
    }
  }
  long double px = 0.0, entropy = 0.0, sumEntropy = 0.0, subFreq = 0.0;
  for(outer = testMap.begin(); outer != testMap.end(); ++outer){
    for(subFreq = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      subFreq += (long double)inner->second;
    }
    for(entropy = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      px = (long double)inner->second / subFreq;
      entropy += (px * log2l(px) + (1.0 - px) * log2l(1.0 - px));
    }
    //this subset entropy times its relative probability. This is essentially an expected value function.
    sumEntropy += (entropy * subFreq);
  }
  //normalize
  sumEntropy /= totalFreq;
  sumEntropy *= -1.0;
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "subEntropy calculation time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "  subEntropy=" << sumEntropy << endl;  
  cout << "-------------end test-------------------" << endl;
  //cin >> ct;

  /*
  //dump the table
  for(outer = testMap.begin(); outer != testMap.end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      cout << outer->first << "->" << inner->first << "|" << inner->second << endl;
    }
  }
  cin >> ct;
  */


}


/* 
  test some typical worst-case query times. The worst-cases occur for very common
  sequences, usually with standard functions words/phrases, such as "the" "and" "a" and so on.

  Some previous output:
    q=the 2-gram lookup time (s): 0.00672692
      p[0]=0.000464956
    q=to the 3-gram lookup time (s): 0.000629206
      p[0]=0.00287829
    q=went to the 4-gram lookup time (s): 2.9487e-05
      p[0]=0.0422084
    q=he went to the 5-gram lookup time (s): 1.2732e-05
      p[0]=0.177419
    q=he went to the   sum subentropy lookup time (s): 0.0239414
    q=he went to the   sum subentropy lookup time (s): 0.017849
    q=he went to the   predictWord_BoolDynamicLinear runtime (s): 195.058
    q=he went to the   predictWord_RealDynamicLinear runtime (s): 197.934
    nres=39246
    q=he went to the   predictWord_BoolStaticLinear runtime (s): 197.245
    q=he went to the   predictWord_RealStaticLinear runtime (s): 194.893
    nres=39246

  Results, after moving model to map<map> instead of multimap above:
  All queries hundreds of times faster!!!
    q=the 2-gram lookup time (s): 1.6666e-05
      p[0]=0.000464956
    q=to the 3-gram lookup time (s): 1.3209e-05
      p[0]=0.00287829
    q=went to the 4-gram lookup time (s): 1.1231e-05
      p[0]=0.0422084
    q=he went to the 5-gram lookup time (s): 9.552e-06
      p[0]=0.177419
    q=he went to the   sum subentropy lookup time (s): 0.0155999
    q=he went to the   predictWord_BoolDynamicLinear runtime (s): 1.35623
    q=he went to the   predictWord_RealDynamicLinear runtime (s): 0.228637
    nres=39246
    q=he went to the   predictWord_BoolStaticLinear runtime (s): 0.160527
    q=he went to the   predictWord_RealStaticLinear runtime (s): 0.209068
    nres=39246

  *same as prior two, but using filter/sort results algorithm (terrible):
    nres=34626
    q=he went to the   predictWord_BoolStaticLinear runtime (s): 0.26187
    q=he went to the   predictWord_RealStaticLinear runtime (s): 0.492248

    q=he went to the   predictWord_BoolDynamicLinear runtime (s): 1.52233
    q=he went to the   predictWord_RealDynamicLinear runtime (s): 0.0340492
    nres=0
    q=he went to the   predictWord_BoolStaticLinear runtime (s): 0.256044
    q=he went to the   predictWord_RealStaticLinear runtime (s): 0.0182469

  *using unordered_set to filter dupes:
    q=he went to the   predictWord_BoolDynamicLinear runtime (s): 1.61074
    q=he went to the   predictWord_RealDynamicLinear runtime (s): 0.437286
    nres=34626
    q=he went to the   predictWord_BoolStaticLinear runtime (s): 0.350746
    q=he went to the   predictWord_RealStaticLinear runtime (s): 0.427427





*/
void nGram::testCocaQueryTimes(void)
{
  int i;
  struct timespec start, end;
  string q[6];
  FreqTablePointer tables[6];
  long double pi[6], subsetEntropy[6];
  list< pair<SubkeyIt,long double> > results;

  q[0] = "<UNK>";
  q[1] = "bathroom"; //next word
  q[2] = "the";
  q[3] = "to the";
  q[4] = "went to the";
  q[5] = "he went to the"; //an intentionally worst-case query: very common terms, and a "the" two-gram query

  //get the table ptrs.
  for(i = 1, tables[0] = NULL; i <= 5; i++){
    tables[i] = getTablePointer(i); //stores wordTable pointers at indices corresponding to wordTable number: 
    if(tables[i] == NULL){
      cout << "wordTable ptr NULL in testCocaQueryTimes(), returning <UNK>" << endl;
      return;
    }
  }

  //using multimaps, two-gram model is the worst-running
  clock_gettime(CLOCK_MONOTONIC,&start);
  pi[0] = (*tables[2])[q[2]][q[1]];
  //pi[0] = getConditionalProbability(q[2],q[1],tables[2]);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[2] << " 2-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  //cout << "  p[0]=" << pi[0] << endl;

  //using multimaps, two-gram model is the worst-running
  clock_gettime(CLOCK_MONOTONIC,&start);
  pi[0] = (*tables[3])[q[3]][q[1]];
  //pi[0] = getConditionalProbability(q[3],q[1],tables[3]);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[3] << " 3-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  //cout << "  p[0]=" << pi[0] << endl;

  //using multimaps, two-gram model is the worst-running
  clock_gettime(CLOCK_MONOTONIC,&start);
  pi[0] = (*tables[4])[q[4]][q[1]];
  //pi[0] = getConditionalProbability(q[4],q[1],tables[4]);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[4] << " 4-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  //cout << "  p[0]=" << pi[0] << endl;

  //using multimaps, two-gram model is the worst-running
  clock_gettime(CLOCK_MONOTONIC,&start);
  pi[0] = (*tables[5])[q[5]][q[1]];
  //pi[0] = getConditionalProbability(q[5],q[1],tables[5]);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[5] << " 5-gram lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  //cout << "  p[0]=" << pi[0] << endl;

  clock_gettime(CLOCK_MONOTONIC,&start);
  subsetEntropy[1] = wordModelStats[1].totalEntropy; //1-gram model entropy is simply that of the entire model
  for(i = 2; i <= 5; i++){
    subsetEntropy[i] = getSubEntropy(q[i], tables[i]);
    if(subsetEntropy[i] < 0.0){
      subsetEntropy[i] = 0.0;
    }
    //cout << "subsetEntropy[" << i << "]= "<< subsetEntropy[i] << endl;
    //cin >> ngrams;
  }
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[5] << "   sum subentropy lookup time (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;

  clock_gettime(CLOCK_MONOTONIC,&start);
  predictWord_BoolDynamicLinear(q[5]);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[5] << "   predictWord_BoolDynamicLinear runtime (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;

  clock_gettime(CLOCK_MONOTONIC,&start);
  predictWord_RealDynamicLinear(q[5],results);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[5] << "   predictWord_RealDynamicLinear runtime (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "nres=" << results.size() << endl;
  results.clear();

  clock_gettime(CLOCK_MONOTONIC,&start);
  predictWord_BoolStaticLinear(q[5],3);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[5] << "   predictWord_BoolStaticLinear runtime (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;

  clock_gettime(CLOCK_MONOTONIC,&start);
  predictWord_RealStaticLinear(q[5],3,results);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "q=" << q[5] << "   predictWord_RealStaticLinear runtime (s): "
  << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "nres=" << results.size() << endl;
  results.clear();

}


/*
   Given a query string and an actual next word, return the conditional probability of the next word, given query.
   For instance, let nextWord = "smelled" and query = "the cat", where query returns the following results:
     the cat was<5>
     the cat had<2>
     the cat smelled<3>

   Then the conditional probability of "smelled" given "the cat" is:  3 / (5 + 2 + 3).

  This is an inner utility, and has no knowledge of which model it is querying. 
  Assume query validation is not the responsibility of this function but of its caller.

  Returns: 0.0 if query not found or nextWord not in subset, else returns the conditional probability of nextWord given query.  

  Time-Testing output: equalRange() takes only about 1.2e-06. Getting the sum frequency takes 0.007! which is a huge time,
  since we call this function often, over lists of around 30000 predicted words, each of which has 5 subqueries by linear
  interpolation methods.
*/
long double nGram::getConditionalProbability(const string& query, const string& nextWord, FreqTablePointer table)
{
  //struct timespec start, end;
  //int freq, sumFreq;
  long double ret;
  KeyIt keyIt;
  SubkeyIt subIt;
  //MapSubset subset;

  //TODO: smooth the 0.0 prob estimates?
  //TODO: can these containment functions be gotten rid of??
  //key not found
  if((keyIt = table->find(query)) == table->end()){
    ret = 0.0;
  }
  //subkey not found
  else if((subIt = keyIt->second.find(nextWord)) == keyIt->second.end()){
    ret =  0.0;
  }
  //key and subkey found, so just return stored conditional probability
  else{
    ret = subIt->second;
  }

  return ret;








  //clock_gettime(CLOCK_MONOTONIC,&end);  
  //cout << "runtime equalRange: " << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  //cout << "end_sec " << end.tv_sec << "  end_nsec " << end.tv_nsec << "  start_sec " << start.tv_sec << "  start_nsec " << start.tv_nsec << endl;

  //clock_gettime(CLOCK_MONOTONIC,&start);
  //both find the nextWord's frequency given query, and also sum the frequencies of this subset
  //TODO: this is extremely slow, and slows down the whole process of querying probabilities in the table.
  /*
  for(freq = 0.0, sumFreq = 0.0, it = subset.first; it != subset.second; ++it){
    if(it->first == nextWord){
      freq = it->second;
    }
    sumFreq += it->second;
  }
  */
  //clock_gettime(CLOCK_MONOTONIC,&end);
  //cout << "runtime sumFreq: " << ((long double)(end.tv_sec - start.tv_sec) + ((long double)(end.tv_nsec - start.tv_nsec) / (long double)1000000000)) << endl;
  //cout << "end_sec " << end.tv_sec << "  end_nsec " << end.tv_nsec << "  start_sec " << start.tv_sec << "  start_nsec " << start.tv_nsec << endl;
  
  /*div-zero check
  if(sumFreq <= 0.0){
    cout << "ERROR math error in getConditionalProbability(). sumFreq <= 0.  sumFreq=" << sumFreq << endl;
  }
  */

  //return the frequency of the nextWord divided by the sum frequency of this subset
  //return (long double)freq / (long double)sumFreq;
}

/*
  The one gram model has a data model that is unique from the others. This function
  handles this case.
  Precond: One-gram model is built AND its stats calculated.

  OBSOLETE with map<map> model

long double nGram::getOneGramProbability(string word, FreqTablePointer table)
{
  if(table->empty()){
    cout << "WARNING getOneGramProbability() called on empty table. returning 0.0" << endl;
    return 0.0;
  }

  if(table->find(word) != table->end()){
    return (long double)(*table)[word][word] / wordModelStats[1].sumFrequency;
  }
  else{
    return 0.0;
  }
}
*/

/*static query over a model. precon: seed is valid

  seed: a string of correct query length (n minus one grams, per nModel value)

  Out of Order right now: re-write, preferably with no iterators (let utilities manage data model access, make agnostic to its)

void nGram::generateFromTrainingModel(string& seed, int nModel)
{
  char buf[64];
  char* gram[8];
  int nseeds = nModel-1, limit = 100;
  int ngrams, last, start, i, j;
  string s;
  KeyIt it;
  MapSubset subset;
  FreqTablePointer tablePtr;
  vector<string> grams; //recursive ring of gram toks

  tablePtr = getTablePointer(nModel);
  if(tablePtr == NULL){
    cout << "ERROR " << nModel << "-gram table not found. cannot generate()" << endl;
    return;
  }
  if(seed.length() > 64){
    cout << "ERROR seed length too long in generate()" << endl;
    return;
  }

  //init the seed ring
  strncpy(buf,seed.c_str(),64);
  ngrams = tokenize(gram,buf," ");
  cout << "Ngrams=" << ngrams << endl;
  for(i = 0; i < ngrams; i++){
    s = gram[i];
    grams.push_back(s);
  }
  //end loop: grams ring contains last nmodel-1 grams (same order as input) 

  //cout << "DEBUG seed is >" << seed << "< nnmodel: " << nModel << endl;
  //cin >> i;
  cout << seed << " " << flush;

  //get the max from each subset, append to next key, and repeat until over limit or no results for a given seed
  start = 0; last = nModel - 2;
  for(it = tablePtr->find(seed); (it.first != it.second) && (i < limit); i++){  

    //get the next predicted word, print, then update seed ring
    //grams[start] = predictWord(seed);
    it = getMaxFromRange(&subset);
    grams[start] = it->second.nextWord;
    cout << grams[start] << " " << flush;

    //advance ring iterators
    last = start;
    start = (start + 1) % nseeds;
    //cout << "last/start: " << last << "/" << start << endl;

    //rebuild seed
    seed.clear();
    j = start;
    while(j != last){
      seed += (grams[j] + " ");
      j = (j + 1) % nseeds;
    }
    seed += grams[last];

    //cout << "[DEBUG next seed >" << seed << "<]" << endl;
    //cin >> j;

    //loop update
    subset = tablePtr->equal_range(seed);
    if(i % 12 == 0){
      cout << endl;
    }
  }
  if(i == limit){
    cout << " ... ... " << endl;
  }
  else{
    cout << "-->no more seeds" << endl;
  }
}
*/

//for kicks. calls predictWord(seed) recursively on its own results, generating language examples from a model.
//generate language from either the static of ddynamic lambda model. if static is chosen, choose lambdas based on lambdaSet params (this param is not used for dynamic models / when staticModel==false)
void nGram::generateFromLambdaModels(string& seed, bool staticModel, int lambdaSet)
{
  char buf[64];
  char* gram[8];
  int nseeds = 4, limit = 100;
  int ngrams, last, start, i, j;
  string s;
  vector<string> gramRing; //recursive ring of gram toks
  
  if(seed.length() > 64){
    cout << "ERROR seed length too long in generate()" << endl;
    return;
  }

  //init seed ring: may need to handle NULL placeholders as below
  gramRing.reserve(4);
  //for(i = 0; i < 3; i++){
  //  gramRing.push_back( string s("<UNK>") );
  //}

  strncpy(buf,seed.c_str(),64);
  ngrams = tokenize(gram,buf," ");
  if(ngrams != 4){
    cout << "incorrect n-tokens in seed >" << seed << "< to generate(), aborted" << endl;
    return;
  }

  for(i = 0; i < ngrams; i++){
    //s = gram[i];
    gramRing.push_back(gram[i]);
    //cout << " i = " << gram[i] << endl;
    //grams.push_back(s);
  }
  //end loop: grams ring contains last nmodel-1 grams (same order as input) 

  //cout << "DEBUG seed is >" << seed << endl;
  //cin >> i;
  //cout << seed << " " << flush;
  //get the max from each subset, append to next key, and repeat until over limit or no results for a given seed
  start = 0; last = 3;
  for(i = 0; i < limit; i++){
  
    //get the next predicted word, print, then update seed ring
    if(staticModel){
      gramRing[start] = predictWord_BoolStaticLinear(seed, lambdaSet);
    }
    else{
      //cout << "calling predictWord_BoolDynamic(" << seed << ")" << endl;
      gramRing[start] = predictWord_BoolDynamicLinear(seed);
      //cout << "  ret: " << gramRing[start] << endl;
    }

    if(gramRing[start] == "<UNK>"){
      break;
    }

    //output this generated word
    cout << gramRing[start] << " " << flush;
    
    //advance ring iterators
    last = start;
    start = (start + 1) % nseeds;
    //cout << "last/start: " << last << "/" << start << endl;

    //build next seed
    seed.clear();
    j = start;
    while(j != last){
      seed += (gramRing[j] + " ");
      j = (j + 1) % nseeds;
    }
    seed += gramRing[last];

    //cout << "[DEBUG next seed >" << seed << "<]" << endl;
    //cin >> j;

    if(i % 12 == 0){
      cout << endl;
    }
  }

  cout << " ... ..." << endl;
}



/*
  Gets the entropy of some subset within a given model. For instance:
    query := "the cat"
    subset := getSubset(query)
    subset{
       "the cat was"|3
       "the cat ran"|5
       "the cat mewwed"|1
       ...
       }
    subEntropy := entropy(subset)
    
    This returns a negative value. It is the caller's responsibilty to do ret * -1.0 to get a valid entropy value.
    This is done because this function is called within large summations, which makes constantly multiplying
    each result by -1.0 a term that can be easily factored out, for speed.
*/
long double nGram::getSubEntropy(const string& query, FreqTablePointer table)
{
  KeyIt set = table->find(query);

  //return 0.0 if no results
  if(set == table->end()){
    return 0.0;
  }

  //inner driver
  return getSubEntropy(set,this->normalized);
}

/*
  inner driver. factoring this out of the main driver allows it to be called without repeatedly creating the outer param.

  NOTE: This returns a negative number, if entropy has a value. Its the caller's responsibility to change it to positive.
  We don't do that here, because often we sum over repeated calls to this function, and convert to positive at the end.

  Note: The bool parameter determines if the table has been normalized yet or not. If true, the conditional probability 
  of each word has been calculated already, so we don't have to do so again.

  Precondition: Caller responsible for verifying "outer" is not map.end()
*/
long double nGram::getSubEntropy(KeyIt outer, bool normalized)
{
  long double subFreq, entropy, px, npx;
  SubkeyIt inner;

  //if table is not yet normalized, we have to sum across the items in this subset in order to calculate each words probability.
  if(!normalized){
    //get inner frequency for this key
    for(subFreq = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      subFreq += (long double)inner->second;
    }
    if(subFreq <= 0.0){
      cout << "ERROR subFreq<=0.0 in getSubEntropy(), returning 0.0" << endl;
      return 0.0;
    }
  }

  for(entropy = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){

    //get p(x) and p(~x)
    if(normalized){
      px = inner->second;
    }
    else{
      px = inner->second / subFreq;
    }
    npx = 1.0 - px;

    //cout << "px/npx = " << px << "/" << npx << endl;
    if(px > 0.0 && npx > 0.0){
      entropy += (px * log2l(px) + (npx) * log2l(npx));
    }
  }
  
  //cout << "returning " << entropy << "  entropy" << endl;
  return entropy;
}

/*
  Calculates the sub-entropy of some model based on the expected value summed
  over all sub-entropy values.

  Getting the unique keys in this function relies heavily on all keys (though not values) being ordered in the multimap.
*/
long double nGram::getExpectedSubEntropy(FreqTablePointer table)
{
  int dummy = 0;
  long double totalFreq, sumEntropy, subFreq;
  KeyIt outer;
  SubkeyIt inner;

  /*
  //DEBUG
  for(listIt = uniqueKeys.begin(); listIt != uniqueKeys.end(); ++listIt){
    cout << *listIt << endl;
  }
  cin >> dummy;
  */
  
  //get the sum frequency of all items
  totalFreq = getSumFrequency(table);
/*
  for(totalFreq = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      totalFreq += (long double)inner->second;
    }
  }
*/  

  //now calculate the entropy of each subset, multiply by its prob, and sum all of them
  for(sumEntropy = 0.0, outer = table->begin(); outer != table->end(); ++outer){
  
    //get inner frequency for this key
    for(subFreq = 0.0, inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      subFreq += inner->second;
    }
  
    //this subset entropy times its relative probability. This is essentially an expected value function.
    sumEntropy += getSubEntropy(outer,this->normalized) * subFreq;
    
    /*
    //ui output. basically a timer icon...
    dummy++;
    if(dummy == 15000){
      dummy = 0;
      cout << "\r                                       \r" << flush;
    }
    if((dummy % 1500) == 0){
      cout << "." << flush;
    }
    */
  }
  //normalize. If you do the math, a 1/totalFreq term can be factored out of the summation, for speed.
  sumEntropy /= totalFreq;
  if(sumEntropy != 0.0){
    sumEntropy *= -1.0;
  }

    /*HACK not a hack, just making a note. need to update this with smoothing.
      many n-gram strings of greater length have a frequency of 1, which gives an entropy
      of zero bits. For these, or perhaps all, n-gram strings we need a method to smooth such
      zero results. We base our models only upon gram strings we have seen, although we clearly
      have not seen all possible configurations of unique words, let alone the real probability distribution.
      We can either use some simple smoothing like LaPlace (?), or use a method that is based on information
      we have actually seen. For instance we could somehow base it on the number of 1-grams (# unique words),
      or otherwise base it on the sparsity of our training data.

      TODO: handle case when subEntropy==0.0

  /*Prevents returning -0.0 when a set has only one member (the entropy of such sets is defined as zero)
  if(sumEntropy != 0.0){
    sumEntropy *= -1;
  }
  */

  //cout << "\r                       " << endl;
  //cout << endl << nModel << "-gram model sumEntropy: " << sumEntropy << endl;

  return sumEntropy;
}

/*
  Another method for tallying up the entropy within a model. Models of
  greater length (more n-grams) should have decreasing entropy, since we
  have more grams, and thus more information (which always decreases entropy).
  
  The question then is how to measure the entropy of n-gram models for n > 1:
  do we look at the total entropy of the model? For instance, sum across all possible
  n-grams the p(n-gram) * lg(p(n-gram)). But this is wrong because this gives increasing
  entropy as we increase the number of grams and the distribution becomes more sparse.

  This doltish method addresses the subEntropy of n-gram models by looking at averaging the
  entropy within all possible subsets of n-1 grams. For instance, the query "the cat" 
  may have three possible 3-gram results: {"the cat was":1, "the cat walked":2, "the cat ate":1}
  We calculate the entropy for this set, and then average it with all other subsets ("the dog", "the hamster", etc).

  Notes: This method is not smart, since it takes no account of the relative value of certain subsets.
  A better method is to sum over the expected value of each entropy value, giving greater weight to larger subsets,
  and smaller weights to smaller subsets. This way entropy values from smaller sets do not have such 
  disproportional influence over the values derived from larger subsets.
*/
long double nGram::getMeanSubEntropy(FreqTablePointer table)
{
  int dummy = 0;
  long double totalFreq, nSubsets, sumEntropy, subFreq;
  KeyIt outer;
  SubkeyIt inner;

  //count number of subsets in table
  for(nSubsets = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    nSubsets++;
  }
  
  if(nSubsets <= 0.0){
    cout << "WARN nSubsets=" << nSubsets << " <= 0.0 in getMeanSubEntropy(), returning 0.0" << endl;
    return 0.0;
  }
  
  //calculate and sum the entropy of each subset
  for(sumEntropy = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    //sum the entropy of each subset
    sumEntropy += getSubEntropy(outer,this->normalized);
    
     //ui output. basically a timer icon...
    dummy++;
    if(dummy == 15000){
      dummy = 0;
      cout << "\r                                       \r" << flush;
    }
    if((dummy % 1500) == 0){
      cout << "." << flush;
    }
  }
  //average
  sumEntropy /= nSubsets;
  if(sumEntropy != 0.0){
    sumEntropy *= -1.0;
  }

  return sumEntropy;
}

/*
  The one-gram table is a multimap, but is exceptional and does not fit the data model of the other tables.
  Hence the qualified function for this model alone. The one gram model is a multimap, when really it should
  be a map of key word-strings mapping to integer frequencies. It is simpler to keep it as a multimap, like the others.

  OBSOLETE This used to be required by the unique model of the one-gram model. It is no longer unique, so this isn't needed.

long double nGram::getTotalEntropy_OneGram(FreqTablePointer table)
{
  int dummy;
  long double sumEntropy, sumFreq, pX, pNotX;
  KeyIt outer;
  SubkeyIt inner;

  if(table->empty() || table->size() < MIN_MODEL_SIZE){
    cout << "ERROR too few items >" << table->size() << "< to calculate total entropy. Returning -1" << endl;
    return -1.0;
  }

  
  //count all the items in the model (sum the frequency of all ngrams)
  for(sumFreq = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      sumFreq += inner->second;
    }
  }
  //cout << "DEBUG sumFreq is: " << sumFreq << endl;
  //cin >> dummy;
  
  if(sumFreq <= 0.0){
    cout << "ERROR sumFreq<=0.0, divZero aborted, INF_ENTROPY returned from totalEntropy()" << endl;
    return -1.0;
  }

  //now calculate and sum the entropy of each word in the model
  for(sumEntropy = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      pX = inner->second;   //px will always be extremely small for a one-gram model
      pNotX = 1.0 - pX;                            //result will be very near to one for one-gram model
      if((pX > 0.0) && (pNotX > 0.0)){
        sumEntropy += ((pX * log2l(pX)) + (pNotX * log2l(pNotX)));
      }
    }
  }
  
  if(sumEntropy != 0.0){
    sumEntropy *= -1.0;
  }  
  //cout << "One gram entropy upperbounded by log2(num words in model). Verify this is the case. log2l(nwords)=" << log2l((long double)table->size()) << " > calculated entropy=" << sumEntropy << " ?" << endl;
  //cin >> dummy;

  return sumEntropy;
}
*/


/*
  Calculates the raw entropy of an entire sets of n-grams.

  Pre-condition: Model is in-memory.

  This is given by -1 multiplied by the summation of each string's probability
  multiplied by the base-2 log of that probability.


  Handles models of up to 2^64 model items.
*/
long double nGram::getTotalEntropy(FreqTablePointer table)
{
  int dummy;
  long double sumEntropy, sumFreq, pX, pNotX;
  KeyIt outer;
  SubkeyIt inner;

  if(table->size() < MIN_MODEL_SIZE){
    cout << "ERROR too few items >" << table->size() << "< to calculate total entropy. Returning -1" << endl;
    return -1.0;
  }
  if(this->normalized){
    cout << "ERROR cannot compute total entropy after tables have been normalized" << endl;
    return -1.0;
  }

  //count all the items in the model (sum the frequency of all ngrams)
  for(sumFreq = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      sumFreq += (long double)inner->second;
    }
  }
  //cout << "DEBUG sumFreq is: " << sumFreq << endl;
  //cin >> dummy;
  
  if(sumFreq <= 0.0){
    cout << "ERROR sumFreq<=0.0, divZero aborted, INF_ENTROPY returned from totalEntropy()" << endl;
    return -1.0;
  }

  //calculate and sum the entropy of every item
  for(sumEntropy = 0.0, outer = table->begin(); outer != table->end(); ++outer){
    for(inner = outer->second.begin(); inner != outer->second.end(); ++inner){
      pX = (long double)inner->second / sumFreq; //result will be extremely small, near zero
      pNotX = 1.0 - pX;                            //result will be very near to one
      if((pX > 0.0) && (pNotX > 0.0)){
        sumEntropy += ((pX * log2l(pX)) + (pNotX * log2l(pNotX)));
      }
      
      /*
      //ui output. basically a timer icon...
      dummy++;
      if(dummy == 15000){
        dummy = 0;
        cout << "\r                                       \r" << flush;
      }
      if((dummy % 1500) == 0){
        cout << "." << flush;
      }
      */
    }
  }
  
  if(sumEntropy <= 0.0){
    sumEntropy *= -1.0;
  }

  return sumEntropy;
}

/*
  Initializes lambdas with arbitrary, user-coded values.

  TODO: there's division here. wrap this in div-zero error checks.
*/
void nGram::initLambdas(void)
{
  int i;
  long double l1, l2, l3, l4, l5, normal;
  
  if(!isCocaBuilt){
    cout << "ERROR cannot init lambdas until COCA models are built and analyzed" << endl;
    return;
  }


  //re-initialize the hit counts to zero
  for(i = 0; i < NLAMBDASETS; i++){
    modelLambdas[i].boolHitRate = 0.0;
    modelLambdas[i].realHitRate = 0.0;
    modelLambdas[i].recall = 0.0;
    modelLambdas[i].topSevenAccuracy = 0.0;
  }

  /*
    Method 1: the first and simplest lambda model: assign equal values to each lambda, such that we weight the models equally.
  */
  for(i = 0; i < NLAMBDAS; i++){
    modelLambdas[0].l[i] = 0.2;
  }

  /* 
    Method 2: Nuther simple one. After training models, "test" the models on the training data itself to estimate the best modelLambdas.
    this is based on the intuition that the modelLambdas will converge to the relative accuracy of each model, we just normalize them.
    For example, if the 3/2/1-gram models have accuracies of 12/4/1%, then the modelLambdas (3-1) are 12/17, 4/17, and 1/17
  
  for(int i = 2; i <= 5; i++){ //skip 1-gram model: it only predicts unconditional probability (iow, it will always predict the most likely word in the entire model, say "cat")
    testModel(trainingData,i,false);  //learn=false, so we do not modify models based on data already seen
  }
  */

  //end loop: real-accuracy values are stored in class-level wordModelStats array. These are only for lambda estimates, they may be overwritten later
  //1-gram model is a special case. The "accuracy" of the 1-gram model is the same as 1/perplexity.
  l1 = 1 / wordModelStats[1].totalPerplexity;
  //for the rest, set modelLambdas equal to the accuracy of each model, scaled by expected sub-entropy. These vals should improve for larger trainiig sets.
  //HACK: for now I'm not training these values, just pasting previous output of program
  wordModelStats[2].realAccuracy = 0.0460124;
  wordModelStats[3].realAccuracy = 0.0296852;
  wordModelStats[4].realAccuracy = 0.0167180;
  wordModelStats[5].realAccuracy = 0.0167180 / 2.0;  //because they appear to be decreasing by slope = -0.5
  l2 = wordModelStats[2].realAccuracy / wordModelStats[2].expectedSubEntropy;
  l3 = wordModelStats[3].realAccuracy / wordModelStats[3].expectedSubEntropy;
  l4 = wordModelStats[4].realAccuracy / wordModelStats[4].expectedSubEntropy;
  l5 = wordModelStats[5].realAccuracy / wordModelStats[5].expectedSubEntropy;
  //normalize the above values, so they sum to 1 and give us a probability distribution summed over each model
  normal = l1 + l2 + l3 + l4 + l5;
  modelLambdas[1].l[1] = l1 / normal;
  modelLambdas[1].l[2] = l2 / normal;
  modelLambdas[1].l[3] = l3 / normal;
  modelLambdas[1].l[4] = l4 / normal;
  modelLambdas[1].l[5] = l5 / normal;

  //using real predictive accuracy may be superfluous. This method uses only entropy
  l1 = 1.0 / wordModelStats[1].totalPerplexity;
  //for the rest, set modelLambdas to be scaled by expected sub-entropy. This is a somewhat a priori method.
  l2 = 1.0 / wordModelStats[2].expectedSubEntropy;
  l3 = 1.0 / wordModelStats[3].expectedSubEntropy;
  l4 = 1.0 / wordModelStats[4].expectedSubEntropy;
  l5 = 1.0 / wordModelStats[5].expectedSubEntropy;
  //normalize the above values, so they sum to 1 and give us a probability distribution summed over each model
  normal = l1 + l2 + l3 + l4 + l5;
  modelLambdas[2].l[1] = l1 / normal;
  modelLambdas[2].l[2] = l2 / normal;
  modelLambdas[2].l[3] = l3 / normal;
  modelLambdas[2].l[4] = l4 / normal;
  modelLambdas[2].l[5] = l5 / normal;

  //test lambdas set by individual model accuracy, normalized. these values were based n-gram models indvidiually against 30% of the enron data.
  modelLambdas[3].l[0] = 0.00;
  modelLambdas[3].l[1] = 0.2; //one-gram model lambda completely unknown. 0.2 is arbitrary. it should be seen as a tie-breaker among predictions that have approximately the same value
  modelLambdas[3].l[2] = 0.959385;
  modelLambdas[3].l[3] = 0.792951;
  modelLambdas[3].l[4] = 0.32796;
  modelLambdas[3].l[5] = 0.110221;
  //normalize
  normal = modelLambdas[3].l[1] + modelLambdas[3].l[2] + modelLambdas[3].l[3] + modelLambdas[3].l[4] + modelLambdas[3].l[5];
  modelLambdas[3].l[1] /= normal;
  modelLambdas[3].l[2] /= normal;
  modelLambdas[3].l[3] /= normal;
  modelLambdas[3].l[4] /= normal;
  modelLambdas[3].l[5] /= normal;


  /*
    Method 3: train the modelLambdas using a simple perceptron neural network. Or use another method to set 
    lambdas according to the accuracy of each model against a hold-out set of training data.
  
  //TODO: perceptron or EM algorithm
  modelLambdas[2].l[1] = 0;
  modelLambdas[2].l[2] = 0;
  modelLambdas[2].l[3] = 0;
  modelLambdas[2].l[4] = 0;
  modelLambdas[2].l[5] = 1;
  */

  printLambdas();
}

/*
  For training the lambda values for linear interpolation prediction methods,
  where we select the max likely next word, given the summation:
     P(w) = lambda1 * P(x,5-gram) + lambda2 * p(x,4-gram) + ...

  This method may end up incorporating multiple methods for optimizing the lambda values,
  such that we a get several sets of modelLambdas for testing.

  Currently just uses linear interpolation (see jurafsky p. 150)

  This may take a few minutes to run.
*/
void nGram::trainLambdas(void)
{
  int i;
  long double l1, l2, l3, l4, l5, normal;
  string enronCorpus = "../enronParsed/parsed.txt";

  cout << "training modelLambdas from " << enronCorpus << "..." << endl;

  if(!isCocaBuilt){
    cout << "ERROR COCA models not yet built. COCA models must be built before calling trainLambdas()" << endl;
    return;
  }
  
  //re-initialize the hit counts to zero
  for(i = 0; i < 8; i++){
    modelLambdas[i].boolHitRate = 0.0;
    modelLambdas[i].realHitRate = 0.0;
  }

  /*
    Method 1: the first and simplest lambda model: assign equal values to each lambda, such that we weight the models equally.
  */
  for(i = 0; i < NLAMBDAS; i++){
    modelLambdas[0].l[i] = 0.2;
  }

  /* 
    Method 2: Nuther simple one. After training models, "test" the models on the training data itself to estimate the best modelLambdas.
    this is based on the intuition that the modelLambdas will converge to the relative accuracy of each model, we just normalize them.
    For example, if the 3/2/1-gram models have accuracies of 12/4/1%, then the modelLambdas (3-1) are 12/17, 4/17, and 1/17
  
  for(int i = 2; i <= 5; i++){ //skip 1-gram model: it only predicts unconditional probability (iow, it will always predict the most likely word in the entire model, say "cat")
    testModel(trainingData,i,false);  //learn=false, so we do not modify models based on data already seen
  }
  */

  //end loop: real-accuracy values are stored in class-level wordModelStats array. These are only for lambda estimates, they may be overwritten later
  //1-gram model is a special case. The "accuracy" of the 1-gram model is the same as 1/perplexity.
  l1 = 1 / wordModelStats[1].totalPerplexity;
  //for the rest, set modelLambdas equal to the accuracy of each model, scaled by expected sub-entropy. These vals should improve for larger trainiig sets.
  //HACK: for now I'm not training these values, just pasting previous output of program
  wordModelStats[2].realAccuracy = 0.0460124;
  wordModelStats[3].realAccuracy = 0.0296852;
  wordModelStats[4].realAccuracy = 0.0167180;
  wordModelStats[5].realAccuracy = 0.0167180 / 2.0;  //because they appear to be decreasing by slope = -0.5
  l2 = wordModelStats[2].realAccuracy / wordModelStats[2].expectedSubEntropy;
  l3 = wordModelStats[3].realAccuracy / wordModelStats[3].expectedSubEntropy;
  l4 = wordModelStats[4].realAccuracy / wordModelStats[4].expectedSubEntropy;
  l5 = wordModelStats[5].realAccuracy / wordModelStats[5].expectedSubEntropy;
  //normalize the above values, so they sum to 1 and give us a probability distribution summed over each model
  normal = l1 + l2 + l3 + l4 + l5;
  modelLambdas[1].l[1] = l1 / normal;
  modelLambdas[1].l[2] = l2 / normal;
  modelLambdas[1].l[3] = l3 / normal;
  modelLambdas[1].l[4] = l4 / normal;
  modelLambdas[1].l[5] = l5 / normal;

  //using real predictive accuracy may be superfluous. This method uses only entropy
  l1 = 1.0 / wordModelStats[1].totalPerplexity;
  //for the rest, set modelLambdas to be scaled by expected sub-entropy. This is a somewhat a priori method.
  l2 = 1.0 / wordModelStats[2].expectedSubEntropy;
  l3 = 1.0 / wordModelStats[3].expectedSubEntropy;
  l4 = 1.0 / wordModelStats[4].expectedSubEntropy;
  l5 = 1.0 / wordModelStats[5].expectedSubEntropy;
  //normalize the above values, so they sum to 1 and give us a probability distribution summed over each model
  normal = l1 + l2 + l3 + l4 + l5;
  modelLambdas[2].l[1] = l1 / normal;
  modelLambdas[2].l[2] = l2 / normal;
  modelLambdas[2].l[3] = l3 / normal;
  modelLambdas[2].l[4] = l4 / normal;
  modelLambdas[2].l[5] = l5 / normal;

  //test lambdas set by individual model accuracy, normalized. these values were based n-gram models indvidiually against 30% of the enron data.
  modelLambdas[3].l[0] = 0.00;
  modelLambdas[3].l[1] = 0.2; //one-gram model lambda completely unknown. 0.2 is arbitrary. it should be seen as a tie-breaker among predictions that have approximately the same value
  modelLambdas[3].l[2] = 0.959385;
  modelLambdas[3].l[3] = 0.792951;
  modelLambdas[3].l[4] = 0.32796;
  modelLambdas[3].l[5] = 0.110221;
  //normalize
  normal = modelLambdas[3].l[1] + modelLambdas[3].l[2] + modelLambdas[3].l[3] + modelLambdas[3].l[4] + modelLambdas[3].l[5];
  modelLambdas[3].l[1] /= normal;
  modelLambdas[3].l[2] /= normal;
  modelLambdas[3].l[3] /= normal;
  modelLambdas[3].l[4] /= normal;
  modelLambdas[3].l[5] /= normal;

  /*
    Method 3: train the modelLambdas using a simple perceptron neural network. Or use another method to set 
    lambdas according to the accuracy of each model against a hold-out set of training data.
  */
  modelLambdas[4].l[1] = modelLambdas[3].l[1]; //inherit initial values from vals above
  modelLambdas[4].l[2] = modelLambdas[3].l[2];
  modelLambdas[4].l[3] = modelLambdas[3].l[3];
  modelLambdas[4].l[4] = modelLambdas[3].l[4];
  modelLambdas[4].l[5] = modelLambdas[3].l[5];

  //lambdaEM(4,enronCorpus);

  deletedInterpolation(5,enronCorpus);

  printLambdas();
}

/*
  Precondition: n-gram models are built.
  See Jurafsky p150 for pseudocode.

  Takes a trainingFile param and a lambdaset param. Trains a bunch of lambdas
  using deleted interpolation, then stores these in the object's lambda[] using
  lambdaset as the index.

  Using my method of incrementing instead of taking max, and my enronParsed.txt training set, this previously converged to:
      100.223% complete (12556kb read)
      pxn[2..5]: 94263.7 124790 75433.1 31611.3
      100.255% complete (12560kb read)
      pxn[2..5]: 94296.4 124833 75469 31625.3
      100.287% complete (12564kb read)
      pxn[2..5]: 94324.9 124862 75483.4 31631.4
      100.319% complete (12568kb read)
      pxn[2..5]: 94358.5 124899 75507 31640.6
      100.336% complete (12571kb read)
      final pxn[2..5]: 94404.2 124967 75541.9 31652.5

  Using the book's method of deleted interpolation for pos-tagging, and enronParsed.txt:

      100.255% complete (12560kb read)
      pxn[2..5]: 1.67497e+06 340266 117497 36257
      100.287% complete (12564kb read)
      pxn[2..5]: 1.6755e+06 340347 117525 36265
      100.319% complete (12568kb read)
      pxn[2..5]: 1.67602e+06 340443 117562 36277
      100.336% complete (12571kb read)
      pxn[2..5]: 1.67652e+06 340530 117582 36279
      final pxn[2..5]: 1.67674e+06 340584 117610 36289

  Book's method gives far greater weight to 2-gram. Growth-rate of that one was exponential,
  whereas the 5-gram count's growth rate wasn't even close: although it did strike. This suggests
  the book's method is too harsh, since the numbers are clearly diverging, as 5g-ct / 2g-ct --> 0 for
  very large training sets.  We need more of a real-valued function, which my method seems to provide,
  even though its a pretty clunky and obvious idea. The book's method is too winner-take-all.

*/
void nGram::deletedInterpolation(int lambdaSet, const string& trainingFile)
{
  long double pxn[6] = {0.0,0.0,0.0,0.0,0.0,0.0};
  int inFile, i, n, lastDelim, dummy;
  unsigned long long bytesRd;
  long double progress;
  struct stat fileInfo;
  char buf[BUFSIZE];
  string obuf(BUFSIZE,'\0');
  //debug
  //time_t tm;

  inFile = open(trainingFile.c_str(), O_RDONLY);
  if(inFile < 0){
    cout << "ERROR could not open input file >" << trainingFile << "< testModel() aborted" << endl;
    return;
  }
  if(fstat(inFile, &fileInfo)){
    cout << "ERROR fstat() call failed in testModel()" << endl;
    return;
  }

  bytesRd = 0; i = 0;
  //we read discrete phrases structures by reading 1024 bytes, then backing file position to the last delimiter
  while(n = read(inFile, buf, READ_SZ)){
    bytesRd += n;
    //i++;
    //if((i %= 2) == 0){
    progress = 99 * ((float)bytesRd / (float)fileInfo.st_size);
    //cout << "\r...  " << progress << "% complete (" << (bytesRd >> 10) << "kb read)   "  << flush;
    cout << progress << "% complete (" << (bytesRd >> 10) << "kb read)" << endl;
    cout << "pxn[2..5]: " << pxn[2] << " " << pxn[3] << " " << pxn[4] << " " << pxn[5] << endl;

/*
      if(progress > 1.0){
        goto done;
      }
*/
    //}

    buf[n] = '\0';
    //return file position to index of last delimiter (the end of the last regular structure), to get a bounded sample.
    lastDelim = seekLastPhraseDelimiter(buf,n);
    if(lastDelim > -1){
      buf[lastDelim] = '\0';
      //prevents infinite loop at end of file: we will continuously wind back to last delim if not checked.
      if(n == READ_SZ){
        lseek(inFile, lastDelim - n + 1, SEEK_CUR);
      }
      //a series of pipes/filters to clean up sample, and map it into our parse semantics
      normalizeText(buf,obuf);
      deletedInterpDriver(strncpy(buf, obuf.c_str(), BUFSIZE),pxn);
    }
    else{
      cout << "WARN no phrase delimiter found in buf, input discarded: >" << buf << "< delim index: " << lastDelim << " phraseDelims: " << phraseDelimiters << endl;
    }
/*
    //debug
    lim++;
    if(lim > 30){
      goto done;
    }
*/
  }

  done:
  close(inFile);
  cout << "final pxn[2..5]: " << pxn[2] << " " << pxn[3] << " " << pxn[4] << " " << pxn[5] << endl;

  for(i = 2; i <= 5; i++){
    modelLambdas[lambdaSet].l[i] = pxn[i];
  }
}

void nGram::deletedInterpDriver(char buf[BUFSIZE], long double pxn[6])
{
  int nTokens, nSubStrs, i, j, k, max_i;
  long double max, tmp;
  char* grams[MAX_TOKENS_PER_READ] = {NULL};
  string gramBuf, nextWord, q[6];
  FreqTablePointer table;

  //for each bounded-sample, tokenize the words within it
  nTokens = tokenize(grams, buf, this->rawDelimiters);
  if(nTokens > MAX_TOKENS_PER_READ){
    cout << "WARNING: MAX_TOKENS_PER_READ exceeded in updateNgramTable(), sample ignored" << endl;
    return;
  }
  if(nTokens < 100){
    cout << "WARN anomalous nPhrases >" << nTokens << "< in updateFreqFile. phrases[0]: >" << grams[0] << "<" << endl;
  }

  //count number of n-gram strings in this buffer, then query each n-gram substring in frequencyTable / map.
  nSubStrs = nTokens - 4;
  for(i = 4; i < nSubStrs; i++){

    //build the queries
    q[2] = grams[i-1];
    for(j = 3, k = 2; j <= 5; j++, k++){
      q[j] = grams[i-k]; q[j] += " "; q[j] += q[j-1];
    }
    nextWord = grams[i];

    /*my method: just sum the probs, then normalize them later.
    for(j = 2; j <= 5; j++){
      table = getTablePointer(j);
      pxn[j] += getConditionalProbability(q[j],nextWord,table);
    }
    */

    //book's method, for POS tagging. get the max probability, and from it the index of the model giving that probability
    for(j = 2, max = -999.0, max_i = 0; j <= 5; j++){
      table = getTablePointer(j);
      tmp = getConditionalProbability(q[j],nextWord,table);
      if(tmp > max){
        max = tmp;
        max_i = j;
      }
    } //post: max_i is set to the model with great probability, or 0, in which case the increment goes nowhere
    pxn[max_i]++;
  }
}



/*
  A very simple expectation-maximation algorithm.
  Takes a lambdaset parameter and a trainingFile string.
  We read input from the input file, and measure the performance.
  After comparing with the performance of each iteration with previous,
  we update the values, re-run, and repeat the update, over and over,
  until the lambdas appear to converge.
*/
void nGram::lambdaEM(int lambdaSet, const string& trainingSet)
{
  string enronData = "../someEnronCorpus.txt";

  testModel(enronData,5,false,true);
}


void nGram::printLambdas(void)
{
  char buf[256];

  //debug: print the modelLambdas
  for(int i = 0; i < NLAMBDASETS; i++){
    sprintf(buf,"lambda[%d]: %2.6LF %2.6LF %2.6LF %2.6LF %2.6LF\0",i,modelLambdas[i].l[1],modelLambdas[i].l[2],modelLambdas[i].l[3],modelLambdas[i].l[4],modelLambdas[i].l[5]);
    cout << buf << "  boolHit: " << modelLambdas[i].boolHitRate << " realHit: " << modelLambdas[i].realHitRate << " top7: " << modelLambdas[i].topSevenAccuracy;
    cout << " recall: " << modelLambdas[i].recall << endl;
  }
}



/*
  Given a test string, we run through the ngrams in the string and lookup
  each ngram in one of our ngram tables built on training data. Misses are
  zero or smoothed.  

  *This function is dynamic, and updates the training model based on the test input as it goes.


  We may want to factor in certain measurements here again, such as word/phrase counts.
  This doesn't need to chop up the buffer as it reads, but may as well in order to share code-base
  with parsing in updateNgramTable().

  Returns: -1 if error, else returns (number of hits / number of n-gram strings * 100%)

  TODO: this function is a ratsnest of pocket lint and spaghetti.
  TODO: Can we depth-bound the result list somehow??? Some result lists are 30k+, especially if they contain
  very common words like "the" which have a huge branching factor.

previous output:
*******************************************
Lambda Model testing stats:
Num n-grams: 22905
Hard-hit count:  2.02382e-4324
Boolean predictive accuracy: -nan%
Soft-hit count:  -2.10031e+4696
Real predictive accuracy:    -nan%
Lambda[0]:10.0109% bool accuracy  16.1703% real accuracy
*******************************************


*/
void nGram::testSample_Raw(char buf[BUFSIZE], int nModel, long double& gramCt, bool learn, bool lineartest)
{
  int nTokens, nSubStrs, i, j, l;
  char* grams[MAX_TOKENS_PER_READ] = {NULL};
  string gramBuf, nextWord;
  KeyPair kp;
  list< std::pair<SubkeyIt,long double> > resultList; //doesn't belong here, or as here, will constantly be allocated and deallocated per testsample call

  //for each bounded-sample, tokenize the words within it
  nTokens = tokenize(grams, buf, this->rawDelimiters);
  if(nTokens > MAX_TOKENS_PER_READ){
    cout << "WARNING: MAX_TOKENS_PER_READ exceeded in updateNgramTable(), sample ignored" << endl;
    return;
  }
  if(nTokens < 2){
    cout << "WARN anomalous nPhrases >" << nTokens << "< in updateFreqFile. phrases[0]: >" << grams[0] << "<" << endl;
  }

  //check enough tokens in this buffer for this n-gram model
  if(nTokens < nModel){
    cout << "too few words (" << nTokens << ") in this phrase for " << nModel << "-grams. sample discarded, continuing..."  << endl;
    return;
  }

  //count number of n-gram strings in this buffer, then query each n-gram substring in frequencyTable / map.
  nSubStrs = nTokens - nModel + 1;
  gramCt += nSubStrs;
  for(i = 0; i < nSubStrs; i++){

    //build query string of n-1 grams. TODO: instead of building each query, we might somehow build all queries in advance, to speed runtime
    gramBuf = grams[i];
    for(j = 1; j < (nModel - 1); j++){
      gramBuf += " ";
      gramBuf += grams[j + i];
    }
    nextWord = grams[j+i];
    //cout << "next query >" << gramBuf << "< actual next word: >" << nextWord << "<" << endl;
    //cin >> dummy;

    //cout << "T1: " << ctime(&tm) << endl;
    //hits = queryTable(gramBuf, sortedHits, nModel);
    //cout << "T2: " << ctime(&tm) << endl;

    /*  //makeKey() is a nice user-level/public function, but totally redundant in this context. we don't want this executing count(all words) times.
    gramBuf += " ";
    gramBuf += nextWord;
    makeKeyPair(gramBuf,&kp);
    */
    
    if(!lineartest){
      //ridiculous function name... shows how poorly factored these testing functions are
      testWord(nModel,gramBuf,nextWord);
    }

    //lambda scoring
    if(lineartest){

/*
      //cout << "testing dynamic-linear model" << endl;
      //test the dynamic lambda methods
      resultList.clear();
      predictWord_RealDynamicLinear(gramBuf, resultList);
      if(resultList.size() > 0){
        if((resultList.begin()->first->first) == nextWord){
          //cout << "predictWord_BoolDynamicLinear hit: " << gramBuf << "==" << nextWord << endl;
          this->modelLambdas[6].boolHitRate++;
        }
        //do linearScoring
        //cout << "scoring dynamic-linear model" << endl;
        this->modelLambdas[6].realHitRate += scoreLinear(nextWord,resultList);
        //cout << "done" << endl;
      }
*/
      //now test the static methods for ALL lambdas. for each lambda set, tally their hits
      for(l = 0; l < 4; l++){
        //cout << "testing static-linear model[" << l << "]" << endl;
        //cout << "predicted: " << predictWord(kp.primaryKey,k) << " for kp.primary: >" << kp.primaryKey << "< and nextword: >" << nextWord << "<" << endl;
        /*
        if(predictWord_BoolStaticLinear(gramBuf,l) == nextWord){
          modelLambdas[l].boolHitRate++;
          
          if(l == 1){
            hitList.push_front(nextWord);
          }
          //cout << "WOW! a hit: " << gramBuf << "->" << nextWord << endl; 
        }
        */
        
        resultList.clear();
        predictWord_RealStaticLinear(gramBuf,l,resultList);
        if(resultList.size() > 0){
          if(resultList.begin()->first->first == nextWord){
            //cout << "predictWord_BoolStaticLinear hit: " << gramBuf << " " << nextWord << endl;
            modelLambdas[l].boolHitRate++;
          }
          scoreLinear(nextWord,l,resultList);
        }
        
      }
    }

    if(learn){
      //we continue to learn from the test data as we go if learn==true. This shouldnormally be off during research.
      incrementKey(gramBuf,nModel);
    }
  }
}

void nGram::testWord(int nModel, string nQuery, string nextWord)
{
  FreqTablePointer table;
  MapSubset subset;

  table = getTablePointer(nModel);
  if(table == NULL){
    cout << "testsample_raw(): table ptr not found" << endl;
    return;
  }

  subset = equalRange(table,nQuery); //get the subset of items for this key
  //subset.first = table->find(nQuery); //get the subset of items for this key
  //only increment hit counts if this key exists in multimap
  if(subset.first != subset.second){
    //track raw hits: occurs when the model predicts the most likely word is in fact the next word.
    //this gives us our "raw" measure of accuracy
    wordModelStats[nModel].booleanAccuracy += getBooleanHitCount(&subset,nextWord);
    //track soft hits: occurs when the model predicts the correct word in its results, but it is not the most likely word
    //this gives us our more continuous measure of accuracy
    wordModelStats[nModel].realAccuracy += getRealHitCount(&subset,nextWord);
    //cout << "T3: " << ctime(&tm) << endl;
  }
}

/*
  This is just for experimenting with direct file access, to see if its capable of operating faster than reading entire
  COCA models into program memory (which takes a long time, and still has long query times for common
  phrases such as "to ____" "a ____" "the ____" and so on.

  Tests: see if binary search the w2c.txt COCA file yields fast query times.

  This is just an experiment. Writing these sorts of low-level io ops (instead of using libraries or possibly SQL) will
  make for terrible, buggy interfaces. --> look for a better more reusable solution

  COCA files are case-sensitive (which also slightly breaks the binsearch requirement that they are sorted), but ignore this for now.

*/
void nGram::testQBinSearch(void)
{
  struct timespec search_start, search_end;
  int i,j,f, dummy, res;
  int firstnewline, key, endkey;
  U64 start, mid, end;
  string testQuery = "they";  //a common query, since "the" has many, many possible next words in any n-gram language model
  string infile = "../../../corpii/coca_ngrams/w2c.txt";
  struct stat fileInfo;
  bool found;
  char buf[256];
  int n;

  f = open(infile.c_str(), O_RDONLY);
  if(f < 0){
    cout << "ERROR could not open input file >" << infile << "< testModel() aborted" << endl;
    return;
  }
  if(fstat(f, &fileInfo)){
    cout << "ERROR fstat() call failed in testModel()" << endl;
    return;
  }

  clock_gettime(CLOCK_MONOTONIC,&search_start);

  //using binsearch, find the file offsets of the subset of the 2-gram model containing "the" as a primary key
  // 1 - binsearch for start of subsett: postcondition=found start offset
  start = 1;  // TODO 1 or 0? there are other off-by-one scnearios below that need to be checked, mostly per the start/mid/end segmentation
  end = (U64)fileInfo.st_size;
  mid = end >> 1; //int div 2
  found = false;
  while( !found && start < end ){

    //cout << "start=" << start << "  end=" << end << "  mid=" << mid << endl;
    //cin >> dummy; 

    //read chunk from start offset
    if(n = pread(f,buf,255,mid)){
      buf[n] = '\0';

      //small FSA searches for pattern \n[0-9]*\t[query]\t in this buf
        firstnewline = findChar(buf,'\n',n);
        //bomb out if no firstnewline
        if(firstnewline == -1){
          cout << "ERROR search aborted, no firstnewline found in buf: " << buf << endl;
          return;
        }
        firstnewline++;

        //from firstnewline, find next tab
        key = findChar(&buf[firstnewline],'\t',n);
        if(key == -1){
          cout << "ERROR search aborted, no tab found in buf after firstnewline: " << &buf[firstnewline+1] << endl;
          return;
        }
        key = firstnewline + key + 1;

        //find next next tab relative to first tab (and chop)
        endkey = findChar(&buf[key],'\t',n);
        if(endkey == -1){
          cout << "ERROR search aborted, no tab found in buf after key: " << &buf[key+1] << endl;
          return;
        }
        endkey = key + endkey;

        buf[endkey] = '\0'; //chop at second tab after first newline. this is our key/query for 2-GRAM MODEL ONLY!
        //cout << "firstnewline= " << firstnewline << "  key=" << key << "  endkey=" << endkey << " verify next term valid: >" << &buf[key] << "<" << endl;
        //cin >> dummy;
      //end FSA

      //advance or regress iterators base on results
      res =  testQuery.compare( &buf[key] );
      if(res == 0){  //query HIT: end search, and change state to bidirectionally search for bounds of subset (simple spool for regex )
        cout << "HIT found subset for q=" << testQuery << "  curkey=" << &buf[key] << endl;
        //cin >> dummy;
        //getSubsetBounds(f,&buf[key],mid,&start,&end); //output params return start and end of this subset, per this key
        found = true;
      }
      else if(res > 0){
        //cout << "Advancing iterators q=" << testQuery << "  curkey=" << &buf[key] << endl;
        //cin >> dummy;
        start = mid + 1;
        mid = (end + start) >> 1;
      } 
      else{ // res > 0
        //cout << "Regressing iterators q=" << testQuery << "  curkey=" << &buf[key] << endl;
        //cin >> dummy;
        end = mid;
        mid = (end + start) >> 1;
      }
      
      if(start >= end){
        cout << "Query not found!" << endl;
      }
    }
    else{
      //HACK
      break;
    }
  }

  // 2 - continuously read to find end of subset: postcondition=found end offset  
  clock_gettime(CLOCK_MONOTONIC,&search_end);
  cout << "runtime: " << ((long double)(search_end.tv_sec - search_start.tv_sec) + ((long double)(search_end.tv_nsec - search_start.tv_nsec) / (long double)1000000000)) << endl;
  cout << "end_sec " << search_end.tv_sec << "  end_nsec " << search_end.tv_nsec << "  start_sec " << search_start.tv_sec << "  start_nsec " << search_start.tv_nsec << endl;
}


//testing the lookup times in the two-gram table, which are quite slow for common prefixes: the, an, a, etc.
// OBSOLETE since changed model to storing long double probs instead of int-freqs
void nGram::testTwoGramLookupTime(void)
{
  struct timespec begin, end;
  string q = "the";
  string t1 = "yellow"; //assume "yellow" is alphabetically near the end of the subset "the"
  string t2 = "action";  //assume "alamo" is near begining of subset "the"
  string t3 = "first";
  long double res, scalar = 1000000000.0;
  FreqTablePointer p = getTablePointer(2);

  cout << "table size: " << p->size() << endl;

  clock_gettime(CLOCK_MONOTONIC,&begin);
  res = getConditionalProbability(q,t1,p);
  clock_gettime(CLOCK_MONOTONIC,&end);
  //cout << "runtime: " << ((long double)(end.tv_sec - begin.tv_sec) + ((long double)(end.tv_nsec - begin.tv_nsec) / (long double)1,000,000,000)) << endl;
  cout << "1 runtime: " << ((long double)(end.tv_nsec - begin.tv_nsec) / scalar) << endl;
  cout << "end_sec " << end.tv_sec << "  end_nsec " << end.tv_nsec << "  start_sec " << begin.tv_sec << "  start_nsec " << begin.tv_nsec << endl;
  cout << "res: " << res << endl;


  clock_gettime(CLOCK_MONOTONIC,&begin);
  res = getConditionalProbability(q,t2,p);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "2 runtime: " << ((long double)(end.tv_sec - begin.tv_sec) + ((long double)(end.tv_nsec - begin.tv_nsec) / scalar)) << endl;
  cout << "end_sec " << end.tv_sec << "  end_nsec " << end.tv_nsec << "  start_sec " << begin.tv_sec << "  start_nsec " << begin.tv_nsec << endl;
  cout << "res: " << res << endl;

  clock_gettime(CLOCK_MONOTONIC,&begin);
  res = getConditionalProbability(q,t3,p);
  clock_gettime(CLOCK_MONOTONIC,&end);
  cout << "res: " << res << endl;
  cout << "runtime: " << ((long double)(end.tv_sec - begin.tv_sec) + ((long double)(end.tv_nsec - begin.tv_nsec) / scalar)) << endl;
  cout << "end_sec " << end.tv_sec << "  end_nsec " << end.tv_nsec << "  start_sec " << begin.tv_sec << "  start_nsec " << begin.tv_nsec << endl;
}


// getSubsetBounds(f,&buf[key],mid,&start,&end); //output params return start and end of this subset, per this key
//find bounds and store the in output parameters start and end.
//this is currently just a runtime experiment. none of this means anything unless we sort the COCA files by key, which are only weakly sorted.
//also, key search is currently only for 2-gram files (single word match)
// NOT TESTED. This was an i/o experiment only.
void nGram::getSubsetBounds(int fdes, char* key, U64 mid, U64* start, U64* end)
{
  char buf[2048]; //this could be based on avg subset span for optimality
  U64 up, down;
  int i, j, n, linebrk;
  bool done;
  //char upstream[2048];
  //char downstream[2048];

  //read bidirectonally upstream and downstream in file. look upstream for first instance of key, downstream for first instance of non-key.
  //spool to first occurence of key, upstream in file. keys left-anchored at pattern "[0-9]\t". TODO: this is currently broken, since the keys are not fully sorted.
  up = mid;
  while(up > 0){

      //set current segment position
      up -= 2048;
      if(up < 0){ up = 0; }

      //read segment
      n = pread(fdes,buf,2048,up);
      //SEEK TO FIRST NEWLINE. This is necessary to verify we get a bounded sample, eg, no disjoint keys
      linebrk = findChar(buf,'\n',256);
      up += linebrk;
      n  -= linebrk;

      //scan this buffer
      //spool left to first newline
      while(buf[i] != '\n' && i > 0){
        i--;
      }
      *start = i;
      //spool right through digits and tab
      while(isdigit(buf[i]) || buf[i] == '\t'){
        i++;
      }
      //spool to next tab char, then chop there
      j = i + 1;
      while(buf[j] != '\t'){
        j++;
      }
      buf[j] = '\0';
      //at current key, so peform key compare
      if(strcmp(&buf[i],key) != 0){
        done = true;
      }
      else{
        i = *start - 8; //set i back to newline (minus a bunch of chars)
      }
    up -= 2048;
  }

  //spool to first instance of non-key, downstream in file.
  down = mid + 16; //add a few chars so we skip current key
  while(n > 0){
    n = pread(fdes,buf,2048,down);
    down += n;

      //regex: '\n[0-9,\t].*'
      //spool right to first newline
      while(buf[i] != '\n'){
        i++;
      }
      *end = i;
      //spool right through digits and tab
      while(isdigit(buf[i]) || buf[i] == '\t'){
        i++;
      }
      //spool to next tab char, then chop there
      j = i + 1;
      while(buf[j] != '\t'){
        j++;
      }
      buf[j] = '\0';
      //at current key, so peform key compare
      if(strcmp(&buf[i],key) != 0){
        done = true;
      }
      else{
        i = *end + 8; //set i back to newline (minus a bunch of chars)
      }
      down -= 2048;
    }
}



/*
  Given a test string, we run through the ngrams in the string and lookup
  each ngram in one of our ngram tables built on training data. Misses are
  zero or smoothed.  

  *This function is dynamic, and updates the training model based on the test input as it goes.


  We may want to factor in certain measurements here again, such as word/phrase counts.
  This doesn't need to chop up the buffer as it reads, but may as well in order to share code-base
  with parsing in updateNgramTable().

  Returns: -1 if error, else returns (number of hits / number of n-gram strings * 100%)

void nGram::testSample_Contextual(char buf[BUFSIZE], int nModel, long double& gramCt, long double& rawHitCt, long double& realHitCt)
{
  int nPhrases, nWords, hits, nSubStrs, i, j, k, dummy;
  char* phrases[MAX_PHRASES_PER_READ];
  char* words[MAX_WORDS_PER_PHRASE];
  string gramBuf;
  string nextWord;
  list<gramTuple> sortedHits;
  map<string,int>* freqTable;

  freqTable = getTablePointer(nModel);
  if(freqTable == NULL){
    cout << "updateNgramTable(): table ptr not found" << endl;
    return;
  }

  nullifyTokens(phrases,MAX_PHRASES_PER_READ);
  nPhrases = tokenize(phrases, buf, this->phraseDelimiters);
  if(nPhrases > (MAX_PHRASES_PER_READ - 1)){
    cout << "WARNING: MAX_PHRASES_PER_READ exceeded in updateNgramTable(), sampled ignored" << endl;
    return;
  }
  if(nPhrases < 2){
    cout << "WARN anomalous nPhrases >" << nPhrases << "< in updateFreqFile. phrases[0]: >" << phrases[0] << "<" << endl;
    //cin >> i;
  }

  //cout << "testing sample..." << endl;
  for(i = 0; i < nPhrases; i++){

    //cout << "tokenizing phrase[" << i << "]>>" << phrases[i] << endl;
    nullifyTokens(words,MAX_WORDS_PER_PHRASE);
    nWords = tokenize(words, phrases[i], this->wordDelimiters);
    wordCt += nWords;
    if(nWords > (MAX_WORDS_PER_PHRASE - 1)){
      cout << "WARNING: MAX_WORDS_PER_PHRASE exceeded in updateNgramTable(), sample aborted before completion" << endl;
      continue;
    }

    if(nWords >= nModel){

      //count number of ngram-strings in this phrase, then look up each one in existing ngram table
      nSubStrs = nWords - nModel + 1;
      gramCt += nSubStrs;
      for(j = 0; j < nSubStrs; j++){

        //build query string of n-1 grams
        gramBuf.clear();
        sortedHits.clear();
        gramBuf = words[j];
        for(k = 1; k < (nModel - 1); k++){
          gramBuf += " ";
          gramBuf += words[k + j];
        }
        nextWord = words[k];
        //cout << "gramStr >" << gramBuf << "< actual next word: >" << nextWord << "<" << endl;
        //cin >> dummy;

        hits = queryTable(gramBuf, sortedHits, nModel);
        if(hits){
          gramBuf += " ";
          gramBuf += nextWord;

          //track raw hits: occurs when the model predicts the most likely word is in fact the next word.
          //this gives us our "raw" measure of accuracy
          rawHitCt += getBooleanHitCount(sortedHits,gramBuf);

          //track soft hits: occurs when the model predicts the correct word in its results, but it is not the most likely word
          //this gives us our more continuous measure of accuracy
          realHitCt += getRealHitCount(sortedHits,gramBuf);

          (*freqTable)[gramBuf]++;  //we continue to learn from test data, so update model
        }
      }
    }
  }

  nextWord.clear();
  gramBuf.clear();
  sortedHits.clear();
}

//OBSOLETE: not up to speed with multimaps
*/

/*
  Calculates the sub-entropy for a subset of some n-gram model. We first build a list of
  all the n-grams matching some query of n-1 grams, and calculate the entropy of the result set.
  This gives us some sort of value of the result set, such that we can weigh this set
  of results against results from other models at higher granularity.

  "Result set": the set of all n-grams from some model for which all n-1 grams match that
  of our n-1 legnth query.  So if our query to a 3-gram model is "the cat", this function
  gathers all 3-grams beginning with "the cat", and calculates the entropy within that subset.
  
  
  Returns: The entropy for a subset of n-grams from some model, or INF_ENTROPY if the query
  is not in our model.

long double nGram::getSubEntropy(const string query, int nModel, long double* sum)
{
  int hits;
  long double entropy = INF_ENTROPY;
  list<gramTuple> sortedHits;
  MapSubset subset;
  FreqTablePointer freqTable;

  freqTable = getTablePointer(nModel);
  if(freqTable == NULL){
    cout << "getSubEntropy(): table ptr not found" << endl;
    return 0.0;
  }

  subset = freqTable->equal_range(query);
  

  



  //get result set
  if( queryTable(query, sortedHits, nModel) > 0 ){
    //printGramList(sortedHits);
    entropy = getSubEntropy(sortedHits, sum);
  }
  else{
    cout << "WARN queryTable(q=" << query << ",nmodel=" << nModel << ") returned -1 in getSubEntropy(). INF_ENTROPY returned." << endl;
  }

  sortedHits.clear();

  return entropy;
}
*/

/*
  Inner utility for outer driver getSubEntropy(string query).
  Given we already have some result list (a subset of n-grams from
  some model), iterate over them and calculate their entropy.:getSubEn

long double nGram::getSubEntropy(list<gramTuple>& subset, long double *sum)
{
  long double sumFreq, sumEntropy, probX;
  list<gramTuple>::iterator it;

  //get the sum of all gram counts in result list
  for(it = subset.begin(), sumFreq = 0.0; it != subset.end(); ++it){
    sumFreq += it->frequency;
  }

  if(sumFreq <= 0.0){
    cout << "ERROR sumItems <= 0.0, divZero aborted, returning INF_ENTROPY" << endl;
    return INF_ENTROPY;
  }

  *sum = sumFreq;

  //sum up the entropy: Entropy = -1 * Sigma[ p(x) * lg( p(x) ) ]
  for(it = subset.begin(), sumEntropy = 0.0; it != subset.end(); ++it){
    probX = ((long double)it->frequency) / sumFreq;
    if(probX <= 0.0){   //error check: log(x) undefined for x <= 0 (should never occur, unless perhaps underflow, NAN, INF, etc occur)
      cout << "ERROR computation error in getSubEntropy(). probX==" << probX << endl;
    }
    else{
      sumEntropy += ( probX * log2l(probX) );
    }
  }

  //Prevents returning -0.0 when a subset has only one member (the entropy of such sets is defined as zero)
  if(sumEntropy != 0.0){
    sumEntropy *= -1;
  }

  return sumEntropy;
}
*/

/*
  Description: Takes in a range iterator over a bunch of map elements and
  returns the iterator pointing at the maximum-frequency element in this subset.

  This function shall ALWAYS return a value, since every subset has some max value.
  When multiple elements have the maximum frequency, the first occurrence is returned.

  Sidenote: The necessity for this function is that multimap values are not stored in any sorted
  order. They are stored in the order of insertion, but even that is standard-specific and should not
  be relied on.
*/
SubkeyIt nGram::getMaxFromRange(const MapSubset* range)
{
  SubkeyIt it, max;
  long double maxFreq = 0;

  if(range->first == range->second){
    cout << "WARN range->first == range->second in getMaxFromRange()" << endl;
    return range->first;
  }
  
  for(max = it = range->first, maxFreq = 0; it != range->second; ++it){
    if(maxFreq < it->second){
      max = it;
      maxFreq = it->second;
    }
  }

  if(max == range->second){
    //should never hit this, since max must always have a value
    cout << "ERROR <iterator>max == range->second in getMax(range)." << endl;
    max = range->first;
  }

  return max;
}



/*
  This returns whether or not a set of results is a raw hit. This occurs foremost likely prediction
  given by the model matches the actual next word. As such, this measure is all-or-nothing: either
  the most likely prediciton matches the actual next word, or it does not, and we don't care if the
  correct result is located somewhere lower in the results.

  From the user's perspective, this is the only measure of accuracy, since we don't display lesser-likely
  results to them. However, its actually a very rough measure of the model's actual accuracy. If a hit
  is contained in the results, but is simply not the most likely prediction, it is still important
  information with respect to the model's accuracy.
*/
long double nGram::getBooleanHitCount(const MapSubset* range, const string& actual)
{
  SubkeyIt max;

  if(range->first == range->second){
    cout << "ERROR range->first==range->second in getBoolHitCt()" << endl;
    return 0.0;
  }

  max = getMaxFromRange(range);
  if(max->first == actual){
    cout << "HIT! next and actual: " << max->second << "/" << actual << endl;
    return 1.0;
  }

  return 0.0;
}

/*
  This function finds a soft hit in some set of results. Our model returns of string predictions, sorted
  by descending probability. Raw hits occur when the model predicts that the max-most likely next word
  is the same as the actual next word. However, this is a poor measure of the model's accuracy, when the
  results list contains the actual next word and it is simply not the most likely word in the results. This function looks
  through the entire results list for the actual next word, so we can get more of a weighted result.

  The old scoring function simply returne the inverse of a the actual word's ranking in the results.
  So the score for a given actual word is 1 divided by the index+1 of that result.
  For instance, if the actual word is the first (most probable) result in the list, then its score
  is 1/1. If the actual word is the second result, then its score is 1/2; if the actual word is the
  fifty-seventh result, then its score is 1/57.

  This scoring gave us a rapidly diminishing score of 1/n, which may not be the most statistically-accurate
  measure (better measures might encompass the entropy of the set, the probability distribution within the set,
  and the distance from the correct result). But its pessimism is in-line with our user-constraints, since 
  from their perspective the value of results diminishes rapidly based on the relative position of words in
  our results, not their internal / relative probability. Other possibilities for scoring might look
  something like Okapi BM25 scoring.

  The current scoring function instead scales the results according to the max frequency. This way, if the
  predicted word is the same as the actual next word, the score is 1. But if some other word W in the result
  set is the actual next word, then we give it a score of frequency(W) / frequency(Max), where Max is the most
  likely word in the results. If a word is not in the result set, we assume we're going to learn, and it will be inducted
  in. Thus we give it a smoothed LaPlace based value slightly smaller than the least likely element in the set.
  This value is 1/ (leastFrequency(W) + 1). Better smoothing methods exist,  whose intent is to estimate the
  probability distribution of missing data.

  Returns: The probability of encountering some word in the subset, as scaled by the probability of the most likely
  word in the subset. If the word is not in the subset, then the return value is 1 divided by the frequency of the least
  likely word in the subset + 1.
*/
long double nGram::getRealHitCount(const MapSubset* range, const string& actualWord)
{
  SubkeyIt it;
  long double maxFreq = range->first->second;
  long double hitFreq = -1.0, ret = 0.0, lowFreq = 9999999.0;

  if(range->first == range->second){
    cout << "ERROR range->first==range->second in getContHitCt()" << endl;
    return 0.0;
  }

  //iterate over entire subrange, gathering stats and storing hit's frequency (if found)
  for(it = range->first; it != range->second; ++it){

    //track the max frequency value in this subset
    if(maxFreq < it->second){
      maxFreq = it->second;
    }
    //track the lowest frequency value so we can use it for smoothing estimation
    if(lowFreq > it->second){
      lowFreq = it->second;
    }

    //store the hit's frequency, if hit is found in range
    if(it->first == actualWord){
      hitFreq = it->second;
    }
  } //end loop

  //found a hit, so scale it by the maximum frequency, giving its relative value
  if((hitFreq > 0.0) && (maxFreq > 0.0)){
    ret = hitFreq / maxFreq;
  }
  /*
  else{
    ret = 1 / (lowFreq + 1);  //a really simple (and most likely dumb) smoothing method, La Place Add-One
  }
  */

  return ret;
}

/*
  Given some set of predictions (a sorted list) from one of our linear models, assign a score based
  on the position of the actual next word within the set of next words predicted by a model/method.

  Uses same scoring function as function above, getRealHitCount(). Could be optimized by pre-sorting list, meaning
  the max most likely item is at the front of the list. This will prevent iterating the entire list every time.
  
  This could use some more natural scoring. For instance, if our "hit" window is whether or not the actual
  next word is within the top 5 to 10 results in the list, we might score that as a boolean hit, instead of using
  some sort of pseudo-Hamming distance. Still, for research purposes this is an analytic function, so we want a real value for estimating
  the accuracy of a particular model/method, not a boolean/discrete scoring function.
*/
void nGram::scoreLinear(const string& actualWord, int lambdaSet, list< std::pair<SubkeyIt,long double> >& resultList)
{
  bool hit;
  U32 i;
  long double maxScore, lowScore, hitScore, ret;
  list< pair<SubkeyIt,long double> >::iterator it;

  //iterate over entire items in resultList looking for nextword somewhere in the list
  maxScore = -99999; lowScore = 99999; hitScore = 0.0;
  for(i = 0, it = resultList.begin(), hit = false; !hit && it != resultList.end(); ++it, i++){

    //track the max score value in this subset: not necessary if list is sorted, since we can assume the max is at front
    if(maxScore < it->second){
      maxScore = it->second;
    }

    //track the lowest score value so we can use it for smoothing estimation
    if(lowScore < it->second){
      lowScore = it->second;
    }

    //store the hit's score, if hit is found in range
    if(it->first->first == actualWord){
      hitScore = it->second;
      modelLambdas[lambdaSet].recall++;
      //DEBUG: ranked score better if closer to one, worse if nearer zero
      //cout << "HIT: " << actualWord << "  ranked score (0-1) =" << (hitFreq / maxFreq) << endl;
      hit = true;

      if(i < 7){
        modelLambdas[lambdaSet].topSevenAccuracy++;
      }
    }
  } //end loop

  //found a hit, so scale it by the maximum frequency, giving its relative value
  if((hitScore > 0.0) && (maxScore > 0.0)){
    modelLambdas[lambdaSet].realHitRate += hitScore / maxScore;
  }
/*
  else{
    ret = 1 / (lowScore + 1);  //a really simple (and most likely dumb)  Add-One smoothing method
  }
*/

  //return ret;
}


void nGram::resetLambdaHits(void)
{
  int i;

  for(i = 0; i < NLAMBDASETS; i++){
    modelLambdas[i].boolHitRate = 0.0;
    modelLambdas[i].realHitRate = 0.0;
    modelLambdas[i].recall = 0.0;
    modelLambdas[i].topSevenAccuracy = 0.0;
  }
}

/*
  Given our models are t on some training data, test them on new test data.
*/
void nGram::testModels(string& testFile, bool learn)
{
  for(int i = 2; i <= 5; i++){
    testModel(testFile,i,learn,false);
  }
}

void nGram::printHitCounts(long double gramCt)
{
  for(int i = 0; i < 4; i++){
    cout << " hits: " << modelLambdas[i].boolHitRate << " " << ((modelLambdas[i].boolHitRate / gramCt) * 100) << "%  real: ";
    cout << ((modelLambdas[i].realHitRate / gramCt) * 100) << "%  recall: " << modelLambdas[i].recall << " " << ((modelLambdas[i].recall / gramCt) * 100) << "%";
    cout << " top7: " << ((modelLambdas[i].topSevenAccuracy / gramCt) * 100) << "%" << endl;
  }
}


/*
  Given a corpus, we divide it into a 90% training set, and a 10% test set.
  Once we "train" on the training set and build our models, this function
  tests the remaining 10% against our models.

  Pre-condition: models are constructed, in-memory.

  Previous output for 1% of the enron model:
    Lambda Model testing stats:
    Num n-grams: 22905
    Real hit-count: 3703.8
    Bool hit-count: 2293
    Giving Lambda[0]:10.0109% bool accuracy  16.1703% real accuracy



*/
void nGram::testModel(const string& testSet, int nModel, bool learn, bool lineartest)
{
  int inFile, i, n, lastDelim, dummy;
  unsigned long long bytesRd;
  long double gramCt, rawHitCt, realHitCt, result, progress;
  struct stat fileInfo;
  char buf[BUFSIZE];
  string obuf(BUFSIZE,'\0');
  //debug
  //time_t tm;

  inFile = open(testSet.c_str(), O_RDONLY);
  if(inFile < 0){
    cout << "ERROR could not open input file >" << testSet << "< testModel() aborted" << endl;
    return;
  }
  if(fstat(inFile, &fileInfo)){
    cout << "ERROR fstat() call failed in testModel()" << endl;
    return;
  }

  resetLambdaHits();
  hitList.clear();

  if(!lineartest){
    cout << "Testing " << nModel << "-gram model...  " << endl;
  }
  else{
    cout << "Testing linear models..." << endl;
  }
  
  rawHitCt = gramCt = realHitCt = 0; bytesRd = 0; i = 0;
  //we read discrete phrases structures by reading 1024 bytes, then backing file position to the last delimiter
  while(n = read(inFile, buf, READ_SZ)){
    bytesRd += n;
    //i++;
    //if((i %= 2) == 0){
      progress = 101 * ((float)bytesRd / (float)fileInfo.st_size);
      //cout << "\r...  " << progress << "% complete (" << (bytesRd >> 10) << "kb read)   "  << flush;
      cout << progress << "% complete (" << (bytesRd >> 10) << "kb read) gramCt: " << gramCt << endl;
      printHitCounts(gramCt);
/*
      if(progress > 1.0){
        goto done;
      }
*/
    //}

    buf[n] = '\0';

    //return file position to index of last delimiter (the end of the last regular structure), to get a bounded sample.
    lastDelim = seekLastPhraseDelimiter(buf,n);
    if(lastDelim > -1){

      buf[lastDelim] = '\0';

      //prevents infinite loop at end of file: we will continuously wind back to last delim if not checked.
      if(n == READ_SZ){
        lseek(inFile, lastDelim - n + 1, SEEK_CUR);
      }

      //a series of pipes/filters to clean up sample, and map it into our parse semantics
      normalizeText(buf,obuf);
      //given a parsed string, look up each sub-string of length n-grams
      //HACK: TODO strcpy is just a temporary thing while I straighten out data types (char[] vs. string)
      //cout << "\Sample..." << endl;
      testSample_Raw(strncpy(buf, obuf.c_str(), BUFSIZE),nModel,gramCt,learn,lineartest);
      //cout << "obuf.size()=" << obuf.size() << endl;
    }
    else{
      cout << "WARN no phrase delimiter found in buf, input discarded: >" << buf << "< delim index: " << lastDelim << " phraseDelims: " << phraseDelimiters << endl;
      cin >> dummy;
    }

/*
    //debug
    lim++;
    if(lim > 30){
      goto done;
    }
*/
  }

  done:
  close(inFile);


  if(!lineartest){
    //verify: frequencyTable.count() == gCt == analyzeModelFile().gCt ???
    if(gramCt > 0.0){
      cout << "\n*******************************************\n" << nModel << "-gram Model testing stats:" << endl;
      cout << "Num n-grams: " << gramCt << endl;
      cout << "Bool-hit count:  " << wordModelStats[nModel].booleanAccuracy << endl;
      wordModelStats[nModel].booleanAccuracy /= gramCt;
      cout << "Boolean predictive accuracy: " << (100.0 * wordModelStats[nModel].booleanAccuracy) << "%" << endl;
      cout << "Soft-hit count:  " << wordModelStats[nModel].realAccuracy << endl;
      wordModelStats[nModel].realAccuracy /= gramCt;
      cout << "Real predictive accuracy:    " << (100.0 * wordModelStats[nModel].realAccuracy) << "%" << endl;
      cout << "model recall: (TODO get and print recall)" << endl;

/*      for(i = 0; i < 4; i++){
        modelLambdas[i].realHitRate /= gramCt;
        modelLambdas[i].boolHitRate /= gramCt;
        cout << "Lambda[" << i << "]:" << (100 * modelLambdas[i].boolHitRate) << "% bool accuracy  "
             << (100 * modelLambdas[i].realHitRate) << "% real accuracy" << endl;
      }
*/
      cout << "*******************************************" << endl;
    }
  }
  else{  //print the lambda results
      cout << "\n*******************************************\nLambda Model testing stats:" << endl;
      cout << "Num n-grams: " << gramCt << endl;
/*      cout << "Bool-hit count:  " << wordModelStats[nModel].booleanAccuracy << endl;
      wordModelStats[nModel].booleanAccuracy /= gramCt;
      cout << "Boolean predictive accuracy: " << (100.0 * wordModelStats[nModel].booleanAccuracy) << "%" << endl;
      cout << "Soft-hit count:  " << wordModelStats[nModel].realAccuracy << endl;
      wordModelStats[nModel].realAccuracy /= gramCt;
      cout << "Real predictive accuracy:    " << (100.0 * wordModelStats[nModel].realAccuracy) << "%" << endl;
*/
      for(i = 0; i < 4; i++){
        cout << "Real hit-count: " << modelLambdas[i].realHitRate << endl;
        cout << "Bool hit-count: " << modelLambdas[i].boolHitRate << endl;     
        modelLambdas[i].realHitRate /= gramCt;
        modelLambdas[i].boolHitRate /= gramCt;        
        cout << "Giving Lambda[" << i << "]:" << (100 * modelLambdas[i].boolHitRate) << "% bool accuracy  "
             << (100 * modelLambdas[i].realHitRate) << "% real accuracy" << endl;
      }
      cout << "model recall: (TODO get and print recall)" << endl;
      cout << "*******************************************" << endl;
      cout << "enter any key to view hitList: " << endl;
      cin >> i;
      for(list<string>::iterator it = hitList.begin(); it != hitList.end(); ++it){
        cout << *it << endl;
      }
  }
  //analyzeModelFile(); //could get min/max/avg words per sentence/phrase, etc., other stats, such as gCt, for verification

}

/*
  Given a query of length n-1, lookup all ngram strings prepended with that
  query and calculate the entropy of the set.


  Intuitively, this function will be for looking up queries of length n-1 in
  some n-gram model. But we should also be able to lookup n-i length queries in 
  some n-gram model (for n-1 > 0). The reason is that the marginal probability of
  queries of n-i length should be the same across models (a property we can leverage
  for model validation, eg, making sure "the cat" has the same marginal probability
  in a 2-gram model as it does for any 2+i gram model (3-gram, 4-gram, etc).

  The goal is to have this function work for all models, not to be constrained
  to a given structure. For instance, we ought to be able to lookup a 2-gram
  query in a 4-gram model, and so on.

  Pre-condition: n-gram table must be in-memory. We could also write a file i/o version.

  Exceptions: For the 1-gram model, the query function parameter will be discarded, and
  we will just evaluate the total entropy of the set.

  


nGram::subEntropy(string query, int nModel)
{




  ct = count(query,' ');
  if(ct >= nModel){
    return 
  }




}
*/


/*
  Count the occurrences of some char in a string.
*/
int nGram::countChar(const string& str, char c)
{
  int i, ct;

  for(i = 0, ct = 0; i < str.length(); i++){
    if(str[i] == c)
      ct++;
  }

  return ct;
}




