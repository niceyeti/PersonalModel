fter buildCocaOneGramTable:
1-gram table size: 53396
2-gram table size: 945976
3-gram table size: 978744
4-gram table size: 1031079
5-gram table size: 1261620
DEBUG sumFreq is: 2.75418e+08
nsubsets in this table: 53396
DEBUG sumFreq is: 1.19765e+08
nsubsets in this table: 247439
DEBUG sumFreq is: 4.2694e+07
nsubsets in this table: 475371
DEBUG sumFreq is: 1.86916e+07
nsubsets in this table: 781414
********************************Model Statistics*********************************
  H(X) is total/raw entropy over all n-grams.
  Expct. H(x) is the expected value of entropy computed for each n-1 gram subset.
  Perplexity is simply pow(2,entropy(x)) or rather 2^(H(x)).

         Raw H(X)  Expct. H(x)   Avg Expct. H(x)   Perplexity   Avg Expct. Perp.
---------------------------------------------------------------------------------
 1-gram  10.8969   NA            NA                   1906.79
    POS   -1.0000   NA            NA                      -1.00
 2-gram  17.8304   8.2849        2.1655             233073.63      311.88
    POS  -1.0000   -1.0000        -1.0000                 -1.00       -1.00
 3-gram  19.6260   5.2445        1.0829             809141.73       37.91
    POS  -1.0000   -1.0000        -1.0000                 -1.00       -1.00
 4-gram  20.2141   3.1517        0.7219            1216291.92        8.89
    POS  -1.0000   -1.0000        -1.0000                 -1.00       -1.00
 5-gram  18.8104   1.9567        0.4885             459735.64        3.88
    POS  -1.0000   -1.0000        -1.0000                 -1.00       -1.00
