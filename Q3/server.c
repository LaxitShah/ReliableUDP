#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8094

struct control_packet{
	int sr_no;
	int no_of_pkt;
	int start_serial_no;
	int end_serial_number;
	int checksum;
};

struct data_packet{
	int sr_no;
	char msg[100];
	long checkSum;
};


struct ack_packet{
	int ack;
	int sr_no;
	long checkSum;
};

void setZero(struct control_packet * pkt){
	pkt->sr_no=0;
	pkt->no_of_pkt=0;
	pkt->checksum=0;
	pkt->end_serial_number=0;
	pkt->start_serial_no=0;
}

int no_of_packet(char * msg){
	int len = strlen(msg);
	int req = len/100;
	double temp = len/(double)100;
	if(temp > req) req++;
	return req;
}

long createSumControl(struct control_packet * c_pkt){
	long sum=0;
	sum+=c_pkt->end_serial_number;
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

long createSumACK(struct ack_packet* ack){
	return ack->ack+ack->sr_no;
}

int checkSumACK(struct ack_packet * ack){
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

int shakeHand(int socket_desc,struct sockaddr_in client_addr,int client_addr_size,
	struct control_packet * c_pkt){
	setZero(c_pkt);

	struct ack_packet ackPkt;
	struct ack_packet * ack = &ackPkt;

	ack->ack=0;
	ack->sr_no=0;

	int valid = 0; 
	while(valid == 0){
		if(recvfrom(socket_desc,c_pkt,sizeof(*c_pkt),0,
			(struct sockaddr *)&client_addr,&client_addr_size)<0){
				printf("\nReceving Error.\n");
				return -1;
		}
		printf("\nRecieved control pkt\n");
		valid = checkSumControl(c_pkt);
		ack->ack=1;
		ack->checkSum = createSumACK(ack);
		if(sendto(socket_desc,ack,sizeof(*ack),0,
			(struct sockaddr *)&client_addr,client_addr_size)<0){
				printf("\nError when sending acknolodgement to client");
				return -1;
		}
	}

	printf("\nSent control to packet ack.\n");
	return 1;

}

int comunicate(int socket_desc,struct sockaddr_in client_addr,int client_addr_size){
	struct data_packet data;
	struct data_packet * data1 = &data;
	memset(data1->msg,0,100);

	struct ack_packet ackPKT;
	struct ack_packet * ack = &ackPKT;
	
	if(recvfrom(socket_desc,data1,sizeof(*data1),0,
		(struct sockaddr *)&client_addr,&client_addr_size)<0){
			printf("\nError Recieved.\n");
			return -1;
	}
	int valid = checkSum(data1);
	printf("\nMessageg From Client: %s",data1->msg);

	if(valid == 1){
		ack->ack=1;
		printf("\nData packet  Valid\n");
	}
	else{
		ack->ack=0;
		printf("\nData packet is not valid\n");
	}
	ack->checkSum = createSumACK(ack);
	char c;
	scanf("%c",&c);

	if(sendto(socket_desc,ack,sizeof(*ack),0,
		(struct sockaddr *)&client_addr,client_addr_size)<0){
			printf("\nError while sending ack to client.");
			return -1;
	}


	if(sendto(socket_desc,ack,sizeof(*ack),0,
		(struct sockaddr *)&client_addr,client_addr_size)<0){
			printf("\nError while sending ack to client.");
			return -1;
	}

}

int comunicatePkt(int socket_desc,struct sockaddr_in client_addr,int client_addr_size,int sr_no){
	struct data_packet data;
	struct data_packet * data1 = &data;
	memset(data1->msg,0,100);

	struct ack_packet ackPKT;
	struct ack_packet * ack = &ackPKT;

	if(recvfrom(socket_desc,data1,sizeof(*data1),0,
		(struct sockaddr *)&client_addr,&client_addr_size)<0){
			printf("\nRecvieved error.\n");
			return -1;
	}
	int valid = checkSum(data1);
	printf("\nChecksum: %ld",createSum(data1));
	printf("\nMessage from client: %s",data1->msg);

	if(valid == 1){
		if(data1->sr_no != sr_no) return -1;
		ack->ack=1;

		//storing in file
		char * filename = "get.txt";
		FILE *fp = fopen(filename,"a");

		fprintf(fp,"%s",data1->msg);
		printf("\nmessage: %s",data1->msg);

		printf("\nData packet is valid\n");
	}
	else{
		ack->ack=-1;
		printf("\nData packet is not valid\n");
	}
	ack->checkSum = createSumACK(ack);

	if(sendto(socket_desc,ack,sizeof(*ack),0,
		(struct sockaddr *)&client_addr,client_addr_size)<0){
			printf("\nError while sending ack to client.");
			return -1;
	}
	return ack->ack;
}

int main(){
	int socket_desc;
	struct sockaddr_in server_addr,client_addr;
	int client_addr_size = sizeof(client_addr);

	socket_desc = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(socket_desc < 0){
		printf("\nError while creating socket.");
		return -1;
	}
	printf("\nCreated socket.");

	//set socket recv and send timeout option pending...

	// seting port and ip
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(socket_desc,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		printf("\nBinding error");
		return -1;
	}
	printf("\nBinding of socket.");

	printf("\nNow listening...");

	struct control_packet pkt;


	int handshakeResult = shakeHand(socket_desc,client_addr,client_addr_size,&pkt);
	while(handshakeResult==-1){
		handshakeResult=shakeHand(socket_desc,client_addr,client_addr_size,&pkt);
		return -1;
	}

	printf("\nPackets : %d\n",pkt.no_of_pkt);

	for(int i=0;i<3;i++){
		int result = comunicatePkt(socket_desc,client_addr,client_addr_size,i);
		while(result == -1){
			result = comunicatePkt(socket_desc,client_addr,client_addr_size,i);
		}
	}
	return 1;
}
