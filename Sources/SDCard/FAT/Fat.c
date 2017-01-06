#include"FAT.h"
//#include"SHOW_HZ_Asc.h"
#include"SD.h"
	

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

uchar FAT32_Buffer[512];
unsigned long DBR_Sector;//��¼DBR��������
ulong Total_Size;//������
ulong FATsectors;//FAT��ռ��������
ulong FirstDirClust;//��Ŀ¼���ڴ�  
ulong BytesPerSector;//ÿ���������ֽ���
ulong SectorsPerClust;//ÿ���ص�������
ulong FirstFATSector;//��һ��FAT���������
ulong FirstDirSector;//��Ŀ¼�������� 
ulong FileFirstCluster;//��̬�����ļ���ʼ�غ�

ulong TargetDirClust;//Ŀ���ļ������ڴ�
ulong TargetDirSector;//Ŀ���ļ���������
uchar success;//�����ɹ���־   0 FAIL  1 �ҵ��ļ�   2 �ҵ�Ŀ¼   


//-------FAT32ϵͳ��ʼ��-------------------------------------------
void FAT32_Init()
{
DBR_Sector=0;//��¼DBR��������
Total_Size=0;//������
FATsectors=0;//FAT��ռ��������
FirstDirClust=0;//��Ŀ¼���ڴ�  
BytesPerSector=0;//ÿ���������ֽ���
SectorsPerClust=0;//ÿ���ص�������
FirstFATSector=0;//��һ��FAT���������
FirstDirSector=0;//��Ŀ¼�������� 
FileFirstCluster=0;//��̬�����ļ���ʼ�غ�
TargetDirClust=0;//Ŀ���ļ������ڴ�
TargetDirSector=0;//Ŀ���ļ���������
success=0;//�ļ������ɹ���־   0 FAIL  1 BMP   2BIN  
FAT32_is_MBR();//��ȡ0�����������û��MBR(��������¼)
Search_DBR();  //��DBR  ��DBR�м������Ҫ����������ʹ��
}

//--------FAT32�ж���������-----------------------------------------
void FAT32_ReadSector(ulong addr,uchar *buf)
{
	SD_Read_Block(addr,buf);
}

//-------FAT32��д�������� ------------------------------------------
void FAT32_WriteSector(ulong addr,uchar *buf)
{
	SD_Write_Block(addr,buf);
}

//---------С��ģʽת��� ----------------------------------------------
ulong LE2BE(uchar *dat,uchar len)// ָ���ֽ����е�ָ�� �ֽ������е��ֽ���
{
	ulong temp=0,fact=1;
	uchar i=0;
	for(i=0;i<len;i++)
	{
		temp+=dat[i]*fact;
		fact*=256;
	}
	return temp;
}

//-----------------Сдת��д-------------------------------------------
char L2U(char c)
{
	if(c>='a'&&c<='z') return c+'A'-'a';
	else return c;
}

//----------��ȡ0�����������û��MBR(��������¼)-------------------------
//------------��⵽MBR  --------------------------- --------------------
//------------δ��⵽MBR   ��⵽DBR  �޸�DBR_Sector --------------
void FAT32_is_MBR()
{
	uchar dat[4],i;
	FAT32_ReadSector(0,FAT32_Buffer);
	if(FAT32_Buffer[0]!=0XEB) 
	{ 
		 if(FAT32_Buffer[446]==0x80) //�жϷ����Ƿ���Ч  ����ֻ֧��һ������
		 {
			 for(i=0;i<4;i++)
		 	 {
			 dat[i]=FAT32_Buffer[454+i];
			 }
			 DBR_Sector=LE2BE(dat,4);
		 }
	}
}
												   
//-----------��DBR  ��DBR�м������Ҫ����������ʹ��-----------------------------
void Search_DBR() 
{
	uchar dat[4],i,dat1[4]={0xC3,0x03,0x00,0x00};
	ulong RsvdSecCnt;//����������
	ulong NumFATs;//FAT����
	FAT32_ReadSector(DBR_Sector,FAT32_Buffer);
	if(FAT32_Buffer[0]==0XEB) 
	{ 		
		for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[32+i];
                          Total_Size=(ulong)(LE2BE(dat,4)/2048);//������  M
		for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[36+i];
                          FATsectors=LE2BE(dat,4);//FAT��ռ��������
		for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[44+i];
                          FirstDirClust=LE2BE(dat,4);//��һ��Ŀ�����ڴ� 
		for(i=0;i<2;i++)  dat[i]=FAT32_Buffer[11+i];
                          BytesPerSector=LE2BE(dat,2);//ÿ���������ֽ���
		                  dat[0]=FAT32_Buffer[13];
                          SectorsPerClust=LE2BE(dat,1);//ÿ���ص�������	
		for(i=0;i<2;i++)  dat[i]=FAT32_Buffer[14+i];
                          RsvdSecCnt=LE2BE(dat,2);//����������						  					  						   
						  FirstFATSector=DBR_Sector+RsvdSecCnt;//��һ��FAT���������
						  dat[0]=FAT32_Buffer[16];
						  NumFATs=LE2BE(dat,1);//FAT����
						  FirstDirSector= FirstFATSector+NumFATs*FATsectors;//��һ��Ŀ¼��������
						  TargetDirClust=FirstDirClust;//Ŀ���ļ������ڴ�
						  TargetDirSector=FirstDirSector;//Ŀ���ļ���������
	}
}

