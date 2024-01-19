#include <string>
#include <iostream>
#include <cstring>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <vector>
#include <stack>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <sstream>
#include <fstream>
using namespace std;

enum KEYS
{
    ENTER = 13,
    UP = 65,
    DOWN = 66,
    LEFT = 68,
    RIGHT = 67,
    BACK_SPACE = 127,
    HOME = 104,
    HOME1 = 72,
    COLON = 58,
    ESC = 27,
    q = 113,
    Q = 81
};
class FileList
{
private:
    char *file_name;
    string name;
    long int file_size;
    string last_modified;
    string dir_name;
    char *user;
    char *group;
    string permissions;

public:
    void set_user(char *user)
    {
        if (user)
            this->user = user;
    }
    char *get_user()
    {
        return this->user;
    }
    void set_group(char *group)
    {
        if (group)
            this->group = group;
    }
    char *get_group()
    {
        return this->group;
    }
    void set_permissions(string permissions)
    {
        this->permissions = permissions;
    }
    string get_permissions()
    {
        return this->permissions;
    }
    void set_file_name(char *file_name)
    {
        if (file_name)
            this->file_name = file_name;
    }

    char *get_file_name()
    {
        return this->file_name;
    }
    void set_name(char *file_name)
    {
        if (file_name)
            this->name = file_name;
    }

    string get_name()
    {
        return this->name;
    }
    void set_last_modified(string last_modified)
    {

        this->last_modified = last_modified;
    }

    string get_last_modified()
    {
        return this->last_modified;
    }
    void set_dir_name(string dir_name)
    {
        this->dir_name = dir_name;
    }
    string get_dir_name()
    {
        return this->dir_name;
    }
    void set_file_size(long int file_size)
    {
        this->file_size = file_size;
    }
    long int get_file_size()
    {
        return this->file_size;
    }
};
class Config
{
public:
    int x, y;
    int numrows;
    int rowoff;
    vector<FileList> f;
    char *curr_dir;
    char *parent;
    struct termios orig_termios;
    int row;
    int col;
};
struct abuf
{
    char *b;
    int len;
};
#define ABUF_INIT \
    {             \
        NULL, 0   \
    }
