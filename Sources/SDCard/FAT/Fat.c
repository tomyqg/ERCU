#include"FAT.h"
//#include"SHOW_HZ_Asc.h"
#include"SD.h"
	

#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

uchar FAT32_Buffer[512];
unsigned long DBR_Sector;//记录DBR的扇区号
ulong Total_Size;//总容量
ulong FATsectors;//FAT表占用扇区数
ulong FirstDirClust;//根目录所在簇  
ulong BytesPerSector;//每个扇区的字节数
ulong SectorsPerClust;//每个簇的扇区数
ulong FirstFATSector;//第一个FAT表的扇区号
ulong FirstDirSector;//根目录的扇区号 
ulong FileFirstCluster;//次态操作文件起始簇号

ulong TargetDirClust;//目标文件夹所在簇
ulong TargetDirSector;//目标文件夹扇区号
uchar success;//搜索成功标志   0 FAIL  1 找到文件   2 找到目录   


//-------FAT32系统初始化-------------------------------------------
void FAT32_Init()
{
DBR_Sector=0;//记录DBR的扇区号
Total_Size=0;//总容量
FATsectors=0;//FAT表占用扇区数
FirstDirClust=0;//根目录所在簇  
BytesPerSector=0;//每个扇区的字节数
SectorsPerClust=0;//每个簇的扇区数
FirstFATSector=0;//第一个FAT表的扇区号
FirstDirSector=0;//根目录的扇区号 
FileFirstCluster=0;//次态操作文件起始簇号
TargetDirClust=0;//目标文件夹所在簇
TargetDirSector=0;//目标文件夹扇区号
success=0;//文件搜索成功标志   0 FAIL  1 BMP   2BIN  
FAT32_is_MBR();//读取0扇区，检测有没有MBR(主引导记录)
Search_DBR();  //读DBR  在DBR中计算出重要参数供后续使用
}

//--------FAT32中读扇区函数-----------------------------------------
void FAT32_ReadSector(ulong addr,uchar *buf)
{
	SD_Read_Block(addr,buf);
}

//-------FAT32中写扇区函数 ------------------------------------------
void FAT32_WriteSector(ulong addr,uchar *buf)
{
	SD_Write_Block(addr,buf);
}

//---------小端模式转大端 ----------------------------------------------
ulong LE2BE(uchar *dat,uchar len)// 指向字节序列的指针 字节序列中的字节数
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

//-----------------小写转大写-------------------------------------------
char L2U(char c)
{
	if(c>='a'&&c<='z') return c+'A'-'a';
	else return c;
}

//----------读取0扇区，检测有没有MBR(主引导记录)-------------------------
//------------检测到MBR  --------------------------- --------------------
//------------未检测到MBR   检测到DBR  修改DBR_Sector --------------
void FAT32_is_MBR()
{
	uchar dat[4],i;
	FAT32_ReadSector(0,FAT32_Buffer);
	if(FAT32_Buffer[0]!=0XEB) 
	{ 
		 if(FAT32_Buffer[446]==0x80) //判断分区是否有效  这里只支持一个分区
		 {
			 for(i=0;i<4;i++)
		 	 {
			 dat[i]=FAT32_Buffer[454+i];
			 }
			 DBR_Sector=LE2BE(dat,4);
		 }
	}
}
												   
//-----------读DBR  在DBR中计算出重要参数供后续使用-----------------------------
void Search_DBR() 
{
	uchar dat[4],i,dat1[4]={0xC3,0x03,0x00,0x00};
	ulong RsvdSecCnt;//保留扇区数
	ulong NumFATs;//FAT表数
	FAT32_ReadSector(DBR_Sector,FAT32_Buffer);
	if(FAT32_Buffer[0]==0XEB) 
	{ 		
		for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[32+i];
                          Total_Size=(ulong)(LE2BE(dat,4)/2048);//总容量  M
		for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[36+i];
                          FATsectors=LE2BE(dat,4);//FAT表占用扇区数
		for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[44+i];
                          FirstDirClust=LE2BE(dat,4);//第一个目标所在簇 
		for(i=0;i<2;i++)  dat[i]=FAT32_Buffer[11+i];
                          BytesPerSector=LE2BE(dat,2);//每个扇区的字节数
		                  dat[0]=FAT32_Buffer[13];
                          SectorsPerClust=LE2BE(dat,1);//每个簇的扇区数	
		for(i=0;i<2;i++)  dat[i]=FAT32_Buffer[14+i];
                          RsvdSecCnt=LE2BE(dat,2);//保留扇区数						  					  						   
						  FirstFATSector=DBR_Sector+RsvdSecCnt;//第一个FAT表的扇区号
						  dat[0]=FAT32_Buffer[16];
						  NumFATs=LE2BE(dat,1);//FAT表数
						  FirstDirSector= FirstFATSector+NumFATs*FATsectors;//第一个目录的扇区号
						  TargetDirClust=FirstDirClust;//目标文件夹所在簇
						  TargetDirSector=FirstDirSector;//目标文件夹扇区号
	}
}

