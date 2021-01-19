#include "winapi-tools.h"

#include <Shlwapi.h>
#include <UIAutomation.h>

#include <algorithm>
#include <vector>

namespace keyboard_auto_type {

std::string winapi_to_string(LPCWSTR api_str) {
    if (!api_str) {
        return "";
    }
    auto size = WideCharToMultiByte(CP_UTF8, 0, api_str, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return "";
    }
    std::vector<char> chars(size);
    WideCharToMultiByte(CP_UTF8, 0, api_str, -1, chars.data(), size, nullptr, nullptr);
    return chars.data();
}

bool includes_case_insensitive(std::string_view str, std::string_view search) {
    return StrStrIA(str.data(), search.data());
}

std::string native_window_text(HWND hwnd) {
    auto length = GetWindowTextLength(hwnd);
    if (length) {
        std::vector<WCHAR> chars(length + 1);
        if (GetWindowText(hwnd, chars.data(), static_cast<int>(chars.size()))) {
            return winapi_to_string(chars.data());
        }
    }
    return "";
}

std::string native_process_main_module_name(DWORD pid) {
    std::string name;
    auto hprocess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid);
    if (hprocess) {
        std::vector<WCHAR> exe_name_chars(MAX_PATH + 1);
        auto size = static_cast<DWORD>(exe_name_chars.size());
        if (QueryFullProcessImageName(hprocess, 0, exe_name_chars.data(), &size) && size > 0) {
            name = winapi_to_string(exe_name_chars.data());
        }
        CloseHandle(hprocess);
    }
    return name;
}

std::string native_browser_url(HWND hwnd) {
    auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
        return "";
    }

    std::vector<IUnknown *> to_release;

    IUIAutomation *client = nullptr;
    hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation,
                          reinterpret_cast<void **>(&client));
    if (hr != S_OK) {
        return "";
    }
    to_release.push_back(client);

    IUIAutomationElement *element = nullptr;
    hr = client->ElementFromHandle(hwnd, &element);
    if (hr != S_OK) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }
    to_release.push_back(element);

    IUIAutomationCondition *condition_property = nullptr;
    VARIANT var_type_id = {VT_I4};
    var_type_id.uintVal = UIA_EditControlTypeId;
    hr = client->CreatePropertyCondition(UIA_ControlTypePropertyId, var_type_id,
                                         &condition_property);
    if (hr != S_OK) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }
    to_release.push_back(condition_property);

    IUIAutomationCondition *condition_value = nullptr;
    VARIANT var_true = {VT_BOOL};
    var_true.boolVal = VARIANT_TRUE;
    hr = client->CreatePropertyCondition(UIA_IsValuePatternAvailablePropertyId, var_true,
                                         &condition_value);
    if (hr != S_OK) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }
    to_release.push_back(condition_value);

    IUIAutomationCondition *condition = nullptr;
    hr = client->CreateAndCondition(condition_property, condition_value, &condition);
    if (hr != S_OK) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }
    to_release.push_back(condition);

    IUIAutomationElement *found_element = nullptr;
    hr = element->FindFirst(TreeScope::TreeScope_Descendants, condition, &found_element);
    if (hr != S_OK || !found_element) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }
    to_release.push_back(found_element);

    IUIAutomationValuePattern *pattern = nullptr;
    hr = found_element->GetCurrentPattern(UIA_ValuePatternId,
                                          reinterpret_cast<IUnknown **>(&pattern));
    if (hr != S_OK || !pattern) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }
    to_release.push_back(pattern);

    BSTR current_value = nullptr;
    hr = pattern->get_CurrentValue(&current_value);
    if (hr != S_OK || !current_value) {
        std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });
        return "";
    }

    auto url = winapi_to_string(current_value);

    SysFreeString(current_value);

    std::for_each(to_release.begin(), to_release.end(), [](auto i) { i->Release(); });

    return url;
}

} // namespace keyboard_auto_type
