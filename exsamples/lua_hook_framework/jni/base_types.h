
#define RET(regs) (regs.ARM_r0)
#define PC(regs) (regs.ARM_pc)
#define SP(regs) (regs.ARM_sp)
#define CPSR(regs) (regs.ARM_cpsr)


#define CPSR_T_MASK     ( 1u << 5 )

#define GET_REMOTE_ADDR( addr, local_base, remote_base ) ( (uint32_t)(addr) + (uint32_t)(remote_base) - (uint32_t)(local_base) )
