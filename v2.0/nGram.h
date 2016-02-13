#include <list>
#include <map>
#include <unordered_set> //use these for result duplicate subkey filtering
#include <vector>
#include <iostream>
#include <cctype>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <wait.h>
#include <utility>
#include <cstdio>
#include <cstring>
#include <string>
#include <algorithm>
#include <cmath>
#include <sys/time.h>
#include <sys/resource.h>


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

//using namespace std;
using std::cout;
using std::getline;
using std::endl;
using std::string;
using std::vector;
using std::cin;
using std::map;
using std::multimap;
using std::unordered_set;
using std::list;
using std::sort;
using std::flush;
using std::pair;
using std::pow;

//TODO: use/map these
enum tableIndices{ NIL, ONE_GRAM, TWO_GRAM, THREE_GRAM, FOUR_GRAM, FIVE_GRAM, SIX_GRAM };
enum testDataIndices{RAW_HITS, REAL_HITS, RAW_LAMBDA_HTS, REAL_LAMBDA_HITS};

typedef unsigned long int U64; //whether or not this is actually a 64-bit uint depends on architecture
typedef unsigned int U32; // may be U64, depending on sys

/*
  A small class containing a gramString and its
  associated frequency. This is used for file processing
  convenience, but this class could easily grow to encompass
  other statistical features of an nGram string.
typedef struct gramTuple{
  //public:
    string gramString;
    int frequency;
    //bool compare(gramTuple& t1, gramTuple& t2);
} GramTuple;
*/

typedef struct freqTuple{
  string nextWord;
  long double freq;
} FreqTuple;

typedef struct keyPair{
  string primaryKey;
  string subKey;
} KeyPair;

typedef struct lambdaSet{
  long double l[NLAMBDAS];
  long double boolHitRate; //some hit counts are real-valued, instead of discrete. For instance, we may want to track if some result set contains the correct nextWord, though it is not the most likely word.
  long double realHitRate;
  long double recall;         //recall tracks if next word is anywhere in result set: bool := nextWord in resultList[]
  long double topSevenAccuracy; //tracks if nextWord is in the top seven results, a typical user-satisfaction window
} LambdaSet;

typedef struct modelStat{
  long double sumFrequency;
  long double totalEntropy;               //raw entropy across a single model. Though seemingly meaningless for anything but 1-gram models, total entropy gives us a sparsity-measure for other n-gram models for n>1.
  long double expectedSubEntropy;  //subentropy of a model is defined as the summation of entropy w/in each n-1 gram subset multiplied by its probability
  long double meanSubEntropy;       //more or less meaningless, since its only a raw mean. expectedSubEntropy (an expected value) provides a more meaningful measure of subEntropy.
  long double totalPerplexity;            //recall that by definition, perplexity is duplicate data, since perplexity = 2^(entropy(x)) for some x
  long double expectedSubPerplexity;
  long double booleanAccuracy;
  long double realAccuracy;
} ModelStat;



/*
typedef std::pair<std::multimap<string,freqTuple>::iterator, std::multimap<string,freqTuple>::iterator> MapSubset;
typedef std::multimap<string,freqTuple>::iterator KeyIt;
typedef std::multimap<string,freqTuple>* FreqTablePointer;
typedef std::pair<string,freqTuple> MapPair;
*/

//typedef std::pair<map<string,map<string,U32> >, map<string,map<string,U32> > > MapSubset;
typedef std::pair<map<string,long double>::iterator,map<string,long double>::iterator> MapSubset;
typedef map<string,map<string,long double> >::iterator KeyIt;
typedef map<string,long double>::iterator SubkeyIt;
typedef map<string, map<string,long double> >* FreqTablePointer;
typedef std::pair<string,freqTuple> MapPair;
//typedef std::pair<map<string,U32>::iterator,bool> mapRet;  // map.insert() returns a pair like this. may be needed in the future for error-checking, if insert() fails

//comparison struct necessary for multimaps
struct mapcmp{
  bool operator()(const string* s1, const string* s2){
    return strcmp(s1->c_str(),s2->c_str()) < 0;
  }
};


//global functions
bool compare(freqTuple t1, freqTuple t2);

    /*a static number of frequency tables is okay, since in NLP we're only generally interested in up to about 5-gram models,
      although this might be better as a vector of tables, to extend a dynamic range of n-gram models */
class nGram{

  private:

    map<string, map<string,long double> > threeGramTable;
    map<string, map<string,long double> > fourGramTable;
    map<string, map<string,long double> > oneGramTable;
    map<string, map<string,long double> > twoGramTable;
    map<string, map<string,long double> > fiveGramTable;
    map<string, map<string,long double> > sixGramTable;    
    map<string, map<string,long double> > oneGramPosTable;
    map<string, map<string,long double> > twoGramPosTable;
    map<string, map<string,long double> > threeGramPosTable;
    map<string, map<string,long double> > fourGramPosTable;
    map<string, map<string,long double> > fiveGramPosTable;
    
