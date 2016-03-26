/***
Keenan Johnstone - 11119412 - kbj182

March 2nd, 2016
**/

RUNNING:
run the "make" command in the same folder as the source files and Makefile, then run 
the following

gobackSend hostname port timeout(ms) windowSize	-for the sender for Part A
gobackRecv errorProbability						-for the Reciever for Part A

STATUS:

part A: 			Complete, see known bugs and Design Document
part B: 			Incomplete, see design Doc for implemantation plan (time constraints), functionally similar to part A currently.
Written Questions: 	Complete

Known Bugs:
In part A with the gobackSend and gobackRecv there is a bug where in if the reciever gets out of sync with the sender, there is no way to get back in sync unless the message being sent HAPPENS to be the same as the one being acknowledged.

Limitations:
Maximum of 100 messages is allowed.
The above bug.

If the user is fast and responsed before the timeout, there are no errors. Once timeouts begin happening errors with becoming out of sync/order occur, see design doc for solutions, didnt have enough time to fix them.

So under ideal conditions this is functional.