#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
#include<shellapi.h>
using namespace std;
int fc_using[8]={0};
bool program_running = true;
// ==================== DeepSeek API 简化版（使用curl命令） ====================
// 不需要额外头文件，不需要链接任何库！

// 替换为动态变量：
char DEEPSEEK_API_KEY[256] = {0};  // 存储API Key

// ========== 编码转换函数 ==========

// GBK转UTF-8（Windows API可靠版）
std::string gbkToUtf8(const std::string& gbkStr) {
	if (gbkStr.empty()) return "";
	
	// GBK转Unicode
	int unicodeLen = MultiByteToWideChar(936, 0, gbkStr.c_str(), -1, NULL, 0);
	if (unicodeLen <= 0) return gbkStr;
	
	wchar_t* wideStr = new wchar_t[unicodeLen];
	MultiByteToWideChar(936, 0, gbkStr.c_str(), -1, wideStr, unicodeLen);
	
	// Unicode转UTF-8
	int utf8Len = WideCharToMultiByte(65001, 0, wideStr, -1, NULL, 0, NULL, NULL);
	if (utf8Len <= 0) {
		delete[] wideStr;
		return gbkStr;
	}
	
	char* utf8Str = new char[utf8Len];
	WideCharToMultiByte(65001, 0, wideStr, -1, utf8Str, utf8Len, NULL, NULL);
	
	std::string result(utf8Str);
	delete[] wideStr;
	delete[] utf8Str;
	
	return result;
}

