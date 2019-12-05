#pragma once

#define PAGE_SIZE 1024
#define PAGE_NUMBER 8
#define MEM_SIZE PAGE_SIZE * PAGE_NUMBER

#include <stdio.h>

typedef enum { false, true } bool;

// Указатель на начало выделенной памяти
void *globalMem;

// Массив указателей на страницы
int pageIds[PAGE_NUMBER];

void *memAlloc(size_t size);

void *memRealloc(void *addr, size_t size);

void memFree(void *addr);

// Создать чистые страницы
void generatePages();
