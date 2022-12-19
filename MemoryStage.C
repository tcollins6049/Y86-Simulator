#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];

    stat = mreg->getstat()->getOutput();
    icode = mreg->geticode()->getOutput();
    uint64_t valE = mreg->getvalE()->getOutput();
    uint64_t dstE = mreg->getdstE()->getOutput();
    uint64_t dstM = mreg->getdstM()->getOutput();
    uint64_t valA = mreg->getvalA()->getOutput();
    valM = 0;
    
    Memory * mem = Memory::getInstance();
    bool error;
    uint64_t address = addr(icode, mreg);
    //printf("ADDRESS %x\n", address);
    if (read(icode)) {
        valM = mem -> getLong(address, error);
    }
    if (write(icode)) {
        mem -> putLong(valA, address, error);
    }

    if (error) {
        stat = SADR;
    }
    setWInput(wreg, stat, icode, valE, valM, dstE, dstM);
    return false;
   
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
    W * wreg = (W *) pregs[WREG];
    M * mreg = (M *) pregs[MREG];

    wreg->getstat()->normal();
    wreg->geticode()->normal();
    wreg->getvalE()->normal();
    wreg->getvalM()->normal();
    wreg->getdstE()->normal();
    wreg->getdstM()->normal();
}

void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode,
                            uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM)
{
    wreg->getstat()->setInput(stat);
    wreg->geticode()->setInput(icode);
    wreg->getvalE()->setInput(valE);
    wreg->getvalM()->setInput(valM);
    wreg->getdstE()->setInput(dstE);
    wreg->getdstM()->setInput(dstM);
    

}

uint64_t MemoryStage::addr(uint64_t m_icode, M * mreg) {
    if (m_icode == IRMMOVQ || m_icode == IPUSHQ || m_icode == ICALL || m_icode == IMRMOVQ) {
       return mreg->getvalE()->getOutput();
    }else if(m_icode == IPOPQ || m_icode == IRET) {
        return mreg->getvalA()->getOutput();
    }
    return 0;
}
bool MemoryStage::read(uint64_t m_icode) {
    if (m_icode == IMRMOVQ || m_icode == IPOPQ || m_icode == IRET) {
        return true;
    }
    return false;
}
bool MemoryStage::write(uint64_t m_icode) {
    if (m_icode == IRMMOVQ || m_icode == IPUSHQ || m_icode == ICALL) {
        return true;
    }
    return false;
}
uint64_t MemoryStage::get_valM() {
    return valM;
}

uint64_t MemoryStage::get_stat() {
    return stat;
}

uint64_t MemoryStage::get_icode() {
    return icode;
}

