#pragma once

#include <Windows.h>
#include <tchar.h>
#define INVALID_VALUE      ((DWORD)-1)
#define INFINITE_LOGFILE_SIZE           ( 0 )

typedef enum
{
    SPLOGLV_NONE = 0,
    SPLOGLV_ERROR = 1,
    SPLOGLV_WARN = 2,
    SPLOGLV_INFO = 3,
    SPLOGLV_DATA = 4, //exclusive, only output data
    SPLOGLV_VERBOSE = 5

}SPLOG_LEVEL;

enum
{
    LOG_READ = 0,
    LOG_WRITE = 1,
    LOG_ASYNC_READ = 2
};

class ISpLog
{
public:

    enum OpenFlags
    {
        modeTimeSuffix = 0x0001,  //  Append time suffix to a file, like: xxx_2009_06_01_12_39_50.log
        modeDateSuffix = 0x0002,  //  Append date suffix to a file, like: xxx_2009_06_12.log
        modeCreate = 0x1000,  //  Directs the constructor to create a new file. 
                                        //  If the file exists already, it is truncated to 0 length. 
                                        modeNoTruncate = 0x2000,  //  Combine this value with modeCreate. 
                                                                        //  If the file being created already exists, it is not truncated to 0 length.

                                                                        typeText = 0x4000,  //  Sets text mode.   Default file extension is *.log     
                                                                        typeBinary = (int)0x8000,  //  Sets binary mode. Default file extension is *.bin

                                                                        // typeBinary only --------- [
                                                                        modeBinNone = 0x0000,  // Log none  bin data
                                                                        modeBinRead = 0x0100,  // Log read  bin data
                                                                        modeBinWrite = 0x0200,  // Log write bin data
                                                                        modeBinReadWrite = 0x0300,  // Log read & Write bin data
                                                                        //  ------------------------]

                                                                        defaultTextFlag = modeCreate | modeDateSuffix | typeText,
                                                                        defaultBinaryFlag = modeCreate | modeDateSuffix | typeBinary | modeBinRead,
                                                                        defaultDualFlag = modeCreate | modeDateSuffix | typeText | typeBinary | modeBinRead
    };

    virtual ~ISpLog(void) {   };

    // ----------------------------------------------------------------------------
    //  Open
    //      Open log file
    //
    //  Parameter
    //      lpszLogPath         : A string that is the path to the desired file. The path can be relative or absolute.
    //                            There is a default string size limit of _MAX_PATH characters
    //      uFlags              : Specifies the action to take when opening the file. 
    //                            You can combine options listed below by using the bitwise-OR (|) operator.
    //                            See the ILog::OpenFlags enum for a list of mode options.
    //      uLogLevel           : Specifies the log level. Only available for 'typeText'. see SPLOG_LEVEL.                    
    //      uMaxFileSizeInMB    : Max. log file size in unit of MB. Note: 0, infinite file size.
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    Open(LPCTSTR lpszLogPath,
        UINT uFlags = ISpLog::defaultTextFlag,
        UINT uLogLevel = SPLOGLV_INFO,
        UINT uMaxFileSizeInMB = INFINITE_LOGFILE_SIZE
    ) = 0;

    // ----------------------------------------------------------------------------
    //  Close
    //      Close log file
    //
    //  Parameter
    //      None.   
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    Close(void) = 0;


    // ----------------------------------------------------------------------------
    //  Release
    //      Delete object
    //
    //  Parameter
    //      None.   
    //
    //  Return
    //      None. 
    //
    //  Remarks
    //      None
    virtual void    Release(void) = 0;


    // ----------------------------------------------------------------------------
    //  LogHexData
    //      Log binary data. 
    //      Example :   4D 5A 90 00 03 00 00 00 04 00 00 00 FF FF 00 00
    //                  B8 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 
    //      uFlag   :   LOG_READ ,  specifies that the 'pHexData' is 'read  data'.
    //                  LOG_WRITE,  specifies that the 'pHexData' is 'write data'.
    //
    //  Parameter
    //      pHexData    : Binary data. 
    //      dwHexSize   : The number of bytes to write to the log file.
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    LogHexData(const BYTE* pHexData, DWORD dwHexSize, UINT uFlag = LOG_READ) = 0;

    // ----------------------------------------------------------------------------
    //  LogRawStrA (ANSI)
    //  LogRawStrW (UNICODE)
    //      Log a string.
    //      Example:    [2009-06-04 16:54:47:500] This is a test.  
    //
    //  Parameter
    //      uLogLevel  : Specifies the level of the 'lpszString'. 
    //      lpszString : A string to write to log file. 
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    LogRawStrA(UINT uLogLevel, LPCSTR  lpszString) = 0;
    virtual BOOL    LogRawStrW(UINT uLogLevel, LPCWSTR lpszString) = 0;

