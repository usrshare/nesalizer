#include "dbg.h"
#include "common.h"
#include "opcodes.h"
#include "cpu.h"
#include "mapper.h"
#include "sdl_backend.h"

static enum { RUN, SINGLE_STEP, TRACE, NEXT_STEP } debug_mode;// = SINGLE_STEP;
static int cursor_cpu = -1;
static int cursor_ram = 0;

int set_debugger_vis(bool vis) {
	if (vis) {
		if (cursor_cpu == -1) cursor_cpu = pc;
	}else {
		cursor_cpu = -1;
	}

	return 0;
}

//
// Debugging and tracing
//

static int read_without_side_effects(uint16_t addr) {
	switch (addr) {
		case 0x0000 ... 0x1FFF: return ram[addr & 0x07FF];
		case 0x6000 ... 0x7FFF: return wram_6000_page ? wram_6000_page[addr & 0x1FFF] : 0;
		case 0x8000 ... 0xFFFF: return read_prg(addr);
		default:                return -1;
	}
}

static char const *decode_addr(uint16_t addr) {
	static char addr_str[64];

	static char const *const desc_2000_regs[]
		= { "PPUCTRL", "PPUMASK"  , "PPUSTATUS", "OAMADDR",
			"OAMDATA", "PPUSCROLL", "PPUADDR"  , "PPUDATA" };

	static char const *const desc_4000_regs[]
		= { // $4000-$4007
			"Pulse 1 duty, loop, and volume", "Pulse 1 sweep unit"           ,
			"Pulse 1 timer low"             , "Pulse 1 length and timer high",
			"Pulse 2 duty, loop, and volume", "Pulse 2 sweep unit"           ,
			"Pulse 2 timer low"             , "Pulse 2 length and timer high",

			// $4008-$400B
			"Triangle linear length", 0, "Triangle timer low", "Triangle length and timer high",

			// $400C-$400F
			"Noise volume", 0, "Noise loop and period", "Noise length",

			// $4010-$4013
			"DMC IRQ, loop, and frequency", "DMC counter"      ,
			"DMC sample address"          , "DMC sample length",

			// $4014-$4017
			"OAM DMA", "APU status", "Read controller 1",
			"Frame counter and read controller 2" };

	char const *addr_desc;
	if (addr >= 0x2000 && addr <= 0x3FFF)
		addr_desc = desc_2000_regs[addr & 7];
	else if (addr >= 0x4000 && addr <= 0x4017)
		addr_desc = desc_4000_regs[addr - 0x4000];
	else
		addr_desc = 0;

	if (addr_desc)
		sprintf(addr_str, "$%04X (%s)", addr, addr_desc);
	else
		sprintf(addr_str, "$%04X", addr);
	return addr_str;
}

// TODO: Tableify

#define INS_IMP(name)    case name         : sdldbg_puts (#name"         \n");                            break;
#define INS_ACC(name)    case name##_ACC   : sdldbg_puts (#name" A       \n");                            break;
#define INS_IMM(name)    case name##_IMM   : sdldbg_printf(#name" #$%02X    \n", op_1);                            break;
#define INS_ZERO(name)   case name##_ZERO  : sdldbg_printf(#name" $%02X     \n", op_1);                            break;
#define INS_ZERO_X(name) case name##_ZERO_X: sdldbg_printf(#name" $%02X,X   \n", op_1);                            break;
#define INS_ZERO_Y(name) case name##_ZERO_Y: sdldbg_printf(#name" $%02X,Y   \n", op_1);                            break;
#define INS_REL(name)    case name         : sdldbg_printf(#name" $%04X   \n"  , uint16_t(addr + 2 + (int8_t)op_1)); break;
#define INS_IND_X(name)  case name##_IND_X : sdldbg_printf(#name" ($%02X,X) \n", op_1);                            break;
#define INS_IND_Y(name)  case name##_IND_Y : sdldbg_printf(#name" ($%02X),Y \n", op_1);                            break;
#define INS_ABS(name)    case name##_ABS   : sdldbg_printf(#name" %s   \n"  , decode_addr((op_2 << 8) | op_1));    break;
#define INS_ABS_X(name)  case name##_ABS_X : sdldbg_printf(#name" %s,X \n"  , decode_addr((op_2 << 8) | op_1));    break;
#define INS_ABS_Y(name)  case name##_ABS_Y : sdldbg_printf(#name" %s,Y \n"  , decode_addr((op_2 << 8) | op_1));    break;
#define INS_IND(name)    case name##_IND   : sdldbg_printf(#name" (%s) \n"  , decode_addr((op_2 << 8) | op_1));    break;