**********************************************************************************
in testPredictWord_RealDynamicLinear(void)
here here
DEBUG q2 q3 q4 q5 => road,the road,down the road,went down the road
subentropy vals for these queries: 10.8969,7.2928,6.26367,5.36369,0
9
working
here2  it->first/it->second.nextWord = road|a
here2  it->first/it->second.nextWord = road|about
here2  it->first/it->second.nextWord = road|access
here2  it->first/it->second.nextWord = road|accidents
here2  it->first/it->second.nextWord = road|across
here2  it->first/it->second.nextWord = road|after
here2  it->first/it->second.nextWord = road|again
here2  it->first/it->second.nextWord = road|against
here2  it->first/it->second.nextWord = road|ahead
here2  it->first/it->second.nextWord = road|all
here2  it->first/it->second.nextWord = road|along
here2  it->first/it->second.nextWord = road|and
here2  it->first/it->second.nextWord = road|are
here2  it->first/it->second.nextWord = road|around
here2  it->first/it->second.nextWord = road|as
here2  it->first/it->second.nextWord = road|at
here2  it->first/it->second.nextWord = road|away
here2  it->first/it->second.nextWord = road|back
here2  it->first/it->second.nextWord = road|became
here2  it->first/it->second.nextWord = road|because
here2  it->first/it->second.nextWord = road|before
here2  it->first/it->second.nextWord = road|behind
here2  it->first/it->second.nextWord = road|below
here2  it->first/it->second.nextWord = road|beside
here2  it->first/it->second.nextWord = road|between
here2  it->first/it->second.nextWord = road|bike
here2  it->first/it->second.nextWord = road|bikes
here2  it->first/it->second.nextWord = road|block
here2  it->first/it->second.nextWord = road|blocks
here2  it->first/it->second.nextWord = road|builders
here2  it->first/it->second.nextWord = road|building
here2  it->first/it->second.nextWord = road|but
here2  it->first/it->second.nextWord = road|by
here2  it->first/it->second.nextWord = road|called
here2  it->first/it->second.nextWord = road|came
here2  it->first/it->second.nextWord = road|can
here2  it->first/it->second.nextWord = road|closures
here2  it->first/it->second.nextWord = road|conditions
here2  it->first/it->second.nextWord = road|construction
here2  it->first/it->second.nextWord = road|could
here2  it->first/it->second.nextWord = road|course
here2  it->first/it->second.nextWord = road|courses
here2  it->first/it->second.nextWord = road|crew
here2  it->first/it->second.nextWord = road|crews
here2  it->first/it->second.nextWord = road|cut
here2  it->first/it->second.nextWord = road|down
here2  it->first/it->second.nextWord = road|during
here2  it->first/it->second.nextWord = road|dust
here2  it->first/it->second.nextWord = road|every
here2  it->first/it->second.nextWord = road|for
here2  it->first/it->second.nextWord = road|from
here2  it->first/it->second.nextWord = road|game
here2  it->first/it->second.nextWord = road|games
here2  it->first/it->second.nextWord = road|gets
here2  it->first/it->second.nextWord = road|goes
here2  it->first/it->second.nextWord = road|going
here2  it->first/it->second.nextWord = road|had
here2  it->first/it->second.nextWord = road|has
here2  it->first/it->second.nextWord = road|have
here2  it->first/it->second.nextWord = road|he
here2  it->first/it->second.nextWord = road|here
here2  it->first/it->second.nextWord = road|home
here2  it->first/it->second.nextWord = road|I
here2  it->first/it->second.nextWord = road|if
here2  it->first/it->second.nextWord = road|improvements
here2  it->first/it->second.nextWord = road|in
here2  it->first/it->second.nextWord = road|into
here2  it->first/it->second.nextWord = road|is
here2  it->first/it->second.nextWord = road|it
here2  it->first/it->second.nextWord = road|itself
here2  it->first/it->second.nextWord = road|just
here2  it->first/it->second.nextWord = road|kill
here2  it->first/it->second.nextWord = road|last
here2  it->first/it->second.nextWord = road|leading
here2  it->first/it->second.nextWord = road|leads
here2  it->first/it->second.nextWord = road|led
here2  it->first/it->second.nextWord = road|less
here2  it->first/it->second.nextWord = road|like
here2  it->first/it->second.nextWord = road|lined
here2  it->first/it->second.nextWord = road|losing
here2  it->first/it->second.nextWord = road|maintenance
here2  it->first/it->second.nextWord = road|manager
here2  it->first/it->second.nextWord = road|map
here2  it->first/it->second.nextWord = road|maps
here2  it->first/it->second.nextWord = road|may
here2  it->first/it->second.nextWord = road|movie
here2  it->first/it->second.nextWord = road|near
here2  it->first/it->second.nextWord = road|network
here2  it->first/it->second.nextWord = road|noise
here2  it->first/it->second.nextWord = road|north
here2  it->first/it->second.nextWord = road|not
here2  it->first/it->second.nextWord = road|now
here2  it->first/it->second.nextWord = road|of
here2  it->first/it->second.nextWord = road|on
here2  it->first/it->second.nextWord = road|one
here2  it->first/it->second.nextWord = road|onto
here2  it->first/it->second.nextWord = road|or
here2  it->first/it->second.nextWord = road|out
here2  it->first/it->second.nextWord = road|outside
here2  it->first/it->second.nextWord = road|over
here2  it->first/it->second.nextWord = road|past
here2  it->first/it->second.nextWord = road|project
here2  it->first/it->second.nextWord = road|projects
here2  it->first/it->second.nextWord = road|race
here2  it->first/it->second.nextWord = road|racer
here2  it->first/it->second.nextWord = road|races
here2  it->first/it->second.nextWord = road|racing
here2  it->first/it->second.nextWord = road|rage
here2  it->first/it->second.nextWord = road|ran
here2  it->first/it->second.nextWord = road|rash
here2  it->first/it->second.nextWord = road|record
here2  it->first/it->second.nextWord = road|running
here2  it->first/it->second.nextWord = road|runs
here2  it->first/it->second.nextWord = road|safety
here2  it->first/it->second.nextWord = road|salt
here2  it->first/it->second.nextWord = road|she
here2  it->first/it->second.nextWord = road|show
here2  it->first/it->second.nextWord = road|sign
here2  it->first/it->second.nextWord = road|signs
here2  it->first/it->second.nextWord = road|since
here2  it->first/it->second.nextWord = road|so
here2  it->first/it->second.nextWord = road|somewhere
here2  it->first/it->second.nextWord = road|south
here2  it->first/it->second.nextWord = road|still
here2  it->first/it->second.nextWord = road|surface
here2  it->first/it->second.nextWord = road|surfaces
here2  it->first/it->second.nextWord = road|system
here2  it->first/it->second.nextWord = road|systems
here2  it->first/it->second.nextWord = road|test
here2  it->first/it->second.nextWord = road|tests
here2  it->first/it->second.nextWord = road|than
here2  it->first/it->second.nextWord = road|that
here2  it->first/it->second.nextWord = road|the
here2  it->first/it->second.nextWord = road|there
here2  it->first/it->second.nextWord = road|they
here2  it->first/it->second.nextWord = road|this
here2  it->first/it->second.nextWord = road|through
here2  it->first/it->second.nextWord = road|to
here2  it->first/it->second.nextWord = road|today
here2  it->first/it->second.nextWord = road|together
here2  it->first/it->second.nextWord = road|toward
here2  it->first/it->second.nextWord = road|towards
here2  it->first/it->second.nextWord = road|traffic
here2  it->first/it->second.nextWord = road|trip
here2  it->first/it->second.nextWord = road|trips
here2  it->first/it->second.nextWord = road|turns
here2  it->first/it->second.nextWord = road|under
here2  it->first/it->second.nextWord = road|until
here2  it->first/it->second.nextWord = road|up
here2  it->first/it->second.nextWord = road|users
here2  it->first/it->second.nextWord = road|victory
here2  it->first/it->second.nextWord = road|warrior
here2  it->first/it->second.nextWord = road|warriors
here2  it->first/it->second.nextWord = road|was
here2  it->first/it->second.nextWord = road|we
here2  it->first/it->second.nextWord = road|went
here2  it->first/it->second.nextWord = road|were
here2  it->first/it->second.nextWord = road|west
here2  it->first/it->second.nextWord = road|when
here2  it->first/it->second.nextWord = road|where
here2  it->first/it->second.nextWord = road|which
here2  it->first/it->second.nextWord = road|while
here2  it->first/it->second.nextWord = road|who
here2  it->first/it->second.nextWord = road|will
here2  it->first/it->second.nextWord = road|win
here2  it->first/it->second.nextWord = road|winds
here2  it->first/it->second.nextWord = road|with
here2  it->first/it->second.nextWord = road|without
here2  it->first/it->second.nextWord = road|work
here2  it->first/it->second.nextWord = road|would
here2  it->first/it->second.nextWord = road|you
working
here2  it->first/it->second.nextWord = the road|a
here2  it->first/it->second.nextWord = the road|after
here2  it->first/it->second.nextWord = the road|again
here2  it->first/it->second.nextWord = the road|against
here2  it->first/it->second.nextWord = the road|ahead
here2  it->first/it->second.nextWord = the road|all
here2  it->first/it->second.nextWord = the road|and
here2  it->first/it->second.nextWord = the road|are
here2  it->first/it->second.nextWord = the road|as
here2  it->first/it->second.nextWord = the road|at
here2  it->first/it->second.nextWord = the road|back
here2  it->first/it->second.nextWord = the road|because
here2  it->first/it->second.nextWord = the road|before
here2  it->first/it->second.nextWord = the road|behind
here2  it->first/it->second.nextWord = the road|below
here2  it->first/it->second.nextWord = the road|between
here2  it->first/it->second.nextWord = the road|but
here2  it->first/it->second.nextWord = the road|by
here2  it->first/it->second.nextWord = the road|crew
here2  it->first/it->second.nextWord = the road|during
here2  it->first/it->second.nextWord = the road|for
here2  it->first/it->second.nextWord = the road|from
here2  it->first/it->second.nextWord = the road|had
here2  it->first/it->second.nextWord = the road|has
here2  it->first/it->second.nextWord = the road|he
here2  it->first/it->second.nextWord = the road|here
here2  it->first/it->second.nextWord = the road|I
here2  it->first/it->second.nextWord = the road|if
here2  it->first/it->second.nextWord = the road|in
here2  it->first/it->second.nextWord = the road|into
here2  it->first/it->second.nextWord = the road|is
here2  it->first/it->second.nextWord = the road|it
here2  it->first/it->second.nextWord = the road|itself
here2  it->first/it->second.nextWord = the road|just
here2  it->first/it->second.nextWord = the road|last
here2  it->first/it->second.nextWord = the road|leading
here2  it->first/it->second.nextWord = the road|less
here2  it->first/it->second.nextWord = the road|like
here2  it->first/it->second.nextWord = the road|map
here2  it->first/it->second.nextWord = the road|near
here2  it->first/it->second.nextWord = the road|not
here2  it->first/it->second.nextWord = the road|now
here2  it->first/it->second.nextWord = the road|of
here2  it->first/it->second.nextWord = the road|on
here2  it->first/it->second.nextWord = the road|or
here2  it->first/it->second.nextWord = the road|out
here2  it->first/it->second.nextWord = the road|so
here2  it->first/it->second.nextWord = the road|surface
here2  it->first/it->second.nextWord = the road|system
here2  it->first/it->second.nextWord = the road|than
here2  it->first/it->second.nextWord = the road|that
here2  it->first/it->second.nextWord = the road|the
here2  it->first/it->second.nextWord = the road|there
here2  it->first/it->second.nextWord = the road|they
here2  it->first/it->second.nextWord = the road|this
here2  it->first/it->second.nextWord = the road|through
here2  it->first/it->second.nextWord = the road|to
here2  it->first/it->second.nextWord = the road|today
here2  it->first/it->second.nextWord = the road|together
here2  it->first/it->second.nextWord = the road|toward
here2  it->first/it->second.nextWord = the road|towards
here2  it->first/it->second.nextWord = the road|trip
here2  it->first/it->second.nextWord = the road|under
here2  it->first/it->second.nextWord = the road|until
here2  it->first/it->second.nextWord = the road|was
here2  it->first/it->second.nextWord = the road|we
here2  it->first/it->second.nextWord = the road|were
here2  it->first/it->second.nextWord = the road|when
here2  it->first/it->second.nextWord = the road|where
here2  it->first/it->second.nextWord = the road|which
here2  it->first/it->second.nextWord = the road|while
here2  it->first/it->second.nextWord = the road|who
here2  it->first/it->second.nextWord = the road|will
here2  it->first/it->second.nextWord = the road|with
here2  it->first/it->second.nextWord = the road|without
here2  it->first/it->second.nextWord = the road|would
here2  it->first/it->second.nextWord = the road|you
working
here2  it->first/it->second.nextWord = down the road|a
here2  it->first/it->second.nextWord = down the road|and
here2  it->first/it->second.nextWord = down the road|as
here2  it->first/it->second.nextWord = down the road|at
here2  it->first/it->second.nextWord = down the road|for
here2  it->first/it->second.nextWord = down the road|from
here2  it->first/it->second.nextWord = down the road|he
here2  it->first/it->second.nextWord = down the road|here
here2  it->first/it->second.nextWord = down the road|I
here2  it->first/it->second.nextWord = down the road|if
here2  it->first/it->second.nextWord = down the road|in
here2  it->first/it->second.nextWord = down the road|is
here2  it->first/it->second.nextWord = down the road|of
here2  it->first/it->second.nextWord = down the road|on
here2  it->first/it->second.nextWord = down the road|or
here2  it->first/it->second.nextWord = down the road|that
here2  it->first/it->second.nextWord = down the road|there
here2  it->first/it->second.nextWord = down the road|they
here2  it->first/it->second.nextWord = down the road|to
here2  it->first/it->second.nextWord = down the road|toward
here2  it->first/it->second.nextWord = down the road|was
here2  it->first/it->second.nextWord = down the road|we
here2  it->first/it->second.nextWord = down the road|when
here2  it->first/it->second.nextWord = down the road|where
here2  it->first/it->second.nextWord = down the road|with
here2  it->first/it->second.nextWord = down the road|you
working
list size is: 274  Confirm first 30 results from predict word: 
the road|the|33.5277
road|the|33.5267
down the road|to|17.4767
the road|to|17.4446
road|to|17.4127
down the road|of|17.3011
the road|of|17.2925
road|of|17.29
down the road|and|16.4616
the road|and|16.4373
road|and|16.4144
down the road|a|14.7659
the road|a|14.7607
road|a|14.7584
down the road|in|11.0801
the road|in|11.0627
road|in|11.0537
down the road|that|7.29091
the road|that|7.2852
road|that|7.28038
down the road|I|6.31396
the road|I|6.31163
road|I|6.31072
down the road|is|6.08121
the road|is|6.07515
road|is|6.07098
down the road|for|5.20326
the road|for|5.19883
road|for|5.19275
down the road|was|4.96217
9
result count: 274
9
the road the|33.5277
road the|33.5267
down the road to|17.4767
the road to|17.4446
road to|17.4127
down the road of|17.3011
the road of|17.2925
road of|17.29
down the road and|16.4616
the road and|16.4373
road and|16.4144
down the road a|14.7659
the road a|14.7607
road a|14.7584
down the road in|11.0801
the road in|11.0627
road in|11.0537
down the road that|7.29091
the road that|7.2852
road that|7.28038
down the road I|6.31396
the road I|6.31163
road I|6.31072
down the road is|6.08121
the road is|6.07515
road is|6.07098
down the road for|5.20326
the road for|5.19883
road for|5.19275
down the road was|4.96217
the road was|4.96054
road was|4.95673
down the road on|4.21149
the road on|4.20729
road on|4.20512
down the road with|4.13489
the road with|4.12732
road with|4.12051
down the road you|3.58129
the road you|3.57978
road you|3.5792
the road it|3.55384
road it|3.55348
down the road as|3.36508
the road as|3.36368
road as|3.3614
down the road he|3.28496
the road he|3.28287
road he|3.28196
road have|3.07673
the road are|2.96199
road are|2.96115
down the road at|2.87988
the road at|2.87358
road at|2.87071
the road not|2.56617
road not|2.56579
down the road from|2.43002
the road from|2.40285
road from|2.39479
the road this|2.2899
road this|2.28878
the road had|2.23736
road had|2.23648
down the road they|2.19768
the road they|2.19523
road they|2.19442
the road by|2.11973
road by|2.11818
down the road or|1.87456
the road or|1.87269
road or|1.87101
down the road we|1.82838
the road we|1.82628
road we|1.82538
the road but|1.81385
road but|1.81324
the road has|1.80618
road has|1.80553
the road were|1.74253
road were|1.74182
road she|1.63872
the road who|1.62989
road who|1.62948
the road would|1.60204
road would|1.60139
road about|1.49574
road one|1.47264
road can|1.46401
the road all|1.36742
road all|1.36676
the road out|1.34228
road out|1.34196
the road will|1.32723
road will|1.32662
road up|1.22623
down the road when|1.1414
the road when|1.13545
road when|1.13386
down the road if|1.09143
the road if|1.08886
road if|1.08831
the road into|1.06849
road into|1.06729
the road which|1.0654
road which|1.06506
road could|1.05722
the road so|1.05546
road so|1.05484
the road like|1.04819
road like|1.04741
the road just|1.0311
road just|1.03041
down the road there|0.984874
the road there|0.983358
road there|0.983045
the road than|0.833044
road than|0.832442
road going|0.813313
the road because|0.752666
road because|0.752089
road over|0.695399
the road back|0.663494
road back|0.662766
road may|0.552963
down the road where|0.550624
the road where|0.548992
road where|0.54755
the road after|0.530867
road after|0.530253
the road through|0.528268
road through|0.527628
road down|0.492297
the road now|0.473431
road now|0.472929
the road last|0.430604
road last|0.430253
the road before|0.415973
road before|0.41502
road still|0.402167
road work|0.364116
road around|0.353151
the road between|0.32426
road between|0.323407
down the road here|0.31373
the road here|0.311398
road here|0.310796
road came|0.301133
the road against|0.288031
road against|0.287391
road every|0.284576
road went|0.273935
the road during|0.25826
road during|0.257883
the road while|0.25369
road while|0.253289
the road under|0.252431
road under|0.252118
the road until|0.248687
road until|0.248123
road home|0.226977
the road without|0.225507
road without|0.225118
road away|0.20679
road since|0.193908
the road less|0.187591
road less|0.187051
road across|0.182236
road along|0.167151
road show|0.165451
road called|0.158548
the road behind|0.154211
down the road toward|0.154143
road behind|0.153446
the road toward|0.146098
road toward|0.144029
road past|0.141006
the road system|0.136789
road system|0.13645
road became|0.111803
the road together|0.109653
road together|0.109339
the road again|0.100735
road cut|0.0990728
road again|0.0984274
road outside|0.0975483
road goes|0.0970445
the road today|0.0878268
road today|0.0871622
road running|0.0849511
road course|0.0837129
road led|0.0831057
the road near|0.0797099
road game|0.0795831
road near|0.0790703
road gets|0.0781875
road onto|0.0675124
road building|0.0672509
the road itself|0.0623265
road ran|0.06193
road itself|0.0618625
road test|0.0576941
road record|0.0557183
road access|0.0549929
road win|0.0538208
road turns|0.0527008
road sign|0.0521029
the road ahead|0.0513747
road ahead|0.0483399
the road leading|0.043868
road leading|0.0429525
road kill|0.0421068
road race|0.0407985
road systems|0.0405448
road project|0.0401134
the road trip|0.0397704
road runs|0.0397657
road games|0.0396076
road trip|0.0391685
road movie|0.0390109
road conditions|0.0379226
the road below|0.0350885
road below|0.0346998
road safety|0.0308339
road north|0.0306867
road beside|0.0304672
road signs|0.0303706
road manager|0.0292916
road construction|0.0292725
road south|0.0285928
road losing|0.0285744
the road surface|0.0284486
road surface|0.0280598
road leads|0.0257935
road somewhere|0.0248398
road tests|0.0240784
road victory|0.0240438
road projects|0.0233129
road network|0.0228733
the road towards|0.0219353
road towards|0.0215215
road block|0.0209984
the road map|0.0198035
road traffic|0.0195822
road salt|0.0190626
road west|0.0177263
road map|0.0172453
the road crew|0.016055
road crew|0.0157289
road courses|0.0157266
road blocks|0.0142367
road dust|0.0131877
road users|0.0130739
road lined|0.0123811
road noise|0.012033
road trips|0.0110193
road bike|0.0101959
road maintenance|0.00905332
road improvements|0.00807923
road races|0.00709981
road winds|0.00698161
road maps|0.00573135
road racing|0.00561125
road rage|0.00469918
road surfaces|0.00385686
road accidents|0.0034045
road crews|0.00325684
road bikes|0.00293974
road builders|0.00186894
road warriors|0.00168
road rash|0.0014799
road warrior|0.00111481
road racer|0.000438504
road closures|0.000406901
testing enron corpus over models...
Testing 2-gram model...  
^C
jesse@jesse-1015E:~/Desktop/TeamGleason/src/nGram/v7.4$ g++ -o nGram nGram.cc main.cc
jesse@jesse-1015E:~/Desktop/TeamGleason/s

