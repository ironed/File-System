#include <string.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;

char ldisc[64][64];/*this array represents the logical disc, only reading and writing a block at a time is permitted*/

struct OFT
{
	int descriptor;
	int current_pos;
	char buff[64];
	int file_size;
}OFT[4];/*Open file table. Allows only four files to be opened at a time*/

void read_block(int i, char *p)/*Reads contents of block number i ldisc to p*/
{
	if(i>63||i<0)
	{
		cout<<"I/O error!"<<endl;
		return;
	}
	for(int j=0;j<64;j++)
		p[j]=ldisc[i][j];
}

void write_block(int i, char *p)/*Writes contents of p to block number i of ldisc*/
{
	if(i>63||i<0)
	{
		cout<<"I/O error!"<<endl;
		return;
	}
	for(int j=0;j<64;j++)
		ldisc[i][j]=p[j];
}

char *check_substr(char *buffer,char *file_name,int name_len)/*returns true if file_name is present in buffer*/
{
	for(int i=0;i<=64-name_len;i++)
	{
		int j;
		for(j=0;j<name_len;j++)
			if(buffer[i+j]!=file_name[j])
				break;
		if(j==name_len)
			return buffer+i;
	}
	return NULL;
}






int to_int(char *buffer,int b_pos)
{
	int num=buffer[b_pos]<<24;
	num=num | buffer[b_pos+1]<<16;
	num=num | buffer[b_pos+2]<<8;
	num=num | buffer[b_pos+3]<<0;
	return num;

}

char *to_char(char *buffer,int pos,int num)
{
	buffer[pos++]=(num>>24)&0xff;
	buffer[pos++]=(num>>16)&0xff;
	buffer[pos++]=(num>>8 )&0xff;
	buffer[pos++]=(num>>0 )&0xff;
	return buffer;
}
int get_free_descriptor()//correct one
{
	char buffer[64];
	for(int reading_block=1;reading_block<=6;reading_block++)
	{
		read_block(reading_block,buffer);
		for(int pos=0;pos<64;)
		{
			if(buffer[pos]=='-')
				return (64*(reading_block-1)+pos)/16;
			else
				pos+=16;
		}
	}
	return -1;
}
int get_free_block()
{
	char bmap[64];
	read_block(0,bmap);
	for(int pos=7;pos<64;pos++)
	{
		if(bmap[pos]=='-')
		{
			bmap[pos]='1';
			write_block(0,bmap);
			return pos;
		}
	}
	return -1;
}

