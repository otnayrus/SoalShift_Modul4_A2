#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/vinsensius/Documents";

static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
  	int fd = 0 ;
	char src[100],dst[100];

	(void) fi;

	if((strstr(fpath,".txt")!=NULL||strstr(fpath,".pdf")!=NULL||strstr(fpath,".doc")!=NULL)&&strstr(fpath,".ditandai")==NULL){
	system("zenity --error --text='Terjadi kesalahan! File berisi konten berbahaya.'");
	sprintf(src,"%s",fpath);
	sprintf(dst,"%s.ditandai",src);
	int rnm = rename(src,dst);
	if(rnm==-1) return -errno;
	
	DIR *drc;
	char newdir[100];
	sprintf(newdir,"%s/rahasia",dirpath);
	drc = opendir(newdir);
	if (drc==NULL) {
	  mkdir(newdir, S_IRWXU | S_IRWXG | S_IRWXO);
	  closedir(drc);
	  drc = opendir(newdir);
	}
	if(drc){
	  char cmd[100];
	  sprintf(cmd,"mv %s %s",dst,newdir);
	  system(cmd);
	  closedir(drc);
	}  
	
	return -1;
	}

else {
	fd = open(fpath,O_RDONLY);
	if (fd == -1) return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1) res = -errno;

	close(fd);
}
	return res;
}



static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
