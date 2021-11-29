/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)，可扩展工具集。

    本工具软件是开源自由软件，您可以遵照 MIT 协议，修改和发布此程序。
    发布此库的目的是希望其有用，但不做任何保证。
    如果将本库用于商业项目，由于本库中的Bug，而引起的任何风险及损失，本作者不承担任何责任。

    开源地址: https://github.com/ets-ddui/ets
    开源协议: The MIT License (MIT)
    作者邮箱: xinghun87@163.com
    官方博客：https://blog.csdn.net/xinghun61
*/
#pragma once

#include <Windows.h>

#ifdef USE64
    #define is_x64 1
#else
    #define is_x64 0
#endif//USE64

#define F_INVALID       0x01
#define F_PREFIX        0x02
#define F_REX           0x04
#define F_MODRM         0x08
#define F_SIB           0x10
#define F_DISP          0x20
#define F_IMM           0x40
#define F_RELATIVE      0x80

struct CAddress
{
    BYTE m_iOffset;
    BYTE m_iSize;
};

struct DasmData
{
    BYTE m_iFlags;
    BYTE m_iRex;
    CAddress m_stOpcode;
    BYTE m_iModRM;
    BYTE m_iSIB;
    CAddress m_stDisplacement;
    CAddress m_stImmediate;
};

int Dasm(DasmData *p_ddDasmData, const BYTE *p_pCode, const bool p_bX64);