    map<string,string> contractionModel;
    map<string,string> abbreviationModel;

    vector<string> cocaNgramFiles;  //stores relative path to filenames of pos-tagged coca n-grams

    //HACK: this is a temp for testing, and is not meaningful for the class. A global list for tracking hit words for some algorithm.
    list<string> hitList;



    //had a few segfaults caused by iterating beyond the bounds of these objects.
    LambdaSet modelLambdas[NLAMBDASETS];
    long double testStats[4];
    ModelStat wordModelStats[6];
    ModelStat posModelStats[6];

    string fileDelimiter;
    string phraseDelimiters;
    string wordDelimiters;
    string delimiters;
    string freqFile;
    string rawDelimiters;
    char wordDelimiter;
    char phraseDelimiter;
    U64 wordCt;
    U64 phraseCt;

    bool rawParse;     //parsing flags. contextual parsing is potentially obsolete. these flags reflect coupling between the model and parsing, which need to be refined
    bool contextualParse;
    bool isCocaBuilt;
    bool normalized;  //TODO: this flag probably belongs in the model stat struct, since it could be a property of individual models, not just all of them

    //user interaction
    void induceLanguage(void);
    bool isValidQuery(const string& query, int nModel);
    void userMainMenu(void);
    void getQueryParameters(string& word, int& nModel, int& nPredictions);
    void getQueryParameters(string& queryStr, int& nPredictions);
    void userQueryMenu(void);
    int  userGetInt(string prompt);

    //utilities
    void printFreqTuple(const string& prefix, freqTuple& tup);
    void printGramList(const string& prefix, list<freqTuple>& gramList);
    bool gramTableHasKey(string gramString, FreqTablePointer tablePtr);
    FreqTablePointer getTablePointer(int i);
    FreqTablePointer getPosTablePointer(int i);
    //given a word, get a list of the n-grams beginning with that word, descent-sorted by freq

