#ifndef Z80_H
#define Z80_H

#ifdef __cplusplus
extern "C" {
#endif


//#include "osd_cpu.h"
//#include "cpuintrf.h"
#include "DrZ80.h"

/* daisy-chain link */
typedef struct {
    void (*reset)(int);             /* reset callback     */
    int  (*interrupt_entry)(int);   /* entry callback     */
    void (*interrupt_reti)(int);    /* reti callback      */
    int irq_param;                  /* callback paramater */
} Z80_DaisyChain;

#define Z80_MAXDAISY	4		/* maximum of daisy chan device */

#define Z80_INT_REQ     0x01    /* interrupt request mask       */
#define Z80_INT_IEO     0x02    /* interrupt disable mask(IEO)  */

#define Z80_VECTOR(device,state) (((device)<<8)|(state))

/****************************************************************************/
/* The Z80 registers. HALT is set to 1 when the CPU is halted, the refresh  */
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)    */
/****************************************************************************/
typedef struct {
	struct DrZ80 regs;
	int pending_irq;
	int irq_max; /* number of daisy chain devices */
	int request_irq; /* daisy chain next request device */
	int service_irq; /* daisy chain next reti handling device */
	int int_state[Z80_MAXDAISY];
	Z80_DaisyChain irq[Z80_MAXDAISY];
}   Z80_Regs;

/*extern int Z80_ICount; // T-state count */
extern Z80_Regs Z80;
//#define Z80_ICount (Z80.regs.cycles)

#define Z80_IGNORE_INT	-1 /* Ignore interrupt */
#define Z80_NMI_INT 	-2 /* Execute NMI */
#define Z80_IRQ_INT 	-1000 /* Execute IRQ */

extern unsigned Z80_GetPC (void); /* Get program counter */
extern int Z80_GetPreviousPC (void);
extern void Z80_GetRegs (Z80_Regs *Regs);  /* Get registers */
extern void Z80_SetRegs (Z80_Regs *Regs);  /* Set registers */
//extern void Z80_Reset (Z80_DaisyChain *daisy_chain);
extern void Z80_Reset(void);
extern int Z80_Execute(int cycles); /* Execute cycles T-States - returns number of cycles actually run */
extern int Z80_Interrupt(void);
extern void Z80_Cause_Interrupt(int type);
extern void Z80_Clear_Pending_Interrupts(void);
extern void Interrupt(void); /* required for DrZ80 int hack */

#ifdef __cplusplus
} /* End of extern "C" */
#endif

#endif

