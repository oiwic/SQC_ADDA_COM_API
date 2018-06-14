#define DLLAPI  __declspec(dllexport)

#include "USTCADCDriver.h"
#include "pcap.h"
#include "USTCADCError.h"
#include <Winsock2.h>

pcap_t *pcapHandle = NULL;									//�������
unsigned char macSrc[6] = {0x34,0x97,0xf6,0x8d,0x41,0x45};	//Դmac��ַ
unsigned char macDst[6] = {0xff,0xff,0xff,0xff,0xff,0xff};	//Ŀ��mac��ַ
unsigned char protocal[2] = {0xaa,0x55};					//Э������

DLLAPI int OpenADC(int num)
{
	char   errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fcode;
	pcap_if_t *firstdev;
	pcap_if_t *selectdev;

	/* �����豸 */
	 pcap_findalldevs(&firstdev,errbuf);
	 selectdev = firstdev;
	 while((num > 1) && selectdev)
	 {
		 selectdev = selectdev->next;
		 num--;
	 }
	 if(selectdev == 0)
	{
		pcap_freealldevs(firstdev);
		return ERR_NONETCARD;
	 }
	/* ������ӿ�,�������֡����Ϊ1500B,��ʱ�ȴ�100ms */  
	 pcapHandle=pcap_open_live(selectdev->name,1500,1,100,errbuf);
	/* ���û����СΪ128MB */
	 pcap_setbuff(pcapHandle,134217728);
	 pcap_setuserbuffer(pcapHandle,134217728);
	/* ���ô��ں˵��û����浥����С�������ݴ�СΪ1KB */
	 pcap_setmintocopy(pcapHandle,1024);
	/* ���÷�����ģʽ */
	// pcap_setnonblock(pcapHandle,1,errbuf);
	/* ��������� ����mac��ַ�ǹ̶��ģ���Ҫ���˵�Դ�������ַ������*/
	if(pcap_compile(pcapHandle,&fcode,"ether proto 43605 && not ether src 34.97.f6.8d.41.45",1,0) < 0)
	{
		pcap_freealldevs(firstdev);
        return ERR_OTHER;
	}
	/* ���ù����� */
	if(pcap_setfilter(pcapHandle,&fcode) < 0)
	{
		pcap_freealldevs(firstdev);
        return ERR_OTHER;
	}
	pcap_freealldevs(firstdev);
	return OK;
}

DLLAPI int CloseADC()
{
	if(pcapHandle != NULL)
		pcap_close(pcapHandle);
	pcapHandle = NULL;
	return OK;
}

DLLAPI int SendData(int len,unsigned char*pData)
{
	unsigned char *buf = NULL;
	int length = len+14;
	int i;
	if(pcapHandle == NULL) return ERR_HANDLE;
	buf = (unsigned char*)malloc(length);
	for(i=0;i<length;i++)
	{
		if(i<6)
			*(buf + i) = macDst[i];//Source mac
		else if(i>5 && i < 12)
			*(buf + i) = macSrc[i - 6];//Destination mac
		else if(i>11 && i<14)
			*(buf + i) = protocal[i - 12];//Protocal
		else
			*(buf + i) = *(pData + i - 14);//Data
	}
	if (pcap_sendpacket(pcapHandle, buf, length) != 0)  return ERR_WINPCAP;
	return OK;
}

DLLAPI int RecvData(int row, int column, unsigned char*pDataI, unsigned char*pDataQ)
{
	unsigned int totalI = 0;
	unsigned int totalQ = 0;
	unsigned int recvcountI = 0;
	unsigned int recvcountQ = 0;
	unsigned int len = row*column;
	int ret;
	short bInit = 0;
	unsigned short counter; //frame count
	unsigned short frameCnt;//recv frame count
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	if(pcapHandle == NULL) return ERR_HANDLE;
	while( totalI < len || totalQ < len)
	{
		ret = pcap_next_ex( pcapHandle, &header, &pkt_data);
		if(ret > 0)
		{
			if(bInit == 0)
			{
				bInit = 1;
				counter = ((*(pkt_data + 14))<<8) + (*(pkt_data + 15));	
			}
			frameCnt = ((*(pkt_data + 14))<<8) + (*(pkt_data + 15));	
			if(frameCnt == counter)
			{
				if(1 == *(pkt_data+16) && totalQ<len)//channel Q
				{
					if(recvcountQ + header->caplen-17 < column)//if recv length is less than column
					{
						memcpy(pDataQ+totalQ+recvcountQ,pkt_data+17,header->caplen-17);
						recvcountQ += (header->caplen-17);
					}
					else//if recv length is more than column
					{
						memcpy(pDataQ+totalQ+recvcountQ,pkt_data+17,column - recvcountQ);
						recvcountQ += (column - recvcountQ);
						totalQ += recvcountQ;
						recvcountQ = 0;
					}
					counter++;
				}
				else if(16 == *(pkt_data+16) && totalI<len)//channel I
				{
					if(recvcountI + header->caplen-17 < column)
					{
						memcpy(pDataI+totalI+recvcountI,pkt_data+17,header->caplen-17);
						recvcountI += (header->caplen-17);
					}
					else//if recv length is more than len
					{
						memcpy(pDataI + totalI + recvcountI,pkt_data+17,column - recvcountI);
						recvcountI += (column - recvcountI);
						totalI += recvcountI;
						recvcountI = 0;
					}
					counter++;
				}
				else	return ERR_CHANNEL;
			}
			else continue;//֡�����������ﲻ���أ���������պ��Գ�ʱ���󷵻�.
		}
		else	return ERR_NODATA;//��ʱ����
	}
	return OK;
}

