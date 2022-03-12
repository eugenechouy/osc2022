#ifndef STRING_H
#define STRING_H

int strcmp (const char *p1, const char *p2);
int strncmp (const char *s1, const char *s2, unsigned int len);
int itoa(int num, char* str, int base);
int atoi(char* str, int base, unsigned int len);

#endif