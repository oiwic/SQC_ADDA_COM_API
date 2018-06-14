#ifndef ADCDRIVER_H
#define ADCDRIVER_H

#ifdef DLLAPI
#else
#define DLLAPI __declspec(dllimport)
#endif

#define MAX_ADCNUM 16

/* ��ADC�豸����Ҫ�ṩĿ��ADC��MAC��ַ��Э�����ͣ��������豸�� */
DLLAPI int OpenADC(int*id,char *macSrc,char *macDst);
/* �ر�ADC�豸 */
DLLAPI int CloseADC(int id);
/* ��ADCд������ */
DLLAPI int SendData(int id, int len, unsigned char *pData);
/* ��ADC�������� */
DLLAPI int RecvData(int id, int len, int column, unsigned char *pDataI, unsigned char *pDataQ);
/* ��ADC���ؽ�ģ���� */
DLLAPI int RecvDemo(int id, int row, int* pData);
/* ��ȡ�����������ַ */
DLLAPI int GetMacAddress(int id,int isDst,unsigned char* pMac);
/* ���ش�����Ϣ */
DLLAPI int GetErrorMsg(int id,int errorcode,char *strMsg);
/* ��ȡ�汾��Ϣ */
DLLAPI int GetSoftInformation(char *pInformation);
#endif