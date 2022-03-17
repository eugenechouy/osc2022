
unsigned int strlen(const char *str) {
	const char *c;
	for (c=str ; *c ; ++c);
	return c - str;
}

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

int strncmp (const char *s1, const char *s2, unsigned int n) {
	unsigned char c1 = '\0';
	unsigned char c2 = '\0';

	// if (n >= 4) {
	// 	unsigned int n4 = n >> 2;
	// 	do {
	// 		c1 = (unsigned char) *s1++;
	// 		c2 = (unsigned char) *s2++;
	// 		if (c1 == '\0' || c1 != c2)
	// 			return c1 - c2;
	// 		c1 = (unsigned char) *s1++;
	// 		c2 = (unsigned char) *s2++;
	// 		if (c1 == '\0' || c1 != c2)
	// 			return c1 - c2;
	// 		c1 = (unsigned char) *s1++;
	// 		c2 = (unsigned char) *s2++;
	// 		if (c1 == '\0' || c1 != c2)
	// 			return c1 - c2;
	// 		c1 = (unsigned char) *s1++;
	// 		c2 = (unsigned char) *s2++;
	// 		if (c1 == '\0' || c1 != c2)
	// 			return c1 - c2;
	// 	} while (--n4 > 0);
	// 	n &= 3;
	// }

	while (n > 0) {
		c1 = (unsigned char) *s1++;
		c2 = (unsigned char) *s2++;
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		n--;
	}
	return c1 - c2;
}

void strrev(char* str, unsigned int len) {
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

int atoi(char* str, int base, unsigned int len) {
	int num = 0;
	int i;
	for (i=0 ; i<len ; i++) {
		num = num * base;
		if (str[i] >= 'A')
			num += str[i] - 'A' + 10;
		else 
			num += str[i] - '0';
	}
	return num;
}