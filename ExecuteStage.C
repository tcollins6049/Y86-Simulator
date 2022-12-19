#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "MemoryStage.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
    M * mreg = (M *) pregs[MREG];
    E * ereg = (E *) pregs[EREG];
    W * wreg = (W *) pregs[WREG];

    uint64_t stat = ereg->getstat()->getOutput();
    icode = ereg->geticode()->getOutput();
    uint64_t ifun = ereg->getifun()->getOutput();
    uint64_t valA = ereg->getvalA()->getOutput();
    uint64_t valB = ereg->getvalB()->getOutput();
    dstE = ereg->getdstE()->getOutput();
    dstM = ereg->getdstM()->getOutput();
    uint64_t valC = ereg->getvalC()->getOutput();
    uint64_t aluA = galuA(icode, ereg);
    uint64_t aluB = galuB(icode, valB);
    uint64_t aluFun = alufun(ereg, icode);
    Cnd = cond(icode, ifun);

    MemoryStage * m = (MemoryStage*)stages[MSTAGE];
    dstE = gdstE(icode, Cnd, dstE);
    valE = ereg->getvalC()->getOutput();
    if (set_cc(icode, wreg, m->get_stat())) {
        valE = aluHelper(aluFun, ereg, aluA, aluB, wreg, m->get_stat());
    }
    calculateControlSignals(m->get_stat(), wreg);
    if (icode == IRRMOVQ)
        valE = aluA;
    else if (icode == IMRMOVQ || icode == IRMMOVQ || icode == IPOPQ || icode == IPUSHQ || icode == ICALL || icode == IRET)
        valE = aluB + aluA;
    else if (icode == IJXX) {
        valE = 0;
    }

    setMInput(mreg, stat, icode, Cnd, valE, valA, dstE, dstM);

    return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
    E * ereg = (E *) pregs[EREG];
    M * mreg = (M *) pregs[MREG];
    
    if (Mbubble == false) {
        mreg->getstat()->normal();
        mreg->geticode()->normal();
        mreg->getCnd()->normal();
        mreg->getvalE()->normal();
        mreg->getvalA()->normal();
        mreg->getdstE()->normal();
        mreg->getdstM()->normal();
    }
    else {
        mreg->getstat()->bubble(SAOK);
        mreg->geticode()->bubble(INOP);
        mreg->getCnd()->bubble();
        mreg->getvalE()->bubble();
        mreg->getvalA()->bubble();
        mreg->getdstE()->bubble(RNONE);
        mreg->getdstM()->bubble(RNONE);
    }
}

void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode,
                            bool Cnd, uint64_t valE,
                            uint64_t valA, uint64_t dstE, uint64_t dstM)
{
    mreg->getstat()->setInput(stat);
    mreg->geticode()->setInput(icode);
    mreg->getCnd()->setInput(Cnd);
    mreg->getvalE()->setInput(valE);
    mreg->getvalA()->setInput(valA);
    mreg->getdstE()->setInput(dstE);
    mreg->getdstM()->setInput(dstM);
    

}

uint64_t ExecuteStage::galuA(uint64_t e_icode, E * ereg){

    uint64_t e_valA = ereg->getvalA()->getOutput();
    uint64_t e_valC = ereg->getvalC()->getOutput();
    if(e_icode == IRRMOVQ || e_icode == IOPQ){
        return e_valA;
    }

    if(e_icode == IIRMOVQ || e_icode == IRMMOVQ || e_icode == IMRMOVQ){
        return e_valC;
    }

    if(e_icode == ICALL || e_icode == IPUSHQ){
        return -8;
    }

    if(e_icode == IRET || e_icode == IPOPQ){
         return 8;
    }
    return 0;
}

uint64_t ExecuteStage::galuB(uint64_t e_icode, uint64_t e_valB){
    if(e_icode == IRMMOVQ || e_icode == IMRMOVQ || e_icode == IOPQ || e_icode == ICALL || e_icode == IPUSHQ || e_icode == IRET || e_icode == IPOPQ ){
        return e_valB;        
    }
    return RNONE;
}

uint64_t ExecuteStage::alufun(E * ereg, uint64_t e_icode){
     uint64_t e_ifun = ereg->getifun()->getOutput();
     if(e_icode == IOPQ){
         return e_ifun;
     }
     return ADDQ;
}

