"""
  Quick script for converting example personal files to raw words sequences. Its just way easier to runthis script
  and process this file than to write the same in c++
"""

def isFinancialDecimal(token):
  for char in token:
    if char not in "()#0987654321+-=$.,%*<>":
      return False
  return True

def isInt(token):
  for char in token:
    if char not in ",0987654321+-":
      return False
  return True

#inly captures data of form: 11/14, 11/14/2014, etc
def isDate(token):
  for char in token:
    if char not in "0987654321/\\.:-":
      return False
  return True


def filterToken(token):
  if isDate(token):
    return "DATENUMERIC"
  if isInt(token):
    return "INTNUMERIC"
  if isFinancialDecimal(token):
    return "DECNUMERIC"
  return token


ofile = open("./germanyRawWords.txt","w+")
ifile = open("./germanyTraining.txt","r")
lines = ifile.readlines()
for line in lines:
  if "EOF" not in line[0:4]:
    if "TO:" not in line[0:4]:
      if "DATE:" not in line[0:6]:
        if "SUBJECT:" not in line[0:9]:
          if "BODY:" not in line[0:5]:
            tokens = line.strip().split(" ")
            #outs = []
            #for token in tokens:
            #  outs.append(filterToken(token))
            #tokens = [filterToken(token) for token in tokens]
            #print "tokens: ",tokens
            oline = ""
            for tok in tokens:
              oline += (filterToken(tok) + " ")
            ofile.write(oline+"\n")
            print "oline: ",oline

ofile.close()
ifile.close()




