#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 8091
struct ctrl_pack{
	int sr_no;
	int no_of_pkt;
	int start_serial_num;
	int end_serial_num;
	int checksum;
};
struct data_pack{
	int sr_no;
	char msg[100];
	long checkSum;
};
struct ack_pack{
	int ack;
	int sr_no;
	long checkSum;
};

void setZero(struct ctrl_pack * packet){
	packet->sr_no=0;
	packet->no_of_pkt=0;
	packet->checksum=0;
	packet->end_serial_num=0;
	packet->start_serial_num=0;
}

int noOfPck(char * msg){
	int len = strlen(msg);
	int req = len/100;
	double temp = len/(double)100;
	if(temp > req) req++;
	return req;
}

long createSumControl(struct ctrl_pack * c_packet){
	long sum=0;
	sum+=c_packet->end_serial_num;
	sum+=c_packet->no_of_pkt;
	sum+=c_packet->sr_no;
	sum+=c_packet->start_serial_num;
	return sum;
}

int checkSumControl(struct ctrl_pack * c_packet){
	long check = createSumControl(c_packet);
	if(c_packet->checksum == check) return 1;
	return 0;
}

long createSumAcknoledgement(struct ack_pack* ack){
	return ack->ack+ack->sr_no;
}

int checkSumAcknolodgement(struct ack_pack * ack){
	long ackSum = createSumAcknoledgement(ack);
	if(ackSum == ack->checkSum) return 1;
	return 0;
}

long createSum(struct data_pack* data){
	long msgSum=0;

	for(int i=0;i<100;i+=2){
		msgSum += ((data->msg[i]<<8) + data->msg[i+1]);
	}

	msgSum += data->sr_no;

	return msgSum;
}

int checkSum(struct data_pack* data){
	long check = createSum(data);

	if(data->checkSum == check)return 1;
	return 0;
}	


int shakeHand(int socket_desc,struct sockaddr_in client_addr,int client_addr_size,
	struct ctrl_pack * c_packet){
	setZero(c_packet);

	//ack
	struct ack_pack ackPkt;
	struct ack_pack * ack = &ackPkt;

	ack->ack=0;
	ack->sr_no=0;

	int valid = 0; 

	while(valid == 0){
		if(recvfrom(socket_desc,c_packet,sizeof(*c_packet),0,
			(struct sockaddr *)&client_addr,&client_addr_size)<0){
				printf("\nReceive error.\n");
				return -1;
		}
		printf("\nControl Packet Received\n");
		valid = checkSumControl(c_packet);
		ack->ack=1;
		ack->checkSum = createSumAcknoledgement(ack);
		if(sendto(socket_desc,ack,sizeof(*ack),0,
			(struct sockaddr *)&client_addr,client_addr_size)<0){
				printf("\nAck not send to client");
				return -1;
		}
	}

	printf("\nSent control pkt ack.\n");
	return 1;

}

int comunicate(int socket_desc,struct sockaddr_in client_addr,int client_addr_size){
	
	struct data_pack data;
	struct data_pack * data1 = &data;
	memset(data1->msg,0,100);

	struct ack_pack ackPKT;
	struct ack_pack * ack = &ackPKT;

	if(recvfrom(socket_desc,data1,sizeof(*data1),0,
		(struct sockaddr *)&client_addr,&client_addr_size)<0){
			printf("\nRecv error.\n");
			return -1;
	}
	int valid = checkSum(data1);
	printf("\nMsg from client: %s",data1->msg);

	if(valid == 1){
		ack->ack=1;
		printf("\nData pkt is valid\n");
	}
	else{
		ack->ack=0;
		printf("\nData pkt is not valid\n");
	}
	ack->checkSum = createSumAcknoledgement(ack);
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

int communicatePacket(int socket_desc,struct sockaddr_in client_addr,int client_addr_size,int sr_no){

	struct data_pack data;
	struct data_pack * data1 = &data;
	memset(data1->msg,0,100);

	//ack
	struct ack_pack ackPKT;
	struct ack_pack * ack = &ackPKT;

	if(recvfrom(socket_desc,data1,sizeof(*data1),0,
		(struct sockaddr *)&client_addr,&client_addr_size)<0){
			printf("\nRecieve Error.\n");
			return -1;
	}
	int valid = checkSum(data1);
	printf("\nchecksum: %ld",createSum(data1));
	printf("\nMsg from client: %s",data1->msg);

	if(valid == 1){
		if(data1->sr_no != sr_no) return -1;
		ack->ack=1;
		printf("\nData packet is valid\n");
	}
	else{
		ack->ack=-1;
		printf("\nData packet is not valid\n");
	}
	ack->checkSum = createSumAcknoledgement(ack);
	char c;
	scanf("%c",&c);


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
		printf("\nError ");
		return -1;
	}
	printf("\nSocket Created");

	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(socket_desc,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){
		printf("\nBinding error");
		return -1;
	}
	printf("\nSocket Binding");

	printf("\nListiening...");

	struct ctrl_pack pkt;


	int handshakeResult = shakeHand(socket_desc,client_addr,client_addr_size,&pkt);
	while(handshakeResult==-1){

		return -1;
	}

	printf("\nPackets : %d\n",pkt.no_of_pkt);

	for(int i=0;i<pkt.no_of_pkt;i++){
		int result = communicatePacket(socket_desc,client_addr,client_addr_size,i);
		while(result == -1){
			result = communicatePacket(socket_desc,client_addr,client_addr_size,i);
		}
	}

	return 1;
}