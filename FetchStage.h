//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      bool f_stall;
      bool d_stall;
      bool d_bubble;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool need_reglds(uint64_t f_icode);
      uint64_t getReglds(uint64_t f_pc);
      bool need_valC(uint64_t f_icode);
      uint64_t buildValC(uint64_t f_icode, uint64_t f_pc);
      uint64_t PredictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      uint64_t PCincrement(uint64_t f_pc, bool nr, bool nv);
      bool instr_valid(uint64_t f_icode);
      uint64_t f_stat(uint64_t f_icode, bool imem_error);
      bool F_stall(uint64_t e_icode, uint64_t d_icode, uint64_t m_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB);
      bool D_stall(uint64_t e_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB);
      void calculateControlSignals(uint64_t e_icode, uint64_t d_icode, uint64_t m_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB, bool e_Cnd);
      void D_bubble(uint64_t e_icode, uint64_t d_icode, uint64_t m_icode, uint64_t e_dstM, uint64_t d_srcA, uint64_t d_srcB, bool e_Cnd);
};
