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
#include <calcLib.h>
#include <stdlib.h>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include "protocol.h"


std::vector<std::string> split(std::string sString,std::string delimiter);

void gsready(std::string &ip, int port, struct sockaddr_in *ipv4,struct sockaddr_in6 *ipv6, int* ipstatus);


std::vector<std::string> nString;


std::string temp;

struct calcMessage recvBufferMessage(int socketfd,int *sent_recv_bytes);

struct calcMessage recvBufferMessageipv6(int socketfd,int *sent_recv_bytes);

struct calcProtocol recvBuffer(int socketfd,int *sent_recv_bytes);

struct calcProtocol recvBufferipv6(int socketfd,int *sent_recv_bytes);


struct calcProtocol calculateValues(struct calcProtocol recvstruct,int *finalintR, double *finalfloatR, int *statusofCal);


void sendBufferIntial(int socketfd,struct calcMessage intial, struct sockaddr_in *server,int *sent_recv_bytes);

void sendBufferIntialipv6(int socketfd,struct calcMessage intial, struct sockaddr_in6 *server,int *sent_recv_bytes);


void sendBuffer(int socketfd,struct calcProtocol message, struct sockaddr_in *server,int *sent_recv_bytes);

void sendBufferipv6(int socketfd,struct calcProtocol message, struct sockaddr_in6 *server,int *sent_recv_bytes);


char* math(std::string string, double a, double b);


struct calcMessage intialMessage;


//Main function


int main(int argc, char *argv[]){

initCalcLib();

intialMessage.type = htons(22);
intialMessage.message = htons(0);
intialMessage.protocol = htons(17);
intialMessage.major_version = htons(1);
intialMessage.minor_version = htons(0);

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

std::cout << "HOST " << ipString << ", and port "<< port << std::endl;

int *intValue = new int;
double *doubleValue = new double;
int *stCal = new int;


struct sockaddr_in6 *ipv6 = new struct sockaddr_in6;

struct sockaddr_in *ipv4 = new struct sockaddr_in;

int *ipstatus = new int;

gsready(ipString ,port, ipv4, ipv6, ipstatus);

if(*ipstatus == 1){

   int socketfd;
   
    socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);   
    
    if(socketfd < 0) {
        std::cerr << "Error in socket creation" << std::endl;
        std::cout << "error socket" << std::endl;
        return 1;
    }
    
    #ifdef DEBUG 
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    
    if (getsockname(socketfd, (struct sockaddr*)&local_addr, &addr_len) == -1){
        perror("getsockname");
        exit(1);
    }
   
    char local_ip[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &(local_addr.sin_addr), local_ip, INET_ADDRSTRLEN);
    
    unsigned int local_port = ntohs(local_addr.sin_port);
    
    std::cout << " local " << local_ip << ":" << local_port << std::endl;
    #endif
    
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    
    if(setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
      perror("Erro setting timeout ");
      close(socketfd);
      return 1;    
    }
    
    
      int *sent_recv_bytes = new int;
      
      struct calcProtocol recvMessage, replyMessage;
      
        
      sendBufferIntial(socketfd, intialMessage, ipv4 , sent_recv_bytes);
      
      recvMessage = recvBuffer(socketfd, sent_recv_bytes);
      
      if(*sent_recv_bytes < 0){
      int repeat = 2;
         while(repeat > 0){
         repeat--;
         sendBufferIntial(socketfd, intialMessage, ipv4 , sent_recv_bytes);
         recvMessage = recvBuffer(socketfd, sent_recv_bytes);
         if(*sent_recv_bytes < 0 && repeat == 0){
          std::cout << "Unable to connect with the server" << std::endl;
          close(socketfd);
          std::exit(0);
         }
         else if(*sent_recv_bytes >0){
          repeat = 0;
         }        
         }   
      }
      
      replyMessage = calculateValues(recvMessage, intValue, doubleValue, stCal);
      
      struct calcMessage confMessage;
      memset(&confMessage, 0, sizeof(confMessage));
      
      sendBuffer(socketfd,replyMessage,ipv4,sent_recv_bytes);
      if(*sent_recv_bytes < 0){
      std::cout << "unable to sent the data " << std::endl;}
      confMessage = recvBufferMessage(socketfd, sent_recv_bytes);
      
      if(*sent_recv_bytes < 0){
      int repeat = 2;
         while(repeat > 0){
         repeat--;
         sendBuffer(socketfd,replyMessage,ipv4,sent_recv_bytes);
         confMessage = recvBufferMessage(socketfd, sent_recv_bytes);
         if(*sent_recv_bytes < 0 && repeat == 0){
          std::cout << "Unable to connect with the server" << std::endl;
          close(socketfd);
          std::exit(0);
         }
         else if(*sent_recv_bytes >0){
         repeat = 0;
         }        
         }   
      }
      
      if(ntohl(confMessage.message) == 1){
      if(*stCal == 1){
      std::cout << "OK " << "(myresult=" << *intValue << ")" << std::endl;}
      else if(*stCal == 2){
      std::cout << "OK " << "(myresult=" << *doubleValue << ")" << std::endl;}
      }
      else {
      std::cout << "Server not OK " << std::endl;}
 
      
    close(socketfd);
 
             }
