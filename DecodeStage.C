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
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ExecuteStage.h"
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
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{

    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    D * dreg = (D *) pregs[DREG];
    E * ereg = (E *) pregs[EREG];
    
    uint64_t stat = dreg->getstat()->getOutput();
    icode = dreg->geticode()->getOutput();
    uint64_t ifun = dreg->getifun()->getOutput();
    uint64_t valC = dreg->getvalC()->getOutput();
    ExecuteStage * e = (ExecuteStage*)stages[ESTAGE];
    MemoryStage * m = (MemoryStage*)stages[MSTAGE];
    DecodeStage * d = (DecodeStage*)stages[DSTAGE];

    uint64_t valA = 0, valB = 0;
    uint64_t d_dstE = RNONE, d_dstM = RNONE;
    d_srcA = RNONE;
    d_srcB = RNONE;

    d_srcA = srcA(dreg,icode);
    d_srcB = srcB(dreg,icode);
    d_dstE = dstE(dreg, icode);
    d_dstM = dstM(dreg, icode);
	
    valA = SelfwdA(d_srcA, dreg, mreg, wreg, e->get_dstE(), e->get_valE(), m->get_valM());
    valB = FwdB(d_srcB, mreg, wreg, e->get_dstE(), e->get_valE(), m->get_valM());

    uint64_t e_icode = ereg->geticode()->getOutput();
    uint64_t e_dstM = ereg->getdstM()->getOutput();
    e_bubble = E_bubble(e_icode, e_dstM, e->get_Cnd(), d->getd_srcA(), d->getd_srcB());

    setEInput(ereg, stat, icode, ifun, valC, valA, valB, d_dstE, d_dstM, d_srcA, d_srcB);
    
    return false; 
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
    M * mreg = (M *) pregs[MREG];
    W * wreg = (W *) pregs[WREG];
    E * ereg = (E *) pregs[EREG];
    D * dreg = (D *) pregs[DREG];

    if (e_bubble == false) {
        ereg->getstat()->normal();
        ereg->geticode()->normal();
        ereg->getifun()->normal();
        ereg->getvalC()->normal();
        ereg->getvalA()->normal();
        ereg->getvalB()->normal();
        ereg->getdstE()->normal();
        ereg->getdstM()->normal();
        ereg->getsrcA()->normal();
        ereg->getsrcB()->normal();
    }
    else {
        ereg->getstat()->bubble(SAOK);
        ereg->geticode()->bubble(INOP);
        ereg->getifun()->bubble();
        ereg->getvalC()->bubble();
        ereg->getvalA()->bubble();
        ereg->getvalB()->bubble();
        ereg->getdstE()->bubble(RNONE);
        ereg->getdstM()->bubble(RNONE);
        ereg->getsrcA()->bubble(RNONE);
        ereg->getsrcB()->bubble(RNONE);
    }
}

void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode,
                            uint64_t ifun, uint64_t valC, uint64_t valA,
                            uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
    ereg->getstat()->setInput(stat);
    ereg->geticode()->setInput(icode);
    ereg->getifun()->setInput(ifun);
    ereg->getvalC()->setInput(valC);
    ereg->getvalA()->setInput(valA);
    ereg->getvalB()->setInput(valB);
    ereg->getdstE()->setInput(dstE);
    ereg->getdstM()->setInput(dstM);
    ereg->getsrcA()->setInput(srcA);
    ereg->getsrcB()->setInput(srcB);

}

uint64_t DecodeStage::srcA(D * dreg, uint64_t d_icode) {
    uint64_t D_rA = dreg->getrA()->getOutput();
    if (d_icode == IRRMOVQ || d_icode == IRMMOVQ || d_icode == IOPQ || d_icode == IPUSHQ) {
        return D_rA;
    }
    else if (d_icode == IPOPQ || d_icode == IRET) {
        return RSP;
    }
    else {
        return RNONE;
    }
}

uint64_t DecodeStage::srcB(D * dreg, uint64_t d_icode) {
    uint64_t D_rB = dreg->getrB()->getOutput();
    if (d_icode == IOPQ || d_icode == IRMMOVQ || d_icode == IMRMOVQ) {
        return D_rB;
    }
    else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET) {
        return RSP;
    }
    else {
        return RNONE;
    }
}

