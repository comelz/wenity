#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <wchar.h>

bool utf8in  = false;
bool utf8out = false;
enum Mode {
    ModeUnknown,
    ModeFileSelection
};
Mode mode = ModeUnknown;
std::wstring title;
std::wstring filename;
std::wstring initial_directory;
bool directory;
bool save;
std::wstring separator = L"|";
bool confirm_overwrite;
std::vector<std::wstring> file_filter;
bool multiple;

int hexdec(wchar_t ch) {
    if(L'0'<=ch && ch<=L'9') return ch-L'0';
    if(L'A'<=ch && ch<=L'F') return ch-L'A'+10;
    if(L'a'<=ch && ch<=L'f') return ch-L'a'+10;
    return -1;
}

void do_output(const std::wstring &s) {
    if(utf8out) {
        std::string utf8sz((s.size()+1)*4, 0);
        int sz = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.size(), &utf8sz[0], utf8sz.size(), 0, NULL);
        utf8sz.resize(sz);
        for(unsigned char ch: utf8sz) {
            if(ch>126 || ch<=32 || strchr("$&+,/:;=?@\"'<>#%{}|\\^~[]`", ch)) {
                printf("%%%02X", unsigned(ch));
            } else {
                putchar(ch);
            }
        }
    } else {
        fputws(s.c_str(), stdout);
    }
}

int my_wmain(int argc, wchar_t *argv[]);

int main() {
    wchar_t *args = GetCommandLineW();
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(args, &argc);
    return my_wmain(argc, argv);
}

