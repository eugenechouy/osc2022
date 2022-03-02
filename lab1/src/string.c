
// https://code.woboq.org/userspace/glibc/string/
int strcmp (const char *p1, const char *p2) {
    const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;
    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);
    return c1 - c2;
}

void strrev(char *str, unsigned int len) {
	int i;
	int j;
	char a;
	for (i = 0, j = len - 1; i < j; i++, j--) {
		a = str[i];
		str[i] = str[j];
		str[j] = a;
	}
}

int itoa(int num, char* str, int base) {
	int sum = num;
	int i = 0;
	int digit;
	do {
		digit = sum % base;
		if (digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
		sum /= base;
	} while(sum);
	if (sum)
		return -1;
	str[i] = '\0';
	strrev(str, i);
	return 0;
}