    // ----------------------------------------------------------------------------
    //  LogFmtStrA (ANSI)
    //  LogFmtStrW (UNICODE)
    //      Log a formatted string.
    //
    //  Parameter
    //      uLogLevel  : Specifies the level of the 'lpszString'. 
    //      lpszFmt : Format control. 
    //      argument: Optional arguments.
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    LogFmtStrA(UINT uLogLevel, LPCSTR  lpszFmt, ...) = 0;
    virtual BOOL    LogFmtStrW(UINT uLogLevel, LPCWSTR lpszFmt, ...) = 0;

    // ----------------------------------------------------------------------------
    //  LogBufData 
    //      Log a buffer. Example :
    //      [2009-06-04 16:54:47:500] --> 114756(0x0001c044) Bytes
    //                                00000000h: 4D 5A 90 00 03 00 00 00 04 00 00 00 FF FF 00 00 ; MZ..............
    //                                00000010h: B8 00 00 00 00 00 00 00 40 00 00 00 00 00 00 00 ; ........@.......
    //                                00000020h: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ; ................
    //
    //  Parameter
    //      uLogLevel: Specifies the level of the 'pBufData'
    //      pBufData : A buffer to write to log file. 
    //      dwBufSize: The number of bytes of the buffer.
    //      uFlag    : LOG_WRITE      specifies '-->'   
    //                 LOG_READ       specifies '<--'
    //		           LOG_ASYNC_READ specifies '<<-'
    //      pUserNeedSize: this size is that user want to Read/Write buffer size
    //                     use this parameter user can see the different between real Read/Write size (dwBufSize) 
    //                     and expired size (*pUserNeedSize)
    //                     if this point is not null,log will like bellow
    //                     [2009-06-04 16:54:47:0500] --> 114756(0x0001c044)/114757(0x0001c045)  Bytes
    //
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    LogBufData(UINT uLogLevel, const BYTE* pBufData, DWORD dwBufSize, UINT uFlag = LOG_WRITE, const DWORD* pUserNeedSize = NULL) = 0;

    // ----------------------------------------------------------------------------
    //  SetProperty/GetProperty 
    //      Set/Get property
    //  Parameter
    //      lAttr   : Attributes, see enum LOG_ATTRIBUTE.
    //      lFlags  : 
    //      lpValue : Values to be set/get.         
    //
    //  Return
    //      If the function succeeds, the return value is nonzero.
    //      If the function fails, the return value is zero. 
    //
    //  Remarks
    //      None
    virtual BOOL    SetProperty(LONG lAttr, LONG lFlags, LPCVOID lpValue) = 0;
    virtual BOOL    GetProperty(LONG lAttr, LONG lFlags, LPVOID  lpValue) = 0;
};

enum CHANNEL_TYPE
{
    CHANNEL_TYPE_COM = 0,
    CHANNEL_TYPE_SOCKET = 1,
    CHANNEL_TYPE_FILE = 2,
    CHANNEL_TYPE_USBMON = 3         // USB monitor
};

typedef struct _CHANNEL_ATTRIBUTE
{
    CHANNEL_TYPE ChannelType;

    union {
        // ComPort
        struct {
            DWORD  dwPortNum;
            DWORD  dwBaudRate;
        } Com;

        // Socket
        struct {
            DWORD dwPort;
            DWORD dwIP;
            DWORD dwFlag; //[in]: 0, Server; 1, Client; [out]: client ID.		               
        } Socket;

        // File
        struct {
            DWORD  dwPackSize;
            DWORD  dwPackFreq;
            WCHAR* pFilePath;
        } File;
    };

} CHANNEL_ATTRIBUTE, * PCHANNEL_ATTRIBUTE;

typedef const PCHANNEL_ATTRIBUTE PCCHANNEL_ATTRIBUTE;