Config config;
void abAppend(struct abuf *ab, const char *s, int len)
{
    char *newstruct = (char *)realloc(ab->b, ab->len + len);
    if (newstruct == NULL)
        return;
    memcpy(&newstruct[ab->len], s, len);
    ab->b = newstruct;
    ab->len += len;
}
void abFree(struct abuf *ab)
{
    free(ab->b);
}
bool compareFileList(FileList i1, FileList i2)
{
    return (i1.get_name() < i2.get_name());
}
void get_individ_permission(struct stat fileStat, string &s)
{
    (S_ISDIR(fileStat.st_mode)) ? s.append(1, 'd') : s.append(1, '-');
    (fileStat.st_mode & S_IRUSR) ? s.append(1, 'r') : s.append(1, '-');
    (fileStat.st_mode & S_IWUSR) ? s.append(1, 'w') : s.append(1, '-');
    (fileStat.st_mode & S_IXUSR) ? s.append(1, 'x') : s.append(1, '-');
    (fileStat.st_mode & S_IRGRP) ? s.append(1, 'r') : s.append(1, '-');
    (fileStat.st_mode & S_IWGRP) ? s.append(1, 'w') : s.append(1, '-');
    (fileStat.st_mode & S_IXGRP) ? s.append(1, 'x') : s.append(1, '-');
    (fileStat.st_mode & S_IROTH) ? s.append(1, 'r') : s.append(1, '-');
    (fileStat.st_mode & S_IWOTH) ? s.append(1, 'w') : s.append(1, '-');
    (fileStat.st_mode & S_IXOTH) ? s.append(1, 'x') : s.append(1, '-');
}
FileList create_file(string dir_name, char *file_name)
{
    FileList f;
    const char *dir = dir_name.c_str();

    struct stat fileStat;
    string full_name = dir_name + "/" + file_name;
    const char *ch = &full_name[0];
    if (stat(ch, &fileStat) < 0)
        return f;

    string permission;
    struct passwd *pw = getpwuid(fileStat.st_uid);
    struct group *gr = getgrgid(fileStat.st_gid);

    get_individ_permission(fileStat, permission);
    f.set_dir_name(dir_name);
    f.set_file_name(file_name);
    f.set_name(file_name);
    f.set_permissions(permission);
    f.set_file_size(fileStat.st_size);
    f.set_group(gr->gr_name);
    f.set_user(pw->pw_name);
    f.set_last_modified(ctime(&fileStat.st_mtime));
    return f;
}
bool search_dir(string search, string dir_name)
{
    const char *ch = &dir_name[0];
    DIR *dir = opendir(ch);
    if (dir)
    {
        dirent *entity = readdir(dir);
        while (entity)
        {
            if (strcmp(entity->d_name, search.c_str()) == 0)
            {
                return 1;
            }
            if (entity->d_type == DT_DIR && strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0)
                if (search_dir(search, dir_name + "/" + entity->d_name))
                {
                    return 1;
                }
            entity = readdir(dir);
        }
    }
    return 0;
}
bool copy_dir(string source_path, string dest_path, string parent)
{
    chdir(parent.c_str());
    struct stat f1;

    stat(source_path.c_str(), &f1);
    if (mkdir(dest_path.c_str(), f1.st_mode) == -1)
    {
        return 0;
    }
    DIR *dir = opendir(source_path.c_str());
    if (dir)
    {
        dirent *entity = readdir(dir);
        while (entity)
        {
            struct stat file;
            string old_path = source_path + "/" + entity->d_name;
            stat(old_path.c_str(), &file);
            string new_path = dest_path + "/" + entity->d_name;
            if (entity->d_type == DT_DIR)
            {
                if (strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0)
                {

                    if (copy_dir(old_path, new_path, source_path) == 0)
                    {
                        return 0;
                    };
                }
            }
            else
            {
                ifstream src;
                ofstream dst;
                src.open(old_path, ios::in | ios::binary);
                dst.open(new_path, ios::out | ios::binary);
                dst << src.rdbuf();
                if (chmod(new_path.c_str(), file.st_mode) == -1)
                {
                    return 0;
                }
                src.close();
                dst.close();
            }
            entity = readdir(dir);
        }
    }
    return 1;
}
bool delete_dir(string source_path, stack<string> &dirs)
{   
    dirs.push(source_path);
    DIR *dir = opendir(source_path.c_str());
    if (dir)
    {
        dirent *entity = readdir(dir);
        while (entity)
        {
            struct stat file;
            string old_path = source_path + "/" + entity->d_name;
            stat(old_path.c_str(), &file);
            if (entity->d_type == DT_DIR)
            {
                if (strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0)
                {

                    if (delete_dir(old_path, dirs) == 0)
                    {
                        return 0;
                    }
                    // remove(old_path);
                }
            }
            else
            {
                if (remove(old_path.c_str()) == -1)
                {
                    return 0;
                };
            }
            entity = readdir(dir);
        }
    }
    return 1;
}
vector<FileList> print_dir(string dir_name)
{
    const char *ch = &dir_name[0];
    vector<FileList> directory;
    vector<FileList> files;
    vector<FileList> l;
    DIR *dir = opendir(ch);
    if (dir)
    {
        dirent *entity = readdir(dir);
        while (entity)
        {
            FileList f = create_file(dir_name, entity->d_name);
            ;
            if (entity->d_type == DT_DIR)
                directory.push_back(f);
            else
                files.push_back(f);
            entity = readdir(dir);
        }
        sort(directory.begin(), directory.end(), compareFileList);
        sort(files.begin(), files.end(), compareFileList);

        for (auto i : directory)
        {
            l.push_back(i);
        }
        for (auto i : files)
        {
            l.push_back(i);
        }
        closedir(dir);
    }
    return l;
}
void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &config.orig_termios);
}
void enableRawMode()
{

    tcgetattr(STDIN_FILENO, &config.orig_termios);

    atexit(disableRawMode); // exit hook
    struct termios raw = config.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
void clear_screen()
{

    write(STDOUT_FILENO, "\x1b[H", 3);
    write(STDOUT_FILENO, "\x1b[2J", 4);
}
void readKey(char *ch)
{

    read(STDIN_FILENO, ch, 3);
}
void editorScroll()
{
    if (config.y < config.rowoff)
    {
        config.rowoff = config.y;
    }
    if (config.y >= config.rowoff + config.row)
    {
        config.rowoff = config.y - config.row + 1;
    }
}
void editorDraw(struct abuf *ab, vector<FileList> &f, bool *normal)
{
    int y;
    string dir = f[0].get_dir_name();
   
    // if(f.size()>config.row) config.row=f.size()+1;
    for (y = 0; y < config.row; y++)
    {    write(STDOUT_FILENO, "\x1b[2K", 4);
        int file_row = y + config.rowoff;
        if (file_row < f.size() && y < config.row - 1)
        {
            char row[1000];
            const char *permission = f[file_row].get_permissions().c_str();
            const char *user = f[file_row].get_user();
            const char *group = f[file_row].get_group();
            double size = f[file_row].get_file_size();

            const char *file_name = f[file_row].get_file_name();
            char ch;
            if(size>=1024){
                ch='K';
                size/=1024;
            }
            if(size>=1024){
                ch='M';
                size/=1024;
            }
            if(size>=1024){
                ch='G';
                size/=1024;
            }
            if(size>=1024){
                ch='T';
                size/=1024;
            }
            int rowlen;
            if(f[file_row].get_permissions()[0]=='d')
                rowlen = snprintf(row, sizeof(row), "\x1b[1m\x1b[34m%-25.20s\x1b[0m\t%s\t%5s\t%5.1lf%cB\t\t%.24s\t%s", f[file_row].get_file_name(), f[file_row].get_user(), f[file_row].get_group(), size,ch, f[file_row].get_last_modified().c_str(), f[file_row].get_permissions().c_str());
            else
                rowlen = snprintf(row, sizeof(row), "\x1b[1m\x1b[32m%-25.20s\x1b[0m\t%s\t%5s\t%5.1lf%cB\t\t%.24s\t%s", f[file_row].get_file_name(), f[file_row].get_user(), f[file_row].get_group(), size,ch, f[file_row].get_last_modified().c_str(), f[file_row].get_permissions().c_str());

            if (rowlen > config.col - 20)
                rowlen = config.col - 20;
            write(STDOUT_FILENO, row, rowlen);
        }
        if (*normal)
        {
            if (y == config.row - 1)
            {
                char banner[80];
                const char *dir_f = dir.c_str();
                int bannerlen = snprintf(banner, sizeof(banner),
                                         "\x1b[1m\x1b[33mNormal Mode:\x1b[1m\x1b[36m%s\x1b[0m", dir_f);
                if (bannerlen > config.col)
                    bannerlen = config.col;
                write(STDOUT_FILENO, banner, bannerlen);
            }
        }
        else
        {
            if (y == config.row - 1)
            {
                char banner[80];
                int bannerlen = snprintf(banner, sizeof(banner),
                                         "\x1b[1m\x1b[31m$\x1b[0m");
                if (bannerlen > config.col)
                    bannerlen = config.col;
                write(STDOUT_FILENO, banner, bannerlen);
            }
        }
        write(STDOUT_FILENO, "\x1b[K", 3);
        //write(STDOUT_FILENO, "\x1b[?7l", 5);

        // abAppend(&ab,"\x1b[?7l", 5);
        //  abAppend(ab, "\x1b[K", 3);
        if (y < config.row - 1)
        {
            write(STDOUT_FILENO, "\r\n", 3);
        }
    }
}
void refreshScreen(vector<FileList> &f, bool *normal)
{

    struct abuf ab = ABUF_INIT;
// \x1b[37mhhfhjg\x1b[0m
    abAppend(&ab, "\x1b[?25l", 6);
    //abAppend(&ab, "\x1b[2K", 4);
    abAppend(&ab, "\x1b[H", 3);
    write(STDOUT_FILENO, ab.b, ab.len);
    editorDraw(&ab, f, normal);
    ab = ABUF_INIT;
    abAppend(&ab, "\x1b[H", 3);
    abAppend(&ab, "\x1b[?25h", 6);

    abAppend(&ab, "\x1b[1;1H", 6);
    write(STDOUT_FILENO, ab.b, ab.len);

    abFree(&ab);
}
int getCursorPosition(int *rows, int *cols)
{
    string s;
    s.resize(32);
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    while (i < s.length() - 1)
    {
        if (read(STDIN_FILENO, &s[i], 1) != 1)
            break;
        if (s[i] == 'R')
            break;
        i++;
    }
    s[i] = '\0';
    if (s[0] != '\x1b' || s[1] != '[')
        return -1;
    if (sscanf(&s[2], "%d;%d", rows, cols) != 2)
        return -1;
    return 0;
}
int getWindowSize(int *rows, int *cols)
{
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
            return -1;
        // editorReadKey();
        return getCursorPosition(rows, cols);
    }
    else
    {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
void set_zero()
{
    config.x = 1;
    config.y = 1;
    config.numrows = 0;
    config.rowoff = 0;
}
void reposition_cursor(int x, int y, int rowoff, int numrows)
{
    config.x = x;
    config.y = y;
    config.numrows = numrows;
    config.rowoff = rowoff;
}
void cmd_set_zero()
{
    config.x = 3;
    config.y = config.row;
    config.numrows = 0;
    config.rowoff = 0;
}
vector<string> split_string(string s, char ch)
{
    vector<string> tokens;

    stringstream ss(s);
    string token;

    while (getline(ss, token, ch))
    {
        tokens.push_back(token);
    }
    return tokens;
}
string getParent_dir(string s)
{
    vector<string> tokens = split_string(s, '/');
    string ret;
    for (int i = 0; i < tokens.size() - 1; i++)
    {
        if (i > 0)
            ret += "/" + tokens[i];
    }
    return ret;
}
void move_cursor(char *buf, vector<FileList> &f, bool *normal)
{
    refreshScreen(f, normal);
    int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
    write(STDOUT_FILENO, buf, len);
}
void copy_file(string src_path, string dest_path, struct stat buffer)
{
    ifstream src;
    ofstream dst;
    src.open(src_path, ios::in | ios::binary);
    dst.open(dest_path, ios::out | ios::binary);
    dst << src.rdbuf();
    chmod(dest_path.c_str(), buffer.st_mode);
    src.close();
    dst.close();
}
void move_file(string src_path, string dest_path, struct stat buffer)
{
    ifstream src;
    ofstream dst;
    src.open(src_path, ios::in | ios::binary);
    dst.open(dest_path, ios::out | ios::binary);
    dst << src.rdbuf();
    chmod(dest_path.c_str(), buffer.st_mode);
    remove(src_path.c_str());
    src.close();
    dst.close();
}
void print_banner(string s)
{
    char banner[80];
    int bannerlen = snprintf(banner, sizeof(banner),
                             "%s", s.c_str());
    write(STDOUT_FILENO, banner, bannerlen);
    while (true)
    {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        if (int(ch) == 13)
            break;
    }
}
void refresh_print_screen(char *buf, vector<FileList> &f, bool *normal, string s)
{
    if (*normal)
        set_zero();
    else
        cmd_set_zero();
    move_cursor(buf, f, normal);
    print_banner(s);
    move_cursor(buf, f, normal);
}
string handle_path(FileList f, string s)
{
    vector<string> tokens = split_string(s, '/');
    string dir;
    vector<string> format;
    if (tokens[0] == "~")
    {
        dir = getenv("HOME");
        for (int i = 1; i < tokens.size(); i++)
        {
            dir+="/"+tokens[i];
        }
    }
    else if (tokens[0] == ".")
    {
        dir = f.get_dir_name();
        for (int i = 1; i < tokens.size(); i++)
        {
            dir+="/"+tokens[i];
        }
    }
    else if (tokens[0] == "..")
    {
        dir = getParent_dir(f.get_dir_name());
        for (int i = 1; i < tokens.size(); i++)
        {
            dir+="/"+tokens[i];
        }
    }
    else
    {   if(s[0]=='/')
            dir = s;
        else
            dir = f.get_dir_name()+"/"+s;
    }
    if (!dir.empty())
    {
        vector<string> new_tokens = split_string(dir, '/');
        for (int i = 1; i < new_tokens.size(); i++)
        {
            format.push_back("/" + new_tokens[i]);
        }
    }
    vector<string> v;
    for (int i = 0; i < format.size(); i++)
    {

        if (format[i] == "/..")
        {
            v.pop_back();
        }
        else if (format[i] == "/.")
        {
            continue;
        }else{
            v.push_back(format[i]);
        }
        
    }
    string x;
    for (int i = 0; i < v.size(); i++)
    {
        x += v[i];
    }
    return x;
}

void command_goto(vector<FileList> &f, vector<string> &l, int *index, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 2)
    {
        string path = handle_path(f[0], tokens[1]);
        DIR *dir = opendir(path.c_str());
        if (dir)
        {
            string new_path = path;
            f = print_dir(new_path);
            if (*index < l.size() - 1)
                l.erase(l.begin() + *index + 1, l.end());
            l.push_back(new_path);
            *index = *index + 1;
            refresh_print_screen(buf, f, normal,"\x1b[1m\x1b[35m"+path+"\x1b[0m");
        }
        else if (ENOENT == errno)
        {

            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mDirectory does not exist\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError while opening Directory\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }
    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_search(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 2)
    {
        int found = search_dir(tokens[1], getenv("HOME"));
        if (found)
        {
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mTrue\x1b[0m");
        }
        else
        {
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mFalse\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }
    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_quit(char *key, vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    *normal = true;
    refreshScreen(f, normal);
    set_zero();
    int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
    s.erase();
    key[0] = 'q';
    write(STDOUT_FILENO, buf, len);
}
void command_create_dir(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 3)
    {   
        DIR *dir = opendir(handle_path(f[0],tokens[2]).c_str());
        if (dir)
        {
            /* Directory exists. */
            string new_path = handle_path(f[0],tokens[2]) + "/" + tokens[1];
            if (mkdir(new_path.c_str(), 0777) == -1)
            {
                refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:Directory creation failed\x1b[0m");
            }
            else
            {

                refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mDirectory Created Successfully\x1b[0m");
            }
        }
        else if (ENOENT == errno)
        {
            /* Directory does not exist. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mDirectory Does Not Exist\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:Directory Open failed\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }

    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_create_file(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 3)
    {   string path=handle_path(f[0],tokens[2]);
        DIR *dir = opendir(path.c_str());
        if (dir)
        {
            /* Directory exists. */
            std::ofstream(path + "/" + tokens[1]);
            f = print_dir(path);
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mFile Created Successfully\x1b[0m");
        }
        else if (ENOENT == errno)
        {
            /* Directory does not exist. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mDirectory Does Not Exist\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:Directory Open failed\x1b[0m");
        }
    }
    else if (tokens.size() == 2)
    {   
        std::ofstream(handle_path(f[0],tokens[1]));
        f = print_dir(f[0].get_dir_name());
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mFile Created Successfully\x1b[0m");
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }
    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_rename(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 3)
    {   
        string full_path = handle_path(f[0],  tokens[1]);
        struct stat buffer;
        if ((stat(full_path.c_str(), &buffer) == 0))
        {
            string new_path = handle_path(f[0], tokens[2]);
            /* Directory exists. */
            if (rename(full_path.c_str(), new_path.c_str()) == 0)
            {
                f = print_dir(f[0].get_dir_name());
                refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mFile Renamed Successfully\x1b[0m");
            }
            else
                refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError while Renaming file\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:File Does not Exist\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }
    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_delete_dir(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 2)
    {
        /* Directory exists. */
        stack<string> v;
        bool fault = false;
        if (delete_dir(handle_path(f[0],tokens[1]), v) == 1)
        {
            while (!v.empty())
            {
                if (remove(v.top().c_str()) == -1)
                {
                    fault = true;
                    break;
                };
                v.pop();
            }
            if (!fault)
                refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mDirectory Deleted Successfully\x1b[0m");
            else
                refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError Occurred while removing Directory\x1b[0m");
        }
        else
        {
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError Occurred while removing Directory\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }

    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_delete_file(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() == 2)
    {
        /* Directory exists. */
        string src_path = handle_path(f[0],tokens[1]);
        struct stat buffer;
        if ((stat(src_path.c_str(), &buffer) == 0))
        {
            remove(src_path.c_str());
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[32mFile Deleted Successfully\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:File Does not Exist\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }

    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_move(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() >= 3)
    {
        DIR *dir = opendir(handle_path(f[0],tokens[tokens.size() - 1]).c_str());
        if (dir)
        {
            string message;
            for (int i = 1; i < tokens.size() - 1; i++)
            {
                /* Directory exists. */
                string src_path = handle_path(f[0], tokens[i]);
                DIR *s_dir = opendir(src_path.c_str());
                if (s_dir)
                {

                    /* Directory exists. */
                    string dest_path = handle_path(f[0],tokens[tokens.size() - 1]) + "/" + tokens[i];
                    if (rename(src_path.c_str(), dest_path.c_str()) == 0)
                    {
                        message += "\x1b[1m\x1b[32m"+tokens[i] + ":Directory Moved Successfully\x1b[0m\t";
                    }
                    else
                        message += "\x1b[1m\x1b[31m"+tokens[i] + ":Error while Moving Directory\x1b[0m\t";
                }
                else
                {
                    struct stat buffer;
                    if ((stat(src_path.c_str(), &buffer) == 0))
                    {
                    string dest_path = handle_path(f[0],tokens[tokens.size() - 1]) + "/" + tokens[i];
                        move_file(src_path, dest_path, buffer);
                        message += "\x1b[1m\x1b[32m"+tokens[i] + ":File Moved Successfully\x1b[0m\t";
                    }
                    else
                    {
                        /* opendir() failed for some other reason. */
                        message += "\x1b[1m\x1b[31m"+tokens[i] + ":Error:File Does not Exist\x1b[0m\t";
                    }
                }
                f = print_dir(f[0].get_dir_name());
                refresh_print_screen(buf, f, normal, message);
            }
        }
        else if (ENOENT == errno)
        {
            /* Directory does not exist. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mDirectory Does Not Exist\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:Directory Open failed\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }

    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_copy(vector<FileList> &f, vector<string> &l, bool *normal, string &s, char *buf, vector<string> tokens)
{
    if (tokens.size() >= 3)
    {
        DIR *dir = opendir(handle_path(f[0],tokens[tokens.size() - 1]).c_str());
        if (dir)
        {
            string message;
            for (int i = 1; i < tokens.size() - 1; i++)
            {
                /* Directory exists. */
                string src_path = handle_path(f[0],tokens[i]);
                DIR *s_dir = opendir(src_path.c_str());
                if (s_dir)
                {
                    if(copy_dir(src_path, handle_path(f[0],tokens[tokens.size() - 1]) + "/" + tokens[i], tokens[tokens.size() - 1]))
                        message += "\x1b[1m\x1b[32m"+tokens[i] + ":Directory copied Successfully\x1b[0m\t";
                    else
                        message += "\x1b[1m\x1b[35m"+tokens[i] + ":Directory already exist\x1b[0m\t";
                }
                else
                {
                    struct stat buffer;
                    if ((stat(src_path.c_str(), &buffer) == 0))
                    {
                        string dest_path = handle_path(f[0],tokens[tokens.size() - 1]) + "/" + tokens[i];
                        copy_file(src_path, dest_path, buffer);
                        message += "\x1b[1m\x1b[32m"+tokens[i] + ":File copied Successfully\x1b[0m\t";
                    }
                    else
                    {
                        /* opendir() failed for some other reason. */
                        message += "\x1b[1m\x1b[31m"+tokens[i] + ":Error-File Does not Exist\x1b[0m\t";
                    }
                }
            }
            f = print_dir(f[0].get_dir_name());
            refresh_print_screen(buf, f, normal, message);
        }
        else if (ENOENT == errno)
        {
            /* Directory does not exist. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mDetination Directory Does Not Exist\x1b[0m");
        }
        else
        {
            /* opendir() failed for some other reason. */
            refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mError:Directory Open failed\x1b[0m");
        }
    }
    else
    {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
    }

    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void command_mode(char *key, vector<FileList> &f, vector<string> &l, int *index, bool *normal, string &s, char *buf)
{
    if (iscntrl(key[0]))
    {
        if (int(key[0]) == 13)
        {
            if (!s.empty())
            {
                vector<string> tokens = split_string(s, ' ');
                if (tokens[0] == "goto")
                {
                    command_goto(f, l, index, normal, s, buf, tokens);
                }
                else if (tokens[0] == "search")
                {
                    command_search(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "copy")
                {
                    command_copy(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "move")
                {
                    command_move(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "delete_dir")
                {
                    command_delete_dir(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "delete_file")
                {
                    command_delete_file(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "create_dir")
                {
                    command_create_dir(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "create_file")
                {
                    command_create_file(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "rename")
                {
                    command_rename(f, l, normal, s, buf, tokens);
                }
                else if (tokens[0] == "quit" || tokens[0] == "QUIT")
                {
                    command_quit(key, f, l, normal, s, buf, tokens);
                }
                else
                {
        refresh_print_screen(buf, f, normal, "\x1b[1m\x1b[31mNot Valid Input\x1b[0m");
                    cmd_set_zero();
                    move_cursor(buf, f, normal);
                    s.erase();
                }
            }
        }
        else if (int(key[0]) == ESC && int(key[1]) == 0)
        {
            *normal = true;
            refreshScreen(f, normal);
            set_zero();
            int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
            s.erase();
            write(STDOUT_FILENO, buf, len);
        }
        else if (int(key[0]) == BACK_SPACE)
        {
            if (!s.empty())
                s.pop_back();
            refreshScreen(f, normal);
            cmd_set_zero();
            int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
            write(STDOUT_FILENO, buf, len);
            write(STDOUT_FILENO, s.c_str(), s.length());
        }
    }
    else
    {
        char *k = key;

        s.push_back(key[0]);
        write(STDOUT_FILENO, k, 1);
    }
}
void move_to_parent(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    FileList file = f[1];
    string new_path = file.get_dir_name() + "/" + file.get_name();
    if (!getParent_dir(file.get_dir_name()).empty())
    {
        new_path = getParent_dir(file.get_dir_name());
        l.push_back(new_path);
        *index = *index + 1;
        f = print_dir(new_path);
        set_zero();
        refreshScreen(f, normal);
    }
}
void move_cursor_down(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    if (config.y < config.row - 1)
    {
        if (config.y < f.size())
            config.y++;
        int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
        write(STDOUT_FILENO, buf, len);
    }
    else
    {
        if (config.y + config.rowoff < f.size())
            config.rowoff++;
        // clear_screen();
        move_cursor(buf, f, normal);
    }
}
void move_cursor_up(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    if (config.y > 1)
    {
        config.y--;
        int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
        write(STDOUT_FILENO, buf, len);
    }
    else
    {
        if (config.y + config.rowoff > 1)
        {
            config.rowoff--;

            move_cursor(buf, f, normal);
        }
    }
}
void move_next_dir(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    if (*index < l.size() - 1)
    {
        *index = *index + 1;
        f = print_dir(l[*index]);
        set_zero();
        refreshScreen(f, normal);
    }
}
void move_previous_dir(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    if (*index > 0)
    {
        *index = *index - 1;
        f = print_dir(l[*index]);
        set_zero();
        refreshScreen(f, normal);
    }
}
void move_to_home_dir(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    string home = getenv("HOME");
    l.push_back(home);
    *index = *index + 1;
    f = print_dir(home);
    set_zero();
    refreshScreen(f, normal);
}
void escape_from_normal_mode(vector<FileList> &f, vector<string> &l, int *index, bool *normal, string &s, char *buf)
{
    *normal = false;
    refreshScreen(f, normal);
    cmd_set_zero();
    move_cursor(buf, f, normal);
    s.erase();
}
void process_file(string new_path)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        /* char* ifname_cchararr = (char*)malloc(new_path.length() + 1);
            strcpy (ifname_cchararr, new_path.c_str());
            char* const argv[4] = {"xterm", "vim", ifname_cchararr, NULL};
        // std::cout << ifname_cchararr<<std::endl;
            execvp ("xterm", argv); */
        execl("/usr/bin/xdg-open", "xdg-open", new_path.c_str(), (char *)0);
        // execl("/usr/bin/vim", "vim", new_path.c_str(), (char *)0);
    }
}
void process_dir(vector<FileList> &f, FileList file, vector<string> &l, int *index, bool *normal, string new_path, char *buf)
{
    if (strcmp(file.get_file_name(), "..") != 0)
    {
        if (*index < l.size() - 1)
            l.erase(l.begin() + *index + 1, l.end());
        l.push_back(new_path);
        *index = *index + 1;
        f = print_dir(new_path);
        set_zero();
        refreshScreen(f, normal);
    }
    else
    {
        if (!getParent_dir(file.get_dir_name()).empty())
        {
            new_path = getParent_dir(file.get_dir_name());
            if (*index < l.size() - 1)
                l.erase(l.begin() + *index + 1, l.end());
            l.push_back(new_path);
            *index = *index + 1;
            f = print_dir(new_path);
            set_zero();
            refreshScreen(f, normal);
        }
    }
}
void process_index(vector<FileList> &f, vector<string> &l, int *index, bool *normal, char *buf)
{
    FileList file = f[config.y + config.rowoff - 1];
    string new_path = file.get_dir_name() + "/" + file.get_name();
    if (file.get_permissions()[0] == 'd' && strcmp(file.get_file_name(), ".") != 0)
    {
        process_dir(f, file, l, index, normal, new_path, buf);
    }
    else if (file.get_permissions()[0] == '-')
    {
        process_file(new_path);
    }
}
void normal_mode(char *key, vector<FileList> &f, vector<string> &l, int *index, bool *normal, string &s, char *buf)
{
    if (iscntrl(key[0]))
    {
        if (int(key[0]) == BACK_SPACE)
        {
            move_to_parent(f, l, index, normal, buf);
        }
        else if (int(key[0]) == 27)
        {

            if (int(key[1]) == 91)
            {

                if (int(key[2]) == DOWN)
                {

                    move_cursor_down(f, l, index, normal, buf);
                    // redraw_scroll(f);
                }
                else if (int(key[2]) == UP)
                {
                    move_cursor_up(f, l, index, normal, buf);
                }
                else if (int(key[2]) == RIGHT)
                {
                    // write(STDOUT_FILENO, "right", 5);
                    move_next_dir(f, l, index, normal, buf);
                }
                else if (int(key[2]) == LEFT)
                {
                    // write(STDOUT_FILENO, "left", 4);
                    move_previous_dir(f, l, index, normal, buf);
                }
                else if (int(key[2]) == HOME1)
                {
                    move_to_home_dir(f, l, index, normal, buf);
                }
            }
        }
        else if (int(key[0]) == ENTER)
        {
            process_index(f, l, index, normal, buf);
        }
    }
    else
    {
        if (int(key[0]) == HOME || int(key[0]) == HOME1)
        {
            move_to_home_dir(f, l, index, normal, buf);
        }
        else if (int(key[0]) == COLON)
        {
            escape_from_normal_mode(f, l, index, normal, s, buf);
        }
    }
}
void editorMoveCursor(char *key, vector<FileList> &f, vector<string> &l, int *index, bool *normal, string &s)
{
    char buf[32];

    if (*normal)
    {
        normal_mode(key, f, l, index, normal, s, buf);
    }
    else
    {
        command_mode(key, f, l, index, normal, s, buf);
    }
}
void init(vector<string> &l)
{
    set_zero();
    l.push_back(getenv("HOME"));
    // config.r=NULL;
    if (getWindowSize(&config.row, &config.col) == -1)
    {
        cout << "No Window size available" << endl;
        return;
    }
    clear_screen();
}
void editor()
{

    vector<string> dir_names;
    enableRawMode();
    init(dir_names);
    // vector<string>::iterator it=dir_names.end();
    vector<FileList> f = print_dir(dir_names.back());
    int index = 0;
    bool normal = true;
    refreshScreen(f, &normal);
    string s;
    int newy, newx;
    while (true)
    {
        write(STDOUT_FILENO, "\x1b[?7l", 5);
        char ch[3] = {'\0'};
        getWindowSize(&newy, &newx);
        if (newy != config.row || newx != config.col)
        {   
            f = print_dir(f[0].get_dir_name());
            config.row = newy;
            config.col = newx;
            if (normal)
                set_zero();
            else
                cmd_set_zero();
            refreshScreen(f, &normal);
            char buf[32];
            int len = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", config.y, config.x);
            write(STDOUT_FILENO, buf, len);
        }
        readKey(ch);

        editorMoveCursor(ch, f, dir_names, &index, &normal, s);

        if (int((ch[0]) == q || int(ch[0]) == Q) && normal)
        {
            clear_screen();
            break;
        }
    }
}
int main()
{

    editor();
    return 0;
}