uint64_t DecodeStage::dstE(D * dreg, uint64_t d_icode) {
    uint64_t D_rB = dreg->getrB()->getOutput();
    if (d_icode == IRRMOVQ || d_icode == IIRMOVQ || d_icode == IOPQ) {
        return D_rB;
    }
    else if (d_icode == IPUSHQ || d_icode == IPOPQ || d_icode == ICALL || d_icode == IRET) {
        return RSP;
    }
    else {
        return RNONE;
    }

}

uint64_t DecodeStage::dstM(D * dreg, uint64_t d_icode) {
    uint64_t D_rA = dreg->getrA()->getOutput();
    if (d_icode == IMRMOVQ || d_icode == IPOPQ) {
        return D_rA;   
    }
    else {
        return RNONE;
    }
}


uint64_t DecodeStage::SelfwdA(uint64_t d_srcA, D * d, M * m, W * w, uint64_t e_dstE, uint64_t e_valE, uint64_t m_valM) {
    uint64_t M_dstE = m->getdstE()->getOutput();
    uint64_t M_dstM = m->getdstM()->getOutput();
    uint64_t W_dstE = w->getdstE()->getOutput();
    uint64_t W_dstM = w->getdstM()->getOutput();
    
    uint64_t d_icode = d->geticode()->getOutput();
    uint64_t w_valM = w->getvalM()->getOutput();
    if (d_icode == ICALL || d_icode == IJXX) {
        return d->getvalP()->getOutput();
    }
    if (d_srcA == RNONE) return 0;
    if (d_srcA == e_dstE) {
      return e_valE;
    }
    if (d_srcA == M_dstM) {
        return m_valM;
    }
    if (d_srcA == M_dstE) {
      return m->getvalE()->getOutput();
    }
    if (d_srcA == W_dstM) {
        return w_valM;
    }
    if (d_srcA == W_dstE) {
      return w->getvalE()->getOutput();
    }

    RegisterFile * r = RegisterFile::getInstance();
    bool imem_error = false;
    uint64_t valA = r->readRegister(d_srcA, imem_error); 
    return valA;
}

/* d_valB
 * returns the appropriate value for srcB based on d_srcB
 *
 * @param: d_srcB - value of rB in decode stage
 */
uint64_t DecodeStage::FwdB(uint64_t d_srcB, M * m, W * w, uint64_t e_dstE, uint64_t e_valE, uint64_t m_valM) {
    uint64_t M_dstE = m->getdstE()->getOutput();
    uint64_t M_dstM = m->getdstM()->getOutput();
    uint64_t W_dstE = w->getdstE()->getOutput();
    uint64_t W_dstM = w->getdstM()->getOutput();
    
    if (d_srcB == RNONE) return 0;
    if (d_srcB == e_dstE) {
      return e_valE;
    }
	//d_srcB == M_dstM: m_valM;  # value obtained from Memory by MemoryStage
	if (d_srcB == M_dstM) {
		return m_valM;
	}
    if (d_srcB == M_dstE) {
      return m->getvalE()->getOutput();
    }
	//d_srcB == W_dstM: W_valM;  # value in W register
	if (d_srcB == W_dstM) {
		return w->getvalM()->getOutput();
	}
    if (d_srcB == W_dstE) {
      return w->getvalE()->getOutput();
    }

    RegisterFile * r = RegisterFile::getInstance();
    bool imem_error = false;
    uint64_t valB = r->readRegister(d_srcB, imem_error); 
    return valB;
	
	//todo: delete
//old valA = DecodeStage::SelfwdA(srcA, mreg, wreg, e->get_dstE(), e->get_valE());
	
	//valA = DecodeStage::SelfwdA(srcA, dreg, mreg, wreg, e->get_dstE(), e->get_valE(), m->get_valM());
    //valB = DecodeStage::FwdB(srcB, mreg, wreg, e->get_dstE(), e->get_valE());
    
}

bool DecodeStage::E_bubble(uint64_t e_icode, uint64_t e_dstM, bool e_Cnd, uint64_t srcA, uint64_t srcB) {

    //return ((e_icode == IJXX && !e_Cnd) || ((e_icode == IMRMOVQ || e_icode == IPOPQ) && (e_dstM == d_srcA || e_dstM == d_srcB)));
    return ((e_icode == IJXX) && !e_Cnd) || (e_icode == IMRMOVQ || e_icode == IPOPQ) && (e_dstM == srcA || e_dstM == srcB);
}

uint64_t DecodeStage::getd_srcA() {
    return d_srcA;
}
uint64_t DecodeStage::getd_srcB() {
    return d_srcB;
}

uint64_t DecodeStage::get_icode() {
    return icode;
}



