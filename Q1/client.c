#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>

#define PORT 8091
struct control_packet{
	int sr_no;
	int no_of_pkt;
	int start_serial_no;
	int end_serial_no;
	int checksum;
};


struct data_packet{
	int sr_no;
	char msg[100];
	long checkSum;
};

struct ack_pkt{
	int ack;
	int sr_no;
	long checkSum;
};


int noOfPck(int size){
	int req = size/100;
	double reqD = size/(double)100;
	if(reqD > req) req++;
	return req;
}


long createSumControl(struct control_packet * c_pkt){
	long sum=0;
	sum+=c_pkt->end_serial_no;
	sum+=c_pkt->no_of_pkt;
	sum+=c_pkt->sr_no;
	sum+=c_pkt->start_serial_no;
	return sum;
}

int checkSumControl(struct control_packet * c_pkt){
	long check = createSumControl(c_pkt);
	if(c_pkt->checksum == check) return 1;
	return 0;
}

long createSumACK(struct ack_pkt* ack){
	return ack->ack+ack->sr_no;
}

int checkSumACK(struct ack_pkt * ack){
	long ackSum = createSumACK(ack);
	if(ackSum == ack->checkSum) return 1;
	return 0;
}

long createSum(struct data_packet* data){
	long msgSum=0;

	for(int i=0;i<100;i+=2){
		msgSum += ((data->msg[i]<<8) + data->msg[i+1]);
	}

	msgSum += data->sr_no;

	return msgSum;
}

int checkSum(struct data_packet* data){
	long check = createSum(data);

	if(data->checkSum == check)return 1;
	return 0;
}


int shakeHand(int sock_fd,struct sockaddr_in server_addr,int sock_length,int size,
	struct control_packet* c_pkt){

	int pkts = noOfPck(size);

	printf("\n[+]Packets in handshake: %d\n",pkts);

	c_pkt->end_serial_no = pkts;
	c_pkt->sr_no = 0;
	c_pkt->no_of_pkt = pkts;
	c_pkt->start_serial_no=1;
	c_pkt->checksum = createSumControl(c_pkt);

	//ack
	struct ack_pkt ackPkt;
	struct ack_pkt * ack = &ackPkt;

	ack->ack=-1;

	
	while(ack->ack != 1){
		if(sendto(sock_fd,c_pkt,sizeof(*c_pkt),0,
			(struct sockaddr *)&server_addr,sock_length)<0){
				printf("\nError while sending message to server.\n");
				return -1;
		}
		printf("\n[+]Sent handshake.");

		//time delay check
		struct timeval t;
		t.tv_sec=5;
	/*fd_set is a data structure used in the C programming language to manage file descriptors for I/O operations. It is part of the POSIX standard, which defines the API for operating systems.

fd_set is actually a bit array where each bit corresponds to a file descriptor. It is used in conjunction with the
 select() function to check the status of multiple file descriptors at once. By setting the corresponding bit for each file descriptor in the fd_set, the select() function can efficiently
  monitor multiple file descriptors for I/O activity, such as whether there is data available for reading or if it is possible to write to a file without blocking.

The fd_set structure is defined in the header file sys/select.h and is typically used in systems programming to implement network servers, 
where multiple connections need to be monitored simultaneously.*/

		fd_set socks;
		FD_ZERO(&socks);
		/*FD_ZERO is a macro in C programming language which is used to initialize the file descriptor set to be empty*/
		FD_SET(sock_fd,&socks);

		if(select(sock_fd+1,&socks,NULL,NULL,&t) && 
			recvfrom(sock_fd,ack,sizeof(*ack),0,
			(struct sockaddr *)&server_addr,&sock_length)<0){
				printf("\nError in delay receive function.");
		}

	}
	return 1;

}


