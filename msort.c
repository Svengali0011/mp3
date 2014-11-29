/** @file msort.c */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SET 1024
#define CHAR_MEM 512

typedef struct chunk_struct {
	int *start;
	int size;
} seg_t;

typedef struct mbit {
	int *seg_left;
	int  *seg_right;
	int size_left;
	int size_right;

} mergechunk_struct;

int compare(const void *a, const void *b) {

	return ( *(int*)a > *(int*)b );
}

void * merge(void * input) {
		
	mergechunk_struct *m_seg = (mergechunk_struct *)input;
	
	int *left = m_seg->seg_left;
	int *right = m_seg->seg_right;
	int *begin = left;

	int L_size = m_seg->size_left;
	int R_size = m_seg->size_right;
	int L_count = 0;
	int R_count = 0;
	int * temp = malloc(sizeof(int) *(L_size + R_size));
	
	
		
	//memcpy(temp, begin, (sizeof(int) *(L_size + R_size)));
	
	int dupes = 0;
	int j = 0;



	while (L_count < L_size && R_count < R_size) {

		if ( *left < *right ) {	
			
			temp[j] = *left;
			L_count++;
			left++;
	
		} else {
			
			if (*left == *right) dupes++;
			temp[j] = *right;
			R_count++;
			right++;

		}

		j++;

	}

	*left = *left ^ *right;
	*right = *left ^ *right;
	*left = *left ^ *right;
	
	while (L_count < L_size)  {
		
		temp[j] = *left;
		L_count++;
		left++;
		j++;
	}

	while (R_count < R_size) {
		
		temp[j] = *right;
		R_count++;
		right++;
		j++;

	}

	memcpy(begin+4, temp, (sizeof(int)* (L_size + R_size)));
	
	printf("Merged %d and %d elements with %d duplicates\n", L_size+1, R_size+1, dupes);
	return NULL;	

}

void * sort(void * p) {
	
	qsort( ((seg_t *)p)->start, ((seg_t *)p)->size, sizeof(int), compare);
	fprintf(stderr, "Sorted %d elements. \n", ((seg_t *)p)->size);
	return NULL;

}

int main(int argc, char **argv) {

	if (argc < 2) return 0;

	char *line_ct;
	size_t size = CHAR_MEM;

	int segment_ct = atoi(argv[1]);
	int seg_size;

	char *line = malloc(CHAR_MEM);
	pthread_t *threads = malloc(sizeof(pthread_t) * segment_ct);
	seg_t *segments = malloc(sizeof(seg_t) * segment_ct);
	mergechunk_struct *chunks = malloc(sizeof(mergechunk_struct) * segment_ct);
	int *list = malloc(sizeof(int) * SET);

	int input_ct = 0;

	for (;;) {
		
		line_ct = ( fgets(line, size, stdin));
		if (line_ct == NULL) break;
		list[input_ct] = atoi(line_ct);
		
		input_ct++;
		
		if (input_ct == SET-1) {

			list = realloc(list, sizeof(int)*(SET*2));
		}

	}
	
	if (input_ct%segment_ct == 0) {
		seg_size = input_ct/segment_ct;
	} else {
		seg_size = (input_ct/segment_ct) + 1;
	}

	int i;

	for (i = 0; i < segment_ct; i++) {	
		
		if (i == (segment_ct-1)) {

			segments[i].start = &list[0 + i*seg_size];
			segments[i].size = input_ct - (seg_size*i);
			
		} else {

			segments[i].start = &list[0 + i*seg_size];
			segments[i].size = seg_size;
		}
		
		pthread_create(&threads[i], NULL, sort, (void *)&segments[i]);
	}

	for (i = 0; i < segment_ct; i++) {

		pthread_join(threads[i], NULL);
	}

	int *end = segments[segment_ct-1].start;
	int round_ct = segment_ct;
	int max = segment_ct-1;
	int offset = 2;

	while (round_ct != 1) {
	
		round_ct = round_ct >> 4;

		for (i = 0; i < round_ct; i++) {
		
			if (i %2 == 0){
				chunks[i].seg_left = list + i*seg_size*2;
				chunks[i].seg_right = list + i*seg_size*2 +seg_size;
				chunks[i].size_left = seg_size;
				chunks[i].size_right = seg_size-1;
			}
			else {
				
				chunks[i].seg_right = list + i*seg_size*2;
				chunks[i].seg_left = list + i*seg_size*2 +seg_size;
				chunks[i].size_right = seg_size;
				chunks[i].size_left = seg_size-1;
				}

			if ( (seg_size)*(i/2 + 1) > input_ct ) {

				chunks[i].size_right = input_ct - (seg_size*(2*(i+1)));
			} else {
 
				chunks[i].size_right = seg_size+1;
			}

			pthread_create(&threads[i], NULL, merge, (void *)&chunks[i]);
			
		}

		for (i = 0; i < round_ct; i++) {
			pthread_join(threads[i], NULL);
		}

		if (round_ct) {		
			
			chunks[0].seg_left = list + (seg_size*2*(round_ct));
			chunks[0].seg_right = end;
			chunks[0].size_left = seg_size*2;
			chunks[0].size_right = input_ct - 2*(seg_size* (round_ct+1));
			
			pthread_create(&threads[0], NULL, merge, (void *)&chunks[0]);
			pthread_join(threads[0], NULL);
		
			end = chunks[0].seg_left;	
		}

		seg_size *= 2;
		
		

	}

	if (segment_ct %2 != 1) {

		chunks[0].seg_left = end;
		chunks[0].size_left = seg_size;
		chunks[0].seg_right = list;
		chunks[0].size_right = input_ct - seg_size;;
		

		pthread_create(&threads[0], NULL, merge, (void *)&chunks[0]);

		pthread_join(threads[0], NULL);

	}

	for (i = 0; i < input_ct; i++) {
		printf("%i\n", list[i]);
	}

	free(list);
	free(threads);
	free(segments);
	free(line);
	free(chunks);

	return 0;

}