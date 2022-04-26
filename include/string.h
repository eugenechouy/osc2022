#ifndef STRING_H
#define STRING_H

unsigned int strlen (const char *str);
int strcmp (const char *p1, const char *p2);
int strncmp (const char *s1, const char *s2, unsigned int len);

int itoa(long num, char* str, int base);
long atoi(char* str, int base, unsigned int len);

void* memcpy (void* dest, const void* src, unsigned int len);

#endif