#include "pch.h"

#include "sdk/cmd/cmd.h"

namespace utils {
   
    constexpr void* get_virtual(void* thisptr, size_t index) noexcept
    {
        return (*static_cast<void***>(thisptr))[index];
    }

    /* @xref: https://github.com/martinjanas/Sensum/blob/9aa495ff5b229ab307dec7f982b146eb15316ec4/src/helpers/utils.cpp#L498 */
    std::uint8_t* pattern_scan(void* module, const char* signature) noexcept
    {
        static auto pattern_to_byte = [](const char* pattern) {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for (auto current = start; current < end; ++current)
            {
                if (*current == '?')
                {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.emplace_back(-1);
                }
                else
                    bytes.emplace_back(strtoul(current, &current, 16));
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for (auto i = 0ul; i < sizeOfImage - s; ++i)
        {
            bool found = true;
            for (auto j = 0ul; j < s; ++j)
            {
                if (scanBytes[i + j] != d[j] && d[j] != -1) {
                    found = false;
                    break;
                }
            }

            if (found)
                return &scanBytes[i];
        }
        return nullptr;
    }

    /* @xref: https://github.com/martinjanas/Sensum/blob/9aa495ff5b229ab307dec7f982b146eb15316ec4/src/helpers/utils.cpp#L493 */
    std::uint8_t* pattern_scan(const char* moduleName, const char* signature) noexcept
    {
        return pattern_scan(LI_FN(LoadLibraryA).safe()(moduleName), signature);
    }

};

namespace hooks {

    VOID(_cdecl* console_print)(const char* format, ...);

    void* client_mode = nullptr;

    struct create_move
    {
        static const int index = 24;
        using fn = bool(__thiscall*)(void*, float, CUserCmd*);
        static bool __fastcall hooked(void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd);

        inline static fn original;
        inline static void* setup;
    };

    void initialize ( ) {

        /* @xres: print message when initializing starts.. */
        printf(xor_("[+] initializing started... [%p]\n"), hooks::initialize);

        console_print = (VOID(*)(const char*, ...))LI_FN(GetProcAddress).safe()(LI_FN(LoadLibraryA).safe()(xor_("tier0.dll")), xor_("Msg"));

        if (MH_Initialize() != MH_OK)
            printf(xor_("[-] cannnot initialize min_hook (!) [%p]"), initialize);

        client_mode = *reinterpret_cast<void**>(utils::pattern_scan(xor_("client.dll"), xor_("B9 ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 53")) + 1);

        create_move::setup = reinterpret_cast<void*>(utils::get_virtual(client_mode, create_move::index));
        if (MH_CreateHook(create_move::setup, &create_move::hooked, reinterpret_cast<void**>(&create_move::original)) != MH_OK)
            printf(xor_("[-] cannnot initialize create_move (!) [%p]"), initialize);


        if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
            printf(xor_("[-] cannnot enable min_hook (!) [%p]"), initialize);
    };
};

bool __fastcall hooks::create_move::hooked(void* ecx, void* edx, float input_sample_frametime, CUserCmd* cmd)
{
    // do something

    return hooks::create_move::original(ecx, input_sample_frametime, cmd);
}

auto init ( ) -> void {

    /* @xres: wait for serverbrowser.dll */
    /* @xref: https://learn.microsoft.com/en-us/windows/win32/dlls/run-time-dynamic-linking */
    while (!LI_FN(LoadLibraryA).safe()(xor_("serverbrowser.dll")))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    /* @xres: debug console */
    AllocConsole();
    if (!freopen(xor_("CONOUT$"), xor_("w"), stdout)) {
        FreeConsole();
        return;
    }

    /* @xres: initialize hooks */
    hooks::initialize();

    /* @xres: if initialization was correct print message */
    printf(xor_("[+] hooks we're initialized (!) [%p]\n"), hooks::initialize);

    /* @xres: if everything was correct print message */
    printf(xor_("[+] credit: github.com/sharkzxo (!) [%p]\n"), init);
};

BOOL APIENTRY DllMain ( HINSTANCE h_instance, DWORD reason_call, LPVOID lp_reserved ) {

    if (reason_call != DLL_PROCESS_ATTACH)
        return FALSE;

    /* @xref: https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-disablethreadlibrarycalls */
    LI_FN(DisableThreadLibraryCalls).safe()(h_instance);

    std::thread(init).detach();

    return TRUE;
}

