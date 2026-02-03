#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
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
	cout<<"jnuo制造";
	Sleep(1000);
	system("cls");
	for(int i=1;i<=2;i++) 
	{
		cout<<"jnuo制造.";
		Sleep(1000);
		system("cls");
		cout<<"jnuo制造..";
		Sleep(1000);
		system("cls");
		cout<<"jnuo制造...";
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
void read_log_detailed() {
    FILE* log_file = fopen("app.log", "r");
    if (!log_file) {
        printf("没有找到日志文件\n");
        system("pause");
        return;
    }
    
    printf("===== 使用日志记录 =====\n\n");
    
    char line[256];
    int line_count = 0;
    while (fgets(line, sizeof(line), log_file)) {
        line_count++;
        printf("%d. %s", line_count, line);
    }
    
    if (line_count == 0) {
        printf("暂无使用记录\n");
    }
    
    fclose(log_file);
    
    printf("\n=======================\n");
    printf("按任意键返回...\n");
    _getch();
    return;
}
void cmd()
{
	cout<<"     ---------------------------------------------\n";
	cout<<"     |               \033[32mjnuo_tool 2.0.0\033[0m             |\n";
	cout<<"     |       -----------------------------       |\n";
	cout<<"     | \033[31m1. 杀死极域\033[0m  \033[34m2. deepseek\033[0m  \033[35m3. 把zip藏进图片\033[0m|\n";
	cout<<"     | \033[36m4. 对拍\033[0m  \033[90m5. miHoYo\033[0m  \033[91m6. 防窥屏\033[0m  \033[95m7. 极域密码\033[0m|\n";
	cout<<"     |       -----------------------------       |\n";
	cout<<"     |                  \033[33mq. 退出\033[0m                  |\n";
	cout<<"     |       -----------------------------       |\n";
	cout<<"     |       \033[33mx. 这次更新\033[0m    \033[33mr. 查看使用记录\033[0m      |\n";
	cout<<"     ---------------------------------------------\n"; 
}
void fake_cmd()
{
	system("cls"); 
	cout<<"     ---------------------------------------------\n";
	cout<<"     |               \033[32mjnuo_tool ?.0\033[0m               |\n";
	cout<<"     |       -----------------------------       |\n";
	cout<<"     |                \033[31m别往后看！！！\033[0m             |\n";
	cout<<"     |                   \033[31mo.？？？\033[0m                |\n";
	cout<<"     |       -----------------------------       |\n";
	cout<<"     |                  \033[33mq. 退出\033[0m                  |\n";
	cout<<"     ---------------------------------------------\n"; 
	char t;
	do{
		cin>>t;
	}while(t!='o'||t!='O');
	system("cls"); 
	cout<<"     ---------------------------------------------\n";
	cout<<"     |               \033[32mjnuo_tool ?.0\033[0m               |\n";
	cout<<"     |       -----------------------------       |\n";
	cout<<"     |                \033[31m什么都没有！！\033[0m             |\n";
	cout<<"     ---------------------------------------------\n";
	system("pause"); 
}
int main()
{	system("title jnuo_tool 2.0.0"); 
	//fake_cmd();
	system("color A"); 
	//start();
	system("color 7");
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	log_program_start();
	char n;
	bool flag2=0,flag5=0,flag6=0;
	while(1)
	{   system("cls");
		cmd(); 
		cout<<"“deepseek”运行状态：" <<flag2<<endl; 
		cout<<"“miHoYo”运行状态：" <<flag5<<endl; 
		cout<<"“防窥屏”运行状态：" <<flag6<<endl; 
		int n=_getch();
		if(n==113||n==81)  break;
		else if(n==106||n==74) system("start https://www.bilibili.com/video/BV1GJ411x7h7");
		else if(n=='r'||n=='R') read_log_detailed();
		else if(n==120||n==88){
			cout<<"\033[32mjnuo_tool 2.0.0更新内容\033[0m\n"; 
			cout<<"1. 更新了日志，保存了上次的使用情况（自动打开）\n";
			cout<<"2. 优化了输入字符的部分\n";
			cout<<"3. 加入了新选项\n"; 
			system("pause"); 
			continue; 
		} 
		else if(n==48){
			int s;
			s=_getch();
			if(s==106||s==74) fake_cmd();
			else if(s==49) cout<<"试试最开始输入“j”吧！";
			else cout<<"啊吧啊吧......";
		} 
		else if(n==49)
		{   int s;
			cout<<"确定要杀死极域吗？\n杀死后需要\033[31m自行寻找快捷方式\033[0m\njnuo对你的行为\033[31m概不负责\033[0m [\033[31my\033[0m/\033[33mn\033[0m]";
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
				cout<<"此功能正在运行中，是否已经关闭？[\033[31my\033[0m/\033[33mn\033[0m]";
				int s=_getch();
				if(s=='y'||s=='Y'){
					fc_using[2]=2;
					log(2);
					continue;
				} 
			}
		}
		else if(n==51)
		{   cout<<"请确保您的\033[31m图片（png）文件\033[0m已命名为\033[31m“1.png”\033[0m\n并且您的\033[31mzip文件\033[0m已命名为\033[31m“2.zip”\033[0m\n它们需要\033[31m和本程序在同一个目录里\033[0m ";
		    cout<<"[\033[31my\033[0m/\033[33mn\033[0m] ";
			int s;
			s=_getch();
			if(s=='y'||s=='Y')  system("copy /b 1.png + 2.zip end.png"); 
			else        continue;
			cout<<"如没有显示找不到文件，请前往程序所在目录，里面会有一个end.png，它是可以用压缩软件打开的图片"; 
			Sleep(2000); 
		}
		else if(n==52){
			cout<<"请确保您的\033[31m正确输出文件\033[0m已命名为\033[31m“1.ans”\033[0m\n并且您的\033[31m程序输出文件\033[0m已命名为\033[31m“2.out”\033[0m\n它们需要\033[31m和本程序在同一个目录里\033[0m ";
		    cout<<"[\033[31my\033[0m/\033[33mn\033[0m] \n不知道什么是对拍？输入\033[31m“x”\033[0m ";
		    int s;
		    s=_getch();
		    if(s=='x'||s=='X'){
		    	cout<<"对拍，是对比两个文件\033[31m是否有不一样\033[0m的地方，一般用于比较自己的输出与正确答案！";
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
				else cout<<"看好了，小子！是\033[31m1和2\033[0m！！！罚你从头来过！";
			}
			else{
				cout<<"此功能正在运行中，是否已经关闭？[\033[31my\033[0m/\033[33mn\033[0m]";
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
				cout<<"输入\033[31mx\033[0m 教我怎么用？\n";
				cout<<"输入其他字符 继续使用jnuo_tool\n";
				int s;
				s=_getch(); 
				fc_using[6]+=1;
				log(6);
				if(s=='x'||s=='X'){
					cout<<"出来后，会有个小电脑，有Windows图标，点一下，变黑了，同时\n"; 
					cout<<"右上角的\033[31m“X”\033[0m变为了\033[31m“-”\033[0m就说明成功了！\n";
					cout<<"然后请最小化，并拖到“^”里（隐藏图标）\n";
					cout<<"请注意：使用时，\033[31m截图也会黑屏\033[0m！\n";
					system("pause");
					continue; 
				}
				else continue;
			}
			else{
				cout<<"此功能正在运行中，是否关闭？[\033[31my\033[0m/\033[33mn\033[0m]";
				int s=_getch();
				if(s=='y'||s=='Y'){
					system("taskkill /f /t /im ScreenWings.exe");
					fc_using[6]=2; 
					log(6);
					flag6=0;
				} 
				continue;
			}
		} 
		else if(n=='7') {
			//pass
		}
		else  cout<<"输入错误，请重新输入！" ;
		Sleep(3000);
	}
	if (program_running) {
        program_running = false;
        log_program_end();
    }
	cout<<"感谢您的使用！\n\n再见 ヾ(￣▽￣)Bye~Bye~"; 
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

