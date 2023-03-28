#include <stdio.h>
#include <Windows.h>

void memState(int state) {
    if (state == 0x2000)
        printf("MEM_RESERVE\n");
    else if (state == 0x10000)
        printf("MEM_FREE\n");
    else if (state == 0x1000)
        printf("MEM_COMMIT\n");
}

int main()
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    DWORD pageSize = sysInfo.dwPageSize;

    printf("Page Size: %d\n", (int)pageSize);

    LPVOID allocatedMem = VirtualAlloc(NULL, (500 * pageSize), MEM_RESERVE, PAGE_NOACCESS);
    if (allocatedMem == NULL) {
        printf("GlobalAlloc failed - %d\n", GetLastError());
        return 0;
    }

    LPTSTR nextPage;
    LPTSTR memPtr = (LPTSTR)allocatedMem;

    MEMORY_BASIC_INFORMATION memBasicInfo;
    SIZE_T query = VirtualQuery(allocatedMem, &memBasicInfo, sizeof(memBasicInfo));
    printf("Allocated Mem State: ");  
    memState((int)memBasicInfo.State);

    VirtualFree(allocatedMem, 0, MEM_RELEASE);

    query = VirtualQuery(allocatedMem, &memBasicInfo, sizeof(memBasicInfo));
    printf("Allocated Mem State: ");  
    memState((int)memBasicInfo.State);

    return 0;
}
