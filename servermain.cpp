#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <stdlib.h>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <random>
#include <signal.h>
#include <calcLib.h>
#include <algorithm>
#include "protocol.h"

std::vector<int> clientid;

void removeClientid(int data) {
    auto it = std::find(clientid.begin(), clientid.end(), data);
    if (it != clientid.end()) {
        clientid.erase(it);}
        }

void printvector(){
    for(int j=0; j < clientid.size(); j++){
        std::cout << clientid[j] << " ";
    }
    std::cout << std::endl;
}


bool checkclientinvector(int data){
    bool a;
   auto it = std::find(clientid.begin(), clientid.end(), data);
    if (it != clientid.end()) {
        a = true;
        }
    else{
      a = false;
    }
  

return a;
}


int getRandomNumberNotInVector() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distribution(0, 10000);

    int randomNum;
    bool isInVector = false;

    do {
        randomNum = distribution(gen); // Generate a random number
        isInVector = std::find(clientid.begin(), clientid.end(), randomNum) != clientid.end();
    } while (isInVector);
    clientid.push_back(randomNum);

    return randomNum;
}

typedef struct pait_{
   int a;
}pait_;

void timer_callback(union sigval arg){
    pait_ *pair = (pait_*)arg.sival_ptr;
    removeClientid(pair->a);
  
}

void timerdemo(int data){
    struct sigevent evp;

    timer_t timer;

    memset(&timer, 0, sizeof(timer_t));

    memset(&evp, 0, sizeof(struct sigevent));

    pait_ *p = new pait_;
    p->a = data;

    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = timer_callback;
    evp.sigev_value.sival_ptr = (void*)p;

    int ret = timer_create(CLOCK_REALTIME, &evp, &timer);

    if(ret < 0){
        perror("error with timer create ");
        exit(1);
    }

    struct itimerspec ts;
    ts.it_value.tv_sec = 10;
    ts.it_value.tv_nsec = 0;

    ts.it_interval.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;

    ret = timer_settime(timer, 0, &ts, NULL);
    if(ret < 0){
        perror("error with time settimeout");
        exit(1);
    }
}


std::vector<std::string> split(std::string sString,std::string delimiter);

struct calcProtocol getCalStruct();

int gsready(std::string &ip, int port,int* ipstatus);

struct calcProtocol calculateValues(struct calcProtocol recvstruct,int *finalintR, double *finalfloatR, int *statusofCal);


char* math(std::string string, double a, double b);

void *clientAddr; 

socklen_t *clientAddrlen;

std::vector<int> tokens;


