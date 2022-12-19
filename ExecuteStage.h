//class to perform the combinational logic of
//the Fetch stage
class ExecuteStage: public Stage
{
   private:
      void setMInput(M * mreg, uint64_t stat, uint64_t icode,
                            bool Cnd, uint64_t valE,
                            uint64_t valA, uint64_t dstE, uint64_t dstM);
      bool Mbubble;
      bool Cnd;
      uint64_t icode;
      uint64_t dstM;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t galuA(uint64_t e_icode, E * ereg);
      uint64_t galuB(uint64_t e_icode, uint64_t e_valB);
      uint64_t alufun(E * ereg, uint64_t e_icode);
      bool set_cc(uint64_t e_icode, W * wreg, uint64_t m_stat);
      uint64_t gdstE(uint64_t e_icode, bool m_Cnd, uint64_t e_dstE);
      void ccHelper(uint64_t icode, bool cnd, uint64_t valE, W * wreg, uint64_t m_stat);
      uint64_t aluHelper(uint64_t temp, E * ereg, uint64_t valA, uint64_t valB, W * wreg, uint64_t m_stat);
      void calculateControlSignals(uint64_t m_stat, W * wreg);
      uint64_t get_dstE();
      uint64_t get_valE();
      bool cond(uint64_t icode, uint64_t ifun);
      bool get_Cnd();
      uint64_t get_icode();
      uint64_t get_dstM();

      uint64_t valE;
      uint64_t dstE;
};
