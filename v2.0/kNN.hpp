#include <list>
#include <map>
#include <unordered_map>
#include <unordered_set> //use these for result duplicate subkey filtering
#include <vector>
#include <iostream>
#include <fstream>
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
#include "stdlib.h"
#include <sys/resource.h>


//defines max foreseeable line length in the freqTable.txt database
#define MAX_LINE_LEN 256

#define MAX_WORD_LEN 27 //determined by looking up on the internet. There are english words over 28 chars, but very uncommon.

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
#define NGRAMS_IN_QSTR 1  //for knn, this is the number of words in the query string, which may be required as order-independent (eg, qstr word1+word2 == word2+word1)
#define NGRAMS_IN_POSTSTR 3
#define U32MAX 0xFFFFFFFF  // ~4 billion
#define U64MAX 0xffffffffffffffff  // ~a very large numbr
#define DBG 0
#define MAX_SUM_WORD_DIST 99999 //effectively infinity
//#define CODIST_DIAMETER 256
#define CODIST_DIAMETER 32
#define CONTEXT_RADIUS 15  //num previous context words for relating semantic distance
#define CLUSTER_RADIUS 5  //size of radius for estimating which cluster we're currently within
#define KMEANS 1000
#define MAX_RAND_MEANS 100
#define MATH_PI 3.14159265358979323846F
#define K_NEIGHBORS 100  //modify this parameter as an input to mergeDuplicates



//using namespace std;
using std::cout;
using std::getline;
using std::endl;
using std::string;
using std::vector;
using std::cin;
using std::map;
using std::unordered_map;
using std::multimap;
using std::unordered_set;
using std::list;
using std::sort;
using std::flush;
using std::pair;
using std::pow;
using std::ios;
using std::fstream;
using std::isnormal;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long int U64;

typedef struct scoreVector{
  double nPredictions;
  long double realAccuracy;
  long double boolAccuracy;
  long double predictionProximity; //this needs work. It can only be assessed using probability values, which I prefer not using due to normalization overhead
  long double topN;
  long double avgResultSize; //avg count of vectors in prediction pool (more the merrier with kNN)
  long double avgWordLen; //a means of monitoring possible KSR. Track the average word length of correctl-predicted words.
  long double recall;
}ScoreVector;

typedef struct Delta{
  long double semanticDist; //semantic distance between two word vectors
  U16 clusterNumber;
  //U8 hrDist;            //distance between datetimes; ignored for now
  //U8 dayDist;
  //U8 monthDist;
  //U8 yearDist;
}delta;


//vector for a personal model, derived from enron data for a prolific emailer. each
//vector example is a conditioning word, the next three words, and a list of addressees
typedef struct personalVector{
  vector<U16> to;       //store personal targets as U16, instead of string
  //string preStr;
  string next3;
  U16 count;
}PersonalVector;



/*
  encode DATETIME as byte-byte-byte in u32 as hr-day-mo-yr 
  where range of each is mo[1-12]:day[1-31]:hr[1-24]:yr[offset_from_1970]  ALL NON-ZERO values. I use zero as a flag for no-date-time-found
*/
class DateTime{
  public:
    DateTime(){
      hr = day = month = year = 255;
    }
    DateTime(U8 h, U8 d, U8 m, U8 y){
      hr = h;
      day = d;
      month = m;
      year = y;
    }
    DateTime& operator=(const DateTime& right){
      if(this != &right){
        hr = right.hr;
        day = right.day;
        month = right.month;
        year = right.year;
      }
      return *this;
    }
    ~DateTime(){}
    
    //roughly diffs two times. month, day, and year don't seem that informative.
    static void diffDateTimes(Delta& d, const DateTime& curDT, const DateTime& srcDT);
    
    U8 hr;
    U8 day;
    U8 month;
    U8 year;
};

typedef struct attributeVector{
  //DateTime dt;
  string postStr;
  U16 count;
}AttributeVector;
typedef AttributeVector attribVec;

typedef map<string,vector<PersonalVector> > PersonalTable;
typedef PersonalTable PersonalVectorTable;
typedef PersonalTable::iterator PersonalTableIt;
typedef vector<PersonalVector>::iterator PersonalVectorIt;
typedef pair<PersonalVectorIt,double> ResultPair;
typedef list<ResultPair > PersonalResultList;
typedef PersonalResultList::iterator PersonalResultListIt;