class ICommChannel
{
public:
	virtual~ICommChannel() = 0;
	/**
	 *  InitLog
	 *  @param [in] pszLogName: log file name, if it is NULL or _T(""), program will set default
	 *  @param [in] uiLogType : log format type
	 *  @param [in] uiLogLevel: see LOG_LEVEL enum
	 *  @param [in] pLogUtil  : set the sub-class pointer of CLogUtil, to do special log content
	 *  @param [in] pszBinLogFileExt: binary log file extension, if you set uiLogType both text
	 *                          and binary,and want to change the binary log file extension,you must
	 *							set this param, otherwise ".bin" extension is set default.
	 *
	 *  @return: TRUE if init success, FALSE otherwise
	 *
	 *  @remarks: this method need not be called before Open always, because program will auto to
	 *            log information by configure file settings. "ChannelConfig.ini"
	 */
	virtual BOOL	InitLog(LPCWSTR pszLogName, UINT uiLogType,
		UINT uiLogLevel = INVALID_VALUE, ISpLog* pLogUtil = NULL,
		LPCWSTR pszBinLogFileExt = NULL) = 0;
	/**
	 *  Set receiver for reading data with async way
	 *  @param [in] ulMsgId		: message ID
	 *  @param [in] bRcvThread	: TRUE, received by thread, FALSE, by window
	 *  @param [in] pReceiver	: receiver based on bRcvThread
	 *
	 *  @return TRUE if set success, FALSE otherwise
	 *
	 *  @remarks: if you call this method after Open, and you set pReceiver to NULL,it will stop
	 *            auto-read thread. if set pReceiver to not NULL, and not call this method before
	 *            Open, this will start auto-read thread.
	 */
	virtual BOOL	SetReceiver(ULONG  ulMsgId,
		BOOL    bRcvThread,
		LPCVOID pReceiver) = 0;
	/**
	 *  Get receiver
	 *  @param [out] ulMsgId	: message ID
	 *  @param [out] bRcvThread	: TRUE, received by thread, FALSE, by window
	 *  @param [out] pReceiver	: receiver based on bRcvThread
	 *
	 *  @return void
	 */
	virtual void	GetReceiver(ULONG& ulMsgId,
		BOOL& bRcvThread,
		LPVOID& pReceiver) = 0;
	/**
	 *  Open channel
	 *  @param [in] pOpenArgument : channel parameters
	 *
	 *  @return TRUE if open success, FALSE otherwise
	 */
	virtual BOOL	Open(PCCHANNEL_ATTRIBUTE pOpenArgument) = 0;
	/**
	 *  Close channel
	 *
	 *  @return void
	 */
	virtual void	Close() = 0;
	/**
	 *  clear channel in&out data buffer
	 *
	 *  @return TRUE if clear success, FALSE otherwise
	 */
	virtual BOOL	Clear() = 0;
	/**
	 *  Read data from channel with sync way
	 *  @param [out] lpData		: store read data
	 *  @param [in] dwDataSize	: lpData allocated size by outside
	 *  @param [in] dwTimeOut	: time out
	 *  @param [in] dwReserved  : reserved, for UART;
	 *                            low word is client ID, high word is reserved for server socket
	 *
	 *  @return real read data size
	 */
	virtual DWORD	Read(LPVOID lpData, DWORD dwDataSize,
		DWORD dwTimeOut, DWORD dwReserved = 0) = 0;
	/**
	 *  Write data to channel
	 *  @param [in] lpData		: store writing data
	 *  @param [in] dwDataSize	: lpData allocated size by outside
	 *  @param [in] dwReserved  : reserved, for UART;
	 *                            low word is client ID, high word is reserved for server socket
	 *
	 *  @return real written data size
	 */
	virtual DWORD	Write(LPVOID lpData, DWORD dwDataSize, DWORD dwReserved = 0) = 0;
	/**
	 *  Free memory allocated by program, only used after async read data
	 *  @param [in] pMemBlock	: memory pointer
	 *
	 *  @return void
	 */
	virtual void	FreeMem(LPVOID pMemBlock) = 0;
	/**
	 *  Get property of this program
	 *  @param [in] lFlags		: reserved
	 *  @param [in] dwPropertyID: property name
	 *  @param [out] pValue		: property value pointer
	 *
	 *  @return TRUE if get success,  FALSE otherwise
	 */
	virtual BOOL	GetProperty(LONG lFlags, DWORD dwPropertyID,
		LPVOID pValue) = 0;
	/**
	 *  Get property of this program
	 *  @param [in] lFlags		: reserved
	 *  @param [in] dwPropertyID: property name
	 *  @param [in] pValue		: property value pointer
	 *
	 *  @return TRUE if set success,  FALSE otherwise
	 */
	virtual BOOL 	SetProperty(LONG lFlags, DWORD dwPropertyID,
		LPCVOID pValue) = 0;
};

typedef BOOL (*pfCreateChannel)( ICommChannel ** , CHANNEL_TYPE);
typedef void (*pfReleaseChannel)( ICommChannel *);

class CBMPlatformApp
{
public:
	CBMPlatformApp();

	pfCreateChannel m_pfCreateChannel;
	pfReleaseChannel m_pfReleaseChannel;

	HMODULE  m_hChannelLib;

	BOOL InitInstance();
	int ExitInstance();
};

class  CBootModeOpr
{
public:
    CBootModeOpr();
    ~CBootModeOpr();
    BOOL   Initialize(DWORD dwPort);
    void   Uninitialize();
    int Read(UCHAR* m_RecvData, int max_len, int dwTimeout);
    int Write(UCHAR* lpData, int iDataSize);
private:
    BOOL   ConnectChannel(DWORD dwPort);
    BOOL   DisconnectChannel();
    BOOL                m_bOpened;
    ICommChannel*       m_pChannel;
};