void create_directory()
{
	int descriptor=get_free_descriptor();//returns 0
	int desc_block=descriptor/4+1;//block number
	int desc_pos=(descriptor%4)*16;
	int block=get_free_block();
	char *buffer=new char[64];
	read_block(desc_block,buffer);
	int size=0;
	buffer=to_char(buffer,desc_pos,size);
	buffer=to_char(buffer,desc_pos+4,block);
	write_block(desc_block,buffer);
	
	for(int i=0;i<64;i++)
		buffer[i]='-';
	write_block(block,buffer);
      
	//  ldisc[0][block]='1';//update used blocks
	OFT[0].descriptor=descriptor;
	read_block(block,OFT[0].buff);//will not be used
	OFT[0].current_pos=0;//will not be used
	OFT[0].file_size=0;
	
}
bool check_filename(char *name,int name_len)
{
    char *directory=new char[64];  
    char *buffer=new char[65];   
    read_block(1,directory);//load directory's block
    int size=4;
    while(size<16&&directory[size]!='-')
    {
		int block_num=to_int(directory,size);
		read_block(block_num,buffer);
		buffer[64]='\0';
		char *name_copy=new char[5];
		int i=0;
		while(i<name_len)
		{
			name_copy[i]=name[i];
			i++;
		}
		while(i<5)
			name_copy[i++]='\0';
		if(check_substr(buffer, name_copy,name_len) != NULL)
		{
			printf("File with this name exists already!\n");
			return false;
		}
		size+=4;   
	}
	return true;
}
void insert_in_directory(char *name,int descriptor,int name_len)
{
    char *directory=new char[64];  
    char *buffer=new char[64];   
    read_block(1,directory);//load directory's block
    int size=4;
	int directory_size=to_int(directory,0);
   // int block_num=1;
    while(size<16&&directory[size]!='-')
    {
        int block_num=to_int(directory,size);
        read_block(block_num,buffer);
        for(int i=0;i<64;i=i+8)
        {
            if(buffer[i]=='-')
            {
				for(int j=0;j<name_len;j++)
					buffer[i+j]=name[j];
				for(int j=name_len;j<4;j++)
					buffer[i+j]='-';
                buffer=to_char(buffer,i+4,descriptor);
                write_block(block_num,buffer);
				to_char(directory,0,directory_size+4);
				write_block(1,directory);
                return;
            }
        }
        size+=4;
    }
    if(directory[size]=='-')
    {
		int block=get_free_block();
		char *new_block_val=new char[64];   
		for(int i=0;i<64;i++)
			new_block_val[i]='-';
		to_char(directory,size,block);
		write_block(1,directory);
		for(int j=0;j<name_len;j++)
			new_block_val[j]=name[j];
		for(int j=name_len;j<4;j++)
			buffer[j]='-';
		new_block_val=to_char(new_block_val,4,descriptor);
		write_block(block,new_block_val);
		to_char(directory,0,directory_size+4);
		write_block(1,directory);
		return;
    }
}
void create(char *name,int name_len)
{
    if(check_filename(name,name_len))//check size also
    {
		int descriptor=get_free_descriptor();
		int block=get_free_block();
		int write_in=descriptor/4+1;//block number of descriptor
		char *buffer=new char[64];
		read_block(write_in,buffer);
		int pos=(descriptor%4)*16;
		int size=0;
		buffer=to_char(buffer,pos,size);
		buffer=to_char(buffer,pos+4,block);
		write_block(write_in,buffer);	//update descriptor with file size and first block number
		
		
		
		//entry in directory
		insert_in_directory(name,descriptor,name_len);
    }
}

int get_descriptor(char *name,char name_len,bool del_file)
{
    char *directory=new char[64]; 
    read_block(1,directory);//load directory's block
    int size=4;
    while(size<16&&directory[size]!='-')
    {
		int block_num=to_int(directory,size); 
		char *buffer=new char[65];   
		read_block(block_num,buffer);
		buffer[64]='\0';
		char *name_copy=new char[5];
		int i=0;
		while(i<name_len)
		{
			name_copy[i]=name[i];
			i++;
		}
		while(i<5)
			name_copy[i++]='\0';
		char *ptr;
		if((ptr=check_substr(buffer, name,name_len)) != NULL)
		{
			int descriptor=to_int(ptr,4);
			if(del_file)
			{
				for(int i=0;i<4;i++)
					if(OFT[i].descriptor==descriptor)
					{
						printf("File open, close it first\n");
						return -1;
					}
				strncpy(ptr,"--------",8);
				write_block(block_num,buffer);
				to_char(directory,0,to_int(directory,0)-4);
				write_block(1,directory);
			}
			return descriptor;
		}
		size+=4;   
	}
	cout<<"File with "<<name<<" name not present"<<endl;
	return -1;
}