else if(*ipstatus == 2){
     int socketfd;
     socketfd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
     
     if(socketfd < 0) {
        std::cout << "Error in socket creation" << std::endl;
        return 1;
    }
    
    #ifdef DEBUG     
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    
    if (getsockname(socketfd, (struct sockaddr*)&local_addr, &addr_len) == -1){
        perror("getsockname");
        exit(1);
    }
   
    char local_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(local_addr.sin_addr), local_ip, INET_ADDRSTRLEN);
    
    unsigned int local_port = ntohs(local_addr.sin_port);
    
    std::cout << " local " << local_ip << ":" << local_port << std::endl;
    #endif
       
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    
    if(setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0){
      perror("Erro setting timeout ");
      close(socketfd);
      return 1;    
    }
    
    
      int *sent_recv_bytes = new int;
      
      struct calcProtocol recvMessage, replyMessage;
      
        
      sendBufferIntialipv6(socketfd, intialMessage, ipv6 , sent_recv_bytes);
      
      recvMessage = recvBufferipv6(socketfd, sent_recv_bytes);
      
      if(*sent_recv_bytes < 0){
      int repeat = 3;
         while(repeat > 0){
         repeat--;
         sendBufferIntialipv6(socketfd, intialMessage, ipv6 , sent_recv_bytes);
         recvMessage = recvBufferipv6(socketfd, sent_recv_bytes);
         if(*sent_recv_bytes < 0 && repeat == 0){
          std::cout << "Unable to connect with the server" << std::endl;
          close(socketfd);
         }
         else if(*sent_recv_bytes >0){
          repeat = 0;
         }        
         }   
      }
                 
      
      replyMessage = calculateValues(recvMessage, intValue, doubleValue, stCal);
      
      struct calcMessage confMessage;
      memset(&confMessage, 0, sizeof(confMessage));
      
      sendBufferipv6(socketfd,replyMessage,ipv6,sent_recv_bytes);
      if(*sent_recv_bytes < 0){
      std::cout << "unable to sent the data " << std::endl;}
      confMessage = recvBufferMessage(socketfd, sent_recv_bytes);
      
      if(*sent_recv_bytes < 0){
      int repeat = 3;
         while(repeat > 0){
         repeat--;
         sendBufferipv6(socketfd,replyMessage,ipv6,sent_recv_bytes);
         confMessage = recvBufferMessageipv6(socketfd, sent_recv_bytes);
         if(*sent_recv_bytes < 0 && repeat == 0){
          std::cout << "Unable to connect with the server" << std::endl;
          close(socketfd);
          std::exit(0);
         }
         else if(*sent_recv_bytes >0){
         repeat = 0;
         }        
         }   
      }
      
      if(ntohl(confMessage.message) == 1){
      if(*stCal == 1){
      std::cout << "OK " << "(myresult=" << *intValue << ")" << std::endl;}
      else if(*stCal == 2){
      std::cout << "OK " << "(myresult=" << *doubleValue << ")" << std::endl;}
      }
      else {
      std::cout << "Server not OK " << std::endl;}
    
     


}
}


