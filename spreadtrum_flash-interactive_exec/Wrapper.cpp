#include "BMPlatform.h"
#include "Wrapper.h"

ClassHandle* createClass() {
    ClassHandle* handle = new ClassHandle;
    handle->obj = new CBootModeOpr();
    return handle;
}

void destroyClass(ClassHandle* handle) {
    if (handle != NULL) {
        delete static_cast<CBootModeOpr*>(handle->obj);
        delete handle;
    }
}

BOOL call_Initialize(ClassHandle* handle, DWORD Port) {
    if (handle != NULL) {
        CBootModeOpr* obj = static_cast<CBootModeOpr*>(handle->obj);
        return obj->Initialize(Port);
    }
    return FALSE;
}

void call_Uninitialize(ClassHandle* handle) {
    if (handle != NULL) {
        CBootModeOpr* obj = static_cast<CBootModeOpr*>(handle->obj);
        obj->Uninitialize();
    }
}

int call_Read(ClassHandle* handle, UCHAR* m_RecvData, int max_len, int dwTimeout) {
    if (handle != NULL) {
        CBootModeOpr* obj = static_cast<CBootModeOpr*>(handle->obj);
        return obj->Read(m_RecvData, max_len, dwTimeout);
    }
    return -1;
}

int call_Write(ClassHandle* handle, UCHAR* lpData, int iDataSize) {
    if (handle != NULL) {
        CBootModeOpr* obj = static_cast<CBootModeOpr*>(handle->obj);
        return obj->Write(lpData, iDataSize);
    }
    return -1;
}
/*
int main() {
    ClassHandle* handle = createClass();
    callMethod(handle);
    destroyClass(handle);
    return 0;
}
*/