void del(char *name,char name_len)//if open show error
{
    int descriptor=get_descriptor(name,name_len,1);
	
	if(descriptor==-1)
		return;
	int desc_block=descriptor/4+1;
	int desc_pos=(descriptor%4)*16;
	char *desc_block_val=new char[64];
	read_block(desc_block,desc_block_val);
	int block_pos=4;
	while(desc_block_val[desc_pos+block_pos]!='-'&&block_pos<16)
	{
		int block_to_free=to_int(desc_block_val,desc_pos+block_pos);
		ldisc[0][block_to_free]='-';
		block_pos+=4;
	}
	strncpy(desc_block_val+desc_pos,"--------",8);
	write_block(desc_block,desc_block_val);
	return;
}
int open(char *name,char name_len)
{
	int desc=get_descriptor(name,name_len,0);
	for(int i=0;i<4;i++)
		if(OFT[i].descriptor==desc)
		{
			printf("File open already\n");
			return -1;
		}
	if(desc==-1)
	{
		cout<<"File opening error"<<endl;
		return -1;
	}
	int i;
	for(i=0;i<4;i++)
		if(OFT[i].descriptor==-1)
			break;
	if(i==4)
	{
		cout<<"4 files open already. Can't open more."<<endl;
		return -1;
	}
	OFT[i].descriptor=desc;
	OFT[i].current_pos=0;
	
	int desc_block=desc/4+1;
	int desc_pos=(desc%4)*16;
	char *desc_block_val=new char[64];
	read_block(desc_block,desc_block_val);//open descriptor to get data block
	int first_block=to_int(desc_block_val,desc_pos+4);
	int file_size=to_int(desc_block_val,desc_pos);
	OFT[i].file_size=file_size;
	
	read_block(first_block,OFT[i].buff);
	//cout<<i<<endl;
	return i;
}

int save(int index)
{
	int current_pos=OFT[index].current_pos;
	int current_block_num=current_pos/64;
	
	int descriptor=OFT[index].descriptor;
	if(descriptor<0)
	{
		printf("File not open!\n");
		return -2;
	}
	int desc_block=descriptor/4+1;
	int desc_pos=(descriptor%4)*16;
	
	char *desc_block_val=new char[64];
	read_block(desc_block,desc_block_val);
			
	to_char(desc_block_val,desc_pos,OFT[index].file_size);//update size in descriptor
	
	int block_to_write=to_int(desc_block_val,desc_pos+4*(current_block_num+1));
			
	write_block(block_to_write,OFT[index].buff);//update block for closed file
	write_block(desc_block,desc_block_val);//update size
	return 0;
}
int close(int index)
{
	if(save(index)>=0)
		OFT[index].descriptor=-1;
	return 1;
}
int read(int index,int count)
{
	if(count<0)
		return -1;
	int descriptor=OFT[index].descriptor;
	if(descriptor<0)
	{
		printf("File not open!\n");
		return -2;
	}
	//if(save(index)<0)
	//	return -1;
	int current_pos=OFT[index].current_pos;
	int file_size=OFT[index].file_size;
	if(current_pos+count>file_size)
		count=file_size-current_pos;
	//int blocks_to_read=(current_pos+count)/64;
	for(int i=1;i<=count;)
	{
		int buff_pos=(current_pos)%64;
		if(buff_pos==0&&current_pos!=0)
		{
			int block_num=current_pos/64;
			
			int desc_block=descriptor/4+1;
			int desc_pos=(descriptor%4)*16;
			
			char *desc_block_val=new char[64];
			read_block(desc_block,desc_block_val);
			
			int block_to_read=to_int(desc_block_val,desc_pos+4*(block_num+1));
			
			read_block(block_to_read,OFT[index].buff);
		}
		printf("%c",OFT[index].buff[buff_pos]);
		i++;
		current_pos++;
	}
	OFT[index].current_pos=current_pos;
	printf("\n");
}

int write(int index,char *str,int count)
{
	int current_pos=OFT[index].current_pos;
	if(current_pos+count>192)
		count=192-current_pos;
	
	int descriptor=OFT[index].descriptor;
	int desc_block=descriptor/4+1;
	int desc_pos=(descriptor%4)*16;
			
	char *desc_block_val=new char[64];
	read_block(desc_block,desc_block_val);

	//int current_size=to_int(desc_block_val,desc_pos);
	for(int i=1;i<=count;)
	{
		int buff_pos=(current_pos)%64;
		if(buff_pos==0&&current_pos!=0)
		{
			int block_num=current_pos/64;
			
			int block_to_write_prev=to_int(desc_block_val,desc_pos+4*(block_num));
			write_block(block_to_write_prev,OFT[index].buff);
			
			int new_block=get_free_block();
			to_char(desc_block_val,desc_pos+(block_num+1)*4,new_block);
			for(int j=0;j<64;j++)
				OFT[index].buff[j]='-';
		}
		OFT[index].buff[buff_pos]=str[i-1];
		i++;
		current_pos++;
	}
	to_char(desc_block_val,desc_pos,current_pos);
	write_block(desc_block,desc_block_val);
	
	OFT[index].file_size=current_pos;

	OFT[index].current_pos=current_pos;
	printf("%d\n",count);
}

