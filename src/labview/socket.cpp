/*
 * socket.cpp
 *
 *  Created on: 2017. 2. 22.
 *      Author: NaYeon Kim
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sstream>



#define PORT_NUM 6341
#define SERVER_IP "192.168.0.7"

using namespace std;

template <typename T>
string ToString(T val)
{
	stringstream stream;
	stream << val;
	return stream.str();

}

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};





int main(void)
{


	char ReceiveData[256];
	int strLen;
	int shmid;
	int shmid2;
	int shmid3;
	int semid;//세마포어
	union semun sem_union;
	void *shared_memory = (void *)0;
	void *shared_memory2 = (void *)0;
	void *shared_memory3 = (void *)0;
	char buff[1024];
	int skey = 5678;
	int skey2 = 1234;
	int skey3 = 2345;
//	int sekey = 1234;

	int *process_num;
	int local_num;

	struct sembuf semopen = {0,-1,SEM_UNDO};
	struct sembuf semclose = {0,1,SEM_UNDO};


	//공유메모리 공간 만들기 1
	shmid = shmget((key_t)skey, sizeof(int), 0777|IPC_CREAT);
	if(shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

//	semid = semget((key_t)sekey,1,IPC_CREAT|0777);
//	if(semid == -1)
//	{
//		perror("semget failed : ");
//		return 1;
//	}

	//세마포어 초기화
//	sem_union.val = 1;
//	if(-1 == semctl(semid,0,SETVAL,sem_union))
//	{
//		return 1;
//	}



	//공유메모리 맵핑 1
	shared_memory = shmat(shmid, (void *)0, 0);
	if(!shared_memory)
	{
		perror("shmat failed");
		exit(0);
	}

	//공유메모리 공간 만들기 2
	shmid2 = shmget((key_t)skey2, sizeof(int), 0777|IPC_CREAT);
	if(shmid2 == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	//공유메모리 맵핑 2
	shared_memory2 = shmat(shmid2, (void *)0, 0);
	if(!shared_memory2)
	{
		perror("shmat failed");
		exit(0);
	}

	//공유메모리 공간 만들기 3
	shmid3 = shmget((key_t)skey3, sizeof(int), 0777|IPC_CREAT);
	if(shmid3 == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}

	//공유메모리 맵핑 3
	shared_memory3 = shmat(shmid3, (void *)0, 0);
	if(!shared_memory3)
	{
		perror("shmat failed");
		exit(0);
	}
	//공유메모리 변수 생성
	  int* w_steer;
	  w_steer = (int *)shared_memory;
	  int* w_brake = (int*)shared_memory2;
	  int* w_accel = (int*)shared_memory3;
	 








	  int s = socket(AF_INET,SOCK_STREAM,0);//socket 생성
	  if (s == -1)
	  {
		  cout << "Socket 오류입니다" << endl;
		  exit(1);
	  }

	  struct sockaddr_in addr;
	  addr.sin_family = AF_INET;
	  addr.sin_port = htons(PORT_NUM);
	  addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	  if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	  {
		  cout << "서버 연결 오류" << endl;
		  exit(1);
	  }

	  cout << "서버 연결 성공" << endl;



 /*************** TCP Setting ****************/

 int car_autosteer = 0;
 int car_autogear = 0;
 int car_speed = 10;
 int car_rpm = 110;
 int car_gear = 0;
 int car_steer = 0;
 int car_odometer = 0;



 int count = 0;
 string str2 = "";

 string test[19] = {"Empty", "Accel","Brake", "Steer","gear","UpperRampStatic","UpperRamp","Ramp","RampAuto","WiperMove","WiperTrigger","WiperOff","WiperAuto"
,"WiperLow","WiperHigh","RightTurn","LeftTurn","Bright","LampTrigger" };





 while (1)
 {



  string str= "dataautosteer" + ToString(car_autosteer) + +"autogear" + ToString(car_autogear)
   + "speed" + ToString(car_speed)
   + "rpm" + ToString(car_rpm)
   + "gear" + ToString(car_gear)
   + "steer" + ToString(car_steer)
   + "odometer" + ToString(car_odometer)
   + "eom";
  const char* cstr = str.c_str();






//  write(s,(void*)cstr,sizeof(cstr));//메세지 보내기

 
 // strLen = read(s, (void*)ReceiveData, sizeof(ReceiveData));//메세지 받기
 

 // ReceiveData[strLen] = 0;


 strLen = 0;
 if((strLen = read(s, ReceiveData, sizeof(ReceiveData)-1))>5 && strLen <= 160)
 { 
  

   ReceiveData[strLen] = 0;	
   
   //printf("Msg : %s \nlength = %d\n", ReceiveData, strLen);
 }
  string to_string(ReceiveData);
//  string s_steer = to_string.strsub(26, 
  char *ptr = strtok(ReceiveData, ".");//각각 값으로 쪼개기
//  printf("%s", ptr);
  while (ptr != NULL)
  {
//   str2 += test[count];
//   str2 += " : ";
//   str2 += ptr;
//   str2 += " ";
	 if(count == 1)
	 {
		*w_accel = atoi(ptr);
		printf("accel : %d\n", *w_accel);
	 }
	 if(count == 2)
	 {
		*w_brake = atoi(ptr);
		printf("brake: %d\n", *w_brake);
	 }
	 if(count == 3)
	 {		
		*w_steer = atoi(ptr);
		string to_string(ptr);
		if(*w_steer == 0) {
			*w_steer = atoi(to_string.substr(6, 3).c_str());
		}
		printf("steer : %d\n", *w_steer);
		break;
	 }

  ptr = strtok(NULL, ".");
   count++;
  }


count =0;


 

  
 }

 


 
 


 close(s);

 return 0;
}