void gsready(std::string &ip, int port, sockaddr_in *ipv4, sockaddr_in6 *ipv6,int* ipstatus){

struct addrinfo hint, *output, *temp;
memset(&hint, 0, sizeof(hint));
hint.ai_family = AF_UNSPEC;
hint.ai_socktype = SOCK_DGRAM;
int status = getaddrinfo(ip.c_str(), NULL, &hint, &output);
char ip_address4[INET_ADDRSTRLEN];
char ip_address6[INET6_ADDRSTRLEN];
if(status != 0){
std::cout << "There is problem in getting getaddrinfo" << std::endl;

}

for(temp=output; temp != NULL;temp->ai_addr){

if(temp->ai_family == AF_INET){
ipv4->sin_family = AF_INET;
ipv4->sin_port = htons(port);
ipv4->sin_addr.s_addr = ((struct sockaddr_in*)temp->ai_addr)->sin_addr.s_addr;
*ipstatus = 1;
#ifdef DEBUG 
inet_ntop(AF_INET, &(ipv4->sin_addr), ip_address4, INET_ADDRSTRLEN);
std::cout << "Connected to " << ip_address4 << ":" << port<< std::endl;
#endif
break;
                            }
                                              
else if(temp->ai_family == AF_INET6){
ipv6->sin6_family = AF_INET6;
ipv6->sin6_port = htons(port);
ipv6->sin6_addr = ((struct sockaddr_in6*)temp->ai_addr)->sin6_addr;
#ifdef DEBUG 
inet_ntop(AF_INET, &(ipv6->sin_addr), ip_address6, INET6_ADDRSTRLEN);
std::cout << "Connected to " << ip_address6 << ":" << port << std::endl;
#endif
*ipstatus = 2;
break;

}

}
freeaddrinfo(output);}


void sendBufferIntial(int socketfd, struct calcMessage intial, struct sockaddr_in *server,int *sent_recv_bytes){

char sendBufIntial[sizeof(struct calcProtocol)];

memcpy(sendBufIntial, &intial, sizeof(struct calcProtocol));
*sent_recv_bytes = 0;
*sent_recv_bytes = sendto(socketfd, sendBufIntial , sizeof(struct calcMessage),0,(struct sockaddr*)server, sizeof(*server));

}

void sendBufferIntialipv6(int socketfd, struct calcMessage intial, struct sockaddr_in6 *server,int *sent_recv_bytes){

char sendBufIntial[sizeof(struct calcProtocol)];

memcpy(sendBufIntial, &intial, sizeof(struct calcProtocol));
*sent_recv_bytes = 0;
*sent_recv_bytes = sendto(socketfd, sendBufIntial , sizeof(struct calcMessage),0,(struct sockaddr*)server, sizeof(*server));

}

void sendBuffer(int socketfd,struct calcProtocol message, struct sockaddr_in *server,int *sent_recv_bytes){
char sendBuf[sizeof(struct calcProtocol)];
memcpy(sendBuf, &message, sizeof(struct calcProtocol));
*sent_recv_bytes = 0;
*sent_recv_bytes = sendto(socketfd, sendBuf, sizeof(struct calcProtocol),0,(struct sockaddr*)server, sizeof(*server));

}