int my_wmain(int argc, wchar_t *argv[]) {
    // normalize equal for long options
    // --title=abcd => --title abcd
    // detect UTF8+percent encoding flag
    std::vector<std::wstring> args;
    for(int i=1; i<argc; ++i) {
        wchar_t *a = argv[i];
        if(a[0]=='-' && a[1]=='-') {
            std::wstring a1;
            while(*a && *a!='=') {
                a1.push_back(*a);
                ++a;
            }
            if(a1==L"--utf8in") utf8in = true;
            else args.push_back(a1);
            if(*a) {
                args.push_back(a+1);
            }
        } else {
            args.push_back(a);
        }
    }
    if(utf8in) {
        // decode percent encoded UTF-8 to UTF-16
        for(auto &a: args) {
            if(a.empty()) continue;
            std::string utf8sz;
            bool allAscii = true;
            for(int i=0, n=a.size(); i<n; ++i) {
                if(a[i]==L'%' && i+2<n) {
                    int c1 = hexdec(a[i+1]);
                    int c2 = hexdec(a[i+2]);
                    if(c2>=0 && c2>=0) {
                        utf8sz.push_back(c1<<4 | c2);
                        i+=2;
                        continue;
                    }
                }
                wchar_t w = a[i];
                if(w > 126) allAscii = false;
                utf8sz.push_back(char(w));
            }
            // if there's any non-ASCII character here leave this parameter alone
            // it doesn't make sense to mix UTF-16 with percent-encoded UTF-8
            if(!allAscii) continue;
            // the original string should be big enough (UTF-16 is surely more compact
            // than UTF-8, even more if percent-encoded)
            // Convert UTF-8 to UTF-16
            int sz = MultiByteToWideChar(CP_UTF8, 0, utf8sz.c_str(), utf8sz.size(), &a[0], a.size());
            if(sz==0) fprintf(stderr, "Cannot convert UTF8 to UTF16: error %u", unsigned(GetLastError()));
            a.resize(sz);
        }
    }
    for(int i=0, n=args.size(); i<n; ++i) {
        std::wstring &a = args[i];
        auto next = [&] {
            if(i+1<n) {
                a = args[++i];
                return true;
            } else {
                fwprintf(stderr, L"Missing argument after `%s`\n", a.c_str());
                return false;
            }
        };
        if(a==L"--file-selection") {
            mode = ModeFileSelection;
        } else if(a==L"--title" && next()) {
            title = a;
        } else if(a==L"--filename" && next()) {
            filename = a;
        } else if(a==L"--separator" && next()) {
            separator = a;
        } else if(a==L"--file-filter" && next()) {
            file_filter.push_back(a);
        } else if(a==L"--multiple") {
            multiple = true;
        } else if(a==L"--directory") {
            directory = true;
        } else if(a==L"--save") {
            save = true;
        } else if(a==L"--initial-directory" && next()) {
            initial_directory = a;
        } else if(a==L"--confirm-overwrite") {
            confirm_overwrite = true;
        } else if(a==L"--utf8out") {
            utf8out = true;
        } else {
            fwprintf(stderr, L"Unrecognized option: `%ls`\n", a.c_str());
        }
    }

    std::vector<std::wstring> output;
    switch(mode) {
    case ModeUnknown:
        fputs("Missing mode switch\n", stderr);
        return 1;
    case ModeFileSelection:
    {
        std::wstring filter_str;
        for(auto &filt: file_filter) {
            const wchar_t *f = filt.c_str();
            while(*f && *f!=L'|') filter_str.push_back(*f++);
            if(*f!=L'|') {
                fwprintf(stderr, L"Invalid filter string: `%ls`\n", filt.c_str());
                return 1;
            }
            filter_str.push_back(0);
            ++f;
            while(*f) {
                while(*f && *f==L' ') ++f;
                while(*f && *f!=L' ') filter_str.push_back(*f++);
                filter_str.push_back(L';');
            }
            filter_str.push_back(0);
        }
        filter_str.push_back(0);
        OPENFILENAMEW OPF;
        ZeroMemory(&OPF, sizeof(OPF));
        OPF.lStructSize = sizeof(OPENFILENAME);
        OPF.hwndOwner = 0;
        OPF.lpstrFilter = filter_str.c_str();
        OPF.nFilterIndex = 1;
        filename.resize(32768);
        OPF.lpstrFile = &filename[0];
        OPF.nMaxFile = filename.size();
        if(initial_directory.size()) OPF.lpstrInitialDir = initial_directory.c_str();
        OPF.lpstrTitle = title.c_str();
        OPF.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_PATHMUSTEXIST;
        if(multiple) OPF.Flags |= OFN_ALLOWMULTISELECT;
        if(save) {
            if(confirm_overwrite)  OPF.Flags |= OFN_OVERWRITEPROMPT;
            if(!GetSaveFileNameW(&OPF)) {
                err = CommDlgExtendedError();
                if(err != 0) {
                    fprintf(stderr, "GetSaveFileName failed with error code %u\n", unsigned(err));
                    return 2;
                }
            }
        } else {
            OPF.Flags |= OFN_FILEMUSTEXIST;
            if(!GetOpenFileNameW(&OPF)) {
                err = CommDlgExtendedError();
                if(err != 0) {
                    fprintf(stderr, "GetOpenFileName failed with error code %u\n", unsigned(err));
                    return 2;
                }
            }
        }
        // Decode
        for(const wchar_t *ptr=filename.c_str();;)
        {
            std::wstring s(ptr);
            if(!s.size()) break;
            ptr+=s.size()+1;
            output.push_back(s);
        }

        if(output.size()>1)
        {
            // If there's more than an element, the first one is the directory,
            // the next ones are file names; fix everything up
            std::wstring prefix = output[0];
            if(prefix.size() && prefix.back()!=L'\\') {
                prefix.push_back(L'\\');
            }
            for(int i=1, n=output.size(); i<n; ++i)
                output[i-1] = prefix + output[i];
            output.pop_back();
        }
        break;
    }
    }

    // Output with the given delimiter
    bool first = true;
    for(auto &out: output) {
        if(!first) fputws(separator.c_str(), stdout);
        first = false;
        do_output(out);
    }
    // Be nice and always add a newline
    putchar('\n');
    return 0;
}