int main(int argc, char *argv[]){
  
initCalcLib();
  
std::string delimiter = ":";

std::vector<std::string> outputString = split(argv[1],":");

std::string ipString = "";

int port;

if(outputString.size() > 2){
  port = atoi(outputString[outputString.size()-1].c_str());
  for(int i=0; i < 8 ; i++){
    if(i > 0){
      ipString = ipString + ":" + outputString[i];}
    else{
      ipString = ipString + outputString[i];}
   }
  }
else{
  port = atoi(outputString[1].c_str());
  ipString = outputString[0];
}

int *ipstatus = new int;

int socketfd = gsready(ipString ,port, ipstatus);

if (*ipstatus == 1) {
  clientAddr = malloc(sizeof(struct sockaddr_in));
  clientAddrlen = (socklen_t *)malloc(sizeof(socklen_t));
  *clientAddrlen = sizeof(clientAddr);
} else if (*ipstatus == 2) {
  clientAddr = malloc(sizeof(struct sockaddr_in6));
  clientAddrlen = (socklen_t *)malloc(sizeof(socklen_t));
  *clientAddrlen = sizeof(clientAddr);
}


while(1){
  char buffer[sizeof(struct calcProtocol)];
  int sent_recv_bytes = recvfrom(socketfd, buffer,sizeof(buffer),0, (struct sockaddr*)clientAddr, clientAddrlen);
  if(sent_recv_bytes < 0){perror("recvfrom error");exit(1);}

  if(sent_recv_bytes == sizeof(struct calcMessage)){
        struct calcMessage recvmes;
        memcpy(&recvmes, buffer, sizeof(recvmes));
        if((ntohs(recvmes.protocol) == 17) && (ntohl(recvmes.message) == 0) &&(ntohs(recvmes.major_version)==1) &&(ntohs(recvmes.minor_version)==0)){
          struct calcProtocol sendmes;
          sendmes = getCalStruct();
          sent_recv_bytes = sendto(socketfd, (char*)&sendmes, sizeof(sendmes),0,(struct sockaddr*)clientAddr, *clientAddrlen);
          if(sent_recv_bytes < 0){perror("send error");exit(1);}                                                                                    }
        else{
          struct calcMessage sendmes;
          sendmes.major_version = htons(1);
          sendmes.minor_version = htons(0);
          sendmes.protocol = htonl(17);
          sendmes.type = htons(2);
          sendmes.message = htonl(2);
          sent_recv_bytes = sendto(socketfd, &sendmes, sizeof(struct calcMessage),0,(struct sockaddr*)clientAddr,*clientAddrlen);
          if(sent_recv_bytes < 0){perror("error while sending data");exit(1);}
           }
                                                   }
        else if(sent_recv_bytes == sizeof(struct calcProtocol)){
          struct calcProtocol recvmes;
          memcpy(&recvmes,buffer, sizeof(recvmes));
          if((checkclientinvector(ntohl(recvmes.id)) == true)&&(ntohs(recvmes.major_version)== 1)&&(ntohs(recvmes.minor_version)==0)){
            removeClientid(recvmes.id);
          if(ntohl(recvmes.arith) < 5){
            int clientresult = ntohl(recvmes.inResult);
            int *serverresulti = new int;
            double *serverresultf = new double;
            int *stat = new int;
            calculateValues(recvmes,serverresulti, serverresultf, stat);
            std::cout << *serverresulti << std::endl;
            if(clientresult == *serverresulti){
              struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(1);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");
                exit(1);
              }
              std::cout << "OK" << std::endl;
              std::cout << std::endl;
            } 
            else{
              struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(2);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");
              }
              std::cout << "NOT OK" << std::endl;
              std::cout << std::endl;
          }         
          delete serverresultf;
          delete serverresulti;
          delete stat;
        }
        else if (ntohl(recvmes.arith) > 4 && ntohl(recvmes.arith) < 9)
        {
          double clientresult = recvmes.flResult;
          int *serverresulti = new int;
          double *serverresultf = new double;
          int *stat = new int;
          calculateValues(recvmes,serverresulti, serverresultf, stat);
          std::cout << *serverresultf << std::endl;
          if(clientresult == *serverresultf){
              struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(1);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");
              }
              std::cout << "OK" << std::endl;
              std::cout << std::endl;
          } 
          else{
              struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(2);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");
              }
              std::cout << "NOT OK" << std::endl;
              std::cout << std::endl;
          } 
          delete serverresultf;
          delete serverresulti;
          delete stat;
          
        }
        else{
              struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(2);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");
              }
              std::cout << "NOT OK" << std::endl;
              std::cout << std::endl;

        }
        }
        else{
              struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(2);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");exit(1);
              }
              std::cout << "NOT OK" << std::endl;
              std::cout << std::endl;

        }

       }
       else{
          struct calcMessage sendm;
              sendm.major_version = htons(1);
              sendm.minor_version = htons(0);
              sendm.protocol = htons(17);
              sendm.message = htonl(2);
              sendm.type = htons(2);
              sent_recv_bytes = sendto(socketfd,(char*)&sendm, sizeof(struct calcMessage), 0 ,(struct sockaddr*)clientAddr, *clientAddrlen);
              if(sent_recv_bytes < 0){
                perror("error while sending data");exit(1);
              }
              std::cout << "NOT OK" << std::endl;
              std::cout << std::endl;

       }
                                      




}   

  delete ipstatus;  
  return 0;
}

