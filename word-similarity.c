#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <dirent.h>

struct word_list {
	char* word;
	size_t occur;
	double freq;
	struct word_list* next;
} word_list;

struct file_list {
	char* filename;
	struct word_list* wl;
	struct file_list* next;
	
} file_list;

struct node_arg{
	struct file_list* fl_one;
	struct file_list* fl_two;
	double value;
}node_arg;

void print_word_list(struct word_list* head) {
	struct word_list* curr = head;
	int toggle = 0;

	while (curr != 0) {
		if (toggle) printf(" --> ");
		printf("%s:%ld:%lf", curr->word, curr->occur, curr->freq);
		curr = curr->next;
		toggle = 1;
	}
	printf("\n");
}

void print_file_list(struct file_list* head) {
	struct file_list* curr = head;
	
	while (curr != 0) {
		printf("File: %s\n",curr->filename);
		print_word_list(curr->wl);
		curr = curr->next;
		printf("\n");
	}
	
}

void clear_word_list(struct word_list* head) {
	struct word_list* curr = head;
	struct word_list* prev = 0;

	while (curr != 0) {
		prev = curr;
		curr = curr->next;
		free(prev->word);
		free(prev);
	}
}

void clear_file_list(struct file_list* head) {
	struct file_list* curr = head;
	struct file_list* prev = 0;

	while (curr != 0) {
		prev = curr;
		curr = curr->next;
		free(prev->filename);
		clear_word_list(prev->wl);
		free(prev);
	}
}

int check_word_exist(char* target, struct word_list* head) {
	struct word_list* curr = head;
	
	while (curr != 0) {
		if (strcmp(target, head->word) == 0) {
			return 1;
		}
	}

	return 0;
}

int increment_word_occur(char* target, struct word_list* head) {
	struct word_list* curr = head;
	
	while (curr != 0) {
		if (strcmp(target, curr->word) == 0) {
			curr->occur++;
			return 1;
		}
		curr = curr->next;
	}

	return 0;
}

int compare_word(struct word_list* w1, struct word_list* w2) {
	return strcmp(w1->word, w2->word);
}

struct word_list* add_word(char* target, struct word_list* head) {
	
	struct word_list *ptr = malloc(sizeof(struct word_list));

	if (ptr == 0) {
		perror("malloc linked list");
		exit(EXIT_FAILURE);
	}

	ptr->word = strdup(target);
	ptr->occur = 1;
	ptr->freq = 0;
	ptr->next = 0;


	if (head == 0 || (compare_word(ptr, head) < 0)) {

		ptr->next = head;
		
		return ptr;		

	} else {

		struct word_list* curr = head;

		while ((curr->next != 0) && (compare_word(ptr, curr->next) >= 0)) {
			curr = curr->next;
		}
		ptr->next = curr->next;
		curr->next = ptr;

		return head;

	}


}

void format_word3(char *str)
{
    unsigned long i = 0;
    unsigned long j = 0;
    char c;

    while ((c = str[i++]) != '\0')
    {
        if (isalnum(c))
        {
            str[j++] = tolower(c);
        }
    }
    str[j] = '\0';
}

void create_word_list(struct file_list* fl, char* pathname) {
	FILE *fp;
	char *line;
	size_t len = 0;
	ssize_t nread;
	char* token;
	const char space[3] = " \t\n";
	char* temp;

	fp = fopen(pathname, "r");

	if (fp == 0) {
		perror(pathname);
		return;
	}

	while ((nread = getline(&line, &len, fp)) != -1) {

		token = strtok(line, space);

		while (token != 0) {

			temp = strdup(token);
			format_word3(temp);

			if (increment_word_occur(temp, fl->wl) == 0) {
				fl->wl = add_word(temp, fl->wl);
			}
			
			free(temp);

			token = strtok(0, space);
		}
	}

	free(line);
	fclose(fp);
	
}

void fill_freqs(struct word_list* head) {
	size_t count = 0;
	
	struct word_list* curr = head;

	while(curr != 0) {
		count += curr->occur;
		curr = curr->next;
	}

	curr = head;

	while (curr != 0) {
		curr->freq = ((double) curr->occur)/count;
		curr = curr->next;
	}

}

struct file_list* file_list_driver(struct file_list* fl, char* pathname) {
	struct file_list* temp_fl = malloc(sizeof(struct file_list));
	temp_fl->filename = strdup(pathname);
	temp_fl->wl = 0;
	temp_fl->next = 0;

	create_word_list(temp_fl, pathname);
	fill_freqs(temp_fl->wl);

