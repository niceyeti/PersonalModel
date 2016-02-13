
ifile = "/home/jesse/Desktop/TeamGleason/src/nGram/huckleberryFinn.txt"
infile = open(ifile,"r")

lines = []
myDict = {}
words = 0
lines = 0
NGRAMS = 3
i = 0
j = 0
k = 0


lines = infile.readlines()

for line in lines:
  lineStr = line 
  tokens = lineStr.split(" ")
  i = 0
  while i < (len(tokens) - NGRAMS):

    j = 0; gramStr = ""
    while j < NGRAMS:
      gramStr = gramStr + tokens[j].lower().replace(".","").replace("?","").replace(";","").replace("!","").replace("-","").replace(",","").replace("\"","") + " "
      j += 1
    gramStr = gramStr.rstrip()

    if myDict.has_key(gramStr):
      myDict[gramStr] += 1
    else:
      myDict[gramStr] = 1

    i += 1
  print "processed: ", line

myMax = 0
myTuple = ()
for item in myDict:
  print item, myDict[item]
  if myMax < myDict[item]:
     myTuple = (item, myDict[item])
     myMax = myDict[item]

print myTuple, "<< myTuple"







