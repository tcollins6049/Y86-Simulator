//class to perform the combinational logic of
//the Fetch stage
class MemoryStage: public Stage
{
   private:
      void setWInput(W * wreg, uint64_t stat, uint64_t icode,
                            uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM);
      uint64_t valM;
      uint64_t stat;
      uint64_t icode;
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t addr(uint64_t m_icode, M * mreg);
      bool read(uint64_t m_icode);
      bool write(uint64_t m_icode);
      uint64_t get_valM();
      uint64_t get_stat();
      uint64_t get_icode();

};
