#include <stdio.h>
#include <string.h>

static char load_data_char_list[] =
	"\n !\"#$%&'()*+,-./0123456789:;<=>?@"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"[\\]^_`"
	"abcdefghijklmnopqrstuvwxyz"
	"{|}~";

int main(void)
{
	size_t len = strlen(load_data_char_list);
	unsigned int i;
	for (i = 0; i < len; i++) {
		if (i % 10 == 0)
			printf("%02d:", i);
		printf(" %c", load_data_char_list[i]);
		if ((i + 1) % 10 == 0)
			printf("\n");
	}
	printf("\n");
	return 0;
}