void sendBufferipv6(int socketfd,struct calcProtocol message, struct sockaddr_in6 *server,int *sent_recv_bytes){
char sendBuf[sizeof(struct calcProtocol)];
memcpy(sendBuf, &message, sizeof(struct calcProtocol));
*sent_recv_bytes = 0;
*sent_recv_bytes = sendto(socketfd, sendBuf, sizeof(struct calcProtocol),0,(struct sockaddr*)server, sizeof(*server));

}

struct calcProtocol recvBufferipv6(int socketfd, int *sent_recv_bytes){
char buffertest[sizeof(calcProtocol)];
struct sockaddr_in6 server;
struct calcProtocol recvstruct;
memset(&recvstruct, 0, sizeof(struct calcProtocol));
socklen_t server_len = sizeof(struct sockaddr);
*sent_recv_bytes = 0;
*sent_recv_bytes = recvfrom(socketfd, buffertest, sizeof(struct calcProtocol),0,(struct sockaddr*)&server,&server_len);
 struct calcMessage messagereturncheck;
 memcpy(&messagereturncheck, buffertest, sizeof(calcMessage));
 memcpy(&recvstruct, buffertest, sizeof(calcProtocol));

if(*sent_recv_bytes < (int)sizeof(recvstruct)){
       
        if(messagereturncheck.type == 2 && messagereturncheck.message == 2 && messagereturncheck.major_version == 1 && messagereturncheck.minor_version == 0){
        std::cout << " NOT OK " << std::endl;
        close(socketfd);
        }
}
return recvstruct;
}



struct calcProtocol recvBuffer(int socketfd, int *sent_recv_bytes){
char buffertest[sizeof(calcProtocol)];
struct sockaddr_in server;
struct calcProtocol recvstruct;
memset(&recvstruct, 0, sizeof(struct calcProtocol));
socklen_t server_len = sizeof(struct sockaddr);
*sent_recv_bytes = 0;
*sent_recv_bytes = recvfrom(socketfd, buffertest, sizeof(struct calcProtocol),0,(struct sockaddr*)&server,&server_len);
 struct calcMessage messagereturncheck;
 memcpy(&messagereturncheck, buffertest, sizeof(calcMessage));
 memcpy(&recvstruct, buffertest, sizeof(calcProtocol));

if(*sent_recv_bytes < (int)sizeof(recvstruct)){
       
        if(messagereturncheck.type == 2 && messagereturncheck.message == 2 && messagereturncheck.major_version == 1 && messagereturncheck.minor_version == 0){
        std::cout << " NOT OK " << std::endl;
        close(socketfd);
        }
}
return recvstruct;
}

//split function
std::vector<std::string> split(std::string sString,std::string delimiter){

std::vector<std::string> nString;
std::string temp;

for(int i=0; i < static_cast<int>(sString.length());i++){
  int  count = 0;
  if(sString[i] == delimiter[0]){
        count++;
        nString.push_back(temp);
        temp  = "";
    }
  else{
        temp = temp +  sString[i];
         }

  if(count==0 && (i == static_cast<int>(sString.length()-1))){
         nString.push_back(temp);}               }


return nString;
}

//recv clacMessage function
struct calcMessage recvBufferMessage(int socketfd,int *sent_recv_bytes){
struct sockaddr_in server;
struct calcMessage checkmessage;
socklen_t server_len = sizeof(struct sockaddr);
*sent_recv_bytes = 0;
*sent_recv_bytes = recvfrom(socketfd, (char*)&checkmessage, sizeof(struct calcMessage),0,(struct sockaddr*)&server,&server_len);
return checkmessage;
}


struct calcMessage recvBufferMessageipv6(int socketfd,int *sent_recv_bytes){
struct sockaddr_in6 server;
struct calcMessage checkmessage;
socklen_t server_len = sizeof(struct sockaddr);
*sent_recv_bytes = 0;
*sent_recv_bytes = recvfrom(socketfd, (char*)&checkmessage, sizeof(struct calcMessage),0,(struct sockaddr*)&server,&server_len);
return checkmessage;
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