DLLAPI int RecvDemo(int row,int* pData)
{
	int res;
	struct pcap_pkthdr *header;
	const u_char *pkt_data;
	unsigned short counter;
	unsigned short frameCnt;
	int i;
	if(pcapHandle == NULL) return ERR_HANDLE;
	for(i=0;i<row;i++)
	{
		res = pcap_next_ex( pcapHandle, &header, &pkt_data);
		if(res > 0)
		{		
			if(i == 0)
			{
				counter = ((*(pkt_data + 14))<<8) + (*(pkt_data + 15));	
			}
			frameCnt = ((*(pkt_data + 14))<<8) + (*(pkt_data + 15));
			if(frameCnt == counter)
			{
				if(34 == *(pkt_data+16))
				{
					unsigned char data[8];
					data[0] = *(pkt_data+3+17);
					data[1] = *(pkt_data+2+17);
					data[2] = *(pkt_data+1+17);
					data[3] = *(pkt_data+0+17);
					data[4] = *(pkt_data+7+17);
					data[5] = *(pkt_data+6+17);
					data[6] = *(pkt_data+5+17);
					data[7] = *(pkt_data+4+17);
					memcpy(pData+2*i,data,8);
					counter++;
				}
				else	return ERR_CHANNEL;
			}
		}
		else	return ERR_NODATA;
	}
	return OK;
}

DLLAPI int GetAdapterList(char *list)
{
	pcap_if_t *firstdev;
	pcap_if_t *selectdev;
	char   errbuf[PCAP_ERRBUF_SIZE];
	int pos = 0;
	pcap_findalldevs(&firstdev,errbuf);
	selectdev = firstdev;
	while(selectdev)
	{
		memcpy(list + pos,selectdev->description,strlen(selectdev->description));
		pos = pos + strlen(selectdev->description)+1;
		*(list + pos - 1) = '\n';
		selectdev = selectdev->next;
	}
	 *(list + pos - 1) = 0;
	 return OK;
}

DLLAPI int GetSoftInformation(char *pInformation)
{
	char *strInfo = "USTCADC Driver v1.2 @20170724";
	memcpy(pInformation,strInfo,strlen(strInfo));
	pInformation[strlen(strInfo)] = 0;
	return OK;
}

DLLAPI int GetErrorMsg(int errorcode ,char * strMsg)
{
	if(errorcode & USERDEF)
	{
		char *prefix = "USTCDACDRIVER API failed: ";
		char *info;
		if(errorcode & FACILITY_WINPCAP)
		{
			info = pcap_geterr(pcapHandle);
		}
		else
		{
			switch(errorcode)
			{
				case ERR_NODATA:info = "Reveive data timeout, check the net status.\n";break;
				case ERR_NONETCARD: info = "Do not exist netcard. call GetAdatpterList to get a valid list.\n";break;
				case ERR_CHANNEL: info = "Data channel error, may be the protocal error.\n";break;
				case ERR_OTHER: info = "Other error, the posibility is less than winning a big lottery.\n";break;
				case ERR_HANDLE: info = "Invalid handle, make sure you have opened the device.\n";break;
				default: info = "Are you sure this was caused by USTCADCDriver?\n";
			}
		}
		strcpy_s(strMsg,1024,prefix);
		strcat_s(strMsg,1024,info);
		return OK;
	}
	else
	{
		HLOCAL hlocal = NULL;
		DWORD dwSystemLocale = MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL );  
		BOOL bOk = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,errorcode,dwSystemLocale,(PTSTR)&hlocal, 0, NULL );
		if(bOk && hlocal != NULL)
		{
			char *prefix = "Windows API failed: ";
			strcpy_s(strMsg,1024,prefix);
			strcat_s(strMsg,1024,hlocal);
			LocalFree(hlocal);
			return OK;
		}
		return ERR_OTHER;
	}
}