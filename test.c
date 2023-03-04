#include <stdio.h>

int main()
{
	char *str[10];
	char uri[10] = "호구";

	*str = uri + 3;

	printf("%s \n", *str);
}