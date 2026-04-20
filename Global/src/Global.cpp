#include "Global.h"

namespace Logic
{

#define CHECK_BOOL(x) \
    if (!(x))         \
    {                 \
        return false; \
    }

    Global::Global()
    {
    }

    Global::~Global()
    {
    }

    void Global::Can_RegisterCanTrans(Can_TransmitFunc_t ptr)
    {
        this->Can_TransmitPtr = ptr;
    }

    bool Global::Transmit(uint32_t id, uint8_t *data, uint16_t len)
    {
        if (this->Can_TransmitPtr == nullptr)
        {
            return false;
        }

        if (!this->Can_TransmitPtr(id, data, len))
        {
            return false;
        }

        return true;
    }

    bool Global::OpenRelay()
    {
        uint8_t reqData[8] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        if (!this->Transmit(this->RelayId, reqData, 8))
        {
            return false;
        }

        this->delay(2);

        return true;
    }

    bool Global::CloseRelay()
    {
        uint8_t reqData[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        if (!this->Transmit(this->RelayId, reqData, 8))
        {
            return false;
        }

        this->delay(2);

        return true;
    }

    bool Global::Can_Receive(uint32_t id, uint8_t *data, uint16_t len, int timeout)
    {
        if (data == nullptr || len == 0)
            return false;

        // --- 1. 初始化接收桶 ---
        memset((void*)this->buck.data, 0, sizeof(this->buck.data));
        this->buck.len = len;
        this->buck.received_size = 0;
        this->buck.recv_id = id;
        this->buck.complete = false;
        this->buck.en = true;
        std::atomic_thread_fence(std::memory_order_release);
        bool success = false;

        // --- 2. 使用真实时间 ---
        auto start = std::chrono::steady_clock::now();

        while (true)
        {
            if (this->buck.received_size >= len)
            {
                break;
            }

            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();

            if (elapsed >= timeout)
            {
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        this->buck.en = false;

        if (this->buck.complete)
        {
            memcpy(data, (void*)this->buck.data, len);
        }
        else
        {
            return false;
        }

        return true;
    }

    bool Global::Step1_SafeUnlock()
    {
        this->CloseRelay(); // 复位的时候关闭继电器
        this->OpenRelay();  // 设备上电
        this->OpenRelay();  // 设备上电

        // 切换扩展会话
        uint8_t reqSession[] = {0x02, 0x10, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        if (!this->Transmit(this->DevPhySicalId, reqSession, 8))
        {
            return false;
        }

        uint8_t resSession[8] = {};
        if (!this->Can_Receive(this->DevResponseId, resSession, 8, 2000))
        {
            return false;
        }

        if (resSession[1] != 0x50 || resSession[0] == 0x7F) // 负响应
        {
            return false;
        }

        // 解锁流程

        // 请求种子
        uint8_t reqSeed[] = {0x02, 0x27, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        uint8_t resSeed[8] = {};
        if (!this->Transmit(this->DevPhySicalId, reqSeed, 8))
        {
            return false;
        }

        if (!this->Can_Receive(this->DevResponseId, resSeed, 8, 2000))
        {
            return false;
        }

        if (resSeed[1] != 0x67 || resSeed[0] == 0x7F) // 负响应
        {
            return false;
        }

        uint8_t seed[4] = {resSeed[3], resSeed[4], resSeed[5], resSeed[6]};
        uint8_t key[4] = {};
        this->CalculateKeyL1(seed, key);
        uint8_t sendKey[8] = {0x06, 0x27, 0x02, key[0], key[1], key[2], key[3], 0xAA}; // 发送密钥
        if (!this->Transmit(this->DevPhySicalId, sendKey, 8))
        {
            return false;
        }
        uint8_t resp[8] = {};
        if (!this->Can_Receive(this->DevResponseId, resp, 8, 2000))
        {
            return false;
        }

        if (resp[1] != 0x67 && resp[2] != 0x02)
        {
            return false;
        }

        return true;
    }

    bool Global::Step2_WriteF1C1()
    {

        this->delay(5);

        uint8_t resp[8] = {};
        // 多帧请求
        uint8_t ff[8] = {0x10, 0x0F, 0x2E, 0xF1, 0xC1, 0x44, 0x00, 0x00};
        if (!this->Transmit(this->DevPhySicalId, ff, 8))
        {
            return false;
        }

        if (!this->Can_Receive(this->DevResponseId, resp, 8, 2000))
        {
            return false;
        }

        if (resp[0] != 0x30) // 接收到流控
        {
            return false;
        }

        memset(resp, 0, 8);

        this->delay(10);
        uint8_t cf1[8] = {0x21, 0x00, 0x00, 0x00, 0x01, 0x60, 0x00, 0x00};
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, cf1, 8));
        this->delay(10);
        uint8_t cf2[8] = {0x22, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, cf2, 8));
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, resp, 8, 2000));

        CHECK_BOOL(resp[1] == 0x6E && resp[2] == 0xF1 && resp[3] == 0xC1);
        return true;
    }

