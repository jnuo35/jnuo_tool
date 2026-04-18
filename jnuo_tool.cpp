#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
#include<shellapi.h>
using namespace std;
int fc_using[8]={0};
bool program_running = true;
// ==================== DeepSeek API 简化版（使用curl命令） ====================
// 不需要额外头文件，不需要链接任何库！

// DeepSeek API配置（请替换成你的真实API Key）
const char* DEEPSEEK_API_KEY = "sk-4f3**************ad0";  // 保护API

// ========== 编码转换函数 ==========

// GBK转UTF-8
std::string gbkToUtf8(const std::string& gbkStr) {
	if (gbkStr.empty()) return "";
	
	int unicodeLen = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
	if (unicodeLen <= 0) return gbkStr;
	
	wchar_t* wideStr = new wchar_t[unicodeLen];
	MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, wideStr, unicodeLen);
	
	int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
	if (utf8Len <= 0) {
		delete[] wideStr;
		return gbkStr;
	}
	
	char* utf8Str = new char[utf8Len];
	WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, utf8Str, utf8Len, NULL, NULL);
	
	std::string result(utf8Str);
	delete[] wideStr;
	delete[] utf8Str;
	
	return result;
}

// UTF-8转GBK
std::string utf8ToGbk(const std::string& utf8Str) {
	if (utf8Str.empty()) return "";
	
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
	if (unicodeLen <= 0) return utf8Str;
	
	wchar_t* wideStr = new wchar_t[unicodeLen];
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideStr, unicodeLen);
	
	int gbkLen = WideCharToMultiByte(CP_ACP, 0, wideStr, -1, NULL, 0, NULL, NULL);
	if (gbkLen <= 0) {
		delete[] wideStr;
		return utf8Str;
	}
	
	char* gbkStr = new char[gbkLen];
	WideCharToMultiByte(CP_ACP, 0, wideStr, -1, gbkStr, gbkLen, NULL, NULL);
	
	std::string result(gbkStr);
	delete[] wideStr;
	delete[] gbkStr;
	
	return result;
}

// ========== JSON处理函数 ==========

// JSON转义函数
std::string escapeJsonString(const std::string& str) {
	std::string escaped;
	for (char c : str) {
		switch (c) {
			case '"': escaped += "\\\""; break;
			case '\\': escaped += "\\\\"; break;
			case '\n': escaped += "\\n"; break;
			case '\r': escaped += "\\r"; break;
			case '\t': escaped += "\\t"; break;
			default: escaped += c; break;
		}
	}
	return escaped;
}

// 从JSON响应中提取content内容
std::string extractContent(const std::string& json) {
	// 查找 "content" 字段
	const char* searchKey = "\"content\"";
	size_t pos = json.find(searchKey);
	if (pos == std::string::npos) return "";
	
	// 找到冒号
	pos = json.find(':', pos);
	if (pos == std::string::npos) return "";
	
	// 找到引号开始
	pos = json.find('"', pos);
	if (pos == std::string::npos) return "";
	pos++;
	
	// 提取内容直到下一个引号
	size_t endPos = pos;
	while (endPos < json.length()) {
		if (json[endPos] == '"' && (endPos == 0 || json[endPos-1] != '\\')) {
			break;
		}
		endPos++;
	}
	
	if (endPos == std::string::npos) return "";
	
	std::string content = json.substr(pos, endPos - pos);
	
	// 处理转义字符
	size_t escapePos;
	while ((escapePos = content.find("\\n")) != std::string::npos) {
		content.replace(escapePos, 2, "\n");
	}
	while ((escapePos = content.find("\\r")) != std::string::npos) {
		content.replace(escapePos, 2, "\r");
	}
	while ((escapePos = content.find("\\t")) != std::string::npos) {
		content.replace(escapePos, 2, "\t");
	}
	while ((escapePos = content.find("\\\"")) != std::string::npos) {
		content.replace(escapePos, 2, "\"");
	}
	while ((escapePos = content.find("\\\\")) != std::string::npos) {
		content.replace(escapePos, 2, "\\");
	}
	
	return content;
}

