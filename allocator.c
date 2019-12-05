#include <math.h>

#include "allocator.h"
#include "page-id-helpers.h"

void fillClassesWithPointers(void * pagePointer, int blocks, int size);
void clearPage(int num);
void generateNewBlockId(int blockStart, int pages, int size);

/**
 * Выделяем память
 * - выбираем размер блока
 * - ищем пустой блок
 * - заполняем идентификатор страницы pageId
 */
void *memAlloc(size_t size) {

	int searchSize = size + sizeof(int);

	// если размер данных меньше, чем половина страницы,
	// то выделяем место в странице и делим её на классы
	if (searchSize < PAGE_SIZE / 2) {
		int classSize = selectSizeOfClassInPage(searchSize);

		// ищем, есть ли уже страница, с подходящим размером классов
		int pageOfClass = findPageWithClass(classSize);

		// если уже размеченной страницы нет ->
		if (pageOfClass == -1) {
			// -> то берём свободную страницу 
			pageOfClass = findFreePage();

			// -> если свободная страница есть ->
			if (pageOfClass != -1) {
				// -> создаём идентификатор class-тип страницы
				pageIds[pageOfClass] = fillClassId(pageOfClass, classSize);
			} else {
				// -> ИЛИ берём страницу с ближайшим бОльшим классом
				pageOfClass = findPageWithGreaterClass(classSize);

				// -> если подходящей страницы нет вообще, то выходим
				if (pageOfClass == -1) {
					return NULL;
				}
			}
		}

		int *pageId = &(pageIds[pageOfClass]);
		int *pointerToPage = (int*)globalMem + PAGE_SIZE * pageOfClass / sizeof(int);
		int *pointerToBlock = getPointerToBlock(*pageId, pointerToPage);

		// устанавливает указатель pageId на следующий свободный блок
		setPointerToBlock(
			pageId, pointerToPage,
			getPointerToBlock(*pointerToBlock, pointerToPage)
		);

		// уменьшаем количество свободных блоков
		setLeftBlocks(pageId, getLeftBlocks(*pageId) - 1);

		// создаём хэдер блока
		int header = generateClassHeader(getSizeOfClass(*pageId), searchSize, pageOfClass);

		// возвращаем указатель на блок 
		*pointerToBlock = header;
		return pointerToBlock;
	}

	// Иначе: выделяем целую страницу или группу страниц

	// посчитать сколько страниц выбрать
	int numOfPages = chooseNumberOfPages(searchSize);

	// находим нужное количество пустых страниц подряд
	int blockStart = findMultPageBlock(numOfPages);

	if (blockStart == -1) {
		return NULL;
	}

	generateNewBlockId(blockStart, numOfPages, searchSize);

	return (void *)((int *)globalMem + blockStart * PAGE_SIZE / sizeof(int));
}

void *memRealloc(void *addr, size_t size) {
	int *pointerToBlock = (int *)addr;

	// пропустить, если не хэдер
	if (isHeadBit(*pointerToBlock) != 1) {
		return NULL;
	}

	int newSize = (int)size + sizeof(int);

	// eсли блок class-типа (<= половины страницы) ->
	if (isTypeClass(*pointerToBlock) == true) {
		// -> если новый размер равен уже предыдущему, то просто вернуть адрес
		if (newSize == getClassBlockAllocedBytes(*pointerToBlock)) {
			return addr;
		}

    // -> если новый размер <= размера старого класса, то просто
		//    выделить память под блок на месте старого куска
		if (newSize <= getSizeOfClass(*pointerToBlock)) {
			setClassBlockAllocedBytes(pointerToBlock, newSize);
			return addr;
		}

		// -> иначе просто выделить новый блок через memAlloc
		void *newPointer = memAlloc(size);
		if (newPointer != NULL) {
			memFree((void *)pointerToBlock);
			return newPointer;
		}
		return NULL;
	}

	// eсли блок не class-типа
	if (newSize == (*pointerToBlock) & 0xffff) {
		return addr;
	}
	
	int currentPage = (pointerToBlock - (int *)globalMem) * sizeof(int) / PAGE_SIZE;
	int pagesInBlock = getPagesInBlock(pageIds[currentPage]);
	
	// если новый размер меньше старого ->
	if (newSize <= pagesInBlock * PAGE_SIZE) {
		int newPages = chooseNumberOfPages(newSize);

		// -> очистить размер, который нам не нужен
		for (int i = newPages; i < pagesInBlock; i++) {
			pageIds[currentPage + i] = fillNonClassId(currentPage + i, 1, false);
		}

		// -> установить новый размер блока
		*pointerToBlock = fillNonClassId(0, 0, true) + newSize;

		// -> заполнить новый идентификатор страницы
		pageIds[currentPage] = fillNonClassId(currentPage, newPages, true);

		return addr;
	}

	// eсли новый размер больше старого, то просто 
	// проинициализировать новый блок через memAlloc
	void *newPointer = memAlloc((size_t)newSize);
	if (newPointer != NULL) {
		memFree(pointerToBlock);
		return newPointer;
	}

	return NULL;
}

