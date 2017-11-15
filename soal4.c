#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/vinsensius/Downloads";

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

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
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

static int xmp_truncate(const char *path, off_t size)
{
	int res;
	char newpath[100];
	sprintf(newpath,"%s%s",dirpath,path);
	res = truncate(newpath, size);
	if (res == -1)
	return -errno;
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
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

	(void) fi;

	if(strstr(fpath,".copy")!=NULL){
	  system("zenity --error --text='File yang anda buka adalah file hasil salinan. File tidak bisa diubah maupun disalin kembali!'");
	  int chm = chmod(fpath,000);
	  if(chm == -1) return -errno;
	  return -1;
	}
else {
	fd = open(fpath, O_RDONLY);
	if (fd == -1) return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1) res = -errno;

	close(fd);
}
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	(void) fi;

	//make & check dir
	DIR *drc;
	char newdir[100];
	sprintf(newdir,"%s/simpanan",dirpath);
	drc = opendir(newdir);
	if (drc==NULL) {
	  mkdir(newdir, S_IRWXU | S_IRWXG | S_IRWXO);
	  closedir(drc);
	  drc = opendir(newdir);
	}
	if(drc){
	  char cmd[100];
	  sprintf(cmd,"cp %s%s %s%s", dirpath, path, newdir, path);
	  system(cmd);
	  closedir(drc);
	}
	else return -errno;  	

	char newpath[100];
	sprintf(newpath,"%s%s",newdir,path);

	fd = open(newpath, O_WRONLY);
	if (fd == -1)
		return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;
	close(fd);
	
	char cppath[100];
	sprintf(cppath,"%s.copy",newpath);
	int rnm = rename(newpath,cppath);
	if (rnm==-1) return -errno;

	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
	.truncate	= xmp_truncate,
	.write		= xmp_write,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