typedef map<string,vector<attribVec> > VectorTable;
typedef map<string,vector<attribVec> >::iterator VecTableIt;
typedef map<string,vector<attribVec> >::iterator VectorTableIt;
typedef vector<attribVec> VecList;
typedef VecList::iterator VectorListIt;
typedef VecList::iterator VecListIt;
typedef list<pair<VecListIt,Delta> > ResultList;
typedef ResultList::iterator ResultListIt;
typedef unordered_map<U32, unordered_map<U32,double> > CoDistanceTable;  //these were unordered_map's, which were faster, at little extra mem size
typedef CoDistanceTable::iterator CoDistOuterIt;
typedef unordered_map<U32,double>::iterator CoDistInnerIt;
typedef unordered_map<string,pair<U32,vector<U32> > > IndexTable;
typedef IndexTable::iterator IndexTableIt;
typedef unordered_map<U32,string> IdTable;  //this complicates things massively, but reduces mem consumption
typedef IdTable::iterator IdTableIt;
typedef unordered_map<string,U32> WordTable;
typedef map<string,U16> ClusterMap;
typedef ClusterMap::iterator ClusterMapIt;
typedef map<U16,string> ClusterIdMap;
typedef ClusterIdMap::iterator ClusterIdMapIt;
typedef U32 wordKey;


class kNN{
  private:
    map<string,U16> emailIds;
    map<U16,string> emailStrs;
    U16 emailIdCounter;
    PersonalTable personalVectorTable;
    CoDistanceTable personalCoDistTable;
    U32 buildWordIndexTable(IndexTable& wordTable, const vector<string>& wordSequence);


    ScoreVector modelScore;
    ScoreVector personalScore;
    string general_dbfile;
    string personal_dbfile;
    string fileDelimiter;
    string phraseDelimiters;
    string wordDelimiters;
    string delimiters;
    string rawDelimiters;
    char wordDelimiter;
    char phraseDelimiter;
    U32 idCounter;
    U64 vecCt;
    U64 numwords;
    bool validDecimalChars[256];

    //vector<string> wordMeans; //holds the mean words after k-means. These are the k-words most strongly connected with other words in model
    VectorTable vectorTable;
    unordered_set<string> stopWords;
    ClusterMap generalMeans;
    ClusterIdMap generalMeansIds;
    ClusterIdMap personalMeansIds;
    ClusterMap personalMeans;
    U16 clusterIdCounter;
    //string meansFile;
    //string kMeansFile;
    // stores the average min co-distance between words a and b
    CoDistanceTable CoDistTable; //expensive to build, but not to store. Best to preprocess this based on research
    IndexTable wordIndexTable;  //an intermediate table for calculating distances and other utilities (k-means)
    //a nuisance, but this is intended to reduce memory size. its just a hack for creating a middleman
    //between strings (many chars) and U32 ids, such that the tables only need to store U32 size keys.
    IdTable wordIdTable;
    WordTable idWordTable;

    //all these belong in their own class
    bool hasColon(const char buf[], int len);
    int findChar(const char buf[], char c, int len);
    void scrubHyphens(string& istr);
    bool isPhraseDelimiter(char c);
    bool isWordDelimiter(char c);
    void toLower(string& myStr);
    void toLower(char buf[BUFSIZE]);
    void seqToLower(vector<string>& wordSequence);
    void delimitText(string& istr);
    int seekLastPhraseDelimiter(const char buf[BUFSIZE], int len);
    void rawPass(string& istr);
    void finalPass(string& buf);
    void normalizeText(char ibuf[BUFSIZE], string& obuf);
    int tokenize(char* ptrs[], char buf[BUFSIZE], const string& delims);
    bool isDelimiter(const char c, const string& delims);
    bool isValidWordExtended(const string& token); //new as of kNN
    bool isValidWordExtended(const char* word); //overload of above foo()
    bool isValidWord(const string& token); //new as of kNN
    bool isValidWord(const char* word); //overload of above foo()
    bool isInt(const string& nstr);
    void initValidDecimalChars(void);
    bool isDecimal(const string& nstr);

