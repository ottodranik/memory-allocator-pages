#include "page-id-helpers.h"

void shiftRight(int * pointer, int shiftValue) {
	*pointer = *pointer >> shiftValue;
}
void shiftLeft(int * pointer, int shiftValue) {
	*pointer = *pointer << shiftValue;
}

void setHeadBit(int *value) {
	int head = 1;
	shiftLeft(&head, 24);
	*value += head;
}
int isHeadBit(int value) {
	shiftRight(&value, 24);
	return value & 255;
}

void setBusy(int *pageId, bool busy) {
	int ifBusy = busy;
	shiftLeft(&ifBusy, 22);

	int oldIfBusy = (int)isBusy(*pageId);
	shiftLeft(&oldIfBusy, 22);

	*pageId += ifBusy - oldIfBusy;
}
bool isBusy(int pageId) {
	shiftRight(&pageId, 22);
	int busy = pageId & 1;
	return true ? busy == 1 : false;
}

void setPageNumber(int *pageId, int num) {
	*pageId += num;
}
int getPageNumber(int pageId) {
	return pageId & 127;
}

void setPagesInBlock(int *pageId, int num) {
	shiftLeft(&num, 14);

	int oldNum = getPagesInBlock(*pageId);
	shiftLeft(&oldNum, 14);

	*pageId += num - oldNum;
}
int getPagesInBlock(int pageId) {
	shiftRight(&pageId, 14);
	return pageId & 255;
}

void setLeftBlocks(int *pageId, int left) {
	shiftLeft(&left, 13);

	int oldLeft = getLeftBlocks(*pageId);
	shiftLeft(&oldLeft, 13);

	*pageId += left - oldLeft;
}
int getLeftBlocks(int pageId) {
	shiftRight(&pageId, 13);
	return pageId & 127;
}

void setPointerToBlock(int *blockPointer, int *pointerToPage, void *pointer) {
	int oldBlockPointer = *blockPointer;
	shiftRight(&oldBlockPointer, 7);
	int oldLocalPointer = oldBlockPointer & 63;

	//int *pagePointer = (int *)globalMem + PAGE_SIZE * getPageNumber(*pageId) / sizeof(int);
	int localPointer = ((int *)pointer - pointerToPage) * sizeof(int) / 16;

	shiftLeft(&localPointer, 7);
	shiftLeft(&oldLocalPointer, 7);

	*blockPointer += localPointer - oldLocalPointer;
}
void *getPointerToBlock(int pageId, int *pointerToPage) {
	//int * pagePointer = (int *)globalMem + PAGE_SIZE * getPageNumber(pageId) / sizeof(int);

	shiftRight(&pageId, 7);
	int localPointer = pageId & 63;

	int *pointer = pointerToPage + localPointer * 16 / sizeof(int);
	return (void *)pointer;
}

void setTypeClass(int *pageId, bool classType) {
	int type = classType;
	shiftLeft(&type, 23);
	*pageId += type;
}
bool isTypeClass(int pageId) {
	shiftRight(&pageId, 23);
	int typeClass = pageId & 1;
	return true ? typeClass == 1 : false;
}

void setSizeOfClass(int *pageId, int size) {
	int pow = -4;
	while (size % 2 == 0) {
		pow++;
		size = size / 2;
	}

	shiftLeft(&pow, 20);
	*pageId += pow;
}
int getSizeOfClass(int pageId) {
	shiftRight(&pageId, 20);
	int pow = (pageId & 7) + 4;
	int size = 1;

	for (int i = 0; i < pow; i++) {
		size *= 2;
	}

	return size;
}

void setClassBlockAllocedBytes(int *header, int allocedBytes) {
	allocedBytes -= 1;
	shiftLeft(&allocedBytes, 11);

	int oldAllocedBytes = getClassBlockAllocedBytes(*header) - 1;
	shiftLeft(&oldAllocedBytes, 11);

	*header += allocedBytes - oldAllocedBytes;
}
int getClassBlockAllocedBytes(int header) {
	shiftRight(&header, 11);
	int allocedBytes = header & 511;
	return allocedBytes + 1;
}

/**
 * b = PAGE_SIZE * i - количество байт от начала кучи
 * w = b / sizeof(int) - количество слов размера int (4 байта) от начала кучи
 * p = (int *)globalMem + w - указатель на память, где хранится информация
 * (*(p)) & (MEM_SIZE - 1) - значение указателя побитово сравниваем с MEM_SIZE,
 * чтобы убрать лишние биты и получить фактическое значение блока
 */
int getPageBlockAllocatedBytes(int i) {
	return (*((int *)globalMem + PAGE_SIZE * i / sizeof(int))) & (MEM_SIZE - 1);
}