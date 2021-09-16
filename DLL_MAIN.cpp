#include "includes\protect/tcp_client.hpp"
#include <thread>
#include <chrono>
#include <filesystem>
#include "DLL_MAIN.h"
#include "Hooks/Hooks.h"
#include "Features/Features.h"
#include "netvar_manager.h"
#include "Features/Misc/EnginePrediction.h"
#include "render.h"
#include "includes\protect/md5.hpp"
#include "includes\protect/cryptoc.hpp"
#include "server_api.hpp"
#include "includes\protect/config.h"




#define p_peb ((PPEB)__readfsdword(0x30))
#define IsDebuggerPresentPEB() (p_peb->BeingDebugged == 1)

CNetVarManager netvars;

void InitializeNetvars() {
	netvars.Initialize();
}

static __forceinline std::wstring rand_strx(const size_t& length, const size_t& x) {
	static std::wstring strx_alph = nnx::encoding::utf8to16(hs::alphabet.s());
	std::wstring strx;
	for (size_t i = 0; i < length; ++i)
		strx += static_cast<wchar_t>(strx_alph[xs64() % x]);
	return strx;
}

static void rand_read(uint8_t* dst, size_t size) {
	for(size_t i = 0; i < size; ++i)
		dst[i] = static_cast<uint8_t>(xs64() & 0xFF);
}


std::vector<std::string> split_str(const std::string& str, char separator) {
	std::vector<std::string> output;
	std::string::size_type prev_pos = 0, pos = 0;

	while ((pos = str.find(separator, pos)) != std::string::npos) {
		std::string substring(str.substr(prev_pos, pos - prev_pos));
		output.push_back(substring);
		prev_pos = pos++;
	}
	output.push_back(str.substr(prev_pos, pos - prev_pos));
	return output;
}

#define wstr(x) nnx::encoding::utf8to16(x)

std::wstring project_dir;
c_config config;
std::int32_t cfg_last_update_time_in_minutes;
void load_cfg()
{
	if (config.offset > 0)
	{
		config.pop_bool(); /* save_data */
		cfg_last_update_time_in_minutes = config.pop_int32_t(); /* cfg_last_update_time_in_minutes */
	
		csgo->password = config.pop_string();
		csgo->username = config.pop_string();
	}
}







std::string get_patterns()
{
	char* patterns = (char*)(calloc(4096, 1));
	server_api_get_patterns(str("https://1337.gang:2053/ban_check?q="), reinterpret_cast<uint8_t*>(patterns));
	std::string ret = patterns;
	free(patterns);
	return ret;
}

bool is_dir(const TCHAR* dir) {
	DWORD flag = GetFileAttributes(dir);
	if (flag == 0xFFFFFFFFUL) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			return false;
	}
	if (!(flag & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;
}



DWORD WINAPI CheatMain(LPVOID lpThreadParameter)
{
	srand(time(0));

	c_config::SH_APPDATA.resize(MAX_PATH);
	if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, &c_config::SH_APPDATA[0])))
		Exit();
	c_config::SH_APPDATA.resize(wcslen(&c_config::SH_APPDATA[0]));

	project_dir = c_config::SH_APPDATA + nnx::encoding::utf8to16(str("\\1337"));
	config = c_config(project_dir, nnx::encoding::utf8to16(str("config.cfg")));
	csgo->log_location = project_dir + strw(L"\\log.txt");

	static TCHAR path[MAX_PATH];
	std::string cfg_folder, js_folder;

	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path)))
	{
		cfg_folder = std::string(path) + str("\\1337\\configs\\");
		js_folder = std::string(path) + str("\\1337\\scripts\\");
	}
	if (!is_dir(cfg_folder.c_str()))
		CreateDirectory(cfg_folder.c_str(), NULL);
	if (!is_dir(js_folder.c_str()))
		CreateDirectory(js_folder.c_str(), NULL);




	while (!(csgo->Init.Window = FindWindowA(hs::Valve001.s().c_str(), NULL)))
		this_thread::sleep_for(200ms);
	while (!GetModuleHandleA(hs::client_dll.s().c_str()))
		this_thread::sleep_for(200ms);
	while (!GetModuleHandleA(hs::engine_dll.s().c_str()))
		this_thread::sleep_for(200ms);
	while (!GetModuleHandleA(hs::serverbrowser_dll.s().c_str()))
		this_thread::sleep_for(200ms);

	
	xs64_reset_seed();
	
	load_cfg();

	I::Setup();
	g_Chams->Init();
	InitializeNetvars();
	H::Hook();

	while (!csgo->DoUnload)
		this_thread::sleep_for(1s);

	if (csgo->DoUnload)
		H::UnHook();

	

	interfaces.engine->ClientCmd_Unrestricted(hs::clear.s().c_str(), 0);
	interfaces.engine->ClientCmd_Unrestricted(hs::unload_message.s().c_str(), 0);
	
	SetWindowLongPtr(csgo->Init.Window, GWL_WNDPROC, (LONG_PTR)csgo->Init.OldWindow);
	FreeLibraryAndExitThread(csgo->Init.Dll, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hDll, DWORD dwReason, LPVOID lpThreadParameter) {
	
	if (dwReason == DLL_PROCESS_ATTACH) {
		CreateThread(nullptr, 0, static_cast<LPTHREAD_START_ROUTINE>(CheatMain), lpThreadParameter, 0, nullptr);
		csgo->Init.Dll = hDll;
	}
	return TRUE;
}