static bool     breakpoint_at[0x10000];
// Optimization to avoid trashing the cache via breakpoint_at lookups when no
// breakpoints are set
static unsigned n_breakpoints_set;

int reset_debugger(void) {
	init_array(breakpoint_at, false);
	return 0;
}

int instr_length(int opcode) {
	switch (opcode) {
		// Implied
		case BRK: case RTI: case RTS: case PHA: case PHP:
		case PLA: case PLP: case CLC: case CLD: case CLI:
		case CLV: case SEC: case SED: case SEI: case DEX:
		case DEY: case INX: case INY: case NO0: case NO1:
		case NO2: case NO3: case NO4: case NO5: case NOP:
		case TAX: case TAY: case TSX: case TXA: case TXS:
		case TYA:

			// KIL instructions implied_:
		case KI0: case KI1: case KI2: case KI3: case KI4:
		case KI5: case KI6: case KI7: case KI8: case KI9:
		case K10: case K11:

			// Accumulator
		case ASL_ACC: case LSR_ACC: case ROL_ACC: case ROR_ACC:

			return 1;

			// Immediate
		case ADC_IMM: case ALR_IMM: case AN0_IMM: case AN1_IMM: case AND_IMM:
		case ARR_IMM: case AXS_IMM: case ATX_IMM: case CMP_IMM: case CPX_IMM:
		case CPY_IMM: case EOR_IMM: case LDA_IMM: case LDX_IMM: case LDY_IMM:
		case NO0_IMM: case NO1_IMM: case NO2_IMM: case NO3_IMM: case NO4_IMM:
		case ORA_IMM: case SB2_IMM: case SBC_IMM: case XAA_IMM:

			// Zero page
		case ADC_ZERO: case AND_ZERO: case BIT_ZERO: case CMP_ZERO:
		case CPX_ZERO: case CPY_ZERO: case DCP_ZERO: case EOR_ZERO:
		case ISC_ZERO: case LAX_ZERO: case LDA_ZERO: case LDX_ZERO:
		case LDY_ZERO: case NO0_ZERO: case NO1_ZERO: case NO2_ZERO:
		case ORA_ZERO: case SBC_ZERO: case SLO_ZERO: case SRE_ZERO:
		case SAX_ZERO: case ASL_ZERO: case LSR_ZERO: case RLA_ZERO:
		case RRA_ZERO: case ROL_ZERO: case ROR_ZERO: case INC_ZERO:
		case DEC_ZERO: case STA_ZERO: case STX_ZERO: case STY_ZERO:

			// Zero page, X
		case ADC_ZERO_X: case AND_ZERO_X: case CMP_ZERO_X: case DCP_ZERO_X:
		case EOR_ZERO_X: case ISC_ZERO_X: case LDA_ZERO_X: case LDY_ZERO_X:
		case NO0_ZERO_X: case NO1_ZERO_X: case NO2_ZERO_X: case NO3_ZERO_X:
		case NO4_ZERO_X: case NO5_ZERO_X: case ORA_ZERO_X: case SBC_ZERO_X:
		case SLO_ZERO_X: case SRE_ZERO_X: case ASL_ZERO_X: case DEC_ZERO_X:
		case INC_ZERO_X: case LSR_ZERO_X: case RLA_ZERO_X: case RRA_ZERO_X:
		case ROL_ZERO_X: case ROR_ZERO_X: case STA_ZERO_X: case STY_ZERO_X:

			// Zero page, Y
		case SAX_ZERO_Y: case LAX_ZERO_Y: case LDX_ZERO_Y: case STX_ZERO_Y:

			// Relative branch_ instructions:
		case BCC: case BCS: case BVC: case BVS: case BEQ:
		case BMI: case BNE: case BPL:

			// Indirect_,X:
		case ADC_IND_X: case AND_IND_X: case CMP_IND_X: case DCP_IND_X:
		case EOR_IND_X: case ISC_IND_X: case LAX_IND_X: case LDA_IND_X:
		case ORA_IND_X: case RLA_IND_X: case RRA_IND_X: case SAX_IND_X:
		case SBC_IND_X: case SLO_IND_X: case SRE_IND_X: case STA_IND_X:

			// Indirect_:,Y
		case ADC_IND_Y: case AND_IND_Y: case AXA_IND_Y: case CMP_IND_Y:
		case DCP_IND_Y: case EOR_IND_Y: case ISC_IND_Y: case LAX_IND_Y:
		case LDA_IND_Y: case ORA_IND_Y: case RLA_IND_Y: case RRA_IND_Y:
		case SBC_IND_Y: case SLO_IND_Y: case SRE_IND_Y: case STA_IND_Y:

			return 2;

			// Absolute
		case JMP_ABS: case JSR_ABS: case ADC_ABS: case AND_ABS: case BIT_ABS:
		case CMP_ABS: case CPX_ABS: case CPY_ABS: case DCP_ABS: case EOR_ABS:
		case ISC_ABS: case LAX_ABS: case LDA_ABS: case LDX_ABS: case LDY_ABS:
		case NOP_ABS: case ORA_ABS: case SBC_ABS: case SLO_ABS: case SRE_ABS:
		case ASL_ABS: case DEC_ABS: case INC_ABS: case LSR_ABS: case RLA_ABS:
		case RRA_ABS: case ROL_ABS: case ROR_ABS: case SAX_ABS: case STA_ABS:
		case STX_ABS: case STY_ABS:

			// Absolute,X
		case ADC_ABS_X: case AND_ABS_X: case CMP_ABS_X: case DCP_ABS_X:
		case EOR_ABS_X: case ISC_ABS_X: case LDA_ABS_X: case LDY_ABS_X:
		case NO0_ABS_X: case NO1_ABS_X: case NO2_ABS_X: case NO3_ABS_X:
		case NO4_ABS_X: case NO5_ABS_X: case ORA_ABS_X: case RLA_ABS_X:
		case RRA_ABS_X: case SAY_ABS_X: case SBC_ABS_X: case SLO_ABS_X:
		case SRE_ABS_X: case ASL_ABS_X: case DEC_ABS_X: case INC_ABS_X:
		case LSR_ABS_X: case ROL_ABS_X: case ROR_ABS_X: case STA_ABS_X:

			// Absolute,Y
		case ADC_ABS_Y: case AND_ABS_Y: case AXA_ABS_Y: case CMP_ABS_Y:
		case DCP_ABS_Y: case EOR_ABS_Y: case ISC_ABS_Y: case LAX_ABS_Y:
		case LAS_ABS_Y: case LDA_ABS_Y: case LDX_ABS_Y: case ORA_ABS_Y:
		case RLA_ABS_Y: case RRA_ABS_Y: case SBC_ABS_Y: case SLO_ABS_Y:
		case SRE_ABS_Y: case STA_ABS_Y: case TAS_ABS_Y: case XAS_ABS_Y:

			// Indirect
		case JMP_IND: return 3;

		default: return 1;
	}
}