    //sequence parsing methods
    void rtrim(char buf[]);
    void filterCocaLine(char buf[]);
    bool isNumericStr(const string& s);
    void finalPass(string& buf);
    bool getWordBounds(const string& ibuf, int i, string wordDelims, int* left, int* right);
    void mapContractions(string& ibuf, string& obuf);
    int countChar(const string& str, char c);
    void toLower(string& myStr);
    int getIthIndexOf(char c, int ith, const string str);
    void nullifyString(char buf[], int len);
    void nullifyString(string buf);
    void nullifyTokens(char* toks[], int ntoks);
    bool parseCorpus(string ifile, string ofile);
    bool isDelimiter(const char c, const string& delims);
    char* getLine(int openfd, char buf[], int len);
    int findChar(const char buf[], char c, int len);
    void scrubHyphens(string& istr);
    bool isPhraseDelimiter(char c);
    bool isWordDelimiter(char c);
    void toLower(char buf[BUFSIZE]);
    void delimitText(string& istr);
    int seekLastPhraseDelimiter(const char buf[BUFSIZE], int len);
    void rawPass(string& istr);
    void normalizeText(char ibuf[BUFSIZE], string& obuf);
    int tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims);
    void clearStrTokens(char* toks[], int len);
    int seekNthLastTokenIndex(int nth, const char buf[BUFSIZE], int len);
    void mapAbbreviations(string& ibuf, string& obuf);
    bool buildAbbreviationModel(void);

    //model, training, and db stuff
    void normalizeTable(FreqTablePointer table);
    void normalizeOneGramTable(FreqTablePointer table);
    MapSubset equalRange(FreqTablePointer table, const string& q);
    void buildPosFreqTuple(FreqTuple* tup, char* toks[], int nModel);
    void buildFreqTuple(FreqTuple* tup, char* toks[], int nModel);
    bool buildCocaPosTable(const string& fname, int nModel); //TODO: not built
    bool tableToCompressedCocaFile(int nModel); //TODO: Not built. Remains to be seen if its useful to have 15-20% smaller files, or just to use coca format as is.
    void calculateModelStats(void);
    bool buildContractionModel(void);
    bool abbreviationTableHasKey(const string& key);
    int updateNgramTable(char buf[BUFSIZE], int len, int nModel, const string& delims);
    int updateNgramTable_Contextual(char buf[BUFSIZE], int len, int nModel); //obsolete
    //unsigned long long tableToFile(const char* outfile, int nModel);  //needs to be rebuilt. more of a user-application foo()
    int fileToTable(const char* fname, int nModel);
    string getMaxFromFile(char* fname, string word);

    //builds ngram model based on COCA ngram files, destroying previous model
    bool buildCocaModel(int nModel);
    bool buildCocaOneGramModel(FreqTablePointer twoGramTable, FreqTablePointer oneGramTable);
    //raw, corpus-based model building methods. Not yet integrated with coca models.
    bool buildModel(string parsedFile, int nModel);
    bool build_N_Models(string parsedFile, int nModels);
    void queryModel(const string& query, int nModel, int nPredictions, list<freqTuple>& resultList);
    void queryAllModels(const string& query, int nPredictions, list<freqTuple>& resultList);

    //data model access
    void resetLambdaHits(void);
    U64 getUniqueKeys(list<string>& uniqueKeys, FreqTablePointer table);
    string buildKey(char* toks[], int nModel);
    string buildPosKey(char* toks[], int nModel);
    long double getConditionalProbability(const string& query, const string& nextWord, FreqTablePointer table);
    long double getOneGramProbability(string query, FreqTablePointer table);
    SubkeyIt getMaxFromRange(const MapSubset* range);
    void makeKeyPair(string& gramString, KeyPair* keys);
    int queryTrainingTable(const string& query, FreqTablePointer table, int nModel, int nPredictions, list<freqTuple>& resultList);
    //string getMaxFromTable(const string query, int nModel);
    SubkeyIt incrementKey(string gramStr, int nModel); //instead of subkeys, these should return a pair<subkey,bool> to detect failure, as map.insert() does
    //void incrementKey(string gramStr, int nModel);
    SubkeyIt updateKeyVal(MapPair& keyValPair, FreqTablePointer table);

    //analysis and testing
    long double getSubEntropy(KeyIt outer, bool normalized);
    long double getSubEntropy(const string& query, FreqTablePointer table);
    long double getSumFrequency(FreqTablePointer table);
    //word prediction
    string predictWord(const string& query, int lambdaSet);
    string predictWord_BoolStaticLinear(const string& query, int lambdaSet);
    int predictWord_RealStaticLinear(const string& query, int lambdaSet, list< std::pair<SubkeyIt,long double> >& resultList);
    int predictWord_RealDynamicLinear(const string& query, list< std::pair<SubkeyIt,long double> >& resultList);
    string predictWord_BoolDynamicLinear(const string& query);
    void scoreLinear(const string& actualWord, int lambdaSet, list< std::pair<SubkeyIt,long double> >& resultList);
    
    long double getExpectedSubPerplexity(FreqTablePointer table, long double expectedSubEntropy);
    long double getTotalPerplexity(FreqTablePointer table, long double totalEntropy);
    long double getExpectedSubEntropy(FreqTablePointer table);
    long double getMeanSubEntropy(FreqTablePointer table);
    //long double getSubEntropy(list<gramTuple>& subset, long double* sum);
    //long double getSubEntropy(const string query, int nModel, long double *sum);
    int getFrequency(string gramString, FreqTablePointer tablePtr);
    double smooth(int docLen, int numGrams, int numSubGrams);  //if freq(gramstr)==0, use some smoothing method to correct
    void testWord(int nModel, string nQuery, string nextWord);
    void testSample_Raw(char buf[BUFSIZE], int nModel, long double& gramCt, bool learn, bool lineartest);
    void testSample_Contextual(char buf[BUFSIZE], int nModel, long double& gramCt, long double& rawHitCt, long double& realHitCt);
    long double getRealHitCount(const MapSubset* range, const string& actualWord);
    long double getBooleanHitCount(const MapSubset* range, const string& actual);
    void getSubsetBounds(int fdes, char* key, U64 mid, U64* start, U64* end);

    //learning and other analytic stuff
    void lambdaEM(int lambdaset, const string& trainingFile);
    void deletedInterpolation(int lambdaSet, const string& trainingFile);
    void deletedInterpDriver(char buf[BUFSIZE], long double pxn[6]);

  public:

    nGram();
    ~nGram();
    //void testDriver(void);
    //get a list of the next predicted n-grams of length numPredictions
    bool isValidSeed(const string& seed);
    void userDriver(void);
    void unitTest(void);
    void buildCocaModels(void);
    void buildModels(string corpus);
    void testQBinSearch(void);
    void testGetGramsFromTable(void);
    long double getTotalEntropy(FreqTablePointer table);
    //long double getTotalEntropy_OneGram(FreqTablePointer table);
    //void generateFromTrainingModel(string& seed, int nModel);  //needs to be fixed, in rebuild of training components
    void generateFromLambdaModels(string& seed, bool staticModel, int lambdaSet);
    void testTwoGramLookupTime(void);
    void printHitCounts(long double gramCt);
    void printModelStats(void);
    void printLambdas(void);
    void trainLambdas(void);
    void initLambdas(void);
    void testModels(string& testFile, bool learn);
    void testPredictWord_RealDynamicLinear(void);
    void testPredictWord_RealStaticLinear(void);
    void testModel(const string& testSet, int nModel, bool learn, bool lineartest);
    void testCocaQueryTimes(void);
    void testMapMapQueryTimes(void);
};







