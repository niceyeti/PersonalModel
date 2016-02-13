
"""
Just splits a test corpus into 90/10 training/testing data.
"""


ifile = open("./allsent.txt","r")
ofile1 = open("./germanyTest.txt","w+")
ofile2 = open("./germanyTraining.txt","w+")

lines = ifile.readlines()

i = 0
while i < int(0.90 * len(lines)):
  ofile1.write(lines[i])
  i+=1



while i < len(lines):
  ofile2.write(lines[i])
  i+=1


ifile.close()
ofile2.close()
ofile1.close()


















