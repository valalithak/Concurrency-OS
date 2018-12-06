#include <sys/types.h>
#include<sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void fill_data(int *shm_arr, int n)
{

	FILE* f = fopen("input.txt", "r");
    
	int i;



	 for (i = 0; i < n; i++){
        fscanf(f, "%d\n", &shm_arr[i] );
    }
}
void merge(int *shm_arr, int l, int r, int n)
{
	int temp[r-l+2];
	int i=l,j=l+((r-l)/2)+1,k=0;
	while(i<=l+((r-l)/2) && j<=r)
	{
		if(shm_arr[i] > shm_arr[j])
			temp[k++]=shm_arr[j++];
		else
			temp[k++]=shm_arr[i++];
	}
	while(i<=l+((r-l)/2) && j > r)
		temp[k++]=shm_arr[i++];
	while(i>l+((r-l)/2) && j<=r)
		temp[k++]=shm_arr[j++];
	j=l;
	for(i=0; i<k; i++)
		shm_arr[j++]=temp[i];
}
void selectionsort(int *shm_arr,int l, int r)
{
	int i,j,min,size=r-l+1;
	for(i=l; i<=r; i++)
	{
		min = i;
		for(j=i+1; j<=r; j++)
		if(shm_arr[j]<shm_arr[min])
			min = j;
		shm_arr[min] = shm_arr[i]+shm_arr[min]-(shm_arr[i] = shm_arr[min]);
	}

}
void mergesort(int *shm_arr, int l, int r, int n)
{
	if(l>=r)
		return;
	if(r-l <= 4)
	{
		selectionsort(shm_arr,l,r);
		return;
	}
	int pid1, pid2;
	pid1 = fork();
	if(pid1<0)
	{
		perror("left child not created\n");
		exit(1);
	}
	else if(pid1 == 0)
	{
		mergesort(shm_arr,l,l+((r-l)/2),n);
		exit(0);
	}
	else
	{
		pid2 = fork();
		if(pid2<0)
		{
			perror("left child not created\n");
			exit(1);
		}
		else if(pid2 == 0)
		{
			mergesort(shm_arr,l+((r-l)/2)+1,r,n);
			exit(0);
		}
	}
	int st;
	waitpid(pid1,&st,0);
	waitpid(pid2,&st,0);
	merge(shm_arr,l,r,n);
	return;
}
int main()
{
	int n=10000;
	//scanf("%d",&n);
	int shm_pointer;//identifier of shared memory
	int *shm_arr;//array of the shared memory (attached part)
	size_t shm_size = sizeof(int)*n;
	shm_pointer=shmget(IPC_PRIVATE,shm_size,IPC_CREAT | 0666);
	if(shm_pointer<0)
	{
		perror("shmget");
		exit(1);
	}
	if ((shm_arr=shmat(shm_pointer,NULL,0)) == (int*)-1)
	{
		perror("shmat");
		exit(1);
	}
	int i;
	fill_data(shm_arr,n);
	mergesort(shm_arr,0,n-1,n);

	for(i=0; i<n; i++)
		printf("%d ",shm_arr[i]);
	printf("\n");
	if(shmdt((void *)shm_arr) == -1)
	{
		perror("shmdt");
		exit(1);
	}
	if(shmctl(shm_pointer,IPC_RMID,NULL) == -1)
	{
		perror("shmctl");
		exit(1);
	}
	return 0;
}
