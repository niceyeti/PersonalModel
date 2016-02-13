
innerf = open("inner.txt","r")
outerf = open("outer.txt","r")

ilines = innerf.readlines()
olines = outerf.readlines()
avgDists = []


#
if len(olines) > len(ilines):

  #init the output vector
  i = 0
  lim = 99999999
  while i < len(olines):
    avgDists.append(lim)
    i+=1

  i = 0
  while i < len(olines):
    nouter = int(olines[i])
    j = 0
    while j < len(ilines):
      ninner = int(ilines[j])      
      
      if nouter > ninner:
        dist = nouter - ninner
      else:
        dist = ninner - nouter

      #update the min distance for this col
      if avgDists[i] > dist:
        avgDists[i] = dist

      j+=1
    i+=1


print "avgMinSums are: "
m_sum = 0
for n in avgDists:
  print str(n)+" "
  m_sum += n

result = m_sum / len(avgDists)


print "result is: "+str(result)



innerf.close()
outerf.close()



