// ========== DeepSeek API调用（使用curl命令） ==========

// 调用DeepSeek API（使用系统curl）
std::string callDeepSeek(const std::string& userMessage) {
	// 构建JSON请求体
	std::string requestBody = "{";
	requestBody += "\"model\": \"deepseek-chat\",";
	requestBody += "\"messages\": [";
	requestBody += "{\"role\": \"user\", \"content\": \"" + escapeJsonString(userMessage) + "\"}";
	requestBody += "],";
	requestBody += "\"stream\": false,";
	requestBody += "\"temperature\": 0.7";
	requestBody += "}";
	
	// 创建临时文件
	std::string requestFile = "temp_req_" + std::to_string(GetCurrentProcessId()) + ".txt";
	std::string responseFile = "temp_res_" + std::to_string(GetCurrentProcessId()) + ".txt";
	
	// 写入请求内容
	FILE* f = fopen(requestFile.c_str(), "w");
	if (!f) {
		return "ERROR:无法创建请求文件";
	}
	fprintf(f, "%s", requestBody.c_str());
	fclose(f);
	
	// 构建curl命令
	std::string command = "curl -s -X POST https://api.deepseek.com/chat/completions ";
	command += "-H \"Content-Type: application/json\" ";
	command += "-H \"Authorization: Bearer ";
	command += DEEPSEEK_API_KEY;
	command += "\" ";
	command += "-d @" + requestFile;
	command += " -o " + responseFile;
	command += " --connect-timeout 30";
	command += " --max-time 60";
	
	// 执行命令
	int result = system(command.c_str());
	
	if (result != 0) {
		remove(requestFile.c_str());
		remove(responseFile.c_str());
		return "ERROR:curl执行失败，请检查网络";
	}
	
	// 读取响应
	std::string response;
	f = fopen(responseFile.c_str(), "r");
	if (!f) {
		remove(requestFile.c_str());
		remove(responseFile.c_str());
		return "ERROR:无法读取响应文件";
	}
	
	char buffer[4096];
	while (fgets(buffer, sizeof(buffer), f)) {
		response += buffer;
	}
	fclose(f);
	
	// 清理临时文件
	remove(requestFile.c_str());
	remove(responseFile.c_str());
	
	// 检查响应是否为空
	if (response.empty()) {
		return "ERROR:API返回空响应";
	}
	
	return response;
}

// ========== DeepSeek对话界面 ==========

