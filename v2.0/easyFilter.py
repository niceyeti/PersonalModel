import sys


if len(sys.argv) < 2: 
  print "ERROR no file passed"

fname = sys.argv[1]
ifile = open(fname,"r")
ofile = open("./germanyRawTest.txt","w")

lines = ifile.readlines()
for line in lines:
  if "EOF" not in line[0:4]: 
    if "BODY:" not in line[0:6]:
      if "DATE:" not in line[0:6]:
        if "TO:" not in line[0:4]:
          if "SUBJECT:" not in line[0:9]:
            ofile.write(line)

ifile.close()
ofile.close()








