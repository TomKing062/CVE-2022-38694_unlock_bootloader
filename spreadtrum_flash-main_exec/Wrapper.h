#ifndef WRAPPER_H
#define WRAPPER_H
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

    typedef struct {
        void* obj;
    } ClassHandle;

    ClassHandle* createClass();
    void destroyClass(ClassHandle* handle);
    BOOL call_Initialize(ClassHandle* handle, DWORD Port);
    void call_Uninitialize(ClassHandle* handle);
    int call_Read(ClassHandle* handle, UCHAR* m_RecvData, int max_len, int dwTimeout);
    int call_Write(ClassHandle* handle, UCHAR* lpData, int iDataSize);

#ifdef __cplusplus
}
#endif

#endif
