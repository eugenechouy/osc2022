#include "mmu.h"
#include "kern/slab.h"
#include "kern/mm_types.h"

void *pgtable_walk(unsigned long *table, unsigned long idx) {
    if (!table) 
        return 0;
    if (!table[idx]) {
        void *new_page = kmalloc(4096);
        if (!new_page)
            return 0;
        table[idx] = VIRT_2_PHY((unsigned long)new_page) | PD_TABLE;
    }
    return (void*)PHY_2_VIRT(table[idx] & PAGE_MASK);
}

void *pgtable_walk_pte(unsigned long *table, unsigned long idx, unsigned long paddr) {
    if (!table) 
        return 0;
    if (!table[idx]) {
        table[idx] = paddr | PTE_NORMAL_ATTR | PD_USER_RW;
    }
    return (void*)PHY_2_VIRT(table[idx] & PAGE_MASK);
}

void *walk(struct mm_struct *mm, unsigned long vaddr, unsigned long paddr) {
    void *pud;
    void *pmd;
    void *pte;
    pud = pgtable_walk(mm->pgd, pgd_index(vaddr));
    pmd = pgtable_walk(pud, pud_index(vaddr));
    pte = pgtable_walk(pmd, pmd_index(vaddr));
    return pgtable_walk_pte(pte, pte_index(vaddr), paddr);
}

void *mappages(struct mm_struct *mm, unsigned long vaddr, unsigned long size, unsigned long paddr) {
    if (!mm->pgd) {
        void *new_page = kmalloc(4096);
        if (!new_page)
            return 0;
        mm->pgd = VIRT_2_PHY(new_page);
    }
    for (int i=0 ; i<size ; i+=4096) 
        walk(mm, vaddr+i, paddr+i);
    return (void*)vaddr;
}