//-----------��ָ��Ŀ¼�£�ȫ����������λĿ���ļ���--------------------------------------------------------- 
void Search_TargetDir(uchar *Dir_name) 
{ 
	uchar i,Cluster_End=0;//��Ŀ¼������־
	ulong Sector,temp=TargetDirSector;
	success=0;
	while(Cluster_End!=1)
	{	for(i=0;i<SectorsPerClust;i++)
		{
			Sector=temp+i;
 			FAT32_ReadSector(Sector,FAT32_Buffer);//������Ŀ¼����
   	  	    Dir_First_Cluster(Dir_name);     //Ѱ���ļ��У���λ�״�
			if(success==2) break; 
		}
		if(success==2) break;
		temp=FAT32_GetNextCluster(TargetDirClust);//��ָ��Ŀ¼����1�ؼ�������Ŀ¼
		if(temp==0x0fffffff) 
			    Cluster_End=1;//������־  
		else
				temp=Cluster_To_Sector(temp);//����Ŀ¼����1�ص������� 
	}
} 

//-----------��Ŀ���ļ���������ȫ������������Ŀ���ļ�--------------------------------------------------------- 
void Read_Target_Sector(uchar *File_name,uchar *Exten_name) 
{ 
	uchar i,Cluster_End=0;//��Ŀ¼������־
	ulong Sector,temp=TargetDirSector;
	while(Cluster_End!=1)
	{	for(i=0;i<SectorsPerClust;i++)
		{
			Sector=temp+i;
 			FAT32_ReadSector(Sector,FAT32_Buffer);//����Ŀ��Ŀ¼��һ������
   	  	    File_First_Cluster(File_name,Exten_name);     //Ѱ���ļ�����λ�״�
			if(success==1) break; 
		}
		if(success==1) break;
		temp=FAT32_GetNextCluster(TargetDirClust);//��Ŀ��Ŀ¼����1�غ�
		if(temp==0x0fffffff) 
			    Cluster_End=1;//������־  
		else
				temp=Cluster_To_Sector(temp);//��Ŀ��Ŀ¼����1�ص������� 
	}
} 

//------��ָ��Ŀ¼����ָ���ļ����͵��ļ���ʼ�غţ�1����16���ļ���¼   ÿ����¼32���ֽ�----------------
void File_First_Cluster(uchar *File_name,uchar *Exten_name)//��֧��8λ�ļ���
{ 
  uchar dat[4];
  uint i;
  uchar j=0;
  for(i=0;i<512;i+=32) 
  {  
	if( 
		FAT32_Buffer[i]   == File_name[0]&& 
		FAT32_Buffer[i+1] == File_name[1]&& 
		FAT32_Buffer[i+2] == File_name[2]&& 
		FAT32_Buffer[i+3] == File_name[3]&& 
		FAT32_Buffer[i+4] == File_name[4]&& 
		FAT32_Buffer[i+5] == File_name[5]&& 
		FAT32_Buffer[i+6] == File_name[6]&& 
		FAT32_Buffer[i+7] == File_name[7]&& 
		FAT32_Buffer[i+8] == Exten_name[0]&& 
		FAT32_Buffer[i+9] == Exten_name[1]&& 
		FAT32_Buffer[i+10]== Exten_name[2]
		)
	{ 
	   dat[j++]=FAT32_Buffer[i+26];
	   dat[j++]=FAT32_Buffer[i+27];
	   dat[j++]=FAT32_Buffer[i+20];
	   dat[j++]=FAT32_Buffer[i+21];
	   FileFirstCluster=LE2BE(dat,4);
	   success=1; //�ҵ�Ŀ���ļ�
	   break;
	} 
  } 
}

void Dir_First_Cluster(uchar *Dir_name)   //��ָ��Ŀ¼��Ѱ��ָ���ļ��У���λ�״�
{									  //��֧��8λ�ļ���
  uchar dat[4];
  uint i;
  uchar j=0;
  for(i=0;i<512;i+=32) 
  {  
	if( 
		FAT32_Buffer[i]   == Dir_name[0]&& 
		FAT32_Buffer[i+1] == Dir_name[1]&& 
		FAT32_Buffer[i+2] == Dir_name[2]&& 
		FAT32_Buffer[i+3] == Dir_name[3]&& 
		FAT32_Buffer[i+4] == Dir_name[4]&& 
		FAT32_Buffer[i+5] == Dir_name[5]&& 
		FAT32_Buffer[i+6] == Dir_name[6]&& 
		FAT32_Buffer[i+7] == Dir_name[7]&& 
		FAT32_Buffer[i+8] == ' '&&     //����չ��
		FAT32_Buffer[i+9] == ' '&& 
		FAT32_Buffer[i+10]== ' '&&
		FAT32_Buffer[i+11]== 16        //����ΪĿ¼
		)
	{ 
	   dat[j++]=FAT32_Buffer[i+26];
	   dat[j++]=FAT32_Buffer[i+27];
	   dat[j++]=FAT32_Buffer[i+20];
	   dat[j++]=FAT32_Buffer[i+21];
	   TargetDirClust=LE2BE(dat,4);//Ŀ���ļ������ڴ�
	   TargetDirSector=Cluster_To_Sector(TargetDirClust);//Ŀ���ļ���������
	   success=2; //�ҵ�Ŀ���ļ���
		break;
	} 
  } 
}

