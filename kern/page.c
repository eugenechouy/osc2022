#include "mmu.h"
#include "string.h"
#include "kern/slab.h"
#include "kern/kio.h"
#include "kern/mm_types.h"

void *pgtable_walk(unsigned long *table, unsigned long idx) {
    if (!table) {
        kprintf("pgtable_walk failed");
        return 0;
    }
    if (!table[idx]) {
        void *new_page = kmalloc(PAGE_SIZE);
        if (!new_page)
            return 0;
        memset(new_page, 0, PAGE_SIZE);
        table[idx] = VIRT_2_PHY((unsigned long)new_page) | PD_TABLE;
    }
    return (void*)PHY_2_VIRT(table[idx] & PAGE_MASK);
}

void *pgtable_walk_pte(unsigned long *table, unsigned long idx, unsigned long paddr) {
    if (!table) {
        kprintf("pgtable_walk_pte failed");
        return 0;
    }
    if (!table[idx]) {
        if (paddr == 0) {
            void *new_page = kmalloc(PAGE_SIZE);
            if (!new_page)
                return 0;
            memset(new_page, 0, PAGE_SIZE);
            table[idx] = VIRT_2_PHY((unsigned long)new_page) | PTE_NORMAL_ATTR | PD_USER_RW;
        } else
            table[idx] = paddr | PTE_NORMAL_ATTR | PD_USER_RW;
    }
    return (void*)PHY_2_VIRT(table[idx] & PAGE_MASK);
}

void pgtable_walk_block(unsigned long *table, unsigned long idx, unsigned long paddr) {
    if (!table) {
        kprintf("pgtable_walk_block failed");
        return;
    }
    if (!table[idx]) {
        table[idx] = paddr | PUD1_ATTR | PD_USER_RW;
    }
}

void *walk(struct mm_struct *mm, unsigned long vaddr, unsigned long paddr) {
    void *pud;
    void *pmd;
    void *pte;
    
    pud = pgtable_walk(mm->pgd, pgd_index(vaddr));
    pmd = pgtable_walk(pud, pud_index(vaddr));
    pte = pgtable_walk(pmd, pmd_index(vaddr));
    return pgtable_walk_pte(pte, pte_index(vaddr), paddr) + (vaddr & (PAGE_SIZE-1));
}

void *mappages(struct mm_struct *mm, unsigned long vaddr, unsigned long size, unsigned long paddr) {
    if (!mm->pgd)
        return 0;
    for (int i=0 ; i<size ; i+=PAGE_SIZE) {
        if (!paddr)
            walk(mm, vaddr+i, 0);
        else
            walk(mm, vaddr+i, paddr+i);
    }
    return (void*)vaddr;
}

void identity_paging(struct mm_struct *mm, unsigned long vaddr, unsigned long paddr) {
    if (!mm->pgd)
        return;
    void *pud;
    pud = pgtable_walk(mm->pgd, pgd_index(vaddr));
    pgtable_walk_block(pud, pud_index(vaddr), paddr);
}

void create_pgd(struct mm_struct *mm) {
    void *new_page = kmalloc(PAGE_SIZE);
    if (!new_page)
        return;
    memset(new_page, 0, PAGE_SIZE);
    mm->pgd = new_page;
}

void free_pgtable(unsigned long *table, int level) {
    void *next;
    for(int i=0 ; i<512 ; i++) {
        if (table[i]) {
            next = PHY_2_VIRT(table[i] & PAGE_MASK);
            if (level != 4) 
                free_pgtable(next, level+1);
            // if device memory
            if (level == 4 && VIRT_2_PHY(next) >= 0x3C000000)
                continue;
            kfree(next);
        }
    }
}

void free_pgd(struct mm_struct *mm) {
    unsigned long *pgd = mm->pgd;
    void *pud;
    for(int i=0 ; i<512 ; i++) {
        if (pgd[i]) {  
            pud = PHY_2_VIRT(pgd[i] & PAGE_MASK);
            free_pgtable(pud, 2);
            kfree(pud);
        }
    }
    kfree(pgd);
}

/*
        level
    pgd   1
    pud   2
    pmd   3
    pte   4
*/
void fork_pgtable(unsigned long *ptable, unsigned long *ctable, int level) {
    void *pnext;
    void *cnext;

    for(int i=0 ; i<512 ; i++) {
        if (ptable[i]) {
            pnext = PHY_2_VIRT(ptable[i] & PAGE_MASK);
            if (level == 4) {
                cnext = pgtable_walk_pte(ctable, i, 0);
                memcpy(cnext, pnext, PAGE_SIZE);
            } else {
                cnext = pgtable_walk(ctable, i);
                fork_pgtable(pnext, cnext, level+1);
            }
        }
    }
}

void fork_pgd(struct mm_struct *parent_mm, struct mm_struct *child_mm) {
    unsigned long *ppgd = parent_mm->pgd;
    unsigned long *cpgd = child_mm->pgd;
    void *ppud;
    void *cpud;
    
    if (!ppgd || !cpgd) 
        return;
    
    for(int i=0 ; i<512 ; i++) {
        if (ppgd[i]) {  
            ppud = PHY_2_VIRT(ppgd[i] & PAGE_MASK);
            cpud = pgtable_walk(cpgd, i);
            fork_pgtable(ppud, cpud, 2);
        }
    }
}