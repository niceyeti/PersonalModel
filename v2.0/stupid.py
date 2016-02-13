
ifile = open("../stopWordsExtended.txt","r")
ofile = open("../stopWordsExtended2.txt","w")

lines = ifile.readlines()
for line in lines:
  s = line.rstrip().lstrip()
  s += "\n"
  if len(s) > 0:
    ofile.write(s)

ofile.close()
ifile.close()
