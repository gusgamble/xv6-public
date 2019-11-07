#include "types.h"
#include "user.h"

int
main(void)
{
	int i;
	int pid = fork();


	if (pid == 0) {
		for (i = 0; i < 50; i++) {
			/*in the child */
			/*hiproc(); */

			printf(1, "+");

			pyield();
		}
		printf(1, "\n");
	}else {
		for (i = 0; i < 50; i++) {
			/* in the parent */
			/*hiproc(); */
			printf(1, "-");

			pyield();
		}
		printf(1, "\n");
	}
	/*wait(); */
	wait();
	exit();
	return 0;
}
