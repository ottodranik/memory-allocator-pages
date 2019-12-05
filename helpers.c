#include "helpers.h"

void memDump() {
	for (int i = 0; i < PAGE_NUMBER; i++) {
		int currentPointer = pageIds[i]; // указатель на хэдер с информацией
		// если страница без деления на классы ->:
		if (isTypeClass(currentPointer) == 0) {
			printf("Page #%d\n\ttype: non-class\n", getPageNumber(currentPointer));

			// -> получить количество страниц в блоке
			int pages = getPagesInBlock(currentPointer);

			// -> если страница занята -> 
			if (isBusy(currentPointer)) {
				// -> если в блоке есть страницы ->
				if (pages != 0) {
					// -> размер, который занимает блок в байтах
					printf("\tbusy: yes, %d\n", getPageBlockAllocatedBytes(i));

					// -> страниц в блоке
					printf("\tpages in block: %d\n", pages);
				} else {
					printf("\tbusy: yes\n");
				}
			} else {
				printf("\tbusy: no\n");
			}
			printf("\n");
			continue;
		}

		// -> если страница с делением на классы:
		
		// получить номер страницы
		printf("Page #%d\n\ttype: class\n", getPageNumber(currentPointer));

		// получить размер класса (кратный степени 2)
		int classSize = getSizeOfClass(currentPointer);

		printf("\tclass size: %d\n", classSize);
		printf("\tleft free classes: %d\n", getLeftBlocks(currentPointer));

		//int * pointerToFree = getPointerToBlock(currentPointer);
		int *pointerToBlock = (int *)globalMem + i * PAGE_SIZE / sizeof(int);

		// печать всех блоков
		for (int i = 0; i < PAGE_SIZE / classSize; i++) {
			// если первый бит - хэдер, то блок занят
			if (isHeadBit(*pointerToBlock) == 1) {
				printf("\tblock #%d\n", i);
				printf("\tbusy: yes\n");
				printf("\t\t%d%c %d%c %d%c\n",
					sizeof(int), HEADER_BYTE,
					getClassBlockAllocedBytes(*pointerToBlock) - sizeof(int), ALLOCED_BYTE,
					classSize - getClassBlockAllocedBytes(*pointerToBlock), EMPTY_BYTE
				);

				pointerToBlock += classSize / sizeof(int);
				continue;
			}

			// Показать пустые блоки в классе
			// printf("block #%d\n", i);
			// printf("busy: no\n");

			pointerToBlock += classSize / sizeof(int);
		}

		printf("\n");
	}
}

void printBeforeStart() {
	printf("\n====== Page size: %d ====== Pages: %d ====== globalMem: %d\n\n", PAGE_SIZE, PAGE_NUMBER, globalMem);
}

void printPageIds() {
	for (int i = 0; i < sizeof(pageIds)/sizeof(pageIds[0]); i++) {
		printf("Value: %d. Memory: %d. \n", pageIds[i], 0);
	}
}
