CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o PipeRegField.o PipeReg.o F.o D.o E.o M.o W.o ExecuteStage.o MemoryStage.o \
DecodeStage.o FetchStage.o WritebackStage.o Simulate.o Memory.o RegisterFile.o ConditionCodes.o Loader.o Tools.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h Loader.h RegisterFile.h ConditionCodes.h PipeReg.h Stage.h Simulate.h  

PipeRegField.o: PipeRegField.h

PipeReg.o: PipeReg.h

F.o: PipeRegField.h PipeReg.h F.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h

E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h E.h Status.h

M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h M.h Status.h

W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h W.h Status.h

ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h Status.h Debug.h Instructions.h FetchStage.h DecodeStage.h MemoryStage.h

MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h MemoryStage.h Status.h Debug.h Instructions.h FetchStage.h DecodeStage.h ExecuteStage.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h DecodeStage.h Status.h Debug.h Instructions.h FetchStage.h ExecuteStage.h MemoryStage.h

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h FetchStage.h Status.h Debug.h Instructions.h DecodeStage.h ExecuteStage.h MemoryStage.h

WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h WritebackStage.h Status.h Debug.h Instructions.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Simulate.h RegisterFile.h ConditionCodes.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h 

ConditionCodes.o: ConditionCodes.h Tools.h 

Loader.o: Loader.h Memory.h

Tools.o: Tools.h 


clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh

