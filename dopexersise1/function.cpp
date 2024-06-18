#include <stdio.h>
#include <stdlib.h>

void processing_argument (int *stat, int *dynamic, int *local){
	printf("Статический аргумент: %d\n", *stat);
	printf("Динамический аргумент: %d\n", *dynamic);
	printf("Локальный аргумент: %d\n", *local);
}

int main (){
	static int StatVar = 10;
	int LocalVar = 66;

	int *DynamicVar = (int *)malloc(sizeof(int));
	if (DynamicVar == NULL) {
		fprintf(stderr, "Ошибка выделения памяти\n");
		return 1;
	}
	*DynamicVar=59;

	processing_argument (&StatVar,DynamicVar,&LocalVar);

	free (DynamicVar);
	return 0;
}
