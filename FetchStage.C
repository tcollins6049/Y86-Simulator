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
#include "FetchStage.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"
#include "Tools.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   uint64_t f_pc = 0, icode = 0, ifun = 0, valC = 0, valP = 0;
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;

   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.
   f_pc = selectPC(freg, mreg, wreg);
   Memory * m = Memory::getInstance();
   bool imem_error;
   uint8_t ibyte = m->getByte(f_pc, imem_error);
   icode = Tools::getBits(ibyte, 4, 7);
   ifun = Tools::getBits(ibyte, 0, 3);
  
   valP = PCincrement(f_pc, need_reglds(icode), need_valC(icode));
   if (need_reglds(icode)) {
        uint64_t id = getReglds(f_pc);
        rA = Tools::getBits(id, 4, 7);
        rB = Tools::getBits(id, 0, 3);
   }
   
   if (need_valC(icode)) {
      valC = buildValC(icode, f_pc);
   }

   freg->getpredPC()->setInput(PredictPC(icode, valC, valP));
   stat = f_stat(icode, imem_error);
   
   if (imem_error) {
      icode = INOP;
      ifun = FNONE;
   }

   DecodeStage * d = (DecodeStage*)stages[DSTAGE];
   ExecuteStage * e = (ExecuteStage*)stages[ESTAGE];
   MemoryStage * mm = (MemoryStage*)stages[MSTAGE];
   calculateControlSignals(e->get_icode(), d->get_icode(), mm->get_icode(), e->get_dstM(), d->getd_srcA(), d->getd_srcB(), e->get_Cnd());

   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
    F * freg = (F *) pregs[FREG];
    D * dreg = (D *) pregs[DREG];
    if (f_stall != true) {
        freg->getpredPC()->normal();
    }
    if (d_bubble) {
       dreg->getstat()->bubble(SAOK);
       dreg->geticode()->bubble(INOP);
       dreg->getifun()->bubble();
       dreg->getrA()->bubble(RNONE);
       dreg->getrB()->bubble(RNONE);
       dreg->getvalC()->bubble();
       dreg->getvalP()->bubble();   
    }
    else if (!d_stall) {  
         dreg->getstat()->normal();
         dreg->geticode()->normal();
         dreg->getifun()->normal();
         dreg->getrA()->normal();
         dreg->getrB()->normal();
         dreg->getvalC()->normal();
         dreg->getvalP()->normal();
    }
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}

uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg) {
   uint64_t m_icode = mreg->geticode()->getOutput();
   uint64_t m_cnd = mreg->getCnd()->getOutput();
   uint64_t m_valA = mreg->getvalA()->getOutput();
   uint64_t w_icode = wreg->geticode()->getOutput();
   uint64_t w_valM = wreg->getvalM()->getOutput();
   uint64_t f_predPC = freg->getpredPC()->getOutput();

   if (m_icode == IJXX && !m_cnd) {
      return m_valA;
   }
   if (w_icode == IRET) {
      return w_valM;
   }
   return f_predPC;
}

bool FetchStage::need_reglds(uint64_t f_icode) {
   if (f_icode == IRRMOVQ || f_icode == IOPQ || f_icode == IPUSHQ || f_icode == IPOPQ || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ) {
         return true;
      }
   else {
      return false;
   }
}

uint64_t FetchStage::getReglds(uint64_t f_pc) {
    Memory * m = Memory::getInstance();
    bool imem_error;
    uint64_t rbyte = m->getByte(f_pc + 1, imem_error);
    return rbyte;
}
bool FetchStage::need_valC(uint64_t f_icode) {
   if (f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || f_icode == IJXX || f_icode == ICALL) {
      return true;
   }
   else {
      return false;
   }
}