void memFree(void * addr) {
	int *pointerToBlock = (int *)addr;

	// пропустить, если не хэдер
	if (isHeadBit(*pointerToBlock) != 1) {
		return;
	}

	// для выделенных блоков class-типа
	if (isTypeClass(*pointerToBlock) == true) {
		int currentPage = getPageNumber(*pointerToBlock);
		int * pointerToPage = (int*)globalMem + PAGE_SIZE * currentPage / sizeof(int);
		int * pageId = &pageIds[currentPage];

		*pointerToBlock = (int)getPointerToBlock(*pageId, pointerToPage);
		setPointerToBlock(pageId, pointerToPage, (void *)pointerToBlock);

		// увеличить количество свободных блоков на странице
		setLeftBlocks(pageId, getLeftBlocks(*pageId) + 1);

		// вернуть страницу в начальное состояние - не class-тип
		if (getLeftBlocks(*pageId) == PAGE_SIZE / getSizeOfClass(*pageId)) {
			*pageId = fillNonClassId(currentPage, 1, false);
		}

		return;
	}

	// для выделенных блоков nonclass-типа
	int currentPage = (pointerToBlock - (int *)globalMem) * sizeof(int) / PAGE_SIZE;
	int pagesInBlock = getPagesInBlock(pageIds[currentPage]);
	for (int i = 0; i < pagesInBlock; i++) {
		clearPage(currentPage + i);
	}
}

/** 
 * Заполнить идентификатор nonclass-страницы данными:
 * @return заполненный id (хэдер) страницы
 */ 
int fillNonClassId(int num, int pagesInBlock, bool busy) {
	int pageId = generatePageId(num, false); // сгенерировать хэдер class-тип страницы
	setBusy(&pageId, busy); // установить флаг занятости
	setPagesInBlock(&pageId, pagesInBlock); // установить количество страниц, которое занимает блок
	return pageId;
}

/**
 * Заполняем id (хэдер) страницы с типа class
 * @return заполненный id (хэдер) страницы
 */
int fillClassId(int num, int size) {
	printf("fill class\n");
	int pageId = generatePageId(num, true); // сгенерировать хэдер class-тип страницы
	setSizeOfClass(&pageId, size); // указать размер класса
	setLeftBlocks(&pageId, PAGE_SIZE / size); // указать количество свободных блоков

	void *pagePointer = (int *)globalMem + num * PAGE_SIZE / sizeof(int);
	fillClassesWithPointers(pagePointer, PAGE_SIZE / size, size); // указатели на начало блоков внутри страницы
	setPointerToBlock(&pageId, (int *)pagePointer, pagePointer); // указатель на начало страницы
	return pageId;
}

/**
 * Заполняем указатели на начальный байт каждого блока
 */
void fillClassesWithPointers(void *pagePointer, int blocks, int size) {
	int *thisBlock = (int *)pagePointer;
	int *nextBlock;

	for (int i = 0; i < blocks - 1; i++) {
		nextBlock = thisBlock + size / sizeof(int);
		*thisBlock = 0;
		setPointerToBlock(thisBlock, pagePointer, (void *)nextBlock);
		thisBlock = nextBlock;
	}
	*thisBlock = 0;
}

/**
 * Очистить все страницы.
 * Заполнить идентификаторы пустыми значениями с busy = false и pagesInBlock = 1
 * Очистить указатели на страницы
 */
void clearPage(int num) {
	pageIds[num] = fillNonClassId(num, 1, false);
	int *pointerToPage = (int *)globalMem + PAGE_SIZE * num / sizeof(int);
	*pointerToPage = 0;
}