//-----------在指定目录下（全部搜索）定位目标文件夹--------------------------------------------------------- 
void Search_TargetDir(uchar *Dir_name) 
{ 
	uchar i,Cluster_End=0;//根目录结束标志
	ulong Sector,temp=TargetDirSector;
	success=0;
	while(Cluster_End!=1)
	{	for(i=0;i<SectorsPerClust;i++)
		{
			Sector=temp+i;
 			FAT32_ReadSector(Sector,FAT32_Buffer);//读出根目录扇区
   	  	    Dir_First_Cluster(Dir_name);     //寻找文件夹，定位首簇
			if(success==2) break; 
		}
		if(success==2) break;
		temp=FAT32_GetNextCluster(TargetDirClust);//读指定目录的下1簇继续搜索目录
		if(temp==0x0fffffff) 
			    Cluster_End=1;//结束标志  
		else
				temp=Cluster_To_Sector(temp);//读根目录的下1簇的扇区号 
	}
} 

//-----------读目标文件夹扇区（全部搜索）搜索目标文件--------------------------------------------------------- 
void Read_Target_Sector(uchar *File_name,uchar *Exten_name) 
{ 
	uchar i,Cluster_End=0;//根目录结束标志
	ulong Sector,temp=TargetDirSector;
	while(Cluster_End!=1)
	{	for(i=0;i<SectorsPerClust;i++)
		{
			Sector=temp+i;
 			FAT32_ReadSector(Sector,FAT32_Buffer);//读出目标目录第一个扇区
   	  	    File_First_Cluster(File_name,Exten_name);     //寻找文件，定位首簇
			if(success==1) break; 
		}
		if(success==1) break;
		temp=FAT32_GetNextCluster(TargetDirClust);//读目标目录的下1簇号
		if(temp==0x0fffffff) 
			    Cluster_End=1;//结束标志  
		else
				temp=Cluster_To_Sector(temp);//读目标目录的下1簇的扇区号 
	}
} 

//------在指定目录，读指定文件类型的文件起始簇号，1扇区16条文件记录   每条记录32的字节----------------
void File_First_Cluster(uchar *File_name,uchar *Exten_name)//仅支持8位文件名
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
	   success=1; //找到目标文件
	   break;
	} 
  } 
}

void Dir_First_Cluster(uchar *Dir_name)   //在指定目录下寻找指定文件夹，定位首簇
{									  //仅支持8位文件名
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
		FAT32_Buffer[i+8] == ' '&&     //无扩展名
		FAT32_Buffer[i+9] == ' '&& 
		FAT32_Buffer[i+10]== ' '&&
		FAT32_Buffer[i+11]== 16        //属性为目录
		)
	{ 
	   dat[j++]=FAT32_Buffer[i+26];
	   dat[j++]=FAT32_Buffer[i+27];
	   dat[j++]=FAT32_Buffer[i+20];
	   dat[j++]=FAT32_Buffer[i+21];
	   TargetDirClust=LE2BE(dat,4);//目标文件夹所在簇
	   TargetDirSector=Cluster_To_Sector(TargetDirClust);//目标文件夹扇区号
	   success=2; //找到目标文件夹
		break;
	} 
  } 
}

//-------------寻找后继簇---------------------------------------------------------------
ulong FAT32_GetNextCluster(ulong LastCluster)//当前簇     返回下一簇的簇号
{
	ulong temp;
	uchar dat[4],i;
	temp=((LastCluster/128)+FirstFATSector);
	FAT32_ReadSector(temp,FAT32_Buffer);
	for(i=0;i<4;i++)  dat[i]=FAT32_Buffer[(LastCluster%128)*4+i];
                      temp=LE2BE(dat,4);//FAT表占用扇区数        
	return temp;
}

//--------------簇号转为扇区号 第一扇区-------------------------------------------------------
ulong Cluster_To_Sector(ulong Cluster)
{
	ulong temp;
	temp=(Cluster-FirstDirClust)*SectorsPerClust+FirstDirSector;
	return temp;
}

//----路径处理函数   根据路径搜索并定位叶子目录-----------------------------------------------
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

//-----------读目标文件夹扇区（全部搜索）搜索指定文件类型的所有文件--------------------------------------------------------- 
void Read_Target_Sector_All_File_Search(uchar *Exten_name) 
{ 
	uchar i,Cluster_End=0;//根目录结束标志
	ulong Sector,temp=TargetDirSector;
	while(Cluster_End!=1)
	{	for(i=0;i<SectorsPerClust;i++)
		{
			Sector=temp+i;
 			FAT32_ReadSector(Sector,FAT32_Buffer);//读出目标目录第一个扇区
   	  	    All_File_Search(Exten_name);     //将指定扩展名的文件名记录下来  并送显
		}
		temp=FAT32_GetNextCluster(TargetDirClust);//读目标目录的下1簇号
		if(temp==0x0fffffff) 
			    Cluster_End=1;//结束标志  
		else
				temp=Cluster_To_Sector(temp);//读目标目录的下1簇的扇区号 
	}
}

//-----------在指定目录下搜索指定类型的文件--------------------------------------------------------------
void All_File_Search(uchar *Exten_name)	
{
  uchar File_Name[11];//文件名
  uchar dat[4];
  uint i;
  uchar j=0,k=0;

  File_Name[8]=0x0d;	//加入回车 方便换行
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
			File_Name[k]=FAT32_Buffer[i+k];	//将指定扩展名的文件名记录下来  并送显
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