// semaphore.c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

void *read(void *arg);
void *accu(void *arg);
sem_t *sem_one;
sem_t *sem_two;
static int num; // 하나씩 입력받을 전역 변수

int main(int argc, char *argv[])
{
	sem_unlink("/sem_one");
	sem_unlink("/sem_two");

	pthread_t id_t1, id_t2;
	// 세마포어 두개 생성 => 하나는 0이고, 하나는 1이다.
	sem_one = sem_open("/sem_one", O_CREAT, 0, 0); // 0으로 초기화 => 키를 갖고있지 않음 => 임계영역 접근 불가능
	sem_two = sem_open("/sem_two", O_CREAT, 0, 1); // 1로 초기화 => 키를 갖고있음 => 임계영역 접근 가능

	if (sem_one == SEM_FAILED || sem_two == SEM_FAILED)
	{
		printf("error \n");
		return -1;
	}

	pthread_create(&id_t1, NULL, read, NULL);
	pthread_create(&id_t2, NULL, accu, NULL);

	pthread_join(id_t1, NULL);
	pthread_join(id_t2, NULL);

	sem_unlink("/sem_one");
	sem_unlink("/sem_two");
	return 0;
}

void *read(void *arg)
{
	int i;
	for (i = 0; i < 5; i++)
	{
		fputs("Input num: ", stdout);

		// 세마포어 변수 sem_two를 이용한 wait와 post 함수 호출
		// 세마포어 변수 sem_one을 이용한 wait과 post 함수 호출
		sem_wait(sem_two); // wait를 통해 sem_two의 값을 1을 감소시킴 -> 키획득
		scanf("%d", &num);
		sem_post(sem_one); // sem_one이 키를 획득 => 값이 1 증가
	}
	return NULL;
}

void *accu(void *arg)
{
	int sum = 0, i; // num을 누적시킬 변수
	for (i = 0; i < 5; i++)
	{

		sem_wait(sem_one); // 키를 얻기위해 wait 호출 => 근데 키가 없음 => num에 접근 불가능
		sum += num;
		sem_post(sem_two); // sem_one의 키가 sem_two한테 반납
	}
	printf("Result : %d \n", sum);
	return NULL;
}
