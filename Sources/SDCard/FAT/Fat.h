#include"SD.h"
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

extern ulong    DBR_Sector;//��¼DBR��������
extern ulong	Total_Size;//������
extern ulong	FATsectors;//FAT��ռ��������
extern ulong	FirstDirClust;//��Ŀ¼���ڴ�  
extern ulong	BytesPerSector;//ÿ���������ֽ���
extern ulong	SectorsPerClust;//ÿ���ص�������
extern ulong	FirstFATSector;//��һ��FAT���������
extern ulong	FirstDirSector;//��Ŀ¼��������
 
extern ulong    FileFirstCluster;//��̬�����ļ���ʼ�غ�

extern ulong    TargetDirClust;//Ŀ���ļ������ڴ�
extern ulong    TargetDirSector;//Ŀ���ļ���������

extern uchar    success;//�����ɹ���־   0 FAIL  1 �ҵ��ļ�   2 �ҵ�Ŀ¼ 




void FAT32_Init();//FAT32ϵͳ��ʼ��
void Search_DBR();//��DBR  ��DBR�м������Ҫ����������ʹ��
void FAT32_is_MBR();//��ȡ0�����������û��MBR(��������¼)
void PathProcess(uchar *path);//·��������   ����·����������λҶ��Ŀ¼

void Search_TargetDir(uchar *Dir_name);//��ָ��Ŀ¼�£�ȫ����������λĿ���ļ��� 
void Read_Target_Sector(uchar *File_name,uchar *Exten_name);//��Ŀ���ļ���������ȫ������������Ŀ���ļ�

void File_First_Cluster(uchar *File_name,uchar *Exten_name);//��ָ��Ŀ¼��ָ���ļ�  ��λ�״�
void Dir_First_Cluster(uchar *Dir_name);   //��ָ��Ŀ¼��Ѱ��ָ���ļ��У���λ�״�

ulong Cluster_To_Sector(ulong Cluster);//�غ�תΪ������ ��һ����

ulong FAT32_GetNextCluster(ulong LastCluster);//Ѱ�Һ�̴�
ulong LE2BE(uchar *dat,uchar len);//С��ģʽת���

void FAT32_WriteSector(ulong addr,uchar *buf);//FAT32��д��������
void FAT32_ReadSector(ulong addr,uchar *buf);//FAT32�ж���������

void All_File_Search(uchar *Exten_name);
void Read_Target_Sector_All_File_Search(uchar *Exten_name);