    //personal model stuff, specific to enron models
    string emailIdToStr(U16 id);
    U16 emailStrToId(const string& addr);
    U16 emailStrToId(const char* addr);


    //k-means stuff
    string findNearestNeighbor_HardMax(const string& word, const vector<string>& means, CoDistanceTable& coDistTable);
    string findNearestNeighbor_SoftMax(const string& word, const vector<string>& means, CoDistanceTable& coDistTable);
    void findTopNeighbors(const string& word, const vector<string>& means, list<pair<string,double> >& neighbors, CoDistanceTable& coDistTable);
    //void variedMeans(int k); //lets the means vary according to which word goes to which mean the strongest
    void buildClusterModels(const string& clusterFile);
    //void kMeansSoft(int k, IndexTable& wordTable, CoDistanceTable& coDistTable, const string& muFile, const string& outfile);
    void cosineClustering(IndexTable& wordTable, CoDistanceTable& coDistTable, int convergence, const string& muFile, const string& outfile);
    void writeMeans(int fd, vector<string>& means);
    bool hasMean(const string& word, const vector<string>& means);
    void initMaxMeans(vector<string>& means, IndexTable& wordTable);
    void initMeans(const int k, vector<string>& means);
    void initRandomMeans(const int k, vector<string>& means);
    void printMeans(const vector<string>& means);
    //double getClusterLikelihood(U16 currentCluster, const vector<string>& context, int i,  CoDistanceTable& coDistTable);
    double getClusterLikelihood(U16 currentCluster, const vector<string>& context, ClusterMap& clusterMeans, ClusterIdMap& clusterIds, int i, CoDistanceTable& coDistTable);
    //U16 estimateCurrentCluster(const vector<string>& context, int i, CoDistanceTable& coDistTable); // returns likeliest cluster given a large context window
    U16 estimateCurrentCluster(const vector<string>& context, int i, ClusterMap& clusterMeans, CoDistanceTable& coDistTable);
    U16 estimateClusterNo(const string& postStr, ClusterMap& clusterMeans, CoDistanceTable& coDistTable);
    //U16 estimateClusterNo(const string& postStr, CoDistanceTable& coDistTable);  // returns likeliest cluster mean given some vector of 3 words
    U16 getClusterLikelihood(U16 currentCluster, const string& postStr);

    //word similarity measures
    long double method_Poisson1(long double n, long double nA, long double nB, long double nAB);
    long double method_Poisson2(long double n, long double nA, long double nB, long double nAB);
    long double method_LogLikelihood(long double n, long double nA, long double nB, long double nAB);
    long double buildSimilarityTable(CoDistanceTable& coOccurrenceTable, CoDistanceTable& similarityTable);
    double method_binaryCosine(CoDistOuterIt it1, CoDistOuterIt it2);
    //double method_weightedCosine(CoDistOuterIt it1, CoDistOuterIt it2);
    double method_weightedCosine(CoDistOuterIt& it1, CoDistOuterIt& it2, U16 min, U16 max);

    //nearest neighbor stuff
    void mergeDuplicates(ResultList& sortedResults);
    void mergeTopKDuplicates(ResultList& sortedResults);
    void pruneWordSequence(const vector<string>& inVec, vector<string>& outVec, IndexTable& wordTable);
    char* getLine(int openfd, char buf[], int len);
    void buildStopWordTable(const string& stopWordFile);
    bool isStopWord(const string& word);
    bool isStopWord(const char* word);
    DateTime searchDateTime(int iFile);
    DateTime strToDateTime(char dtstr[]);
    void dStopWordTable(const string& stopWordFile);
    U32 updateVectorTable(vector<string>& wordVec, DateTime utime);
    void putVectors(const string& fname);
    void getVectors(const string& fname);
    void consolidateGeneralExamples(void);
    void consolidatePersonalExamples(PersonalTable& exampleVectors);
    void insertVector(const string& key, attribVec& vec);
    void initWordIdTable(void);
    void textToWordSequence(const string& trainingFile, vector<string>& wordSequence, bool filterStopWords);
    U32 buildWordIndexTable(const vector<string>& wordSequence);
    void validateWordKeyTables(void);
    void clearWordIndexTable(void);
    U32 allocId(const string& word);
    U32 allocId(const string& word, IndexTableIt it);
    string idToStr(const U32 id);
    U32 strToId(const string& word);
    void wordToIdSequence(vector<string>& wordSequence, vector<U32> idSequence);