int selectSizeOfClassInPage(int size) {
	int classSize = 16;
	while (classSize < size) {
		classSize *= 2;
	}
	return classSize;
}

/**
 * Посчитать сколько страниц выделить для нового блока
 * @return количество страниц
 */
int chooseNumberOfPages(int size) {
	int pageNum = PAGE_SIZE;
	while (pageNum < size) {
		pageNum += PAGE_SIZE;
	}
	return pageNum / PAGE_SIZE;
}

/**
 * Выделить память под нужное количество страниц
 * @return количество страниц
 */
int findMultPageBlock(int pages) {
	int blockStart = 0;
	while (PAGE_NUMBER > pages + blockStart) {
		int pageId;

		//int currentPage = blockStart;
		for (int i = blockStart; i < blockStart + pages; i++) {
			pageId = pageIds[blockStart];

			// пропустить, если страница занята или страница class-типа
			if (isTypeClass(pageId) != 0 || isBusy(pageId) != 0) {
				blockStart = i + 1;
				break;
			}
		}

		// если страница не занята и не class-типа, то вернуть адрес для блока
		if (isTypeClass(pageId) == 0 && isBusy(pageId) == 0) {
			return blockStart;
		}
	}
	return -1;
}

/**
 * Проходимся по страницам и находим страницу class-типа,
 * с указанным размером класса
 * @return индекс найденной страницы, либо -1
 */ 
int findPageWithClass(int classSize) {
	for (int i = 0; i < PAGE_NUMBER; i++) {
		int pageId = pageIds[i];
		if (
			isTypeClass(pageId) == 1 // страница должна быть поделена на классы 
			&& getSizeOfClass(pageId) == classSize // класс должен быть нужного размера
			&& getLeftBlocks(pageId) > 0 // количество свободных блоков должно быть >0
		) {
			return i;
		}
	}
	return -1;
}

/**
 * Найти свободную страницу без классов
 * @return индекс найденной страницы, либо -1
 */ 
int findFreePage() {
	for (int i = 0; i < PAGE_NUMBER; i++) {
		int pageId = pageIds[i];
		// если не class-тип и не занят, то вернуть индекс страницы
		if (isTypeClass(pageId) == 0 && isBusy(pageId) == 0) {
			return i;
		}
	}
	return -1;
}

/**
 * Найти страницу с ближайшим подходящим по размеру классом,
 * бОльшим данного (увеличиваем класс и ищем пока не найдём)
 * @return индекс найденной страницы, либо -1
 */
int findPageWithGreaterClass(int classSize) {
	classSize *= 2;
	while (classSize < PAGE_SIZE) {
		int page = findPageWithClass(classSize);
		if (page != -1) {
			return page;
		}
	}
	return -1;
}

/**
 * Генерация id (хэдера) страницы
 * @return идентификатор с данными
 */ 
int generatePageId(int page, bool classType) {
	int pageId = 0;

	// 1 в первом бите указывает на то, что это хэдер
	setHeadBit(&pageId);

	// установка бита, указывающего на тип страницы
	setTypeClass(&pageId, classType);

	// установить номер страницы
	setPageNumber(&pageId, page);

	return pageId;
}

/**
 * Генерация id нового блока
 */
void generateNewBlockId(int blockStart, int pages, int size) {
	pageIds[blockStart] = fillNonClassId(blockStart, pages, true);
	int *pointerToPage = (int *)globalMem + blockStart * PAGE_SIZE / sizeof(int);
	*pointerToPage = fillNonClassId(0, 0, true) + size;

	for (int i = 1; i < pages; i++) {
		pageIds[blockStart + i] = fillNonClassId(blockStart + i, 0, true);
	}
}

/**
 * Создаём хэдер-байт класса
 */
int generateClassHeader(int classSize, int busyBytes, int num) {
	int header = 0;
	setHeadBit(&header); // бит хэдера
	setTypeClass(&header, true); // типа class = true
	setSizeOfClass(&header, classSize); // размер класса
	setClassBlockAllocedBytes(&header, busyBytes); // количество занятых байтов
	setPageNumber(&header, num); // номер страницы

	return header;
}

/**
 * Создаём страницы
 * (по умолчанию - пустые и nonclass-типа)
 */
void generatePages() {
	for (int i = 0; i < PAGE_NUMBER; i++) {
		clearPage(i);
	}
}
