#ifndef SYNC_H
#define SYNC_H

/*
    ESR-EL1
*/
#define EC_BITS(esr_el1) ((esr_el1 >> 26) & 0b111111) 
#define EC_SVC_32 0b010001
#define EC_SVC_64 0b010101
#define EC_IA_EL0 0b100000
#define EC_IA_EL1 0b100001
#define EC_DA_EL0 0b100100
#define EC_DA_EL1 0b100101

#define IFSC(esr_el1) (esr_el1 & 0b111111)
#define DFSC(esr_el1) (esr_el1 & 0b111111)

#define TRANS_FAULT_0 0b000100

#endif