int gsready(std::string &ip, int port,int* ipstatus){

int socketfd; 
struct sockaddr_in ipv4;
struct sockaddr_in6 ipv6;
struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_STREAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
if(status != 0){
std::cout << "There is problem in getting getaddrinfo" << std::endl;

}

for(temp=output; temp != NULL;temp->ai_addr){

if(temp->ai_family == AF_INET){
ipv4.sin_family = AF_INET;
ipv4.sin_port = htons(port);
ipv4.sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
socketfd = socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv4,sizeof(struct sockaddr)) < 0){perror("error with binding the ip address");exit(1);}
  std::cout << "Listening on " << ip << " port " << port << std::endl;
  *ipstatus = 1;
   break;
}}
                                              
else if(temp->ai_family == AF_INET6){
ipv6.sin6_family = AF_INET6;
ipv6.sin6_port = htons(port);
ipv6.sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
socketfd = socket(AF_INET6,SOCK_DGRAM, IPPROTO_UDP);
if(socketfd > 0){
  if(bind(socketfd,(struct sockaddr*)&ipv6,sizeof(struct sockaddr_in6)) < 0){perror("error with binding the ip address6");exit(1);}
   std::cout << "Listening on " << ip << " port " << port << std::endl;
  *ipstatus = 2;
   break;
}}}

if(*ipstatus != 1 && *ipstatus != 2){
  perror("error with socket");
  exit(1);}


  
freeaddrinfo(output);
return socketfd;
}



std::vector<std::string> split(std::string sString, std::string delimiter) {
    std::vector<std::string> nString;
    std::string temp;
    int count = 0; 

    for (int i = 0; i < static_cast<int>(sString.length()); i++) {
        if (sString[i] == delimiter[0]) {
            count++;
            nString.push_back(temp);
            temp = "";
        } else {
            temp = temp + sString[i];
        }
    }

   
    if (count == 0 || (count > 0 && temp != "")) {
        nString.push_back(temp);
    }

    return nString;
}






struct calcProtocol calculateValues(struct calcProtocol recvstruct,int *finalintR, double *finalfloatR, int *statusofCal){

int result,value1,value2;
double fresult,dvalue1,dvalue2;
value1 = ntohl(recvstruct.inValue1);
value2 = ntohl(recvstruct.inValue2);
dvalue1 = recvstruct.flValue1; 
dvalue2 = recvstruct.flValue2;
std::cout << "Assignment : ";
switch(ntohl(recvstruct.arith)){   

    case 1:
          result = value1 + value2;
          std::cout << "add " << value1 << " " << value2 << std::endl;
          #ifdef DEBUG 
          std::cout <<"Calculate the result to " <<  result << std::endl;
          #endif
          *finalintR = result;
          *statusofCal = 1;
          recvstruct.inResult = htonl(result);    
          break;
    case 2:
          result = value1 - value2;
          std::cout << "sub " << value1 << " " << value2 << std::endl;
          #ifdef DEBUG 
          std::cout <<"Calculate the result to "<<  result << std::endl;
          #endif
          *finalintR = result;
          *statusofCal = 1;
          recvstruct.inResult = htonl(result); 
          break;
          
     case 3:
           result = value1 * value2;
           std::cout << "mul " << value1 << " " << value2 << std::endl; 
           #ifdef DEBUG 
           std::cout <<"Calculate the result to "<<  result << std::endl;
           #endif
           *finalintR = result;
           *statusofCal = 1;
           recvstruct.inResult = htonl(result);            
          break;
     
     case 4:
            result = value1/value2;
            std::cout << "div " << value1 << " " << value2 << std::endl; 
            #ifdef DEBUG 
            std::cout <<"Calculate the result to "<<  result << std::endl;
            #endif
            *finalintR = result;
            *statusofCal = 1;
            recvstruct.inResult = htonl(result);
            break;
           
      case 5:
             fresult = dvalue1 + dvalue2;
             std::cout << "fadd " << dvalue1 << " " << dvalue2 << std::endl;
              recvstruct.flResult = fresult;
              *finalfloatR = fresult;
              *statusofCal = 2;
              #ifdef DEBUG 
              std::cout <<"Calculate the result to " << fresult << std::endl;
              #endif
             break;
           
       case 6:
             fresult = dvalue1 - dvalue2;
             std::cout << "fsub " << dvalue1 << " " << dvalue2 << std::endl;
             recvstruct.flResult = fresult;
             *finalfloatR = fresult;
             *statusofCal = 2;
             #ifdef DEBUG 
             std::cout <<"Calculate the result to "<< fresult << std::endl;
             #endif
            break;
            
        case 7:        
              fresult = dvalue1 * dvalue2;
              std::cout << "fmul " << dvalue1 << " " << dvalue2 << std::endl;
               recvstruct.flResult = fresult;
               *finalfloatR = fresult;
               *statusofCal = 2;
               #ifdef DEBUG 
               std::cout <<"Calculate the result to "<< fresult << std::endl;
               #endif
              break;
            
        case 8:
              fresult = dvalue1/dvalue2;
              std::cout << "fdiv " << dvalue1 << " " << dvalue2 << std::endl;
              recvstruct.flResult = fresult;
              *finalfloatR = fresult;
              *statusofCal = 2;
              #ifdef DEBUG 
              std::cout <<"Calculate the result to "<< fresult << std::endl;
              #endif
              break;
      
    
}

return recvstruct;
}

