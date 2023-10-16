#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main(int argc, char*argv[])
{
	struct sockaddr_in addr1, addr2;
	char *str_ptr;
	char str_arr[20];

	addr1.sin_addr.s_addr=htonl(0x1020304);
	addr2.sin_addr.s_addr=htonl(0x1010101);

	str_ptr = inet_ntoa(addr1.sin_addr);
	strcpy(str_arr, str_ptr); // strcpy(dest, src)    buffer value exchange   str_ptr is just buffer referrence !! so we copy this value because exchange whenever  every inet_ntoa call!!
	printf("Dotted-Decimal notation1: %s \n",str_ptr);

	str_ptr = inet_ntoa(addr2.sin_addr); 			// we can use  inet_ntoa(addr2.sin_addr) .. line 15 explain why.. but we use this expression Why?? this expression looks Good !!~!~! > <
	printf("Dotted_Decimal notation2: %s \n", str_ptr);
	return 0;

}
