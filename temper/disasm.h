#ifndef DISASM_H
#define DISASM_H

u32 disasm_instruction(char *output, u32 *_pc);
void disasm_function(u32 pc);
void disasm_block(u32 pc, u32 count);

#endif