// DeepSeek对话界面
void deepseekChat() {
	system("cls");
	
	printf("╔══════════════════════════════════════════════════════════╗\n");
	printf("║                    \033[34mDeepSeek AI 助手\033[0m                       ║\n");
	printf("║══════════════════════════════════════════════════════════║\n");
	printf("║  \033[33m说明：\033[0m直接输入问题开始对话，输入 \033[31mexit\033[0m 退出               ║\n");
	printf("║  \033[33m提示：\033[0m每次提问独立，不保存历史记录                         ║\n");
	printf("║  \033[33m要求：\033[0m需要联网，Windows 10/11 自带curl                      ║\n");
	printf("╚══════════════════════════════════════════════════════════╝\n\n");
	
	// 检查curl是否可用
	printf("正在检查网络连接...\n");
	if (system("curl --version > nul 2>&1") != 0) {
		printf("\033[31m[错误] 未找到curl命令！\033[0m\n");
		printf("Windows 10/11 应该自带curl，如果确实没有，请安装curl\n");
		printf("或使用Windows 10 1803以上版本\n");
		printf("\n按任意键返回...");
		_getch();
		return;
	}
	
	// 测试网络连接
	printf("正在测试网络连接...\n");
	if (system("ping -n 1 api.deepseek.com > nul 2>&1") != 0) {
		printf("\033[33m[警告] 无法连接到api.deepseek.com，请检查网络\033[0m\n");
		printf("按任意键继续尝试...");
		_getch();
	}
	
	printf("\n\033[32m准备就绪！开始对话吧~\033[0m\n\n");
	
	while (true) {
		printf("\033[32m[你] \033[0m");
		
		// 读取用户输入
		char input[2048];
		if (!fgets(input, sizeof(input), stdin)) {
			break;
		}
		
		// 去除换行符
		input[strcspn(input, "\n")] = 0;
		std::string userInputGbk(input);
		
		// 去除首尾空格
		userInputGbk.erase(0, userInputGbk.find_first_not_of(" \t"));
		userInputGbk.erase(userInputGbk.find_last_not_of(" \t") + 1);
		
		// 检查退出
		if (userInputGbk == "exit" || userInputGbk == "退出" || userInputGbk == "q") {
			printf("\n\033[33m[系统] 已退出DeepSeek对话模式\033[0m\n");
			break;
		}
		
		if (userInputGbk.empty()) {
			continue;
		}
		
		// 显示思考动画
		printf("\033[34m[DeepSeek] \033[0m正在思考");
		for(int i = 0; i < 3; i++) {
			printf(".");
			fflush(stdout);
			Sleep(300);
		}
		printf("\r\033[K");
		
		// GBK转UTF-8发送给API
		std::string userInputUtf8 = gbkToUtf8(userInputGbk);
		
		// 调用API
		std::string response = callDeepSeek(userInputUtf8);
		
		// 检查是否出错
		if (response.find("ERROR:") == 0) {
			printf("\033[31m[错误] %s\033[0m\n", response.c_str());
			printf("请检查：\n");
			printf("  1. API Key是否正确\n");
			printf("  2. 网络连接是否正常\n");
			printf("  3. API账户是否有余额\n\n");
		} else {
			// 提取回复内容
			std::string replyUtf8 = extractContent(response);
			if (!replyUtf8.empty()) {
				// UTF-8转GBK显示
				std::string replyGbk = utf8ToGbk(replyUtf8);
				printf("\033[34m[DeepSeek] \033[0m%s\n\n", replyGbk.c_str());
			} else {
				printf("\033[31m[错误] 解析响应失败\033[0m\n");
				// 调试模式（取消注释可查看原始响应）
				// std::string responseGbk = utf8ToGbk(response);
				// printf("原始响应: %s\n", responseGbk.c_str());
			}
		}
		
		printf("\n");
	}
}
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
/*void RunBatInSourceFolderSimple() {
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
}*/
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
//string password(){
//	//to get password
//}
void cmd()
{
	printf("     ---------------------------------------------\n");
	printf("     |               \033[32mjnuo_tool 2.0.0\033[0m             |\n");
	printf("     |       -----------------------------       |\n");
	printf("     | \033[31m1. 杀死极域\033[0m  \033[34m2. deepseek\033[0m  \033[35m3. 把zip藏进图片\033[0m|\n");
	printf("     | \033[36m4. 对拍\033[0m  \033[90m5. miHoYo\033[0m  \033[91m6. 防窥屏\033[0m  \033[95m7. 暂无功能 \033[0m|\n");
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
		else if(n == 50) {  // 功能2: DeepSeek
			if(!flag2) {
				flag2 = 1;
				fc_using[2] = 1;
				log(2);
				
				// 设置控制台编码
				system("chcp 65001 > nul");
				
				// 启动DeepSeek对话
				deepseekChat();
				
				// 恢复控制台编码
				system("chcp 936 > nul");
				
				// 询问是否关闭功能
				printf("\n是否关闭DeepSeek功能？[y/n]: ");
				int s = _getch();
				if(s == 'y' || s == 'Y') {
					flag2 = 0;
					fc_using[2] = 2;
					log(2);
					printf("DeepSeek功能已关闭\n");
				} else {
					printf("DeepSeek功能继续运行\n");
				}
				Sleep(1000);
			} else {
				printf("此功能正在运行中，是否已经关闭？[\033[31my\033[0m/\033[33mn\033[0m]");
				int s = _getch();
				if(s == 'y' || s == 'Y') {
					flag2 = 0;
					fc_using[2] = 2;
					log(2);
				}
				continue;
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
//		else if(n=='7') {
//			RunBatInSourceFolderSimple();
//		}
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