static void print_instruction(uint16_t addr) {
	int opcode, op_1, op_2;

	if (breakpoint_at[addr]) {
		sdldbg_printf("\361%c\361%04X: \360", (addr == pc) ? '#' : '*' , addr);
	} else {
		sdldbg_printf("\361%c\365%04X: \360", (addr == pc) ? '>' : ' ' , addr);
	}

	if ((opcode = read_without_side_effects(addr)) == -1) {
		sdldbg_puts("(strange address while reading opcode - skipping)\n");
		return;
	}

	int ol = instr_length(opcode);

	if (addr == cursor_cpu) sdldbg_puts("\363"); else sdldbg_puts("\360");

	for (int i=0; i < 3; i++) {
		if (i < ol) 
			sdldbg_printf("%02X ",read_without_side_effects(addr+i));
		else
			sdldbg_puts("   ");
	}

	switch (opcode) {
		// Implied
		INS_IMP(BRK) INS_IMP(RTI) INS_IMP(RTS) INS_IMP(PHA) INS_IMP(PHP)
			INS_IMP(PLA) INS_IMP(PLP) INS_IMP(CLC) INS_IMP(CLD) INS_IMP(CLI)
			INS_IMP(CLV) INS_IMP(SEC) INS_IMP(SED) INS_IMP(SEI) INS_IMP(DEX)
			INS_IMP(DEY) INS_IMP(INX) INS_IMP(INY) INS_IMP(NO0) INS_IMP(NO1)
			INS_IMP(NO2) INS_IMP(NO3) INS_IMP(NO4) INS_IMP(NO5) INS_IMP(NOP)
			INS_IMP(TAX) INS_IMP(TAY) INS_IMP(TSX) INS_IMP(TXA) INS_IMP(TXS)
			INS_IMP(TYA)

			// KIL instructions (implied)
			INS_IMP(KI0) INS_IMP(KI1) INS_IMP(KI2) INS_IMP(KI3) INS_IMP(KI4)
			INS_IMP(KI5) INS_IMP(KI6) INS_IMP(KI7) INS_IMP(KI8) INS_IMP(KI9)
			INS_IMP(K10) INS_IMP(K11)

			// Accumulator
			INS_ACC(ASL) INS_ACC(LSR) INS_ACC(ROL) INS_ACC(ROR)

		default: goto needs_first_operand;
	}
	return;

needs_first_operand:
	if ((op_1 = read_without_side_effects(addr + 1)) == -1) {
		sdldbg_puts("(strange address while reading first operand byte - skipping)\n");
		return;
	}

	switch (opcode) {
		// Immediate
		INS_IMM(ADC) INS_IMM(ALR) INS_IMM(AN0) INS_IMM(AN1) INS_IMM(AND)
			INS_IMM(ARR) INS_IMM(AXS) INS_IMM(ATX) INS_IMM(CMP) INS_IMM(CPX)
			INS_IMM(CPY) INS_IMM(EOR) INS_IMM(LDA) INS_IMM(LDX) INS_IMM(LDY)
			INS_IMM(NO0) INS_IMM(NO1) INS_IMM(NO2) INS_IMM(NO3) INS_IMM(NO4)
			INS_IMM(ORA) INS_IMM(SB2) INS_IMM(SBC) INS_IMM(XAA)

			// Zero page
			INS_ZERO(ADC) INS_ZERO(AND) INS_ZERO(BIT) INS_ZERO(CMP)
			INS_ZERO(CPX) INS_ZERO(CPY) INS_ZERO(DCP) INS_ZERO(EOR)
			INS_ZERO(ISC) INS_ZERO(LAX) INS_ZERO(LDA) INS_ZERO(LDX)
			INS_ZERO(LDY) INS_ZERO(NO0) INS_ZERO(NO1) INS_ZERO(NO2)
			INS_ZERO(ORA) INS_ZERO(SBC) INS_ZERO(SLO) INS_ZERO(SRE)
			INS_ZERO(SAX) INS_ZERO(ASL) INS_ZERO(LSR) INS_ZERO(RLA)
			INS_ZERO(RRA) INS_ZERO(ROL) INS_ZERO(ROR) INS_ZERO(INC)
			INS_ZERO(DEC) INS_ZERO(STA) INS_ZERO(STX) INS_ZERO(STY)

			// Zero page, X
			INS_ZERO_X(ADC) INS_ZERO_X(AND) INS_ZERO_X(CMP) INS_ZERO_X(DCP)
			INS_ZERO_X(EOR) INS_ZERO_X(ISC) INS_ZERO_X(LDA) INS_ZERO_X(LDY)
			INS_ZERO_X(NO0) INS_ZERO_X(NO1) INS_ZERO_X(NO2) INS_ZERO_X(NO3)
			INS_ZERO_X(NO4) INS_ZERO_X(NO5) INS_ZERO_X(ORA) INS_ZERO_X(SBC)
			INS_ZERO_X(SLO) INS_ZERO_X(SRE) INS_ZERO_X(ASL) INS_ZERO_X(DEC)
			INS_ZERO_X(INC) INS_ZERO_X(LSR) INS_ZERO_X(RLA) INS_ZERO_X(RRA)
			INS_ZERO_X(ROL) INS_ZERO_X(ROR) INS_ZERO_X(STA) INS_ZERO_X(STY)

			// Zero page, Y
			INS_ZERO_Y(SAX) INS_ZERO_Y(LAX) INS_ZERO_Y(LDX) INS_ZERO_Y(STX)

			// Relative (branch instructions)
			INS_REL(BCC) INS_REL(BCS) INS_REL(BVC) INS_REL(BVS) INS_REL(BEQ)
			INS_REL(BMI) INS_REL(BNE) INS_REL(BPL)

			// (Indirect,X)
			INS_IND_X(ADC) INS_IND_X(AND) INS_IND_X(CMP) INS_IND_X(DCP)
			INS_IND_X(EOR) INS_IND_X(ISC) INS_IND_X(LAX) INS_IND_X(LDA)
			INS_IND_X(ORA) INS_IND_X(RLA) INS_IND_X(RRA) INS_IND_X(SAX)
			INS_IND_X(SBC) INS_IND_X(SLO) INS_IND_X(SRE) INS_IND_X(STA)

			// (Indirect),Y
			INS_IND_Y(ADC) INS_IND_Y(AND) INS_IND_Y(AXA) INS_IND_Y(CMP)
			INS_IND_Y(DCP) INS_IND_Y(EOR) INS_IND_Y(ISC) INS_IND_Y(LAX)
			INS_IND_Y(LDA) INS_IND_Y(ORA) INS_IND_Y(RLA) INS_IND_Y(RRA)
			INS_IND_Y(SBC) INS_IND_Y(SLO) INS_IND_Y(SRE) INS_IND_Y(STA)

		default: goto needs_second_operand;
	}

	return;

needs_second_operand:
	if ((op_2 = read_without_side_effects(addr + 2)) == -1) {
		puts("(strange address while reading second operand byte - skipping)");
		return;
	}

	switch (opcode) {
		// Absolute
		INS_ABS(JMP) INS_ABS(JSR) INS_ABS(ADC) INS_ABS(AND) INS_ABS(BIT)
			INS_ABS(CMP) INS_ABS(CPX) INS_ABS(CPY) INS_ABS(DCP) INS_ABS(EOR)
			INS_ABS(ISC) INS_ABS(LAX) INS_ABS(LDA) INS_ABS(LDX) INS_ABS(LDY)
			INS_ABS(NOP) INS_ABS(ORA) INS_ABS(SBC) INS_ABS(SLO) INS_ABS(SRE)
			INS_ABS(ASL) INS_ABS(DEC) INS_ABS(INC) INS_ABS(LSR) INS_ABS(RLA)
			INS_ABS(RRA) INS_ABS(ROL) INS_ABS(ROR) INS_ABS(SAX) INS_ABS(STA)
			INS_ABS(STX) INS_ABS(STY)

			// Absolute,X
			INS_ABS_X(ADC) INS_ABS_X(AND) INS_ABS_X(CMP) INS_ABS_X(DCP)
			INS_ABS_X(EOR) INS_ABS_X(ISC) INS_ABS_X(LDA) INS_ABS_X(LDY)
			INS_ABS_X(NO0) INS_ABS_X(NO1) INS_ABS_X(NO2) INS_ABS_X(NO3)
			INS_ABS_X(NO4) INS_ABS_X(NO5) INS_ABS_X(ORA) INS_ABS_X(RLA)
			INS_ABS_X(RRA) INS_ABS_X(SAY) INS_ABS_X(SBC) INS_ABS_X(SLO)
			INS_ABS_X(SRE) INS_ABS_X(ASL) INS_ABS_X(DEC) INS_ABS_X(INC)
			INS_ABS_X(LSR) INS_ABS_X(ROL) INS_ABS_X(ROR) INS_ABS_X(STA)

			// Absolute,Y
			INS_ABS_Y(ADC) INS_ABS_Y(AND) INS_ABS_Y(AXA) INS_ABS_Y(CMP)
			INS_ABS_Y(DCP) INS_ABS_Y(EOR) INS_ABS_Y(ISC) INS_ABS_Y(LAX)
			INS_ABS_Y(LAS) INS_ABS_Y(LDA) INS_ABS_Y(LDX) INS_ABS_Y(ORA)
			INS_ABS_Y(RLA) INS_ABS_Y(RRA) INS_ABS_Y(SBC) INS_ABS_Y(SLO)
			INS_ABS_Y(SRE) INS_ABS_Y(STA) INS_ABS_Y(TAS) INS_ABS_Y(XAS)

			// Indirect
			INS_IND(JMP)

		default: UNREACHABLE
	}
}