	if (fl == 0) {
		return temp_fl;
	}

	struct file_list* curr = fl;

	while(curr->next != 0) {
		curr = curr->next;
	}
	
	curr->next = temp_fl;

	return fl;
}

struct comp_q{
	char* pathname;
	struct comp_q* next;
} comp_q;

struct comp_q* enq(struct comp_q* head, char* pathname){
	struct comp_q* ptr = malloc(sizeof(struct comp_q));

	if (ptr == 0) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	ptr->pathname = malloc(strlen(pathname)+1);
	ptr->pathname = strcpy(ptr->pathname, pathname);
	ptr->next = 0;

	if (head == 0) {return ptr;}

	struct comp_q* curr = head;

	while (curr->next != 0) {
		curr = curr->next;
	}

	curr->next = ptr;

	return head;
}

struct comp_q* deq(struct comp_q* head, char** target){
	
	*target = malloc(strlen(head->pathname)+1);	
	
	strcpy(*target, head->pathname);

	struct comp_q* curr = head->next;

	free(head->pathname);
	free(head);

	return curr;
}

void clear_q(struct comp_q* head) {
	struct comp_q* curr = head;
	struct comp_q* prev = 0;

	while (curr != 0) {
		prev = curr;
		curr = curr->next;
		free(prev->pathname);
		free(prev);
	}
}

void print_comp_q(struct comp_q* head) {
	struct comp_q* curr = head;

	int toggle = 0;

	while(curr != 0) {
		if (toggle) printf(" --> ");
		printf("%s", curr->pathname);
		curr = curr->next;
		toggle = 1;
	}

	printf("\n");
}

int is_dir(char *name) {
	struct stat data;

	int err = stat(name, &data);

	if (err) {
		perror(name);
		return 0;
	}

	if (S_ISDIR(data.st_mode)) {return 1;}

	return 0;
}

int correct_file(char* pathname, char* target) {
	
	if (strlen(target) == 0) return 1;

	if (strlen(target) > strlen(pathname)) return 0;
	
	char* extension = &pathname[strlen(pathname) - strlen(target)];

	if (strcmp(extension, target) != 0) return 0;

	return 1;
	
}

double jensen_calc(struct node_arg* node){
	struct word_list* temp_one = node->fl_one->wl;
	double count_first = 0;
	double count_sec = 0;
	while(temp_one != 0){
		struct word_list* temp_two = node->fl_two->wl;
		double mean_freq = 0;
		double sec_num = 0;
		while(temp_two != 0){
			if(strcmp(temp_one->word, temp_two->word) == 0){
				sec_num = temp_two->freq;
			}
			temp_two = temp_two->next;
		}
		mean_freq = (temp_one->freq + sec_num) / 2;
		count_first += temp_one->freq * log2(temp_one->freq / mean_freq);
		temp_one = temp_one->next;
	}
	struct word_list* temp_two = node->fl_two->wl;
	while(temp_two != 0){
		struct word_list* temp_three = node->fl_one->wl;
		double mean_freq = 0;
		double sec_num = 0;
		while(temp_three != 0){
			if(strcmp(temp_two->word, temp_three->word) == 0){
				sec_num = temp_three->freq;
			}
			temp_three = temp_three->next;
		}
		mean_freq = (temp_two->freq + sec_num) / 2.0;
		count_sec += temp_two->freq * log2(temp_two->freq / mean_freq);
		temp_two = temp_two->next;
	}
	double temp_val = (count_first / 2.0) + (count_sec / 2.0);
	if(temp_val <=0){return 0;}
	double jensen_val = sqrt(temp_val);
	return jensen_val;
}
int amt_calc(int amt){
	return (0.5)*(amt)*(amt + 1);
}
int sortVal(const void* x, const void* y){
	if(*(double*)x > *(double*)y ){return 1;}
	if(*(double*)x < *(double*)y ){return -1;}
	return 0;
}

