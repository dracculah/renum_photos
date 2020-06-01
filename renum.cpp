#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

string ExtractFileName(string str)
{
	int last=-1;
	for (int i=0;i<str.length();i++)
		if (str[i] == '/')
			last = i;
	if (last<0) return "";
	return str.substr(last,str.length()-last);
}

string ExtractFileExt(string str)
{
	string fn = ExtractFileName(str);
	int last=-1;
	for (int i=0;i<str.length();i++)
		if (str[i] == '.')
			last = i;
	if (last<0) return "";
	return str.substr(last,str.length()-last);
}

int getdir (string dir, list<string> &files, vector<string> filter)
{
    DIR *dp;
    struct dirent *dirp;
	if (dir[dir.size()-1] != '/')
		dir += "/";
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
		if (dirp->d_name[0] != '.')
		{
			if (filter.size() == 0)
				files.push_back(string(dirp->d_name));
			else
			{
				string fn = string(dirp->d_name);
				for (vector<string>::iterator it=filter.begin();it!=filter.end();it++)
				{
					string _ext = *it;
					if (ExtractFileExt(fn) == _ext)
					{
						files.push_back(fn);
						break; // avoid adding the same file twice accidentally (would happen if you do -f twice with the same ext)
					}
				}
			}
		}
    }
    closedir(dp);
    return 0;
}

void printUsage()
{
	printf("renum directory (options)\n");
	printf("all files found within given directory will be used unless you give an extension filter.\n");
	printf("It does not search directories recursively (for a reason).\n");
	printf("The new file names will be created like this: prefix + number + extension\n");
	printf("Options:\n");
	printf("\t-f extension -> filter the files with this extension (for existing filenames) [default is all files]\n");
	printf("\t\t multiple extensions might be used like that: -f ext1 -f ext2 ...\n");
	printf("\t-e extension -> change extension of new filenames (format is .ext) [default is .jpg]\n");
	printf("\t-p prefix -> prefix of new filenames [default is \"image_\"]\n");
	printf("\t-c number -> start counting from this number (for new filenames) [default is 1]\n");
	printf("\t-d -> do it. PLEASE make ABSOLUTELY sure you know what you're doing before using this [without -d -> it is pretend mode]\n");
	printf("\t\t please note that -d does unrevertable changes, please make sure you have run this exact setup\n");
	printf("\t\t without -d before and you are happy with the outcome and accept the risks.\n");
	printf("\t\t The program does not check about overlapping filenames in any manner -> DATA LOSS might happen!\n");
	printf("\t\t <<---  I am NOT responsible for any damage whatsoever  --->>\n");	
	printf("\t\t !!! --- Having a BACKUP is HIGHLY ADVISED! Use it on YOUR OWN RISK! --- !!!\n");
}

void doRenum(string _dir, vector<string> filter, const char *prefix = NULL, const char* ext = NULL, int start_cnt_from = 1, bool do_it = false)
{
	if (do_it)
		printf("Running in DO_IT MODE! All changes listed will actually happen!\n");
	list<string> fileList;
	getdir(_dir.c_str(), fileList, filter);
	printf("INFO: found %d files\n", fileList.size());
	if (fileList.size() >= 1000*1000)
	{
		printf("OOPS! found >= 1.000.000 files, can't proceed with the current numbering format, sorry. Aborted\n");
		// if you really need this -> increase the limit here and change the lines "char buff[6+1]; sprintf(buff, "%06d", cnt);" accordingly
		return;
	}
	fileList.sort();
    printf("INFO: files sorted\n");
	int cnt=start_cnt_from;
	for (list<string>::iterator fi=fileList.begin();fi!=fileList.end();fi++)
	{

        if (fi==fileList.end())
            break;

		string old_name = _dir;
		old_name += (*fi);
		string new_name = _dir;
		string s_prefix = prefix==NULL?"image_":prefix;
		new_name += s_prefix;

		char buff[6+1]; // +1 -> just in case it needs a terminating zero at the end
		sprintf(buff, "%06d", cnt);
		string digits = buff;

		string s_ext = ext==NULL?".jpg":ext;
		new_name += digits + s_ext;

		printf("rename: %s -> %s\n",old_name.c_str(),new_name.c_str());

		if (do_it)
		{
			if (rename(old_name.c_str(),new_name.c_str()) != 0)
			{
				printf("error renaming file: %s -> %s\n",old_name.c_str(),new_name.c_str());
				printf("Aborted!\n");
				return;
			}
		}

		cnt++;
	}
}

int main(int argc, char** argv)
{
	string s_dir;
	vector<string> v_filter; // can have 0, 1 or multiple filters
	char* c_ext = NULL;
	char* c_prefix = NULL;
	int start_from = 1;
	bool do_it = false;
	//
	const char* p_f = "-f";
	const char* p_e = "-e";
	const char* p_p = "-p";
	const char* p_c = "-c";
	const char* p_d = "-d";
	//
	if (argc == 1)
	{
		printUsage();
		return 0;
	}
	if (argc > 1)
	{	
		s_dir = argv[1];
		if ((s_dir == ".") || (s_dir == "./"))
		{
			printf("It is dangerous to use current directory, you might overwrite the program file! Aborted\n");
			return 0;
		}
		if (s_dir[s_dir.size()-1] != '/')
			s_dir += "/";
		int i = 2;
		while (i < argc)
		{
			if (!memcmp(argv[i], p_f, strlen(p_f)))
			{
				if (argc == i+1)
				{
					printUsage();
					return 0;
				}
				string s_filter = argv[i+1];
				v_filter.push_back(s_filter);
			}
			if (!memcmp(argv[i], p_e, strlen(p_e)))
			{
				if (argc == i+1)
				{
					printUsage();
					return 0;
				}
				c_ext = argv[i+1];
			}
			if (!memcmp(argv[i], p_p, strlen(p_p)))
			{
				if (argc == i+1)
				{
					printUsage();
					return 0;
				}
				c_prefix = argv[i+1];
			}
			if (!memcmp(argv[i], p_c, strlen(p_c)))
			{
				if (argc == i+1)
				{
					printUsage();
					return 0;
				}
				char *c_num = argv[i+1];
				start_from = atoi(c_num);
			}
			if (!memcmp(argv[i], p_d, strlen(p_d)))
				do_it = true;
			i+=2;
		}
		doRenum(s_dir, v_filter, c_prefix, c_ext, start_from, do_it);
	}
	return 0;
}