int comunicate(int sock_fd,struct sockaddr_in server_addr,int sock_length){
	//msg
	struct data_packet dataSend;
	struct data_packet * data1 = &dataSend;
	memset(data1->msg,0,100);

	printf("\nEnter msg: ");

	//setting values of data_packet
	fgets(data1->msg,100,stdin);
	data1->sr_no=1;

	data1->checkSum = createSum(data1);


	//sending data_packet
	if(sendto(sock_fd,data1,sizeof(*data1),0,
		(struct sockaddr *)&server_addr,sock_length)<0){
			printf("\nError while sending message to Server.");
			return -1;
	}
	// data1->msg[0] = 'h';
	printf("\nSent: %s",data1->msg);

	
	//ack
	struct ack_pkt dataRecv;
	struct ack_pkt * ack = &dataRecv;
	ack->ack=-1;
	//time delay check
	struct timeval t;
	t.tv_sec=5;
	fd_set socks;
	FD_ZERO(&socks);
	FD_SET(sock_fd,&socks);

	if(select(sock_fd+1,&socks,NULL,NULL,&t) && 
		recvfrom(sock_fd,ack,sizeof(*ack),0,
		(struct sockaddr *)&server_addr,&sock_length)<0){
			printf("\nError in Delay of Recive Function");
	
	}

	if(ack->ack==-1){
		printf("\nNo acknolodgement recived");
	}
	else{
		printf("\nAcknolodgement recived");
	}



	return 1;
}



int comunicatePkt(int sock_fd,struct sockaddr_in server_addr,int sock_length,char * msg,int sr_no){
	//data packet
	struct data_packet dataSend;
	struct data_packet * data = &dataSend;
	memset(data->msg,0,100);

	//ack packet
	struct ack_pkt ackPkt;
	struct ack_pkt * ack = &ackPkt;

	ack->ack=-1;

	int i=0;
	while(*msg != '\0'){
		data->msg[i]=*msg;
		msg++;i++;
	}

	data->sr_no = sr_no;
	data->checkSum = createSum(data);

	if(sendto(sock_fd,data,sizeof(*data),0,
		(struct sockaddr *)&server_addr,sock_length)<0){
			printf("\nError while sending message to server");
			return -1;
	}

	printf("\n[+]Sent packet %d : %s ",sr_no,data->msg);
	printf("\n[+]checksum: %ld",data->checkSum);

	//time delay check
	struct timeval t;
	t.tv_sec=5;
	fd_set socks;
	FD_ZERO(&socks);
	FD_SET(sock_fd,&socks);
	if(select(sock_fd+1,&socks,NULL,NULL,&t) && 
		recvfrom(sock_fd,ack,sizeof(*ack),0,
		(struct sockaddr *)&server_addr,&sock_length)<0){
			printf("\nError in delay recv function.");
	}
	printf("\n[+]ack: %d",ack->ack);
	return ack->ack;

}


int main(){
	int sock_fd;
	struct sockaddr_in server_addr;
	int sock_length = sizeof(server_addr);

	sock_fd = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

	if(sock_fd < 0){
		printf("\nFailed to create Socket.");
		return -1;
	}
	printf("\nSocket created.");

	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);
	server_addr.sin_family = AF_INET;

	char msg[200];
	int size = sizeof(msg)/sizeof(msg[0]);

	struct control_packet pkt;

	int handshakeResult = shakeHand(sock_fd,server_addr,sock_length,size,&pkt);
	while(handshakeResult==-1){
		handshakeResult=shakeHand(sock_fd,server_addr,sock_length,size,&pkt);
	}

	printf("\nHandshake Sucessfull\n");
	printf("\nNumber of packets: %d.",pkt.no_of_pkt);

	char *pkt_datas[2];
	char * f = "This is the random message 1";
	char * s = "This is the  random mesage 2";
	printf("here");
	pkt_datas[0]=f;
	pkt_datas[1]=s;

	
	for(int i=0;i<pkt.no_of_pkt;i++){
		int result = comunicatePkt(sock_fd,server_addr,sock_length,pkt_datas[i],i);
		while(result == -1){
			result = comunicatePkt(sock_fd,server_addr,sock_length,pkt_datas[i],i);
		}
	}

	return 0;
}