    void printTopWords(int n);
    void copyVecs(const vector<string>& src, vector<string>& dest);
    void buildDummyWordIndexTable(void);
    U32 getMeanFrequency(IndexTable& wordTable);
    void pruneWordIndexTable(IndexTable& wordTable);
    //U32 getAvgMinDist(IndexTableIt& outer, IndexTableIt& inner, vector<U32>& minDist);
    bool CoDistTableHasKey(CoDistanceTable& coDistTable, const U32 key1, const U32 key2, CoDistInnerIt& it);
    bool CoDistTableHasKey(CoDistanceTable& coDistTable, const string& k1, const string& k2, CoDistInnerIt& it);
    void updateCoDistTable(CoDistanceTable& coDistTable, const string& key1, const string& key2);
    void coDistTableToFile(CoDistanceTable& coDistTable, string& fname);
    void appendToOutputString(string& ostr, const string& k1, const string& k2, const string& distStr);
    void predict(vector<string>& wordSeq, int wordIndex, const DateTime& dt, ResultList& resultList);
    //void predict(const string& w1, const DateTime& dt, ResultList& resultList);
    void scorePrediction(const string& actual, ResultList& resultList);
    void scorePersonalPrediction(const vector<string>& sequence, int i, PersonalResultList& resultList, ScoreVector& score);
    void getGeneralExamples(const string& w1, ResultList& resultList);
    void getPersonalExamples(const string& w1, PersonalResultList& resultList);
    //void rankResults(const string& w1, const DateTime& curDT, ResultList& resultList);
    void rankPersonalResults(vector<string>& context, int i, PersonalResultList& results, U16 currentCluster, CoDistanceTable& coDistTable);
    void rankResults(vector<string>& context, int wordIndex, ResultList& resultList, U16 currentCluster, ClusterIdMap& meanIds, CoDistanceTable& coDistTable);
    long double getSumDistance(const string& w1, const string& targets, CoDistanceTable& coDistTable);
    long double getClusterDistance(U16 clusterNo, const string& postStr, ClusterIdMap& meanIds, CoDistanceTable& coDistTable);
    //U32 getSimilarity(const string& w1, const string& w2); //old static version: returned nAB
    double getSimilarity(const string& w1, const string& w2, CoDistanceTable& coDistTable);
    int getSynonyms(const string& q, list<pair<U32,double> >& synonyms, int k, CoDistanceTable& coDistTable);
    U32 getSynonym(const string& q, CoDistanceTable& coDistTable);
    //double getSumSimilarity(const U32 w1);
    long double getSumSimilarity(const string& w1, CoDistanceTable& coDistTable);
    long double calculateSignificance(const U32 k1, const U32 k2, IndexTable& wordTable, CoDistanceTable& coDistTable);
    void clearScore(ScoreVector& score);

  public:
    kNN();
    ~kNN();
    void writeClusters(const string& meansFile, map<string,vector<string> >& clusters, vector<string>& means);
    void train(const string& trainingFile);

    //train a personal model. this uses different data model, which is a nuisance
    void trainPersonal(const string& trainingFile, const string& rawWordsFile);
    void printTopWords(IndexTable& wordTable);
    void InsertPersonalVector(const string& pre, PersonalVector& v);

    //given input of three words, predict the next most likely three words
    void predictPersonal(vector<string>& wordSequence, int i, vector<U16> targetList, U16 currentCluster, PersonalResultList& results, CoDistanceTable& coDistTable);
    string predict(const string& query);  //predict a string of length 3-words, based on nearest-neighbor analysis
    void buildCoDistTable(CoDistanceTable& coDistTable, vector<string>& wordSequence);
    void buildCoDistTable(vector<string>& wordSequence);
    void buildCoDistTableFromFile(CoDistanceTable& coDistTable, const string& fname);
    void printTopCollocates(void);
    void testEnronParsed(const string& dir);
    void testModel(const string& testFile);
    void testPersonalModel(const string& fname);
    void printScore(ScoreVector& scoreVec) const;
    void recallMeansFromFile(const string& clusterFile, ClusterMap& clustermap, ClusterIdMap& meanIds, U16& counter);
};





























