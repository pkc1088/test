#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *logedUsr[3] = {NULL}; // 포인터 배열 선언
	char sample[40] = "sample data";

	logedUsr[0] = sample;

	printf("%s\n", logedUsr[0]);
/*
    logedUsr[1] = (char *)malloc(sizeof(int) * 10); 
    if (logedUsr[1] == NULL) {
        fprintf(stderr, "메모리 할당 오류\n");
        exit(1);
    }
    
    strcpy(logedUsr[1], "Hellowwwwwwwwww");
    printf("logedUsr[1]: %s\n", logedUsr[1]);
*/
    free(logedUsr[1]);

    return 0;
}


/*
#define NUM_THREADS 2

int global_value = 0;

void* thread_func(void* arg) {
    int thread_num = *(int*)arg;
    for (int i = 0; i < 5; i++) {
        printf("Thread %d: global_value = %d\n", thread_num, ++global_value);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    int rc;
    int i;

    // create threads
    for (i = 0; i < NUM_THREADS; i++) {
        thread_args[i] = i;
        rc = pthread_create(&threads[i], NULL, thread_func, &thread_args[i]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    // join threads
    for (i = 0; i < NUM_THREADS; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }
    printf("global_value = %d\n", global_value);
    return 0;
}


int value = 5;
int main()
{
	pid_t pid;
	pid = fork();
	if (pid == 0) {
		value += 15;
		return 0;
	}
	else if (pid > 0) { 
		wait(NULL);
		printf("PARENT: value = %d",value); 
		return 0;
	}
}

int main() {

pid_t pid;
pid = fork();

int value = 0;

if (pid == 0) {	//child process 
	int k = 5;
	while(k-- >0){ 
		value++;
		printf("CHILD: value = %d\n",value);
	}
}

else if (pid > 0) {	//parent process
	int k = 5;
	while(k-- >0){ 
		value++;
		printf("PARENT: value = %d\n",value);
	}
}

	printf("pid : %d, value : %d\n", pid, value);
} 


*/


/*
int main() {
    int pid, count = 0;
    pid = fork();
    if (pid == 0) { // Child process
        count+= 5;
        printf("Child count: %d\n", count);
    } else if (pid > 0) { // Parent process
        wait(NULL);
		count+= 100;
        printf("Parent count: %d\n", count);
    }

	printf("here count : %d\n", count);
    return 0;
}*/
/*




/*

#include <stdio.h>
#include <stdlib.h>

#define MAX_TERMS 101
typedef struct {
	float coef;	// 밑
	int expon;	// 제곱
} polynomial;
polynomial terms1[MAX_TERMS];
polynomial terms2[MAX_TERMS];
polynomial terms3[MAX_TERMS];

static int cnt = 0;

char compare(int a, int b)
{
	if (a>b) return '>';
	else if (a == b) return '=';
	else return '<';
}

void attach(float coef, int expon)
{
	
	if (avail > MAX_TERMS) {
		fprintf(stderr, "항의 개수가 너무 많음\n");
		exit(1);
	}
	terms3[cnt].coef = coef;
	terms3[cnt].expon = expon;
	cnt++;
}

// C = A + B, As는 배열 start, Ae는 배열 end
void poly_add2(int As, int Ae, int Bs, int Be)
{
	float tempcoef;
	while (As <= Ae && Bs <= Be)
		switch (compare(terms1[As].expon, terms2[Bs].expon)) {
		case '>': 	// A의 차수 > B의 차수
			attach(terms1[As].coef, terms1[As].expon);
			As++;			
			break;

		case '=': 	// A의 차수 == B의 차수
			tempcoef = terms1[As].coef + terms2[Bs].coef;
			if (tempcoef != 0)
				attach(tempcoef, terms1[As].expon);
			As++; Bs++;		
			break;

		case '<': 	// A의 차수 < B의 차수
			attach(terms2[Bs].coef, terms2[Bs].expon);
			Bs++;			
			break;
		}
	for (; As <= Ae; As++)
		attach(terms1[As].coef, terms1[As].expon);
	for (; Bs <= Be; Bs++)
		attach(terms2[Bs].coef, terms2[Bs].expon);
	
	cnt--;
}

void print_poly()
{
	for (int i = 0; i < cnt; i++)
		printf("%3.1fx^%d + ", terms3[i].coef, terms3[i].expon);
	printf("%3.1fx^%d\n", terms3[cnt].coef, terms3[cnt].expon);
}

int main(void)
{
	int len1, len2;

	scanf("%d", &len1);
	for(int i = 0; i < len1; i++)
		scanf("%f %d", &terms1[i].coef, &terms1[i].expon);
	scanf("%d", &len2);
	for(int i = 0; i < len2; i++)
		scanf("%f %d", &terms2[i].coef, &terms2[i].expon);

	poly_add2(0, len1, 0, len2);

	print_poly();
	return 0;
}

*/


/*

#include <stdio.h>
#define MAX(a,b) (((a)>(b))?(a):(b))
#define MAX_DEGREE 101

typedef struct { 			// 다항식 구조체 타입 선언
	int degree;			// 다항식의 차수
	float coef[MAX_DEGREE];	// 다항식의 계수
} polynomial;

// C = A+B 여기서 A와 B는 다항식이다. 구조체가 반환된다. 
polynomial poly_add1(polynomial A, polynomial B)
{
	polynomial C;				// 결과 다항식
	int Apos = 0, Bpos = 0, Cpos = 0;	// 배열 인덱스 변수
	int degree_a = A.degree;
	int degree_b = B.degree;
	C.degree = MAX(A.degree, B.degree); // 결과 다항식 차수

	while (Apos <= A.degree && Bpos <= B.degree) {
		if (degree_a > degree_b) {  // A항 > B항
			C.coef[Cpos++] = A.coef[Apos++];
			degree_a--;
		}
		else if (degree_a == degree_b) {  // A항 == B항
			C.coef[Cpos++] = A.coef[Apos++] + B.coef[Bpos++];
			degree_a--; degree_b--;
		}
		else {			// B항 > A항
			C.coef[Cpos++] = B.coef[Bpos++];
			degree_b--;
		}
	}
	return C;
}

void print_poly(polynomial p)
{
	for (int i = p.degree; i>0; i--)
		printf("%3.1fx^%d + ", p.coef[p.degree - i], i);
	printf("%3.1f \n", p.coef[p.degree]);
}

// 주함수
int main(void)
{
	polynomial a = { 5,{ 3, 6, 0, 0, 0, 10 } };
	polynomial b = { 4,{ 7, 0, 5, 0, 1 } };
	polynomial c;

	print_poly(a);
	print_poly(b);
	c = poly_add1(a, b);
	printf("---------------------------------------\n");
	print_poly(c);
	return 0;
}
*/
