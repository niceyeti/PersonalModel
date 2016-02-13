This code is written for Linux, all it is currently is a proof-of-concept for n-gram based text prediction and
other language analysis.

The project requires that you download the COCA n-gram files from here:
http://www.ngrams.info/download_coca.asp
Grab the 2, 3, 4, and 5 gram case-sensitive files with part-of-speech tags.

In the constructor, I build paths to each of the files and store them in an array of strings called 
CocaNGramFiles[], so you can also just modify those strings to point to where you put the COCA files.

Hopefully there are not too many other dependencies in order to get it running, but the code is more for
reference right now. You will need g++ with the most current C-11 standard updates, which you probly have.

There is a very simple makefile. On the command line just enter:
make -f makefile -l

And it should compile to an executable called nGram. So just use ./nGram to run it.
Pretty much the only thing you'll be able to do with it is watch it start,
build models, and finally get to the user loop in main: "main.cc enter number: (99 to quit)"
Read main.cc to look at some of the options. I think at most you'll be able to generate
language from the model (options 5 or 6), for kicks. Enter a four-word string and watch it generate
the most likely path through the Markov model. You'll immediately see some funny problems with the Markov approach.
Such as this, where I select option 5 and then enter "once upon a time", and the model hits a cycle:

main.cc enter number: (99 to quit)
5
Enter a four-word string: once upon a time
in 
the future of the country and the world of the living room 
and the kitchen table and the other hand and the other hand 
and the other hand and the other hand and the other hand 
and the other hand and the other hand and the other hand 
and the other hand and the other hand and the other hand 
and the other hand and the other hand and the other hand 
and the other hand and the other hand and the other hand 
and the other hand and the other hand and the other hand 
and the other  ... ...