//-------------Ѱ�Һ�̴�---------------------------------------------------------------
ulong FAT32_GetNextCluster(ulong LastCluster)//��ǰ��     ������һ�صĴغ�
{
	ulong temp;
	uchar dat[4],i;
	temp=((LastCluster/128)+FirstFATSector);
	FAT32_ReadSector(temp,FAT32_Buffer);
	for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[(LastCluster%128)*4+i];
                      temp=LE2BE(dat,4);//FAT��ռ��������        
	return temp;
}

//--------------�غ�תΪ������ ��һ����-------------------------------------------------------
ulong Cluster_To_Sector(ulong Cluster)
{
	ulong temp;
	temp=(Cluster-FirstDirClust)*SectorsPerClust+FirstDirSector;
	return temp;
}

//----·��������   ����·����������λҶ��Ŀ¼-----------------------------------------------
void PathProcess(uchar *Path)
{
	uchar i=0,j=0,t=0,temp=0,flag=0;
	uchar Dir_name[8];

	TargetDirSector=FirstDirSector;
	for(t=0;t<8;t++)    Dir_name[t]=' ';
	while(Path[temp]!='\0') temp++;
	 
	for(i=0;i<temp;i++)
	{
		 if(flag==1&&Path[i]!='/'&&Path[i]!='\0')
		 		{
				  Dir_name[j++]=Path[i]; 
				 }
		 if(Path[i]=='/')    flag=0;
		 if(flag==0)
		 		{
					for(t=0;t<8;t++)
					{
						if(Dir_name[t]!=' ')  Search_TargetDir(Dir_name);break;
					} 
					for(t=0;t<8;t++)    Dir_name[t]=' ';
					j=0;
				}
		 if(Path[i]=='/'&&Path[i+1]!='\0')	  flag=1;
	 }

}

//-----------��Ŀ���ļ���������ȫ������������ָ���ļ����͵������ļ�--------------------------------------------------------- 
void Read_Target_Sector_All_File_Search(uchar *Exten_name) 
{ 
	uchar i,Cluster_End=0;//��Ŀ¼������־
	ulong Sector,temp=TargetDirSector;
	while(Cluster_End!=1)
	{	for(i=0;i<SectorsPerClust;i++)
		{
			Sector=temp+i;
 			FAT32_ReadSector(Sector,FAT32_Buffer);//����Ŀ��Ŀ¼��һ������
   	  	    All_File_Search(Exten_name);     //��ָ����չ�����ļ�����¼����  ������
		}
		temp=FAT32_GetNextCluster(TargetDirClust);//��Ŀ��Ŀ¼����1�غ�
		if(temp==0x0fffffff) 
			    Cluster_End=1;//������־  
		else
				temp=Cluster_To_Sector(temp);//��Ŀ��Ŀ¼����1�ص������� 
	}
}

//-----------��ָ��Ŀ¼������ָ�����͵��ļ�--------------------------------------------------------------
void All_File_Search(uchar *Exten_name)	
{
  uchar File_Name[11];//�ļ���
  uchar dat[4];
  uint i;
  uchar j=0,k=0;

  File_Name[8]=0x0d;	//����س� ���㻻��
  File_Name[9]=0x0a;
  File_Name[10]='\0';

  for(i=0;i<512;i+=32) 
  {  
	if( 
		FAT32_Buffer[i+8] == Exten_name[0]&& 
		FAT32_Buffer[i+9] == Exten_name[1]&& 
		FAT32_Buffer[i+10]== Exten_name[2]
		)
	{ 
	for(k=0;k<8;k++)
		{
			File_Name[k]=FAT32_Buffer[i+k];	//��ָ����չ�����ļ�����¼����  ������
		}
    /*	File_Name[0]=FAT32_Buffer[0];
		File_Name[1]=FAT32_Buffer[1];
		File_Name[2]=FAT32_Buffer[2];
		File_Name[3]=FAT32_Buffer[3];
		File_Name[4]=FAT32_Buffer[4];
		File_Name[5]=FAT32_Buffer[5];
		File_Name[6]=FAT32_Buffer[6];
		File_Name[7]=FAT32_Buffer[7];  */
		//P0=0;
	   dat[j++]=FAT32_Buffer[i+26];
	   dat[j++]=FAT32_Buffer[i+27];
	   dat[j++]=FAT32_Buffer[i+20];
	   dat[j++]=FAT32_Buffer[i+21];
	   //FileFirstCluster=LE2BE(dat,4);
	} 
  }
}