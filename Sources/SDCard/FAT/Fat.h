#include"SD.h"
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long

extern ulong    DBR_Sector;//记录DBR的扇区号
extern ulong	Total_Size;//总容量
extern ulong	FATsectors;//FAT表占用扇区数
extern ulong	FirstDirClust;//根目录所在簇  
extern ulong	BytesPerSector;//每个扇区的字节数
extern ulong	SectorsPerClust;//每个簇的扇区数
extern ulong	FirstFATSector;//第一个FAT表的扇区号
extern ulong	FirstDirSector;//根目录的扇区号
 
extern ulong    FileFirstCluster;//次态操作文件起始簇号

extern ulong    TargetDirClust;//目标文件夹所在簇
extern ulong    TargetDirSector;//目标文件夹扇区号

extern uchar    success;//搜索成功标志   0 FAIL  1 找到文件   2 找到目录 




void FAT32_Init();//FAT32系统初始化
void Search_DBR();//读DBR  在DBR中计算出重要参数供后续使用
void FAT32_is_MBR();//读取0扇区，检测有没有MBR(主引导记录)
void PathProcess(uchar *path);//路径处理函数   根据路径搜索并定位叶子目录

void Search_TargetDir(uchar *Dir_name);//在指定目录下（全部搜索）定位目标文件夹 
void Read_Target_Sector(uchar *File_name,uchar *Exten_name);//读目标文件夹扇区（全部搜索）搜索目标文件

void File_First_Cluster(uchar *File_name,uchar *Exten_name);//在指定目录读指定文件  定位首簇
void Dir_First_Cluster(uchar *Dir_name);   //在指定目录下寻找指定文件夹，定位首簇

ulong Cluster_To_Sector(ulong Cluster);//簇号转为扇区号 第一扇区

ulong FAT32_GetNextCluster(ulong LastCluster);//寻找后继簇
ulong LE2BE(uchar *dat,uchar len);//小端模式转大端

void FAT32_WriteSector(ulong addr,uchar *buf);//FAT32中写扇区函数
void FAT32_ReadSector(ulong addr,uchar *buf);//FAT32中读扇区函数

void All_File_Search(uchar *Exten_name);
void Read_Target_Sector_All_File_Search(uchar *Exten_name);