int find_lookback(int addr, int* instr_c) {
	//find how many bytes one has to look back to discover (instr_c) valid 6502 instructions.

	int test_addr = addr;
	int last_addr = addr;

	int instr_found = 0;

	int prev_addr = addr;
	int prev_instr = 0;

	do {
		instr_found = 0;
		test_addr--;
		last_addr = test_addr;
		while (last_addr < addr) { 
			last_addr += instr_length(read_without_side_effects(last_addr));
			instr_found++; }

		//printf("Lookback (%04X,%d) @ %04X returns %04X in %d instructions\n",addr,*instr_c,test_addr,last_addr,instr_found);
		if ((prev_addr == last_addr) && (instr_found > prev_instr) && (instr_found >= (*instr_c))) break; 
		prev_addr = last_addr; prev_instr = instr_found;

	} while ( (test_addr > 0) && ((last_addr != addr) || (instr_found < *instr_c)) );

	*instr_c = instr_found;
	return test_addr;
}

int find_forward(int addr, int* instr_c) {

	int test_addr = addr;
	int i=0;

	while ((i < (*instr_c)) && (test_addr <= 0xFFFF)) {

		test_addr += instr_length(read_without_side_effects(test_addr));
		i++;
	}

	return test_addr;
}

int kbdinput_ign = 0;

