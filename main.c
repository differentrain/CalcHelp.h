#include "calcHelper.h"
#include <stdio.h>
#include <string.h>


int main()
{
	CALCH_error err;
	CALCH_int32 intResult;
	CALCH_decimal decResult;
	int stopLoop, mode;
	char buffer[CALCH_MAX_EXP_LENGTH];
	stopLoop = 0;
	mode = 0;
	printf("Simple Calculator Version:%d\n", CALCH_VERSION);
	while (stopLoop == 0)
	{
		printf("=================\npro=programmer mode\nmth=math mode\nexit=exit\nor input an exp to compute:\n");
		scanf("%s", &buffer);
		if (buffer[0] == 'e' && buffer[1] == 'x' && buffer[2] == 'i'  && buffer[3] == 't')
		{
			stopLoop = 1;
		}
		else if (buffer[0] == 'p' && buffer[1] == 'r' && buffer[2] == 'o')
		{
			mode = 0;
		}
		else if (buffer[0] == 'm' && buffer[1] == 't' && buffer[2] == 'h')
		{
			mode = 1;
		}
		else if (mode == 0)
		{
			err = CALCH_computeExpInt32(buffer, strlen(buffer), &intResult);
			if (err == CALCH_NO_ERROR)
			{
				printf("result is %d\n", intResult);
			}
			else
			{
				printf("ERROR\n");
			}
		}
		else
		{
			err = CALCH_computeExpMath(buffer, strlen(buffer), &decResult);
			if (err == CALCH_NO_ERROR)
			{
				printf("result is %f\n", decResult);
			}
			else
			{
				printf("ERROR\n=================\n");
			}
		}

	}

	return 0;
}
