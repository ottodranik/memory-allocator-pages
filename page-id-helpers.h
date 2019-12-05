#pragma once

#include "allocator.h"

/** 
 * Устанавливает/возвращает первый бит переданного 4-байтного DWORD слова,
 * чтобы понять это хэдер или нет
 */
void setHeadBit(int *value);
int isHeadBit(int value);

/** Устанавливает/возвращает занятость страницы
 */
void setBusy(int *pageId, bool busy);
bool isBusy(int pageId);

/** 
 * Устанавливает/возвращает номер страницы
 */
void setPageNumber(int *pageId, int num);
int getPageNumber(int pageId);

/** Устанавливает/возвращает количество занятых страниц в блоке
 */
void setPagesInBlock(int *pageId, int num);
int getPagesInBlock(int pageId);

/** 
 * Устанавливает/возвращает количество свободных блоков слева
 */
void setLeftBlocks(int *pageId, int left);
int getLeftBlocks(int pageId);

/** 
 * Устанавливает/возвращает указатель на первый свободный блок
 */
void setPointerToBlock(int *pageId, int *pointerToPage, void *pointer);
void *getPointerToBlock(int pageId, int *pointerToPage);

/** 
 * Устанавливает/возвращает поделена ли страница на подклассы
 */
void setTypeClass(int *pageId, bool classType);
bool isTypeClass(int pageId);

/** 
 * Устанавливает/возвращает размер подкласса (степень двойки)
 */
void setSizeOfClass(int *pageId, int size);
int getSizeOfClass(int pageId);

/** 
 * Устанавливает/возвращает количество выделенных байт в блоке в подклассе страницы
 */
void setClassBlockAllocedBytes(int *header, int allocedBytes);
int getClassBlockAllocedBytes(int header);

/** 
 * Вернуть размер блока, записанный в страницу(ы)
 */
int getPageBlockAllocatedBytes(int i);