static int breakpoint_set (uint16_t addr) {
	n_breakpoints_set += !breakpoint_at[addr];
	breakpoint_at[addr] = true;
	return 0;
}

static int breakpoint_remove (uint16_t addr) {
	n_breakpoints_set -= breakpoint_at[addr];
	if (!breakpoint_at[addr]) return 1;
	breakpoint_at[addr] = false;
	return 0;
}

static int breakpoint_toggle (uint16_t addr) {
	return breakpoint_at[addr] ? breakpoint_remove(addr) : breakpoint_set(addr);
}

static void dbg_kbdinput(int keycode) {
	char arg[80];
	memset(arg,0,80);

	switch(keycode) {

		case SDLK_UP: {
				      int ib = 1;
				      cursor_cpu = find_lookback(cursor_cpu,&ib);
				      break;
			      }
		case SDLK_DOWN: {
					cursor_cpu += instr_length(read_without_side_effects(cursor_cpu));
					break;
				}
		case SDLK_PAGEUP: {
					  int ib = 24;
					  cursor_cpu = find_lookback(cursor_cpu,&ib);
					  break;
				  }
		case SDLK_PAGEDOWN: {
					    int ib = 24;
					    cursor_cpu = find_forward(cursor_cpu,&ib);
					    break;
				    }
		case SDLK_RETURN: {
					  if (!sdl_text_prompt("Goto address:",arg,80)) {
						  break;
					  }
					  unsigned addr;
					  sscanf(arg, "%x", &addr);
					  if (addr > 0xFFFF)
						  puts("Address out of range");
					  else cursor_cpu = addr;
				  }
				  break;
		case 's':	{
					debug_mode = NEXT_STEP;
				}
				break;
		case (KM_SHIFT | 's'):	{
					// "step over" by installing a breakpoint on the instruction
					// next to the one at PC
					if (debug_mode == SINGLE_STEP) debug_mode = RUN;
					int f=1;
					breakpoint_set(find_forward(pc,&f));
				}
					break;
		case 'b':
				{
					if (!sdl_text_prompt("Breakpoint address:",arg,80)) {
						puts("Missing address");
						break;
					}
					unsigned addr;
					sscanf(arg, "%x", &addr);
					breakpoint_set(addr);
					break;
				}

				break;
		case 'd':
				{
					if (!sdl_text_prompt("Breakpoint address:",arg,80)) {
						puts("Missing address");
						break;
					}
					unsigned addr;
					sscanf(arg, "%x", &addr);
					breakpoint_remove(addr);
					break;
				}
		case ' ':
				{
					breakpoint_toggle(cursor_cpu);
				}
				break;
		case (KM_SHIFT | 'd'):
				for (unsigned i = 0; i < ARRAY_LEN(breakpoint_at); ++i) {
					if (breakpoint_at[i]) {
						printf("Deleted breakpoint at %04X\n", i);
						breakpoint_at[i] = false;
					}
				}
				n_breakpoints_set = 0;
				break;

		case 'i':
				for (unsigned i = 0; i < ARRAY_LEN(breakpoint_at); ++i)
					if (breakpoint_at[i])
						printf("Breakpoint at %04X\n", i);
				break;
		case 'r':
				if (debug_mode == SINGLE_STEP) debug_mode = RUN;
		default:
				break;
	}
}