int lseek(int index,int pos)
{
	int curr_pos=OFT[index].current_pos;
	int new_pos=pos;
	if(new_pos>=0&&new_pos<OFT[index].file_size)
	{
		int descriptor=OFT[index].descriptor;
		int desc_block=descriptor/4+1;
		int desc_pos=(descriptor%4)*16;

		char *desc_block_val=new char[64];
		read_block(desc_block,desc_block_val);
		
		if(new_pos/64!=curr_pos/64)
		{
			int block_to_write=to_int(desc_block_val,desc_pos+4*(curr_pos/64+1));
			write_block(block_to_write,OFT[index].buff);
			
			int block_to_read=to_int(desc_block_val,desc_pos+4*(new_pos/64+1));
			read_block(block_to_read,OFT[index].buff);
			OFT[index].current_pos=new_pos;
			return 0;
		}
		else
		{
			OFT[index].current_pos=new_pos;
			return 0;
		}
	}
	return -1;
}

void directory()
{
    char *directory=new char[64];  
    char *buffer=new char[65];   
    read_block(1,directory);//load directory's block
    int size=4;
    while(size<16&&directory[size]!='-')
    {
		int block_num=to_int(directory,size);
		read_block(block_num,buffer);
		for(int i=0;i<64;)
		{
			if(buffer[i]!='-')
			{
				int j=0;
				while(buffer[i+j]!='-'&&j<4)
				{
					printf("%c",buffer[i+j]);
					j++;
				}
				cout<<endl;
			}
			i+=8;
		}
		size+=4;   
	}
}

int main()
{
	for(int i=1;i<4;i++)
		OFT[i].descriptor=-1;
	char dash[64];
	for(int i=0;i<64;i++)
            dash[i]='-';
	for(int j=0;j<64;j++)
		write_block(j,dash);
	ldisc[0][0]='1';
	create_directory();

	while(1)
	{
		char cmd[3];
		char arg1[5];
		char arg2[193];
		int arg3;
		scanf("%s",cmd);
		if(cmd[0]=='c'&&cmd[1]=='r')
		{
			scanf("%s",arg1);
			create(arg1,strlen(arg1));
			
		}
		else if(cmd[0]=='d'&&cmd[1]=='e')
		{
			scanf("%s",arg1);
			del(arg1,strlen(arg1));
		}
		else if(cmd[0]=='o'&&cmd[1]=='p')
		{
			scanf("%s",arg1);
			printf("%d\n",open(arg1,strlen(arg1)));
		}
		else if(cmd[0]=='c'&&cmd[1]=='l')
		{
			int index;
			scanf("%d",&index);
			close(index);
		}
		else if(cmd[0]=='r'&&cmd[1]=='d')
		{
			int index,count;
			scanf("%d %d",&index,&count);
			read(index,count);
		}
		else if(cmd[0]=='w'&&cmd[1]=='r')
		{
			int index;
			char str[193];
			scanf("%d %[^\n]%*c",&index,str);
			write(index,str,strlen(str));
		}
		else if(cmd[0]=='s'&&cmd[1]=='k')
		{
			int index,pos;
			scanf("%d %d",&index,&pos);
			lseek(index,pos);
		}
		else if(cmd[0]=='d'&&cmd[1]=='r')
		{
			directory();
		}
		else if(cmd[0]=='i'&&cmd[1]=='n')
		{
			
		}
		else if(cmd[0]=='s'&&cmd[1]=='v')
		{
			
		}
	}
}