bool ExecuteStage::set_cc(uint64_t e_icode, W * wreg, uint64_t m_stat){ 
    bool m_stat_cond = false;
    bool w_stat_cond = false;
    uint64_t w_stat = wreg->getstat()->getOutput();
    if (m_stat == SADR || m_stat == SINS || m_stat == SHLT) {
       m_stat_cond = true;
    }
    if (w_stat == SADR || w_stat == SINS || w_stat == SHLT) {
        w_stat_cond = true;
    }
    
    return ((e_icode == IOPQ) && (!m_stat_cond) && (!w_stat_cond));
}

uint64_t ExecuteStage::gdstE(uint64_t e_icode, bool m_Cnd, uint64_t e_dstE){
    if(e_icode == IRRMOVQ && !m_Cnd){
        return RNONE;
    }
    return e_dstE;
}

void ExecuteStage::ccHelper(uint64_t icode, bool cnd, uint64_t valE, W * wreg, uint64_t m_stat){
    if(set_cc(icode, wreg, m_stat)){  
        ConditionCodes * cc = ConditionCodes::getInstance();
        bool error;
        cc -> setConditionCode(cnd, OF, error);
        if (valE == 0) {
            cc -> setConditionCode(true, ZF, error);
        }
        else {
            cc -> setConditionCode(false, ZF, error);
        }
        if ((Tools::sign(valE)) == 1) {
            cc -> setConditionCode(true, SF, error);
        }
        else {
            cc -> setConditionCode(false, SF, error);
        }
   }
}

uint64_t ExecuteStage::aluHelper(uint64_t temp, E * ereg, uint64_t valA, uint64_t valB, W * wreg, uint64_t m_stat){
    
    bool cnd;
    uint64_t icode = ereg->geticode()->getOutput();
    uint64_t valE = 0;
    if(temp == ADDQ){
        valE = valA + valB;
        cnd = Tools::addOverflow(valA, valB);
        ccHelper(icode, cnd, valE, wreg, m_stat);
        bool error = false;
    }
    else if(temp == SUBQ){
        valE = valB - valA;
        cnd = Tools::subOverflow(valA, valB);
        ccHelper(icode, cnd, valE, wreg, m_stat);
    }
    else if(temp == XORQ){
        valE = valA ^ valB;
        ccHelper(icode, false, valE, wreg, m_stat);
    }
    else if(temp == ANDQ){
        valE = valA & valB;
        ccHelper(icode, false, valE, wreg, m_stat);
    }
    return valE;
}

uint64_t ExecuteStage::get_dstE() {
    return dstE;
}
uint64_t ExecuteStage::get_valE() {
    return valE;
}  

bool ExecuteStage::cond(uint64_t icode, uint64_t ifun) {
   ConditionCodes * cc = ConditionCodes::getInstance();
   bool error;
   bool sf = cc -> getConditionCode(SF, error);
   bool of = cc -> getConditionCode(OF, error);
   bool zf = cc -> getConditionCode(ZF, error);
   if (icode != IJXX && icode != ICMOVXX) {
       return 0;
   }else if(ifun == UNCOND || (icode == IRRMOVQ && ifun == 0)) {
       return 1;
   }else if(ifun == LESSEQ) {
       return ((sf ^ of) | zf);
   }else if(ifun == LESS){
       return (sf ^ of);
   }else if(ifun == EQUAL){
       return zf;
   }else if(ifun == NOTEQUAL){
       return !zf;
   }else if(ifun == GREATER){
       return ((!(sf ^ of)) && !zf);      
   }else if(ifun == GREATEREQ){
       return (!(sf ^ of));
   }
   return 0;
}

void ExecuteStage::calculateControlSignals(uint64_t m_stat, W * wreg) {
    uint64_t w_stat = wreg->getstat()->getOutput();
    if ((m_stat == SADR || m_stat == SINS || m_stat == SHLT) || (w_stat == SADR || w_stat == SINS || w_stat == SHLT)){
        Mbubble = true;
    }
    else {
        Mbubble = false;
    }
}

bool ExecuteStage::get_Cnd(){
    return Cnd;
}

uint64_t ExecuteStage::get_icode() {
    return icode;
}

uint64_t ExecuteStage::get_dstM() {
    return dstM;
}






