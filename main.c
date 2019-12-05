// Главный файл программы MemoryAllocatorPages
//

#include "allocator.h"
#include "helpers.h"
#include <stdlib.h>

int main() {
	globalMem = malloc(MEM_SIZE);
	printBeforeStart();
	
	generatePages();

	memDump();

	printf("*** a = memAlloc(1555) ***\n");
	void * a = memAlloc(1555);	
	memDump();

	printf("*** c1 = memAlloc(250) ***\n");
	void * c1 = memAlloc(250);	/** +4 = 254 < 256. Попадает в 256 байт.
															  * Делит одну пустую страницу на 4 класса по 256 байт и занимает 1 класс.
															  */
	memDump();

	printf("*** b = memAlloc(1024) ***\n");
	void * b = memAlloc(1024);	/** +4 = 1028 > 1024. Превышает размер страницы.
															  * Занимает две страницы.
															  */
	memDump();

	printf("*** c2 = memAlloc(46) ***\n");
	void * c2 = memAlloc(46); /** +4 = 50 < 64. Попадает в 64 байт.
															 * Делит одну пустую страницу на 16 класса по 64 байта и занимает 1 класс.
															 */
	memDump();

	printf("*** c3 = memAlloc(126) ***\n");
	void * c3 = memAlloc(126); /** +4 = 130 > 128. Попадает в 256 байт. 
															  * Такой класс уже есть выше - c1. Там были пустые 3 класса.
															  * Итог: +1 занятый класс
															  */	
	memDump();

	printf("*** c4 = memAlloc(34) ***\n");
	void * c4 = memAlloc(34); /** +4 = 38 > 32. Попадает в 64 байт. 
															 * Такой класс уже есть выше - c2. Там были пустые 15 классов.
															 * Итог: +1 занятый класс
															 */
	memDump();

	printf("*** memFree(с2)) ***\n");
	memFree(c2); /** Освобождает с2. Освобождает 1 из 16-ти 64-ёхбайтных класса. 
								  * Итог: -1 занятый класс
								  */
	memDump();

	printf("*** c1 = memRealloc(c1, 789) ***\n");
	c1 = memRealloc(c1, 789); /** Переиспользует c1.
															 * Так как прошлый размер =250+4, а новый =789+4,
															 * то освобождает 1 из 4-ёх 256-тибайтных класса
															 * и занимает под свои нужды свободный page.
														   */
	memDump();

	// printf("*** a = memRealloc(a, 3000) ***\n");
	// a = memRealloc(a, 3000);
	// memDump();

	// printf("*** memFree(c) ***\n");
	// memFree(c);
	// memDump();

	printPageIds();

  return 0;
}

