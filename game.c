#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<stdlib.h>


int referee_count, player_count;
int n, total; // we take total as 3n to take care of the probability
int var, var1, var2;
typedef struct node {
  int play, referee, index;
  pthread_t tid;
}Game;

sem_t organiserfull, play_at_org, ref_at_org, play_at_orgs, ref_at_orgs, before_game, game;
referee_count = 0;
player_count = 0;
Game people[100008];

void start_game(void * x)
{
  Game * temp;
  temp =(Game *) x;
  printf("Game started by Referee %d\n", temp->referee);
  sem_post(&game);
}

void adjustEquipment(void * x)
{
  Game* temp;
  temp = (Game *)x;
  printf("Referee %d starts adjusting the equipment\n",temp->referee);
  sleep(0.5);
  sem_wait(&before_game);
  sleep(0.5);
  //sem_wait(&before_game);
  start_game(temp);
}

void warmUp(void * x)
{
  Game* temp;
  temp = (Game *)x;
  sleep(1);
  sem_post(&before_game);
}


void enterCourt( void * x)
{
  Game* temp;
  temp = (Game *)x;
  int flag;
  if(temp->index != 0)
  {
    if(temp -> referee != 0)
    {
      printf("Referee %d enters the court\n", temp->referee);
      sleep(0.5);
      flag = 1;
    }
    else if( temp -> play > 0)
    {
      printf("Player %d enters the court\n", temp->play);
      sleep(0.5);
      printf("Player %d starts warm up\n", temp->play);
      flag = 2;
    }
    if(flag == 1)
      adjustEquipment(temp);
    else if(flag==2)
    {
      sleep(1);
      warmUp(temp);
    }
  }

}


void meetOrganiser( void * x)
{
  Game* temp;
  temp = (Game *)x;
  if( temp -> referee == 0)
  {
    printf("Player %d waits for the organiser\n", temp->play);
    sleep(1);
    sem_wait(&play_at_org);
    sem_post(&play_at_orgs);
    printf("Player %d meets the organiser\n", temp->play);
    sleep(1);
    sem_wait(&organiserfull);
    enterCourt(temp);
  }

  if( temp -> play == 0)
  {
    printf("Referee %d waits for the organiser\n", temp->referee);
    sleep(1);
    sem_wait(&ref_at_org);
    sem_post(&ref_at_orgs);
    printf("Referee %d meets the organiser\n", temp->referee);
    sleep(1);
    sem_wait(&organiserfull);
    enterCourt(temp);
  }

}


void * startGame(void * x)
{
  Game* temp;
  temp = (Game *)x;
    if(temp->play != 0)
    {
      sleep(1);
      printf("Player %d enters academy\n", temp->play);
      sleep(1);
      meetOrganiser(temp);
    }
    else if (temp->referee != 0)
    {
      sleep(1);
      printf("Referee %d enters the academy\n", temp->referee);
      sleep(1);
      meetOrganiser(temp);
    }
}

void * busyorganiser()
{
  Game* temp;
  int max_players = 2;
  int org_max_count = 3;
  int i = 0;
  int j = 0;
  int k = 0;

  while(1)
  {
    while(i<max_players){
       sem_wait(&play_at_orgs);
       i+=1;
    }

    sem_wait(&ref_at_orgs);

    while(j<org_max_count){
       sem_post(&organiserfull);
       j+=1;
    }

    sem_wait(&game);

    sem_post(&ref_at_org);
    while(k<max_players){
      sem_post(&play_at_org);
      k+=1;
    }
  }
}

int main()
{

  scanf("%d",&n);
  pthread_t organiser;
  srand(time(NULL));

  sem_init(&game, 0, 0);
  sem_init(&before_game, 0, 0);
  sem_init(&organiserfull,0,0);
  total = 3*n;
  pthread_create(&organiser, NULL, &busyorganiser, NULL);

  sem_init(&play_at_org,0,2);
  sem_init(&ref_at_org,0,1);
  sem_init(&play_at_orgs, 0 , 0);
  sem_init(&ref_at_orgs, 0, 0);

  while(var<total)
  {
    var2 = var + 1;
    people[var2].index = var2;
    if(var2 % 3 == 0)
    {
      //referee_count ++;
      people[var2].referee = ++referee_count;
      people[var2].play = 0;
      var+=1;
    }
    else
    {
      //player_count++;
      people[var2].referee = 0;
      people[var2].play = ++player_count;
      var+=1;
    }
    pthread_create(&people[var2].tid, NULL, &startGame, &people[var2]);
    int rand_var = rand()%3 + 1;
    sleep(rand_var);
  }

  while(var1<total){
    var1+=1;
    pthread_join(people[var2].tid, NULL);
  }

return 0;

}
