#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libcoro.h"
#include "vector.h"

#define INF ~(1<<31)

/**
 * You can compile and run this code using the commands:
 *
 * $> gcc solution.c libcoro.c
 * $> ./a.out
 */


// TARGET LATENCY is in microseconds
static int TARGET_LATENCY;
static int CORO_NUM;


struct segment {int l, r;};

struct segment* create_segment(int l, int r) {
	struct segment* seg = malloc(sizeof(struct segment));
	seg->l = l; seg->r = r;
	return seg;
} 

struct vector* read_array(char* filename) {
	struct vector* array = vector_create(1);
	
	FILE* input_file = fopen(filename, "r");
	
	int* num = malloc(sizeof(int));
	while (fscanf(input_file, "%d", num) != EOF) {
		vector_push(array, num);
		num = malloc(sizeof(int));
	}

	fclose(input_file);
	return array;
}

void write_array(char* filename, struct vector *array) {
	FILE* output_file = fopen(filename, "w");

	for (int i = 0; i <= array->top; i++) {
		fprintf(output_file, "%d ", *(int *)array->data[i]);
	}

	fclose(output_file);
}

double max_coro_time_in_seconds() {
	return ((double)TARGET_LATENCY / CORO_NUM) / 1000000.;
}

int partition(struct vector* array, struct segment* seg) {
	int v = *(int* )array->data[(seg->l + seg->r) / 2];
	int i = seg->l, j = seg->r;
	while (i <= j) {
		while (*(int *)array->data[i] < v) i++;
		while (*(int *)array->data[j] > v) j--;
		if (i >= j) break;

		void* tmp = array->data[i];
		array->data[i] = array->data[j];
		array->data[j] = tmp;

		i++; j--;
	}
	return j;
}

void sort(struct vector* array, struct coro* c) {
	struct vector* segments_stack = vector_create(1);

	vector_push(segments_stack, create_segment(0, array->top));
	
	while (segments_stack->top != -1) {
		struct segment* seg = vector_pop(segments_stack);
		
		if (seg->r <= seg->l) continue;
		
		int p = partition(array, seg); 

		// Мы помещаем больший из двух подмассивов в стек первым с тем, чтобы максимальная глубина стека 
		// при сортировке N элементов не превосходила величины logN
		// ref: https://neerc.ifmo.ru/wiki/index.php?title=Быстрая_сортировка
		if (p - seg->l > seg->r - p) {
			vector_push(segments_stack, create_segment(seg->l, p));
			vector_push(segments_stack, create_segment(p + 1, seg->r));
		} else {
			vector_push(segments_stack, create_segment(p + 1, seg->r));
			vector_push(segments_stack, create_segment(seg->l, p));
		}

		free(seg);

		if (coro_last_start_work_time(c) > max_coro_time_in_seconds()) coro_yield();
	}
}


/**
 * Coroutine body. This code is executed by all the coroutines. Here you
 * implement your solution, sort each individual file.
 */
static int
coroutine_func_f(void *context)
{
	struct coro *this = coro_this();
	struct vector* files = context;

	while(!vector_empty(files)) {
		char* filename = vector_pop(files);

		struct vector *array = read_array(filename);
		sort(array, this);

		write_array(filename, array);

		vector_delete(array);
	}	

	return 0;
}


struct vector *merge(struct vector **arrays, int arrays_num) {
	struct vector* merged = vector_create(1);
	int pointers[arrays_num];

	for (int i = 0; i < arrays_num; i++) pointers[i] = 0;
	
	while (true) {
		int *min_num = malloc(sizeof(int)); *min_num = INF;
		int min_p = -1;

		for (int i = 0; i < arrays_num; i++) {
			if (pointers[i] != -1) {
				int val = *(int *)arrays[i]->data[pointers[i]];
				if (val <= *min_num) {
					*min_num = val;
					min_p = i;
				}
			}
		}
		if (min_p == -1) break;
		pointers[min_p] = pointers[min_p] < arrays[min_p]->top ? pointers[min_p] + 1 : -1;
		vector_push(merged, min_num);
	}

	return merged;
}


struct vector*
execute_by_pull(coro_f func, struct vector* filenames, int coro_num)
{
	struct coro* c;

	struct vector* coros = vector_create(coro_num);

	for (int i = 0; i < coro_num; i++) vector_push(coros, coro_new(func, filenames));

	while ((c = coro_sched_wait()) != NULL) {}

	return coros;
}


int
main(int argc, char **argv)
{
	/* Initialize our coroutine global cooperative scheduler. */
	coro_sched_init();

	// TARGET LATENCY is in microseconds!!
	TARGET_LATENCY = atoi(argv[1]);
	CORO_NUM = atoi(argv[2]);

	int files_start_index = 3;
	int files_num = argc - files_start_index;

	struct vector *filenames = vector_create(files_num);
	for (int i = 0; i < files_num; i++) {
		vector_push(filenames, argv[files_start_index + i]);
	}

	printf("Coro report:\n");
	struct vector* coros = execute_by_pull(coroutine_func_f, filenames, CORO_NUM);
	struct coro* c;
	for (int i = 0; i <= coros->top; i++) {
		c = coros->data[i];
		printf("---------\nCoro %d:\nSwitch count: %lld\nExecution time: %f\n", 
			   i,
			   coro_switch_count(c),
			   coro_work_time(c));
		coro_delete(c);
	}
	
	struct vector* arrays[files_num];
	for (int i = 0; i < files_num; i++) arrays[i] = read_array(argv[files_start_index + 1]);

	struct vector* merged = merge(arrays, files_num);
	write_array("result.txt", merged);

	vector_delete(merged);
	for (int i = 0; i < files_num; i++) vector_delete(arrays[i]);

	return 0;
}
