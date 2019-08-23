#include <stdio.h>
#include <math.h>


#define NUMTASKS	8
#define OVERHEAD	0.153	//microseconds

typedef unsigned char uchar_t;

typedef struct timingconstraints
{
	uchar_t feature[100];
	float period;			//milliseconds
} timingconst;

typedef struct task_t
{
	uchar_t taskname[100];
	float 	runtime;				//milliseconds
	float	resource_table_usage;	//milliseconds
	float	channel_usage;			//milliseconds
	float	disk_usage;				//milliseconds
	float	max_blocktime;			//milliseconds
}task;

int main()
{
	int i, k, l, index, l_lim;
	float sum;

	timingconst constraints[] = {
		{"Compute attitude data",10.56},
		{"Compute velocity data" ,40.96 },
		{"Compose attitude message",61.44},
		{"Display data",100.00 },
		{"Compose navigation message",165.00},
		{"Run-time Built-In Test (BIT)",285.00},
		{"Compute position data",350.00 },
		{"Compose test message",700.00}
	};

	task tasks_to_schedule[] = {
		{"attitude", 1.30, 0.20, 0.0, 2.00, 3.30},
		{"velocity", 4.70, 0.20, 0.0, 3.00, 9.30},
		{"position", 3.00, 0.20, 0.0, 3.00, 0.00},
		{"display", 23.00, 0.30, 0.0, 0.00, 5.20},
		{"runtime BIT", 10.00, 0.0, 0.0, 1.00, 5.20},
		{"att message", 9.00, 0.0, 3.00, 0.00, 9.30},
		{"nav message", 38.30, 0.00, 6.00, 0.00, 5.20},
		{"test message",2.00, 0.0, 2.0, 0.00, 0.00}
	};

	/* RMA ALGORITHM */

	for(i=0; i<NUMTASKS; i++)
	{
		for(k = 0; k < (i+1); k++)
		{
			l_lim = floor(constraints[i].period / constraints[k].period);
			for(l = 0; l < l_lim; l++)
			{
				sum = 0;

				//add all previous tasks as long as index is less than i
				for(index = 0; index < i; index++)
				{
					sum += (tasks_to_schedule[index].runtime + OVERHEAD) * ceil(((l + 1)*constraints[k].period) / constraints[index].period);
				}

				sum += tasks_to_schedule[i].runtime + OVERHEAD + tasks_to_schedule[i].max_blocktime;

				if (sum < constraints[k].period*(l + 1))
				{
					printf("The feature: %s is schedulable at k = %d, l = %d. \n", constraints[i].feature, (k + 1), (l +1));

					break;
				}
			}

			//go to the next feature
			if (l < l_lim)
			{
				break;
			}
		}

		if (k == (i+1))
		{
			printf("Index %d: Feature %s is not schedulable.\n",i, constraints[i].feature);
		}
	}

	return 0;
}