uint64_t FetchStage::buildValC(uint64_t f_icode, uint64_t f_pc) {
    Memory * m = Memory::getInstance();
   bool imem_error;
   if (f_icode == IJXX || f_icode == ICALL) {
      uint8_t arr[8];
      arr[0] = m->getByte(f_pc + 1, imem_error);
      arr[1] = m->getByte(f_pc + 2, imem_error);
      arr[2] = m->getByte(f_pc + 3, imem_error);
      arr[3] = m->getByte(f_pc + 4, imem_error);
      arr[4] = m->getByte(f_pc + 5, imem_error);
      arr[5] = m->getByte(f_pc + 6, imem_error);
      arr[6] = m->getByte(f_pc + 7, imem_error);
      arr[7] = m->getByte(f_pc + 8, imem_error);
      return Tools::buildLong(arr);
   }
    if (need_valC(f_icode)) {
        uint8_t arr[8];
        arr[0] = m->getByte(f_pc + 2, imem_error);
        arr[1] = m->getByte(f_pc + 3, imem_error);
        arr[2] = m->getByte(f_pc + 4, imem_error);
        arr[3] = m->getByte(f_pc + 5, imem_error);
        arr[4] = m->getByte(f_pc + 6, imem_error);
        arr[5] = m->getByte(f_pc + 7, imem_error);
        arr[6] = m->getByte(f_pc + 8, imem_error);
        arr[7] = m->getByte(f_pc + 9, imem_error);
        return Tools::buildLong(arr);
    }
    return 0;
}

uint64_t FetchStage::PredictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP) {
   if (f_icode == IJXX || f_icode == ICALL) {
      return f_valC;
   }
   
   return f_valP;
   
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool nr, bool nv) {
   if (nr) {
      f_pc = f_pc + 1;
   }
   if (nv) {
      f_pc = f_pc + 8;
   }
   f_pc = f_pc + 1;
   return f_pc;
}

bool FetchStage::instr_valid(uint64_t f_icode) {
    if (f_icode == INOP || f_icode == IHALT || f_icode == IRRMOVQ || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ
        || f_icode == IOPQ || f_icode == IJXX || f_icode == ICALL || f_icode == IRET || f_icode == IPUSHQ || f_icode == IPOPQ) {
        return true;
    }
    return false;
}

uint64_t FetchStage::f_stat(uint64_t f_icode, bool mem_error) {
    if (mem_error) return SADR;
    if(!(instr_valid(f_icode))) return SINS;
    if (f_icode == IHALT) return SHLT;
    return SAOK;
}
 
bool FetchStage::F_stall(uint64_t e_icode, uint64_t d_icode, uint64_t m_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB) {
    return ((e_icode == IMRMOVQ || e_icode == IPOPQ) && (e_dstM == d_srcA || e_dstM == d_srcB)) || (IRET == d_icode || IRET == e_icode || IRET == m_icode); 
}
bool FetchStage::D_stall(uint64_t e_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB) {
    return (e_icode == IMRMOVQ || e_icode == IPOPQ) && (e_dstM == d_srcA || e_dstM == d_srcB);
}
void FetchStage::calculateControlSignals(uint64_t e_icode, uint64_t d_icode, uint64_t m_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB, bool e_Cnd) {
    f_stall = F_stall(e_icode, d_icode, m_icode, e_dstM, d_srcA, d_srcB);
    d_stall = D_stall(e_icode, e_dstM, d_srcA, d_srcB);
    D_bubble(e_icode, d_icode, m_icode, e_dstM, d_srcA, d_srcB, e_Cnd);
}

void FetchStage::D_bubble(uint64_t e_icode, uint64_t d_icode, uint64_t m_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB, bool e_Cnd) {
    //d_bubble = ((e_icode == IJXX) && !e_Cnd) || ((!((e_icode == IMRMOVQ || e_icode == IPOPQ)) && (e_dstM == d_srcA || e_dstM == d_srcB))) && (IRET == d_icode || IRET == e_icode || IRET == m_icode);
    d_bubble = ((e_icode == IJXX) && !e_Cnd) || ((!((e_icode == IMRMOVQ || e_icode == IPOPQ) && (e_dstM == d_srcA || e_dstM == d_srcB))) && (d_icode == IRET || e_icode == IRET || m_icode == IRET));
}
 
 
 
 
 
 
 
     