int nGram::predictWord_RealDynamicLinear(const string& query, list<pair<MapIt,long double> >& resultList)
{
  int ngrams, i, j, nres;
  char buf[128];
  char* toks[16];
  long double pi[6], subsetEntropy[6];
  string q[6];
  MapSubset subset;
  MapIt it;
  FreqTablePtr wordTable[6]; //Ya, there are only up to 5-gram tables. But indices start at 0, so alloc'ing 6 pointers allows us to index by nModel: eg, to index to 2-gram model, use table[2]->foo()
  //FreqTablePtr posTable[6];
  //the result set for all possible subqueries in the query: the cat had, the cat was, the cat, the dog, the goldfish, the
  pair<MapIt,long double> listPair;

  if(query.length() > 128){
    cout << "ERROR query string too long in predictWord, returning 0 results, query discarded: " << query << endl;
    return 0;
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
  cout << "DEBUG q2 q3 q4 q5 => " << q[2] << "," << q[3] << "," << q[4] << "," << q[5] << endl;


//Real version with dynamic lambdas: build result list, then find next word in list and give it a weighted success score. Good for performance measurements.
  //get each subset.
//PROBABLY dont need to normalize lambdas (dynamic or static), but I need to think about this some more.
  //iterate over each subset, appending to list ordered by probability
  //1) return most likely word in list (running max, so no need for list) 2) adjusted likelihood of item in list
  subsetEntropy[1] = wordModelStats[1].totalEntropy; //1-gram model entropy is simply that of the entire model
  //for this member of this subset, get the subentropy 
  // ??? THIS DOESNT MAKE SENSE. NOTE THAT THE ENTROPY QUERIES ARE CONSTANT, AND CALCS COULD BE MOVED OUTSIDE THIS LOOP  ??
  //subsetEntropy[1] = wordModelStats[1].totalEntropy;  //look up...
  for(i = 2; i <= 5; i++){
    subsetEntropy[i] = getSubEntropy(q[i], wordTable[i]);
  }
  cout << "subentropy vals for these queries: " << subsetEntropy[1] << "," << subsetEntropy[2] << "," << subsetEntropy[3] << "," << subsetEntropy[4] << "," << subsetEntropy[5] << endl;
  cin >> i;

  //iterates over the subsets, summing the linear probability for each. notice this gives us duplicates in our result list.
  // for instance: go to the park today is summed over the 5-1 gram queries it contains, but we then do the same for "to the park today".
  // the list is sorted after being built, and experiment shows these repeat queries are closely clustered, with the 5-gram prevailing.
  // this captures results not in the 5-gram query, but remember, there are dupes in the resultList!
  for(i = 2, nres = 0; i <= 5; i++){
    cout << "working" << endl;
    subset = wordTable[i]->equal_range( q[i] );

    if(subset.first == subset.second){
      cout << "DEBUG warning: query >" << q[i] << "< not in model[" << i << "]" << endl;
    }

    //for each possible next word from the n-gram models, calculate the linear interpolation of probabilities across/within that string
    for(it = subset.first; it != subset.second; ++it){
      cout << "debug it->first/it->second.nextWord = " << it->first << "|" << it->second.nextWord << endl;
      //get and store each probability in prob vector
      pi[1] = getOneGramProbability(it->second.nextWord, wordTable[1]);
      for(j = 2; j <= i; j++){
        pi[j] = getConditionalProbability(q[j], it->second.nextWord, wordTable[j]);
        cout << "p(" << it->second.nextWord << "|" << q[j] << ") = " << pi[j] << endl;
      }
      //get the interpolated probability of this prediction in subset using dynamic lambdas
      for(pi[0] = 0.0, j = 1; j <= i; j++){
        if(subsetEntropy[j] <= 0){
          cout << "subsetEntropy[" << j << "] =" << subsetEntropy[j] << " <= 0  in predictWord_RealDynamicLinear() for query=" << q[j] << endl;
        }
        else{
          pi[0] += (pi[j] / subsetEntropy[j]);  //hell, don't even use the static lambdas for now. Let the dynamic lambdas dominate.
        }
      }
      cout << "resultant prob sum: p[0] = " << pi[0] << endl;
       
      listPair.second = pi[0];
      listPair.first = it;
      resultList.push_front( listPair );  //insert in order
      nres++;
    }
  }

  resultList.sort( compareResultPair ); //this may be unnecessary: if we run analytics over the entire list, there is no need for order
  cout << "list size is: " << resultList.size() << "  Confirm first 30 results from predict word: " << endl;
  j = 0;
  for(list<pair<MapIt,long double> >::iterator listit = resultList.begin(); listit != resultList.end() && j < 30; ++listit, j++){
    cout << listit->first->first << "|" << listit->first->second.nextWord  << "|" << listit->second << endl;
  }
  cin >> j;

  return nres;
}




































