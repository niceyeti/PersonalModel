
bool byFrequency(const pair<string,long double>& left, const pair<string,long double>& right)
{
  return left.second > right.second;
}


/*
  Many of these patterns are pretty lazy, for getting some random k words in the top 200 wordset.
*/
void kNN::initMeans(int k, vector<string>& means, unordered_map<string,vector<U32> >& wordIndexTable)
{
  int i, j, r, l;
  unordered_map<string,vector<U32> >::iterator it;
  list<pair<string,long double> >::iterator listIt;
  list<pair<string,long double> > wordList; //list of words, to be ordered by probability
  list<pair<string,long double> > outputList; 

  means.clear();

  //get a list of all the unique words
  for(it = wordIndexTable.begin(); it != wordIndexTable.end(); ++it){
    pair<string,long double> p;
    p.first = it->first;
    p.second = it->second.size();
    wordList.push_front(p);
  }
  wordList.sort(byFrequency);
  //dbg. make sure the list is in descending order
  i = 0;
  for(listIt = wordList.begin(); listIt != wordList.end(); ++listIt){
    cout << (i++) << ": " << listIt->first << "|" << listIt->second << endl;
    if(i > 150){
      break;
    }
  }
  //clear all the items > than index 200
  for(i = 0, listIt = wordList.begin(); listIt != wordList.end(); ++listIt){
    outputList.push_back(*listIt);
    i++;
    if(i > 200){
      break;
    }
  }
  wordList.clear(); //done with word list

  //now grab k random means from the output list of top frequency words
  for(j = 0; (j < i) && (j < outputList.size()); j++){
    r = rand() % i;
    //spool to index k (list doesn't support [] operator)
    vecIt = outputList.begin();
    for(l = 0; (l < r) && (listIt != outputList.end()); l++){
      vecIt++;
    }
    means.push_front(vecIt->first);
  }

  //dbg
  cout << "the random means: " << endl;
  for(list<string>::iterator oit = outputList.begin(); oit != outputList.end(); ++oit){
    cout << *oit << endl;
  }
}

/*
  Given some word, find the word in means vector that it is nearest. If none is found,
  returns "NIL"
*/

string kNN::findNearestNeighbor(const string& word, const vector<string>& means)
{
  U32 temp, max = 0;
  string ret_s;
  
  for(int i = 0; i < means.size(); i++){
    temp = getDistance(word, means[i]);
    if(temp > max){
      max = temp;
      ret_s = means[i];
    }
  }

  if(max == 0){
    ret_s = "NIL";
  }

  return s;
}



/*
  Given some k, assign k random means. To increase the likelihood of these means
  having sufficient neighbors, these means are chosen from the top 20% most likely
  terms. This is to make sure these terms have the most relationships between 

  Pre-condition: CoDistance table, and 1-gram word probability tables must exist in memory.
*/
void kNN::kMeans(int k, unordered_map<string,vector<U32> >& wordIndexTable)
{
  U32 sum;
  string s, max_s;
  unordered_map<string,vector<U32> >::iterator it;
  vector<string> means;
  unordered_map<string,list<string> > kSubsets;

  kSubsets.resize(k); 

  if(CoDistTable.empty()){
    cout << "CoDistTable[][] empty in kMeans, returning" << endl;
    return;
  }
  if(wordIndexTable.empty()){
    cout << "wordIndexTable[] empty in kMeans, returning" << endl;
    return;
  }

  initMeans(k, means, wordIndexTable);

  //initial state, partition according to the random means among the most frequent (top k+200) terms
  //for each mean, partition the entire set of words according to which mean they are nearest
  for(it = wordIndexTable.begin(); it != wordIndexTable.end(); ++it){
    //for k very small (very sparse co-distance matrices) this will often return "UNK"
    kSubsets[ findNearestNeighbor(it->first,means) ].push_front(it->first);    
  }
  s = "NIL";
  cout << "sizeof unknown words for this set of initial andom  means: " << kSubsets[s].size() << endl;

  while(true){

    //until convergence, do:  
    // 1) partition word set by means
    // 2) find new set of means within each set

    //for each mean, partition the entire set of words according to which mean they are nearest
    for(it = wordIndexTable.begin(); it != wordIndexTable.end(); ++it){
      //for k very small (very sparse co-distance matrices) this will often return "UNK"
      kSubsets[ findNearestNeighbor(it->first,means) ].push_front(it->first);
    }

    //now find the "centroid" of each set of words
    //iterate the sets: O(k * n^2)
    for(subIt = kSubsets.begin(); subIt != kSubset.end(); ++subIt){
      //sum each words proximity to every other word, then take the max. this is a somewhat naive update.
      // the update could also incorporate relative probabilities into each sum, or other more advanced formulae
      // O(n^2)
      for(max = 0, j = 0; j < subIt->second.size(); j++){
        //get this word's sum-relatedness to every other word in the list
        for(sum = 0, k = 0; k < subIt->second.size(); k++){
          if(i != k){
            sum += getDistance(subIt->first,subIt->second[j]);
          }
        }
        if(sum > max){
          max = sum;
          max_s = subIt->second[j];
        }
      }
      means[i] = max_s;
    }

    //run until no changes in means, then iteratively re-run to find the most frequently-occurring means
    printMeans(means);
  }

}

void kNN::printMeans(const vector<string>& means)
{
  cout << "the means: " << endl;
  for(int i = 0; i < means.size(); i++){
    cout << means[i] << " ";
    if((i % 10) == 0)
      cout << "\n";
  }
  cout << endl;
}