// UTF-8转GBK（Windows API可靠版）
std::string utf8ToGbk(const std::string& utf8Str) {
	if (utf8Str.empty()) return "";
	
	// UTF-8转Unicode
	int unicodeLen = MultiByteToWideChar(65001, 0, utf8Str.c_str(), -1, NULL, 0);
	if (unicodeLen <= 0) {
		// 转换失败，可能输入不是UTF-8，直接返回原字符串
		return utf8Str;
	}
	
	wchar_t* wideStr = new wchar_t[unicodeLen];
	MultiByteToWideChar(65001, 0, utf8Str.c_str(), -1, wideStr, unicodeLen);
	
	// Unicode转GBK
	int gbkLen = WideCharToMultiByte(936, 0, wideStr, -1, NULL, 0, NULL, NULL);
	if (gbkLen <= 0) {
		delete[] wideStr;
		return utf8Str;
	}
	
	char* gbkStr = new char[gbkLen];
	WideCharToMultiByte(936, 0, wideStr, -1, gbkStr, gbkLen, NULL, NULL);
	
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
// API Key 加密密钥（与使用次数密钥不同，增加安全性）
const unsigned char API_KEY_ENCRYPT_KEY = 0xA3;

bool saveApiKey(const std::string& apiKey) {
	if (apiKey.empty()) return false;
	
	// 确保source目录存在
	CreateDirectoryA("source", NULL);
	
	// 创建或覆盖文件
	FILE* f = fopen("source/jnuo_tool.jnuofile", "wb");
	if (!f) return false;
	
	// 写入文件头 "JNUO"（4字节标识）
	const unsigned char header[4] = {'J', 'N', 'U', 'O'};
	fwrite(header, 1, 4, f);
	
	// 写入版本号（1字节）
	unsigned char version = 1;
	fwrite(&version, 1, 1, f);
	
	// 写入API Key长度（2字节）
	unsigned short len = (unsigned short)apiKey.length();
	fwrite(&len, 1, 2, f);
	
	// 加密并写入API Key
	for (size_t i = 0; i < apiKey.length(); i++) {
		unsigned char encrypted = apiKey[i] ^ API_KEY_ENCRYPT_KEY;
		// 使用位置相关的异或增加安全性
		encrypted ^= (unsigned char)(i % 256);
		fwrite(&encrypted, 1, 1, f);
	}
	
	// 计算并写入校验和（1字节）
	unsigned char checksum = 0;
	for (char c : apiKey) {
		checksum ^= c;
	}
	fwrite(&checksum, 1, 1, f);
	
	// 随机填充数据伪装（20字节）
	for (int i = 0; i < 20; i++) {
		unsigned char padding = rand() % 256;
		fwrite(&padding, 1, 1, f);
	}
	
	fclose(f);
	
	// 更新内存中的Key
	strncpy(DEEPSEEK_API_KEY, apiKey.c_str(), sizeof(DEEPSEEK_API_KEY) - 1);
	DEEPSEEK_API_KEY[sizeof(DEEPSEEK_API_KEY) - 1] = '\0';
	
	return true;
}
// 解密并读取API Key
std::string loadApiKey() {
	FILE* f = fopen("source/jnuo_tool.jnuofile", "rb");
	if (!f) {
		// 文件不存在是正常的（首次使用），不输出错误信息
		return "";
	}
	
	// 验证文件头
	char header[5] = {0};
	if (fread(header, 1, 4, f) != 4) {
		fclose(f);
		return "";
	}
	
	// 检查是否是"JNUO"标识
	if (header[0] != 'J' || header[1] != 'N' || 
		header[2] != 'U' || header[3] != 'O') {
		fclose(f);
		return "";
	}
	
	// 读取版本号
	unsigned char version;
	if (fread(&version, 1, 1, f) != 1) {
		fclose(f);
		return "";
	}
	
	// 读取长度
	unsigned short len = 0;
	if (fread(&len, 1, 2, f) != 2) {
		fclose(f);
		return "";
	}
	
	// 限制长度防止异常（DeepSeek API Key通常是30-50字符）
	if (len == 0 || len > 200) {
		fclose(f);
		return "";
	}
	
	// 读取并解密API Key
	char* encrypted = new char[len];
	if (fread(encrypted, 1, len, f) != len) {
		delete[] encrypted;
		fclose(f);
		return "";
	}
	
	std::string apiKey;
	for (size_t i = 0; i < len; i++) {
		char decrypted = encrypted[i] ^ API_KEY_ENCRYPT_KEY;
		decrypted ^= (unsigned char)(i % 256);
		apiKey += decrypted;
	}
	delete[] encrypted;
	
	// 读取校验和
	unsigned char savedChecksum;
	if (fread(&savedChecksum, 1, 1, f) != 1) {
		fclose(f);
		return "";
	}
	
	fclose(f);
	
	// 验证校验和
	unsigned char calculatedChecksum = 0;
	for (char c : apiKey) {
		calculatedChecksum ^= c;
	}
	
	if (calculatedChecksum != savedChecksum) {
		return "";  // 校验失败
	}
	
	return apiKey;
}
// 初始化API Key（程序启动时调用）
void initApiKey() {
	std::string savedKey = loadApiKey();
	if (!savedKey.empty()) {
		strncpy(DEEPSEEK_API_KEY, savedKey.c_str(), sizeof(DEEPSEEK_API_KEY) - 1);
		DEEPSEEK_API_KEY[sizeof(DEEPSEEK_API_KEY) - 1] = '\0';
	}
}
// 从JSON响应中提取content内容
// 正确的JSON解析函数
// 超简单版 - 直接搜索 "content":" 取后面的内容
std::string extractContent(const std::string& json) {
	// 方法1: 搜索 "content":"
	std::string searchStr = "\"content\":\"";
	size_t startPos = json.find(searchStr);
	
	if (startPos == std::string::npos) {
		// 方法2: 搜索 "content": "（带空格）
		searchStr = "\"content\": \"";
		startPos = json.find(searchStr);
	}
	
	if (startPos == std::string::npos) {
		// 方法3: 搜索 "content":"（不带转义）
		searchStr = "\"content\":\"";
		startPos = json.find(searchStr);
	}
	
	if (startPos == std::string::npos) {
		// 方法4: 处理Unicode转义的情况
		// 搜索完整的content字段，包括可能的空格
		const char* patterns[] = {
			"\"content\":\"",
			"\"content\": \"",
			"\"content\" :\"",
			"\"content\" : \""
		};
		
		for (const char* pattern : patterns) {
			startPos = json.find(pattern);
			if (startPos != std::string::npos) {
				searchStr = pattern;
				break;
			}
		}
	}
	
	if (startPos == std::string::npos) {
		return ""; // 所有方法都失败
	}
	
	startPos += searchStr.length();
	
	// 找到结束的引号（正确处理转义）
	size_t endPos = startPos;
	while (endPos < json.length()) {
		if (json[endPos] == '\\') {
			// 跳过转义字符
			endPos += 2;
			continue;
		}
		if (json[endPos] == '"') {
			break;
		}
		endPos++;
	}
	
	if (endPos >= json.length()) {
		return "";
	}
	
	// 提取内容（可能是UTF-8编码的）
	std::string rawContent = json.substr(startPos, endPos - startPos);
	
	// 处理Unicode转义序列（如\uXXXX）
	std::string content;
	for (size_t i = 0; i < rawContent.length(); i++) {
		if (rawContent[i] == '\\' && i + 1 < rawContent.length()) {
			char next = rawContent[i + 1];
			switch (next) {
				case 'n': content += '\n'; i++; break;
				case 'r': content += '\r'; i++; break;
				case 't': content += '\t'; i++; break;
				case '\\': content += '\\'; i++; break;
				case '"': content += '"'; i++; break;
				case 'u': {
					// 处理Unicode转义序列
					if (i + 5 < rawContent.length()) {
						std::string hexStr = rawContent.substr(i + 2, 4);
						unsigned int codePoint;
						std::stringstream ss;
						ss << std::hex << hexStr;
						ss >> codePoint;
						
						// 转换为UTF-8
						if (codePoint < 0x80) {
							content += static_cast<char>(codePoint);
						} else if (codePoint < 0x800) {
							content += static_cast<char>(0xC0 | (codePoint >> 6));
							content += static_cast<char>(0x80 | (codePoint & 0x3F));
						} else {
							content += static_cast<char>(0xE0 | (codePoint >> 12));
							content += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
							content += static_cast<char>(0x80 | (codePoint & 0x3F));
						}
						i += 5;
					} else {
						content += rawContent[i];
					}
					break;
				}
				default: content += next; i++; break;
			}
		} else {
			content += rawContent[i];
		}
	}
	
	// 如果是UTF-8，转换为GBK
	// 检查是否已经是GBK（或者不需要转换）
	return utf8ToGbk(content);
}

// 提示用户输入API Key的界面
void setupApiKey() {
	system("cls");
	
	printf("\n");
	printf("+----------------------------------------------------------+\n");
	printf("|                 [ API Key 设置 ]                        |\n");
	printf("+----------------------------------------------------------+\n");
	printf("|  请粘贴您的DeepSeek API Key：                            |\n");
	printf("|  (格式：sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxx)                |\n");
	printf("+----------------------------------------------------------+\n");
	printf("|  【重要提示】                                            |\n");
	printf("|  jnuo 不会获取、记录或上传您的 API Key                   |\n");
	printf("|  API Key 将加密保存在本地的 jnuo_tool.jnuofile 文件中    |\n");
	printf("|  请放心使用！                                            |\n");
	printf("+----------------------------------------------------------+\n");
	printf("\nAPI Key: ");
	
	char input[256];
	fgets(input, sizeof(input), stdin);
	input[strcspn(input, "\n")] = 0;  // 去除换行符
	
	std::string apiKey(input);
	
	// 去除首尾空格
	apiKey.erase(0, apiKey.find_first_not_of(" \t"));
	apiKey.erase(apiKey.find_last_not_of(" \t") + 1);
	
	if (apiKey.empty()) {
		printf("\n[错误] API Key 不能为空！\n");
		system("pause");
		return;
	}
	
	// 基本格式验证
	if (apiKey.length() < 10 || apiKey.substr(0, 3) != "sk-") {
		printf("\n[警告] API Key 格式可能不正确（应以 sk- 开头）\n");
		printf("确认保存？[y/n]: ");
		int s = _getch();
		if (s != 'y' && s != 'Y') {
			printf("\n已取消\n");
			system("pause");
			return;
		}
	}
	
	// 保存加密的API Key
	if (saveApiKey(apiKey)) {
		// 更新内存中的Key
		strncpy(DEEPSEEK_API_KEY, apiKey.c_str(), sizeof(DEEPSEEK_API_KEY) - 1);
		DEEPSEEK_API_KEY[sizeof(DEEPSEEK_API_KEY) - 1] = '\0';
		
		printf("\n[成功] API Key 已加密保存！\n");
		printf("文件位置：source/jnuo_tool.jnuofile\n");
		printf("如需重新设置，请选择菜单中的重置选项\n");
	} else {
		printf("\n[错误] 保存失败，请检查磁盘空间或权限\n");
	}
	
	system("pause");
}

// ========== DeepSeek API调用（使用curl命令） ==========

// 调用DeepSeek API（使用系统curl）
// 调用DeepSeek API（使用系统curl）
std::string callDeepSeek(const std::string& userMessage) {
	// 用户消息已经是GBK，需要转成UTF-8发送给API
	std::string userInputUtf8 = gbkToUtf8(userMessage);
	
	// 构建JSON请求体
	std::string requestBody = "{";
	requestBody += "\"model\": \"deepseek-v4-flash\",";
	requestBody += "\"messages\": [";
	requestBody += "{\"role\": \"user\", \"content\": \"" + escapeJsonString(userInputUtf8) + "\"}";
	requestBody += "],";
	requestBody += "\"stream\": false,";
	requestBody += "\"temperature\": 0.7";
	requestBody += "}";
	
	// 创建临时文件
	std::string requestFile = "temp_req_" + std::to_string(GetCurrentProcessId()) + ".txt";
	std::string responseFile = "temp_res_" + std::to_string(GetCurrentProcessId()) + ".txt";
	
	// 写入请求内容（UTF-8格式）
	FILE* f = fopen(requestFile.c_str(), "wb, ccs=UTF-8");
	if (!f) {
		// 如果上面的方式失败，用普通方式
		FILE* f = fopen(requestFile.c_str(), "wb");  // 直接用二进制模式
		if (!f) {
			return "ERROR:无法创建请求文件";
		}
		fwrite(requestBody.c_str(), 1, requestBody.length(), f);
		fclose(f);
	} else {
		fwrite(requestBody.c_str(), 1, requestBody.length(), f);
		fclose(f);
	}
	
	// 构建curl命令 - 使用和你测试成功一样的格式
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
	
	// 读取响应（直接读取，不转换）
	std::string response;
	f = fopen(responseFile.c_str(), "rb");
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


void deepseekChat() {
	system("cls");
	
	// 检查是否已设置API Key
	if (strlen(DEEPSEEK_API_KEY) == 0) {
		printf("\n");
		printf("+----------------------------------------------------------+\n");
		printf("|                 [ API Key 未设置 ]                       |\n");
		printf("+----------------------------------------------------------+\n");
		printf("|  您还没有设置DeepSeek API Key                            |\n");
		printf("|  请先在主菜单按 \"k\" 设置您的API Key                      |\n");
		printf("|  您的API Key会被加密保存在本地，jnuo无法获取             |\n");
		printf("+----------------------------------------------------------+\n");
		printf("\n[提示] 按任意键返回主菜单...\n");
		_getch();
		return;
	}
	
	// 启用ANSI转义序列支持
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hConsole, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hConsole, dwMode);
	SetConsoleOutputCP(936);
	
	// 显示界面
	printf("\033[36m============================================================\033[0m\n");
	printf("\033[36m||\033[0m                    \033[33mDeepSeek AI \033[35m助\033[32m手\033[36m\033[0m                    \033[36m||\033[0m\n");
	printf("\033[36m||========================================================||\033[0m\n");
	printf("\033[36m||\033[0m  \033[90m[说明]\033[0m \033[37m直接输入问题开始对话，输入 \033[33mexit\033[37m 退出\033[0m           \033[36m||\033[0m\n");
	printf("\033[36m||\033[0m  \033[90m[提示]\033[0m \033[37m使用您自己的API Key，无使用次数限制\033[0m            \033[36m||\033[0m\n");
	printf("\033[36m============================================================\033[0m\n\n");
	
	while (true) {
		printf("\033[92m[你] \033[0m");
		
		char input[2048];
		if (!fgets(input, sizeof(input), stdin)) break;
		
		input[strcspn(input, "\n")] = 0;
		std::string userInput(input);
		
		if (userInput == "exit" || userInput == "q") {
			printf("\n\033[93m[系统] 已退出，感谢使用\033[0m\n");
			break;
		}
		
		if (userInput.empty()) continue;
		
		printf("\033[90m[DeepSeek] 思考中...\r\033[0m");
		fflush(stdout);
		
		std::string response = callDeepSeek(userInput);
		
		if (response.find("ERROR:") == 0) {
			printf("\033[91m[错误] %s\033[0m\n", response.c_str());
			printf("\033[90m[提示] 请检查网络连接或API Key是否正确\033[0m\n\n");
		} else {
			std::string reply = extractContent(response);
			if (!reply.empty()) {
				printf("\033[96m[DeepSeek]\033[0m \033[37m%s\033[0m\n\n", reply.c_str());
			} else {
				printf("\033[91m[错误] 解析响应失败，请重试\033[0m\n\n");
			}
		}
	}
}

// 新增：查看当前API Key状态
void showApiKeyStatus() {
	system("cls");
	printf("\n");
	printf("+----------------------------------------------------------+\n");
	printf("|                  [ API Key 状态 ]                        |\n");
	printf("+----------------------------------------------------------+\n");
	
	if (strlen(DEEPSEEK_API_KEY) == 0) {
		printf("|  状态：未设置                                            |\n");
	} else {
		// 只显示部分Key，保护隐私
		char maskedKey[256];
		strncpy(maskedKey, DEEPSEEK_API_KEY, sizeof(maskedKey));
		int len = strlen(maskedKey);
		if (len > 10) {
			// 保留前7位和后4位，中间用****替代
			char temp[256];
			snprintf(temp, sizeof(temp), "%.7s**********%s", maskedKey, maskedKey + len - 4);
			printf("|  状态：已设置                                            |\n");
			printf("|  Key：%s                              |\n", temp);
		} else {
			printf("|  状态：已设置                                            |\n");
			printf("|  Key：%s                              |\n", "**********");  // 太短的key完全隐藏
		}
	}
	printf("|                                                          |\n");
	printf("|  您的API Key加密保存在 source/jnuo_tool.jnuofile         |\n");
	printf("|  jnuo 无法获取您的API Key                                |\n");
	printf("+----------------------------------------------------------+\n");
	printf("\n");
	
	if (strlen(DEEPSEEK_API_KEY) > 0) {
		printf("是否要重新设置API Key？[y/n]: ");
		int s = _getch();
		if (s == 'y' || s == 'Y') {
			setupApiKey();
		}
	} else {
		printf("是否要设置API Key？[y/n]: ");
		int s = _getch();
		if (s == 'y' || s == 'Y') {
			setupApiKey();
		}
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
	printf("     |               \033[32mjnuo_tool 2.1.0\033[0m             |\n");
	printf("     |       -----------------------------       |\n");
	printf("     | \033[31m1. 杀死极域\033[0m  \033[34m2. deepseek\033[0m  \033[35m3. 把zip藏进图片\033[0m|\n");
	printf("     | \033[36m4. 对拍\033[0m  \033[90m5. miHoYo\033[0m  \033[91m6. 防窥屏\033[0m  \033[95m7. 暂无功能\033[0m|\n");
	printf("     |       -----------------------------       |\n");
	printf("     |                  \033[33mq. 退出\033[0m                  |\n");
	printf("     |       -----------------------------       |\n");
	printf("     |       \033[33mx. 这次更新\033[0m    \033[33mr. 查看使用记录\033[0m      |\n");
	printf("     |     \033[33mk. 设置API Key\033[0m   \033[33ml. 查看API Key状态\033[0m   |\n");  // 新增
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
{	system("title jnuo_tool 2.1.0"); 
	srand((unsigned int)time(NULL));
	//resetUsageCount();
	//fake_cmd();
	initApiKey();
	system("color A"); 
	start();
	system("color 7");
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	log_program_start();
	bool flag2=0,flag5=0,flag6=0;
	while(1)
	{   system("cls");
		cmd(); 
//		printf("“deepseek”运行状态：%d\n",flag2); 
//		printf("“miHoYo”运行状态：%d\n",flag5); 
//		printf("“防窥屏”运行状态：%d\n",flag6); 
		int n=_getch();
		if(n==113||n==81)  break;
		else if(n=='k'||n=='K') {
			setupApiKey();
			continue;
		}
		else if(n=='l'||n=='L') showApiKeyStatus();
		else if(n==106||n==74) system("start https://www.bilibili.com/video/BV1GJ411x7h7");
		else if(n=='r'||n=='R') {
			cout<<"\n"; 
			read_log_detailed();
			continue;
		}
		else if(n==120||n==88){
			printf("\033[32mjnuo_tool 2.1.0更新内容\033[0m\n"); 
			printf("1. 接入了deepseek，并生成了必要的DLL文件（自动打开）\n");
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
				
				// 启动DeepSeek对话
				deepseekChat();
				
				
				// 询问是否关闭功能
				printf("\n是否关闭DeepSeek功能？[y/n]: ");
				int s = _getch();
				if(s == 'y' || s == 'Y') {
					flag2 = 0;
					fc_using[2] = 2;
					log(2);
					printf("DeepSeek功能已关闭\n");
					continue;
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
