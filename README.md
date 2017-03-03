# LVCSR
1 .LVCSR_API_API int LVCSR_Init(const char* sysDir,const char*  cfgFile,int lineNum);

初始化系统模型，线程资源准备等工作

sysDir [in]： 模型路径，一般为 ../model

cfgFile[in]：配置文件名称，一般为scripts下的配置文件名

lineNum[in]： 线程开启个数


2.LVCSR_API_API int LVCSR_Exit();
退出识别引擎

3.LVCSR_API_API int LVCSR_Start(int sessionId=0);
开启一路识别资源
sessionId[in]: 用户拟申请的路id号
返回值：返回引擎实际可用的路id号


4.LVCSR_API_API int LVCSR_StopRecording(int sessionId = 0);
告知引擎一路语音的所有seg送完
sessionId[in]: LVCSR_Start的返回值

5.LVCSR_API_API int LVCSR_Stop(int sessionId);
关闭一路识别资源
sessionId[in]: LVCSR_Start的返回值

6.LVCSR_API_API int LVCSR_SendData(FeSegInfo* segInfo,FeSpeechAttri* dataInfo,int sessionId);
送语音分段进行识别
1）segInfo语音片段信息
离线配置下，是整路语音的所有seg，采用链表表示
半在线模式下，是一个片段信息，不是链表，detect_status 为FE_SEG_ALL
在线模式下，是一句话中的某一包的信息，通过detect_status来判断属于一句话的中哪个位置
2）dataInfo：数据信息
离线模式下,data_buf指向整通录音的起始地址，dataLen是整通录音的长度，send_status置为FE_SEND_ALL
半在线模式下，data_buf指向片段的起始地址，dataLen是片段的长度，send_status置为FE_SEND_ALL
在线模式下，data_buf指向的是片段中当前语音包地址，dataLen是当前包的长度

7. LVCSR_API_API int LVCSR_SetResultCallbackFunc(LVCSR_ResultCallback pFunc, int sessionId = 0);
设置识别结果回到函数，识别结果引擎采用回调方式返回
离线模式下，一次返回整路语音的结果
半在线模式下，每次返回一个片段的结果
在线模式下，分次返回整路语音中每个片段的结果
typedef void (*LVCSR_ResultCallback) (FeSegInfo * pResultArray ,bool bIsOver,int sessionId);   
pResultArray[out] : 结果返回，离线模式下，是一个链表；半在线模式下是一个片段
bIsOver[out]: 标识此次回调是否是最后一次回调即是否识别完成

8.LVCSR_API_API int LVCSR_SampleRate()
获取识别系统的配置的采样率
