//class to perform the combinational logic of
//the Fetch stage
class DecodeStage: public Stage
{
   private:
      void setEInput(E * ereg, uint64_t stat, uint64_t icode,
                            uint64_t ifun, uint64_t valC, uint64_t valA,
                            uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB);
      uint64_t d_srcA;
      uint64_t d_srcB;
      bool e_bubble;
      uint64_t icode;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t srcA(D * dreg, uint64_t d_icode);
      uint64_t srcB(D * dreg, uint64_t d_icode);
      uint64_t dstE(D * dreg, uint64_t d_icode);
      uint64_t dstM(D * dreg, uint64_t d_icode);
      uint64_t SelfwdA(uint64_t d_srcA, D * d, M * m, W * w, uint64_t e_dstE, uint64_t e_valE, uint64_t m_valM);
      uint64_t FwdB(uint64_t d_srcB, M * m, W * w, uint64_t e_dstE, uint64_t e_valE, uint64_t m_valM);
      uint64_t getd_srcA();
      uint64_t getd_srcB();
      bool E_bubble(uint64_t e_icode, uint64_t e_dstM, bool e_Cnd, uint64_t srcA, uint64_t srcB);
      uint64_t get_icode();
};