int dbg_log_instruction() {

	if (debug_mode == NEXT_STEP) { debug_mode = SINGLE_STEP; cursor_cpu = pc; }

	if (debug_mode == RUN) {

		if (n_breakpoints_set > 0 && breakpoint_at[pc]) {
			debug_mode = SINGLE_STEP;
			show_debugger = 1;
			cursor_cpu = pc;
			printf("forcing cursor\n");
		}
	}

	if ( (show_debugger) && ( (debug_mode == SINGLE_STEP) || (frame_offset == 0)) ) {
		//every frame, output new debugger values

		mvsdldbg_printf(96, 0, "\361PC: \360%04X", pc);
		mvsdldbg_printf(96, 1, "\361A:  \360%02X", a);
		mvsdldbg_printf(96, 2, "\361X:  \360%02X", x);
		mvsdldbg_printf(96, 3, "\361Y:  \360%02X", y);
		mvsdldbg_printf(96, 4, "\361SP: \360%02X", s);

		mvsdldbg_printf(96, 5, "\362%c%c%c%c%c%c\360",carry ? 'C' : 'c', !(zn & 0xFF) ? 'Z' : 'z', irq_disable ? 'I' : 'i', decimal ? 'D' : 'd', overflow ? 'V' : 'v', !!(zn & 0x180) ? 'N' : 'n');
		
		if (pending_nmi && pending_irq)
			mvsdldbg_puts(96,6," (pending NMI and IRQ)");
		else if (pending_nmi)
			mvsdldbg_puts(96,6," (pending NMI)");
		else if (pending_irq)
			mvsdldbg_puts(96,6," (pending IRQ)");
		else
			mvsdldbg_clear(96,6,24,1);

		sdldbg_move(0,0);
		sdldbg_clear(64, 50);   

		int instr_f = 24;
		int addr_lb = find_lookback(cursor_cpu >= 0 ? cursor_cpu : pc, &instr_f);

		sdldbg_move(0, 24 - instr_f);

		for (int i =0; i < instr_f + 24; i++) {
			print_instruction(addr_lb);
			int il = instr_length(read_without_side_effects(addr_lb));

			if ( (addr_lb < cursor_cpu) && ((addr_lb + il) > cursor_cpu) ) {
				sdldbg_printf("\361--------\360\n");
				addr_lb = cursor_cpu;
			} else addr_lb += il; 
		}

		//print zeropage
		for (int iy = 0; iy < 16; iy++) mvsdldbg_printf(77 - 3, 33 + iy, "\xF1%01Xx", iy);
		for (int ix = 0; ix < 16; ix++) mvsdldbg_printf(77 + (3*ix), 33 - 1, "\xF5x%01X", ix );
		for (int ix = 0; ix < 16; ix++) mvsdldbg_printf(77 + (3*ix), 33 + 16, "\xF5x%01X", ix );
		for (int iy = 0; iy < 16; iy++) mvsdldbg_printf(77 + (16*3), 33 + iy, "\xF1%01Xx", iy);

		for (int iy = 0; iy < 16; iy++)
			for (int ix = 0; ix < 16; ix++)
				mvsdldbg_printf(77 + (3*ix), 33 + iy, "%c%02X", ((ix+iy)%2 ? 0xF3 : 0xF4) , ram[iy * 16 + ix] );

		//print RAM

		for (int iy = 0; iy < 8; iy++) {

			mvsdldbg_printf(23,51+iy,"\365%04X: \360",cursor_ram + 32*iy);

			for (int ix=0; ix<32; ix++) {
				sdldbg_printf("%c%02X", ( (ix%8 == 0) ? 0366 : ((ix%2) ? 0364 : 0360) ), read_without_side_effects(cursor_ram + (32*iy) + ix) );
				if ((ix & 15) == 15) sdldbg_puts(" ");
			}

			sdldbg_puts("\360");

			for (int ix=0; ix<32; ix++) {
				uint8_t v = read_without_side_effects(cursor_ram + (32*iy) + ix);
				sdldbg_printf("%c", ((v >= 32) && (v < 128)) ? v : '.');
			}
		}
	}

	if (show_debugger) {
		int k = sdldbg_getkey_nonblock();
		if (k) {
			dbg_kbdinput(k);	
		}
	}

	if (debug_mode == SINGLE_STEP) audio_pause(1); else audio_pause(0);
	return (debug_mode != SINGLE_STEP);
}