struct calcProtocol getCalStruct(){

struct calcProtocol calstru;

double float1,float2;
int integer1,integer2;

char *oper;

oper = randomType();

integer1 = randomInt();
integer2 = randomInt();

float1 = randomFloat();
float2 = randomFloat();

    if(strcmp(oper,"fadd")==0){
      calstru.arith = htonl(5);
      calstru.flValue1 = float1;
      calstru.flValue2 = float2;
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
     std::cout << oper << " " << float1 << " " << float2 << std::endl;
    } else if (strcmp(oper, "fsub")==0){
      calstru.arith = htonl(6);
      calstru.flValue1 = float1;
      calstru.flValue2 = float2;
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
      std::cout << oper << " " << float1 << " " << float2 << std::endl;
    } else if (strcmp(oper, "fmul")==0){
        calstru.arith = htonl(7);
      calstru.flValue1 = float1;
      calstru.flValue2 = float2;
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(0);
      std::cout << oper << " " << float1 << " " << float2 << std::endl;
    } else if (strcmp(oper, "fdiv")==0){
      calstru.arith = htonl(8);
      calstru.flValue1 = float1;
      calstru.flValue2 = float2;
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
      std::cout << oper << " " << float1 << " " << float2 << std::endl;
    }
    else if(strcmp(oper,"add")==0){
      calstru.arith = htonl(1);
      calstru.inValue1 = htonl(integer1);
      calstru.inValue2 = htonl(integer2);
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
      std::cout << oper << " " << integer1 << " " << integer2 << std::endl;
    } else if (strcmp(oper, "sub")==0){
      calstru.arith = htonl(2);
      calstru.inValue1 = htonl(integer1);
      calstru.inValue2 = htonl(integer2);
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
      std::cout << oper << " " << integer1 << " " << integer2 << std::endl;
    } else if (strcmp(oper, "mul")==0){
      calstru.arith = htonl(3);
      calstru.inValue1 = htonl(integer1);
      calstru.inValue2 = htonl(integer2);
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
      std::cout << oper << " " << integer1 << " " << integer2 << std::endl;
    } else if (strcmp(oper, "div")==0){
      calstru.arith = htonl(4);
      calstru.inValue1 = htonl(integer1);
      calstru.inValue2 = htonl(integer2);
      calstru.major_version = htons(1);
      calstru.minor_version = htons(0);
      calstru.type = htons(1);
      std::cout << oper << " " << integer1 << " " << integer2 << std::endl;
    }

    int a = getRandomNumberNotInVector();
    timerdemo(a);
    calstru.id = htonl(a);

return calstru;
}