    bool Global::Step3_ReadF1C1()
    {
        // this->delay(500);
        // 断电
        CHECK_BOOL(this->CloseRelay());
        CHECK_BOOL(this->CloseRelay());

        this->delay(10);
        CHECK_BOOL(this->OpenRelay());
        CHECK_BOOL(this->OpenRelay());

        uint8_t rescf[16] = {};
        uint8_t req[8] = {0x03, 0x22, 0xF1, 0xC1, 0xAA, 0xAA, 0xAA, 0xAA}; // 发请求
        uint8_t resff[8] = {};
        uint8_t fc[8] = {0x30, 0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, req, 8));
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, resff, 8, 2000)); // 接收到首帧
        CHECK_BOOL((resff[0] & 0xF0) == 0x10);                               // 检查是否是首帧
        // 发送流控
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, fc, 8));
        // 接收两帧连续帧
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, rescf, 16, 1000));

        CHECK_BOOL(resff[5] == 0x44 && rescf[5] == 0x60); // 检查数据是否合乎预期

        return true;
    }

    bool Global::Step4_ReadF180()
    {
        // 切换扩展会话
        this->delay(5);
        uint8_t reqSession03[8] = {0x02, 0x10, 0x03, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        uint8_t resp[8] = {};
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, reqSession03, 8));
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, resp, 8, 2000));

        CHECK_BOOL(resp[1] == 0x50); // 正响应

        uint8_t reqSession02[8] = {0x02, 0x10, 0x02, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        memset(resp, 0, 8);
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, reqSession03, 8));
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, resp, 8, 2000));
        // CHECK_BOOL(resp[1] == 0x50); // 正响应
        if (resp[1] == 0x7F) // 负响应等待 有个字节是0x78来着，具体是哪个忘了，后面加吧
        {
            memset(resp, 0, 8);
            CHECK_BOOL(this->Can_Receive(this->DevResponseId, resp, 8, 2000));
            CHECK_BOOL(resp[1] == 0x50); // 正响应
        }

        uint8_t reqVersion[8] = {0x03, 0x22, 0xF1, 0x80, 0xAA, 0xAA, 0xAA, 0xAA};
        memset(resp, 0, 8);
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, reqVersion, 8));
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, resp, 8, 2000));
        CHECK_BOOL(resp[2] == 0x62 && resp[0] == 0x10); // 首帧

        // memset(resp,0,8);
        uint8_t resCF[16] = {};
        uint8_t reqFC[8] = {0x30, 0x00, 0x14, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA}; // 流控
        CHECK_BOOL(this->Transmit(this->DevPhySicalId, reqFC, 8));
        CHECK_BOOL(this->Can_Receive(this->DevResponseId, resCF, 16, 2000)); // 收16个字节
        CHECK_BOOL(resCF[9] == 0x4C && resCF[10] == 0x78 && resCF[11] == 0x5F && resCF[12] == 0x03 && resCF[13] == 0x02);

        // 切换回默认会话
        uint8_t reqDefaultSess[8] = {0x02, 0x10, 0x01, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        this->Transmit(this->DevPhySicalId, reqDefaultSess, 8); // 这个应该不算失败，就算没回去也不影响,不做bool检查

        return true;
    }

    void Global::delay(uint32_t ms)
    {
        auto start = std::chrono::steady_clock::now();
        auto target = start + std::chrono::milliseconds(ms);

        while (true)
        {
            auto now = std::chrono::steady_clock::now();

            if (now >= target)
                break;

            auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(target - now).count();

            // 剩余时间比较长，用 sleep 降低CPU占用
            if (remaining > 2)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(remaining - 1));
            }
            else
            {
                // 最后1~2ms，用忙等待提高精度
                while (std::chrono::steady_clock::now() < target)
                {
                    // busy wait
                }
                break;
            }
        }
    }

    void Global::CalculateKeyL1(uint8_t *seed, uint8_t *out)
    {
        uint8_t calData[4] = {};
        uint8_t AKey_L1[4] = {};

        for (int i = 0; i < 4; i++)
        {
            calData[i] = (uint8_t)(seed[i] ^ xorArray[i]);
        }

        AKey_L1[0] = (uint8_t)(((calData[3] & 0x0F) << 4) | (calData[3] & 0xF0));
        AKey_L1[1] = (uint8_t)(((calData[1] & 0x0F) << 4) | ((calData[0] & 0xF0) >> 4));
        AKey_L1[2] = (uint8_t)((calData[1] & 0xF0) | ((calData[2] & 0xF0) >> 4));
        AKey_L1[3] = (uint8_t)(((calData[0] & 0x0F) << 4) | (calData[2] & 0x0F));

        memcpy(out, AKey_L1, 4);
    }

    void Global::Can_ReviveCallback(uint32_t id, uint8_t *data, uint16_t len)
    {

        if (!this->buck.en || id != this->buck.recv_id || data == nullptr || len == 0)
        {
            return;
        }

        // if(!this->buck.en == false)
        // {
        //     return;
        // }

        // if(id != this->buck.recv_id)
        // {
        //     return;
        // }

        if (this->buck.received_size + len > this->buck.len)
        {
            len = this->buck.len - this->buck.received_size;
            if (len <= 0)
                return;
        }

        memcpy((void*)(this->buck.data + this->buck.received_size), data, len);

        this->buck.received_size += len;

        if (this->buck.received_size >= this->buck.len)
        {
            this->buck.complete = true;
            this->buck.en = false;
        }
    }
}
