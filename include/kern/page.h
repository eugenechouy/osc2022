#ifndef PAGE_H
#define PAGE_H
 
#define PAGE_SIZE   4096
#define PAGE_SHIFT  12
#define PAGE_MASK   (~(PAGE_SIZE-1))


#define PGD_SHIFT               39
#define PUD_SHIFT               30
#define PMD_SHIFT               21

#define PTRS_PER_PGD            512
#define PTRS_PER_PUD            512
#define PTRS_PER_PMD            512
#define PTRS_PER_PTE            512

#define pgd_index(vaddr)      (((vaddr) >> PGD_SHIFT) & (PTRS_PER_PGD - 1))
#define pgd_offset(mm, vaddr) ((mm)->pgd + pgd_index(vaddr))

#define pud_index(vaddr)      (((vaddr) >> PUD_SHIFT) & (PTRS_PER_PUD - 1))

#define pmd_index(vaddr)      (((vaddr) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))

#define pte_index(vaddr)      (((vaddr) >> PAGE_SHIFT) & (PTRS_PER_PTE - 1))

void create_pgd(struct mm_struct *mm);
void free_pgd(struct mm_struct *mm);
void fork_pgd(struct mm_struct *parent_mm, struct mm_struct *child_mm);

void *walk(struct mm_struct *mm, unsigned long vaddr, unsigned long paddr);
void *mappages(struct mm_struct *mm, unsigned long vaddr, unsigned long size, unsigned long paddr);

void identity_paging(struct mm_struct *mm, unsigned long vaddr, unsigned long paddr);

#endif