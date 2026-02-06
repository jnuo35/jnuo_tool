#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
#include<shellapi.h>
using namespace std;
int fc_using[8]={0};
bool program_running = true;
// 控制台信号处理函数
// 程序开始日志
void log_program_start() {
    time_t now = time(NULL);
    tm* local_time = localtime(&now);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m-%d %H:%M", local_time);
    
    FILE* log_file = fopen("app.log", "a");
    if (!log_file) {
        log_file = fopen("app.log", "w");
        if (!log_file) return;
    }
    
    fprintf(log_file, "==========start========== [%s]\n", time_str);
    fclose(log_file);
}
void RunBatInSourceFolderSimple() {
    // 直接使用相对路径（相对于当前工作目录）
    // 注意：如果程序从其他目录启动，这可能不可靠
    
    LPCWSTR relativePath = L"source\\ban_jiyu\\useU.bat";
    
    // 以管理员身份运行
    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = "runas";
    sei.lpFile = relativePath;
    sei.nShow = SW_SHOW;
    
    if (!ShellExecuteEx(&sei)) {
        DWORD err = GetLastError();
        if (err == ERROR_CANCELLED) {
            wcout <<"用户取消了操作" << endl;
        } else {
            wcout << "执行失败，错误代码: " << err << endl;
        }
    }
}
// 程序结束日志
void log_program_end() {
    time_t now = time(NULL);
    tm* local_time = localtime(&now);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m-%d %H:%M", local_time);
    
    FILE* log_file = fopen("app.log", "a");
    if (!log_file) {
        log_file = fopen("app.log", "a"); // 尝试用追加模式
        if (!log_file) return;
    }
    
    fprintf(log_file, "==========end============ [%s]\n\n", time_str);
    fclose(log_file);
}
BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:        // Ctrl+C
    case CTRL_BREAK_EVENT:    // Ctrl+Break
    case CTRL_CLOSE_EVENT:    // 点击关闭按钮
    case CTRL_LOGOFF_EVENT:   // 用户注销
    case CTRL_SHUTDOWN_EVENT: // 系统关机
        if (program_running) {
            program_running = false;
            log_program_end();
        }
        break;
    }
    return FALSE;
}
void start()
{
	printf("jnuo制造");
	Sleep(1000);
	system("cls");
	for(int i=1;i<=2;i++) 
	{
		printf("jnuo制造.");
		Sleep(1000);
		system("cls");
		printf("jnuo制造..");
		Sleep(1000);
		system("cls");
		printf("jnuo制造...");
		Sleep(1000);
		system("cls");
	}
}
void log(int function_id) {
    bool running_status=!(fc_using[function_id]%2==0);
    time_t now = time(NULL);
    tm* local_time = localtime(&now);
    
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%m-%d %H:%M", local_time);
    
    FILE* log_file = fopen("app.log", "a");
    if (!log_file) {
        log_file = fopen("app.log", "w");
        if (!log_file) return;
    }
    
    fprintf(log_file, "[%s] used:%d, running:%d\n", time_str, function_id, running_status ? 1 : 0);
    fclose(log_file);
}
char* strrstr(const char* str, const char* substr) {
    if (!str || !substr) return NULL;
    
    const char* last = NULL;
    const char* current = str;
    
    while ((current = strstr(current, substr)) != NULL) {
        last = current;
        current++; // 继续查找下一个
    }
    
    return (char*)last;
}
void read_log_detailed() {
    FILE* log_file = fopen("app.log", "r");
    if (!log_file) {
        printf("没有找到日志文件\n");
        system("pause");
        return;
    }
    
    printf("正在读取上次使用记录并恢复功能...\n");
    
    // 先找到文件末尾，从后往前读取
    fseek(log_file, 0, SEEK_END);
    long file_size = ftell(log_file);
    
    // 从文件末尾开始，查找最近的end标记
    long pos = file_size - 1;
    char buffer[1024];
    bool found_end = false;
    long end_pos = -1;
    long start_pos = -1;
    
    // 从后往前查找end标记
    while (pos >= 0 && pos >= file_size - 5000) { // 最多往回找5000字节
        fseek(log_file, pos, SEEK_SET);
        fgets(buffer, sizeof(buffer), log_file);
        
        if (strstr(buffer, "==========end============")) {
            found_end = true;
            end_pos = pos;
            break;
        }
        pos--;
    }
    
    if (!found_end) {
        printf("未找到完整的上次使用记录\n");
        fclose(log_file);
        system("pause");
        return;
    }
    
    // 从end标记往前找对应的start标记
    pos = end_pos - 1;
    bool found_start = false;
    
    while (pos >= 0) {
        fseek(log_file, pos, SEEK_SET);
        fgets(buffer, sizeof(buffer), log_file);
        
        if (strstr(buffer, "==========start==========")) {
            found_start = true;
            start_pos = pos;
            break;
        }
        pos--;
    }
    
    if (!found_start) {
        printf("未找到对应的开始记录\n");
        fclose(log_file);
        system("pause");
        return;
    }
    
    // 读取整个区块的内容
    fseek(log_file, start_pos, SEEK_SET);
    long block_size = end_pos - start_pos + 100; // +100确保包含end行
    char* block_content = (char*)malloc(block_size + 1);
    fread(block_content, 1, block_size, log_file);
    block_content[block_size] = '\0';
    
    fclose(log_file);
    
    // 解析区块内容
    printf("找到上次使用记录：\n");
    int last_function_status[8] = {0};
    
    char* line = strtok(block_content, "\n");
    while (line) {
        // 跳过标记行
        if (strstr(line, "==========start==========") || 
            strstr(line, "==========end============")) {
            line = strtok(NULL, "\n");
            continue;
        }
        
        int function_id, running_status;
        if (sscanf(line, "[%*[^]]] used:%d, running:%d", &function_id, &running_status) == 2) {
            if (function_id >= 1 && function_id <= 7) {
                last_function_status[function_id] = running_status + 1;
                printf("  功能%d: %s\n", function_id, running_status ? "运行中" : "已停止");
            }
        }
        
        line = strtok(NULL, "\n");
    }
    
    // 恢复功能（和之前相同）
    printf("\n正在恢复功能...\n");
    int recovered_count = 0;
    
    for (int i = 1; i <= 7; i++) {
        if (last_function_status[i] == 2) {
            printf("恢复功能 %d: ", i);
            
            if (i == 2) {
                printf("deepseek\n");
                system("start https://chat.deepseek.com");
                system(".\\source\\Web\\deekseep\\deekseep.html");
                fc_using[2] = 1;
                recovered_count++;
            } else if (i == 5) {
                printf("miHoYo\n");
                fc_using[5] = 1;
            } else if (i == 6) {
                printf("防窥屏\n");
                system(".\\source\\ScreenWings.exe");
                fc_using[6] = 1;
                recovered_count++;
            } else {
                printf("功能代码待实现\n");
            }
            
            Sleep(500);
        }
    }
    
    if (last_function_status[5] == 2) {
        printf("\n检测到上次使用了miHoYo功能，请选择游戏：\n");
        printf("1. 原神\n");
        printf("2. 崩坏·星穹铁道\n");
        printf("输入其他字符跳过\n");
        
        int s = _getch();
        if (s == '1') {
            system("start https://ys.mihoyo.com/cloud/#");
            system("start https://ys-api.mihoyo.com/event/download_porter/link/ys_cn/official/pc_backup316");
            recovered_count++;
        } else if (s == '2') {
            system("start https://sr.mihoyo.com/cloud/#");
            system("start https://autopatchcn.bhsr.com/client/cn/20251126183400_yvLQxEpk9CTuJjg6/gw_PC/StarRail_setup_1.12.0.exe");
            recovered_count++;
        }
    }
    
    printf("\n已恢复 %d 个功能\n", recovered_count);
    
    free(block_content);
    printf("按任意键继续...\n");
    _getch();
}
string password(){
	//to get password
}
void cmd()
{
	printf("     ---------------------------------------------\n");
	printf("     |               \033[32mjnuo_tool 2.0.0\033[0m             |\n");
	printf("     |       -----------------------------       |\n");
	printf("     | \033[31m1. 杀死极域\033[0m  \033[34m2. deepseek\033[0m  \033[35m3. 把zip藏进图片\033[0m|\n");
	printf("     | \033[36m4. 对拍\033[0m  \033[90m5. miHoYo\033[0m  \033[91m6. 防窥屏\033[0m  \033[95m7. 解除限制\033[0m|\n");
	printf("     |       -----------------------------       |\n");
	printf("     |                  \033[33mq. 退出\033[0m                  |\n");
	printf("     |       -----------------------------       |\n");
	printf("     |       \033[33mx. 这次更新\033[0m    \033[33mr. 查看使用记录\033[0m      |\n");
	printf("     ---------------------------------------------\n"); 
}
void fake_cmd()
{
	system("cls"); 
	printf("     ---------------------------------------------\n");
	printf("     |               \033[32mjnuo_tool ?.0\033[0m               |\n");
	printf("     |       -----------------------------       |\n");
	printf("     |                \033[31m别往后看！！！\033[0m             |\n");
	printf("     |                   \033[31mo.？？？\033[0m                |\n");
	printf("     |       -----------------------------       |\n");
	printf("     |                  \033[33mq. 退出\033[0m                  |\n");
	printf("     ---------------------------------------------\n"); 
	char t;
	do{
		cin>>t;
	}while(t!='o'||t!='O');
	system("cls"); 
	printf("     ---------------------------------------------\n");
	printf("     |               \033[32mjnuo_tool ?.0\033[0m               |\n");
	printf("     |       -----------------------------       |\n");
	printf("     |                \033[31m什么都没有！！\033[0m             |\n");
	printf("     ---------------------------------------------\n");
	system("pause"); 
}
int main()
{	system("title jnuo_tool 2.0.0"); 
	//fake_cmd();
	system("color A"); 
	start();
	system("color 7");
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	log_program_start();
	char n;
	bool flag2=0,flag5=0,flag6=0;
	while(1)
	{   system("cls");
		cmd(); 
		printf("“deepseek”运行状态：%d\n",flag2); 
		printf("“miHoYo”运行状态：%d\n",flag5); 
		printf("“防窥屏”运行状态：%d\n",flag6); 
		int n=_getch();
		if(n==113||n==81)  break;
		else if(n==106||n==74) system("start https://www.bilibili.com/video/BV1GJ411x7h7");
		else if(n=='r'||n=='R') {
			cout<<"\n"; 
			read_log_detailed();
			continue;
		}
		else if(n==120||n==88){
			printf("\033[32mjnuo_tool 2.0.0更新内容\033[0m\n"); 
			printf("1. 更新了日志，保存了上次的使用情况（自动打开）\n");
			printf("2. 优化了输入字符的部分\n");
			printf("3. 加入了新选项\n");
			system("pause"); 
			continue; 
		} 
		else if(n==48){
			int s;
			s=_getch();
			if(s==106||s==74) fake_cmd();
			else if(s==49) printf("试试最开始输入“j”吧！");
			else printf("啊吧啊吧......");
		} 
		else if(n==49)
		{   int s;
			printf("确定要杀死极域吗？\n杀死后需要\033[31m自行寻找快捷方式\033[0m\njnuo对你的行为\033[31m概不负责\033[0m [\033[31my\033[0m/\033[33mn\033[0m]");
			s=_getch();
			if(s==121||s==89)
				system("taskkill /f /t /im studentmain.exe");
			else 
				continue;
		}
		else if(n==50){
			if(!flag2){
				flag2=1;
				system("start https://chat.deepseek.com");
				system(".\\source\\Web\\deekseep\\deekseep.html");  // Windows
				fc_using[2]=1;
				log(2);
			}
			else{
				flag2=0;
				printf("此功能正在运行中，是否已经关闭？[\033[31my\033[0m/\033[33mn\033[0m]");
				int s=_getch();
				if(s=='y'||s=='Y'){
					fc_using[2]=2;
					log(2);
					continue;
				} 
			}
		}
		else if(n==51)
		{   printf("请确保您的\033[31m图片（png）文件\033[0m已命名为\033[31m“1.png”\033[0m\n并且您的\033[31mzip文件\033[0m已命名为\033[31m“2.zip”\033[0m\n它们需要\033[31m和本程序在同一个目录里\033[0m ");
		    printf("[\033[31my\033[0m/\033[33mn\033[0m] ");
			int s;
			s=_getch();
			if(s=='y'||s=='Y')  system("copy /b 1.png + 2.zip end.png"); 
			else        continue;
			printf("如没有显示找不到文件，请前往程序所在目录，里面会有一个end.png，它是可以用压缩软件打开的图片"); 
			Sleep(2000); 
		}
		else if(n==52){
			printf("请确保您的\033[31m正确输出文件和程序输出文件\033[0m已命名为\033[31m“2.out”\033[0m\n它们需要\033[31m和本程序在同一个目录里\033[0m ");
		    printf("[\033[31my\033[0m/\033[33mn\033[0m] \n不知道什么是对拍？输入\033[31m“x”\033[0m ");
		    int s;
		    s=_getch();
		    if(s=='x'||s=='X'){
		    	printf("对拍，是对比两个文件\033[31m是否有不一样\033[0m的地方，一般用于比较自己的输出与正确答案！");
			}
			else if(s=='y'||s=='Y') system("fc 1.ans 2.out");
			else continue; 
		}
		else if(n==53){
			if(!flag5){
				flag5=1;
				int s;
				cout<<"1. 原神\n2. 崩坏·星穹铁道 \n";
				s=_getch();
				fc_using[5]+=1;
				log(5);
				if(s=='1'){
					system("start https://ys.mihoyo.com/cloud/#");
					system("start https://ys-api.mihoyo.com/event/download_porter/link/ys_cn/official/pc_backup316"); 
				}
				else if(s=='2'){
					system("start https://sr.mihoyo.com/cloud/#");
					system("start https://autopatchcn.bhsr.com/client/cn/20251126183400_yvLQxEpk9CTuJjg6/gw_PC/StarRail_setup_1.12.0.exe");
				}
				else printf("看好了，小子！是\033[31m1和2\033[0m！！！罚你从头来过！");
			}
			else{
				printf("此功能正在运行中，是否已经关闭？[\033[31my\033[0m/\033[33mn\033[0m]");
				int s=_getch();
				if(s=='y'||s=='Y'){
					fc_using[5]=2;
					log(5);
					flag5=0;
				} 
				continue;
			}
		}
		else if(n==54){
			if(!flag6){
				flag6=1;
				system(".\\source\\ScreenWings.exe");
				printf("输入\033[31mx\033[0m 教我怎么用？\n");
				printf("输入其他字符 继续使用jnuo_tool\n");
				int s;
				s=_getch(); 
				fc_using[6]+=1;
				log(6);
				if(s=='x'||s=='X'){
					printf("出来后，会有个小电脑，有Windows图标，点一下，变黑了，同时\n"); 
					printf("右上角的\033[31m“X”\033[0m变为了\033[31m“-”\033[0m就说明成功了！\n");
					printf("然后请最小化，并拖到“^”里（隐藏图标）\n");
					printf("请注意：使用时，\033[31m截图也会黑屏\033[0m！\n");
					system("pause");
					continue; 
				}
				else continue;
			}
			else{
				printf("此功能正在运行中，是否已经关闭？[\033[31my\033[0m/\033[33mn\033[0m]");
				int s=_getch();
				if(s=='y'||s=='Y'){
					fc_using[6]=2; 
					log(6);
					flag6=0;
				} 
				continue;
			}
		} 
		else if(n=='7') {
			RunBatInSourceFolderSimple();
		}
		else  printf("输入错误，请重新输入！") ;
		Sleep(3000);
	}
	if (program_running) {
        program_running = false;
        log_program_end();
    }
	printf("感谢您的使用！\n\n再见 ヾ(￣▽￣)Bye~Bye~"); 
	Sleep(3000); 
}
/*
             _____    _____        _____    _____       _____           __________
             |@@@|    |@@@@\       |@@@|    |@@@|       |@@@|          /@@@@@@@@@@\
             |@@@|    |@@@@@\      |@@@|    |@@@|       |@@@|         /@@@@@@@@@@@@\
             |@@@|    |@@@@@@\     |@@@|    |@@@|       |@@@|        /@@@@------@@@@\
             |@@@|    |@@@@@@@\    |@@@|    |@@@|       |@@@|       /@@@@/      \@@@@\
             |@@@|    |@@@|\@@@\   |@@@|    |@@@|       |@@@|      /@@@@/        \@@@@\
             |@@@|    |@@@| \@@@\  |@@@|    |@@@|       |@@@|     /@@@@/          \@@@@\
     _____   |@@@|    |@@@|  \@@@\ |@@@|    |@@@|       |@@@|    |@@@@\           /@@@@|
     |@@@|   |@@@|    |@@@|   \@@@\|@@@|    \@@@\       /@@@/     \@@@@\         /@@@@/
     \@@@\___|@@@|    |@@@|    \@@@@@@@|     \@@@\____ /@@@/       \@@@@\______ /@@@@/
      \@@@@@@@@@/     |@@@|     \@@@@@@|      \@@@@@@@@@@@/         \@@@@@@@@@@@@@@@/ 
       \@@@@@@@/      |@@@|      \@@@@@|       \@@@@@@@@@/           \@@@@@@@@@@@@@/
        -------       -----       ------        --------              -------------    
*/
