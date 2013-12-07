enum {
  // Implied
  BRK = 0x00, CLC = 0x18, CLD = 0xD8, CLI = 0x58, CLV = 0xB8, DEX = 0xCA, DEY = 0x88,
  INX = 0xE8, INY = 0xC8, NO0 = 0x1A, NO1 = 0x3A, NO2 = 0x5A, NO3 = 0x7A, NO4 = 0xDA,
  NO5 = 0xFA, NOP = 0xEA, PHA = 0x48, PHP = 0x08, PLA = 0x68, PLP = 0x28, RTI = 0x40,
  RTS = 0x60, SEC = 0x38, SED = 0xF8, SEI = 0x78, TAX = 0xAA, TAY = 0xA8, TSX = 0xBA,
  TXA = 0x8A, TXS = 0x9A, TYA = 0x98,

  // Accumulator
  ASL_ACC = 0x0A, LSR_ACC = 0x4A, ROL_ACC = 0x2A, ROR_ACC = 0x6A,

  // Immediate
  ADC_IMM = 0x69, ALR_IMM = 0x4B, AN0_IMM = 0x0B, AN1_IMM = 0x2B, AND_IMM = 0x29,
  ARR_IMM = 0x6B, ATX_IMM = 0xAB, AXS_IMM = 0xCB, CMP_IMM = 0xC9, CPX_IMM = 0xE0,
  CPY_IMM = 0xC0, EOR_IMM = 0x49, LDA_IMM = 0xA9, LDX_IMM = 0xA2, LDY_IMM = 0xA0,
  NO0_IMM = 0x80, NO1_IMM = 0x82, NO2_IMM = 0x89, NO3_IMM = 0xC2, NO4_IMM = 0xE2,
  ORA_IMM = 0x09, SB2_IMM = 0xEB, SBC_IMM = 0xE9, XAA_IMM = 0x8B,

  // Absolute
  ADC_ABS = 0x6D, AND_ABS = 0x2D, ASL_ABS = 0x0E, BIT_ABS = 0x2C, CMP_ABS = 0xCD,
  CPX_ABS = 0xEC, CPY_ABS = 0xCC, DCP_ABS = 0xCF, DEC_ABS = 0xCE, EOR_ABS = 0x4D,
  INC_ABS = 0xEE, ISC_ABS = 0xEF, JMP_ABS = 0x4C, JSR_ABS = 0x20, LAX_ABS = 0xAF,
  LDA_ABS = 0xAD, LDX_ABS = 0xAE, LDY_ABS = 0xAC, LSR_ABS = 0x4E, NOP_ABS = 0x0C,
  ORA_ABS = 0x0D, RLA_ABS = 0x2F, ROL_ABS = 0x2E, ROR_ABS = 0x6E, RRA_ABS = 0x6F,
  SAX_ABS = 0x8F, SBC_ABS = 0xED, SLO_ABS = 0x0F, SRE_ABS = 0x4F, STA_ABS = 0x8D,
  STX_ABS = 0x8E, STY_ABS = 0x8C,

  // Absolute,X
  ADC_ABS_X = 0x7D, AND_ABS_X = 0x3D, ASL_ABS_X = 0x1E, CMP_ABS_X = 0xDD,
  DCP_ABS_X = 0xDF, DEC_ABS_X = 0xDE, EOR_ABS_X = 0x5D, INC_ABS_X = 0xFE,
  ISC_ABS_X = 0xFF, LDA_ABS_X = 0xBD, LDY_ABS_X = 0xBC, LSR_ABS_X = 0x5E,
  ORA_ABS_X = 0x1D, NO0_ABS_X = 0x1C, NO1_ABS_X = 0x3C, NO2_ABS_X = 0x5C,
  NO3_ABS_X = 0x7C, NO4_ABS_X = 0xDC, NO5_ABS_X = 0xFC, RLA_ABS_X = 0x3F,
  ROL_ABS_X = 0x3E, ROR_ABS_X = 0x7E, RRA_ABS_X = 0x7F, SAY_ABS_X = 0x9C,
  SBC_ABS_X = 0xFD, SLO_ABS_X = 0x1F, SRE_ABS_X = 0x5F, STA_ABS_X = 0x9D,

  // Absolute,Y
  ADC_ABS_Y = 0x79, AND_ABS_Y = 0x39, AXA_ABS_Y = 0x9F, CMP_ABS_Y = 0xD9,
  DCP_ABS_Y = 0xDB, EOR_ABS_Y = 0x59, ISC_ABS_Y = 0xFB, LAS_ABS_Y = 0xBB,
  LAX_ABS_Y = 0xBF, LDA_ABS_Y = 0xB9, LDX_ABS_Y = 0xBE, ORA_ABS_Y = 0x19,
  RLA_ABS_Y = 0x3B, RRA_ABS_Y = 0x7B, SBC_ABS_Y = 0xF9, SLO_ABS_Y = 0x1B,
  SRE_ABS_Y = 0x5B, STA_ABS_Y = 0x99, TAS_ABS_Y = 0x9B, XAS_ABS_Y = 0x9E,

  // Zero page
  ADC_ZERO = 0x65, AND_ZERO = 0x25, BIT_ZERO = 0x24, CMP_ZERO = 0xC5,
  CPX_ZERO = 0xE4, CPY_ZERO = 0xC4, DCP_ZERO = 0xC7, EOR_ZERO = 0x45,
  ISC_ZERO = 0xE7, LAX_ZERO = 0xA7, LDA_ZERO = 0xA5, LDX_ZERO = 0xA6,
  LDY_ZERO = 0xA4, NO0_ZERO = 0x04, NO1_ZERO = 0x44, NO2_ZERO = 0x64,
  ORA_ZERO = 0x05, RLA_ZERO = 0x27, RRA_ZERO = 0x67, SBC_ZERO = 0xE5,
  SLO_ZERO = 0x07, SRE_ZERO = 0x47, ASL_ZERO = 0x06, LSR_ZERO = 0x46,
  ROL_ZERO = 0x26, ROR_ZERO = 0x66, INC_ZERO = 0xE6, DEC_ZERO = 0xC6,
  SAX_ZERO = 0x87, STA_ZERO = 0x85, STX_ZERO = 0x86, STY_ZERO = 0x84,

  // Zero page,X
  ADC_ZERO_X = 0x75, AND_ZERO_X = 0x35, ASL_ZERO_X = 0x16, CMP_ZERO_X = 0xD5,
  DCP_ZERO_X = 0xD7, DEC_ZERO_X = 0xD6, EOR_ZERO_X = 0x55, INC_ZERO_X = 0xF6,
  ISC_ZERO_X = 0xF7, LDA_ZERO_X = 0xB5, LDY_ZERO_X = 0xB4, LSR_ZERO_X = 0x56,
  NO0_ZERO_X = 0x14, NO1_ZERO_X = 0x34, NO2_ZERO_X = 0x54, NO3_ZERO_X = 0x74,
  NO4_ZERO_X = 0xD4, NO5_ZERO_X = 0xF4, ORA_ZERO_X = 0x15, RLA_ZERO_X = 0x37,
  ROL_ZERO_X = 0x36, ROR_ZERO_X = 0x76, RRA_ZERO_X = 0x77, SBC_ZERO_X = 0xF5,
  SLO_ZERO_X = 0x17, SRE_ZERO_X = 0x57, STA_ZERO_X = 0x95, STY_ZERO_X = 0x94,

  // Zero page,Y
  LAX_ZERO_Y = 0xB7, LDX_ZERO_Y = 0xB6, SAX_ZERO_Y = 0x97, STX_ZERO_Y = 0x96,

  // (Indirect,X)
  ADC_IND_X = 0x61, AND_IND_X = 0x21, CMP_IND_X = 0xC1, DCP_IND_X = 0xC3,
  EOR_IND_X = 0x41, ISC_IND_X = 0xE3, LAX_IND_X = 0xA3, LDA_IND_X = 0xA1,
  ORA_IND_X = 0x01, RLA_IND_X = 0x23, RRA_IND_X = 0x63, SAX_IND_X = 0x83,
  SBC_IND_X = 0xE1, SLO_IND_X = 0x03, SRE_IND_X = 0x43, STA_IND_X = 0x81,

  // (Indirect),Y
  ADC_IND_Y = 0x71, AND_IND_Y = 0x31, AXA_IND_Y = 0x93, CMP_IND_Y = 0xD1,
  DCP_IND_Y = 0xD3, EOR_IND_Y = 0x51, ISC_IND_Y = 0xF3, LAX_IND_Y = 0xB3,
  LDA_IND_Y = 0xB1, ORA_IND_Y = 0x11, RLA_IND_Y = 0x33, RRA_IND_Y = 0x73,
  SBC_IND_Y = 0xF1, SLO_IND_Y = 0x13, SRE_IND_Y = 0x53, STA_IND_Y = 0x91,

  // Relative (branch instructions)
  BCC = 0x90, BCS = 0xB0, BEQ = 0xF0, BMI = 0x30, BNE = 0xD0, BPL = 0x10,
  BVC = 0x50, BVS = 0x70,

  // Indirect (indirect jump)
  JMP_IND = 0x6C,

  // KIL instructions
  KI0 = 0x02, KI1 = 0x12, KI2 = 0x22, KI3 = 0x32, KI4 = 0x42, KI5 = 0x52,
  KI6 = 0x62, KI7 = 0x72, KI8 = 0x92, KI9 = 0xB2, K10 = 0xD2, K11 = 0xF2
};