int main(int argc, char* argv[argc+1]) {

	char* suffix = ".txt";
	int dir_t = 1;
	int file_t = 1;
	int anal_t = 1;
	int toggle = 0;

	struct comp_q* dir_q = 0;
	struct comp_q* file_q = 0;

	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-", 1) == 0) {

			

			if (strncmp(argv[i], "-d", 2) == 0) {
				if (strlen(argv[i]) < 3) return EXIT_FAILURE;
				dir_t = atoi(&argv[i][2]);
			} else if (strncmp(argv[i], "-f", 2) == 0) {
				if (strlen(argv[i]) < 3) return EXIT_FAILURE;
				file_t = atoi(&argv[i][2]);
			} else if (strncmp(argv[i], "-a", 2) == 0) {
				if (strlen(argv[i]) < 3) return EXIT_FAILURE;
				anal_t = atoi(&argv[i][2]);
			} else if (strncmp(argv[i], "-s", 2) == 0) {
				suffix = malloc(strlen(argv[i]) - 1);
				strcpy(suffix, &argv[i][2]);
				toggle = 1;
			} else {
				return EXIT_FAILURE;
			}
		}
	}

	if (dir_t == 0 || file_t == 0 || anal_t == 0) {
		if (toggle) free(suffix);
		return EXIT_FAILURE;
	}	


	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-", 1) == 0) continue;
		if (is_dir(argv[i])) {
			dir_q = enq(dir_q, argv[i]);
		} else if(correct_file(argv[i], suffix)) {
			file_q = enq(file_q, argv[i]);
		}
	}

	while(dir_q != 0) {
		char* dir_path = "";
	
		dir_q = deq(dir_q, &dir_path);

		struct dirent* de;

		DIR *dr = opendir(dir_path);

		if (dr == 0) {
			perror("directory");
			return 0;
		}

		while ((de = readdir(dr)) != NULL) {

			if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) {
				continue;
			}

			char* new_path = malloc(strlen(de->d_name) + strlen(dir_path) + 2);
			strcpy(new_path, dir_path);
			strcat(new_path, "/");
			strcat(new_path, de->d_name);

			if (is_dir(new_path)) {
				dir_q = enq(dir_q, new_path);
			} else if (correct_file(new_path, suffix)) {
				file_q = enq(file_q, new_path);
			}

			
			free(new_path);

		}

		free(dir_path);

		closedir(dr);
	}

/*
	printf("dir_q:\n");
	print_comp_q(dir_q);

	printf("file_q:\n");
	print_comp_q(file_q);
*/

	char* temp = "";
	struct file_list* fl = 0;

	while (file_q != 0) {
		file_q = deq(file_q, &temp);
		fl = file_list_driver(fl, temp);
		free(temp);
	}

	struct file_list* temp_count = fl;
	int arr_size = 0;
	while(temp_count != 0){
		arr_size++;
		temp_count = temp_count->next;
	}

	int grid_count[arr_size][arr_size];
	for(int i = 0; i < arr_size; i++){
		for(int j = 0; j < arr_size; j++){
			if(i == j){grid_count[i][j] = 1;}
			else{grid_count[i][j] = 0;}
		}
	}
	int i = 0;
	int j = 0;

	int queue_amt = amt_calc(arr_size - 1);
	int thread = 3;
	int thread_count = 0;
	struct file_list* temp_first = fl;
	double values[queue_amt];
	double temp_values[queue_amt];
	for(int i = 0; i < queue_amt; i++){
		values[i] = 0;
		temp_values[i] = 0;
	}
	char** arr_file_one = malloc(queue_amt * sizeof(char*));
	char** arr_file_two = malloc(queue_amt * sizeof(char*));
	int arr_ctr = 0;
	while(temp_first != 0){
		struct file_list* temp_second = fl;
		j = 0;
		while(temp_second != 0){
			if(grid_count[i][j] == 0){
				struct node_arg* node = malloc(3*sizeof(node_arg));
				node->fl_one = temp_first; 
				node->fl_two = temp_second;
				double val = jensen_calc(node);
				values[arr_ctr] = val;
				temp_values[arr_ctr] = val;
				arr_file_one[arr_ctr] = temp_first->filename;
				arr_file_two[arr_ctr] = temp_second->filename;
				arr_ctr++;
				free(node);
			}
			temp_second = temp_second->next;
			grid_count[i][j] = 1;
			grid_count[j][i] = 1;
			j++;
		}
		temp_first = temp_first->next;
		i++;
	}
	int arr_check[queue_amt];
	for(int i = 0; i < queue_amt; i++){
		arr_check[i] = 0;
	}
	qsort(values, arr_ctr, sizeof(double), sortVal); 
	for(int i = 0; i < arr_ctr; i++){
		for(int j = 0; j < arr_ctr; j++){
			if(values[i] == temp_values[j] && arr_check[j] == 0){
				printf("%f %s %s\n", temp_values[j], arr_file_one[j], arr_file_two[j]);
				arr_check[j] = 1;
			}
		}
	}
	free(arr_file_one);
	free(arr_file_two);
	clear_q(dir_q);
	clear_q(file_q);


	if (toggle) free(suffix);


	return EXIT_SUCCESS;

}
