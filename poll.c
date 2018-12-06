#include<stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>

typedef struct Booth
{
  int numVoters,numEvm,numSlots,currEvm,boothId,curr1,curr2;
  pthread_mutex_t ready,vote;
  pthread_cond_t cond;


}B;

void polling_ready_evm(B *booth,int count,int evmId)
{

  pthread_mutex_lock(&(*booth).ready);
  if( (*booth).numVoters<=0)
   {
    pthread_mutex_unlock(&(*booth).ready);
    return;
   }
  struct timespec ts;
  ts.tv_sec = 1;
  ts.tv_nsec = 500000000;
  printf("Evm %d at Booth %d ready with slots = %d\n",evmId,(*booth).boothId,count);
  int i;


  for(i=0;i<count;i++)
    {
      if((*booth).numVoters<0)
			break;
      (*booth).numVoters--;


      pthread_cond_signal(&(*booth).cond);
      nanosleep(&ts, NULL);
    }


  pthread_mutex_unlock(&(*booth).ready);

  printf("Evm %d at Booth %d moves to voting stage\n",evmId,(*booth).boothId);
  if((*booth).numVoters<=0){
      printf("\033[1;32m");
      printf("Voters at booth %d are done with voting\n",(*booth).boothId);
       printf("\033[0m;");

  }
  return;

}
void *polling_ready_evm_util(void *arg)
{
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = 500000000;
  B *booth= ((B*)(arg));


  int in=(*booth).curr1++;



  while((*booth).numVoters>0)
  {
      int rand_count = rand()%10 + 1;
      polling_ready_evm(booth,rand_count,in+1);
  }

  //return;
}

void voter_wait_for_evm(B *booth,int in)
{
  pthread_mutex_lock(&(*booth).vote);
  pthread_cond_wait(&(*booth).cond,&(*booth).vote);
  printf("Voter %d Booth %d got allocated EVM %d\n",in,(*booth).boothId,(*booth).currEvm+1);
  pthread_mutex_unlock(&(*booth).vote);
  return;
}
void *voter_wait_for_evm_util(void *arg)
{
  B *booth= ((B*)(arg));
  int in=(*booth).curr2++;
  voter_wait_for_evm(booth,in+1);
}



void booth_init(B *booth)
{
  pthread_mutex_init(&(*booth).vote, NULL);
  pthread_mutex_init(&(*booth).ready, NULL);
  pthread_cond_init(&(*booth).cond, NULL);
  int i;
  pthread_t tid;
  for(i=0;i<(*booth).numVoters;i++)
      pthread_create(&tid,NULL,voter_wait_for_evm_util,booth);
  for(i=0;i<(*booth).numEvm;i++)
      pthread_create(&tid,NULL,polling_ready_evm_util,booth);
}

int main()
{
  int i,j,numBooths;
  B booth[100];
  printf("Enter number of booths : ");
  scanf("%d",&numBooths);
  for(i=0;i<numBooths;i++)
  {
      scanf("%d %d",&booth[i].numVoters,&booth[i].numEvm);
     booth[i].boothId=i+1;
  }
  for(i=0;i<numBooths;i++)
  {
    booth_init(&booth[i]);
  }
  pthread_exit(NULL);
}
