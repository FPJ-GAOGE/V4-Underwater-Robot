#ifndef FINEMOTE_DSHOT_H
#define FINEMOTE_DSHOT_H

#define DSHOT_MIN_THROTTLE              (48)
#define DSHOT_MAX_THROTTLE              (2047)
#define DSHOT_3D_FORWARD_MIN_THROTTLE   (1048)
#define DSHOT_3D_BACKWARD_MAX_THROTTLE  (1047)
#define DSHOT_RANGE                     (1999)
#define DSHOT_PACK_HEAD_LENGTH (2)
#define DSHOT_PACK_TAIL_LENGTH (2)
#define DSHOT_FRAME_LENGTH (16)
#define DSHOT_PACK_LENGTH (DSHOT_FRAME_LENGTH + DSHOT_PACK_HEAD_LENGTH + DSHOT_PACK_TAIL_LENGTH)
#define DSHOT_RX_LENGTH (100)
#define MOTOR_POLE_PAIR  12 // 电机极对数


#include "BSP_DSHOT.h"
#include "DeviceBase.h"
#include "etl/map.h"
#include "etl/vector.h"
#include "arm_math.h"


class DSHOT_Port : public DeviceBase {
private:
    struct DSHOT_GPIO_PortPins {
        GPIO_TypeDef *port;
        etl::map<uint16_t, uint16_t, 16> pins; // 使用map存储每个引脚的DshotPack
        DSHOT_GPIO_PortPins(GPIO_TypeDef *_port, uint16_t ID) : port(_port) {
            pins[ID] = 0;
        }
    };
    etl::vector<DSHOT_GPIO_PortPins, 4> gpioPortsQueue = {};
    float RPMs[DSHOT_PIN_NUMBER + 1]{};
    uint16_t rxBuffer[DSHOT_RX_LENGTH] {};
    uint32_t outPutData[DSHOT_PACK_LENGTH * 3] {};
    uint16_t currentIndex = 0;
    bool isInverted = true; // 是否反相传输，开启时启动接收与解码功能

    DSHOT_Port(){}

    void UpdateMiddleBit(etl::map<uint16_t, uint16_t, 16> &pins)
    {
        for (int symbol_index = DSHOT_PACK_HEAD_LENGTH; symbol_index < DSHOT_FRAME_LENGTH + DSHOT_PACK_HEAD_LENGTH; symbol_index++) {
            outPutData[symbol_index * 3 + 1] = 0;          // Reset bits
        }
        for(auto &pin : pins) {
            uint16_t value = pin.second;
            uint16_t gpioPinMusk = BSP_DHOSTPinList[pin.first];
            uint32_t middleBitHigh = isInverted ? gpioPinMusk << 16 : gpioPinMusk;
            uint32_t middleBitLow = isInverted ? gpioPinMusk : gpioPinMusk << 16;
            for (int pos = DSHOT_PACK_HEAD_LENGTH; pos < DSHOT_FRAME_LENGTH + DSHOT_PACK_HEAD_LENGTH; pos++) {
                if (value & 0x8000) {
                    outPutData[pos * 3 + 1] |= middleBitHigh;
                }else {
                    outPutData[pos * 3 + 1] |= middleBitLow;
                }
                value <<= 1;
            }
        }
    }

    void GenerateMessage(etl::map<uint16_t, uint16_t, 16> &pins)
    {
        uint32_t portMask = 0;
        for(auto &pin : pins) {
            portMask |= BSP_DHOSTPinList[pin.first];
        }
        uint32_t resetMask;
        uint32_t setMask;

        if (isInverted) {
            resetMask = portMask;
            setMask = (portMask << 16);
        } else {
            resetMask = (portMask << 16);
            setMask = portMask;
        }

        for(int hold_bit_index = 0; hold_bit_index < DSHOT_PACK_HEAD_LENGTH; hold_bit_index++) {
            outPutData[hold_bit_index * 3 + 0] = resetMask; // Always reset all ports
            outPutData[hold_bit_index * 3 + 1] = resetMask;
            outPutData[hold_bit_index * 3 + 2] = resetMask;
        }
        for (int symbol_index = DSHOT_PACK_HEAD_LENGTH; symbol_index < DSHOT_FRAME_LENGTH + DSHOT_PACK_HEAD_LENGTH; symbol_index++) {
            outPutData[symbol_index * 3 + 0] = setMask ; // Always set all ports
            outPutData[symbol_index * 3 + 1] = 0;          // Reset bits are port dependent
            outPutData[symbol_index * 3 + 2] = resetMask; // Always reset all ports
        }
        for(int hold_bit_index = DSHOT_FRAME_LENGTH + DSHOT_PACK_HEAD_LENGTH; hold_bit_index < DSHOT_PACK_LENGTH; hold_bit_index++) {
            outPutData[hold_bit_index * 3 + 0] = resetMask; // Always reset all ports
            outPutData[hold_bit_index * 3 + 1] = resetMask;
            outPutData[hold_bit_index * 3 + 2] = resetMask;
        }

        UpdateMiddleBit(pins);
    }

    /*** 以下为新增的接收与解码函数 ***/

    // 将GCR编码的5位段转换为4位数据
    static uint8_t GCRToData(uint8_t encoded) {
        const uint8_t gcr_table[32] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0x9,  0xA,  0xB,  0xFF, 0xD,  0xE,  0xF,
            0xFF, 0x2,  0x3,  0xFF, 0x5,  0x6,  0x7,  0xFF,
            0x0,  0x8,  0x1,  0xFF, 0x4,  0xC,  0xFF, 0xFF
        };
        if (encoded < 32) {
            uint8_t data = gcr_table[encoded];
            if (data != 0xFF) return data;
        }
        return 0xFF; // 无效编码
    }

    // 在buffer中从start位置开始寻找边沿，返回长度（采样点数）
    static uint8_t FindEdge(const uint8_t* buffer) {
        uint8_t i = 0;
        while (buffer[i] == buffer[0]) {
            if (i > 9) { return 0xFF; } // 在GCR编码中不可能采样到两个以上0
            ++i;
        }
        return i;
    }

    // 将单个引脚的位序列解码为RPM，返回负值表示失败
    float CalaVelForBuffer(const uint8_t* buffer, size_t len) {
        uint8_t GCRData[20]{};
        size_t position = 0;

        // 找到一个起始点（寻找第一个0，保持与原实现兼容）
        while (position < len && buffer[position] != 0) {
            ++position;
        }
        if (position >= len) return -1;

        for (int i = 0; i < 20;) {
            uint8_t length = FindEdge(&buffer[position]);
            if (length == 0xFF) return -1;
            position += length;
            if (length <= 3) {
                GCRData[i] = 1;
                i++;
            }
            else if (length > 3 && length <= 6) {
                GCRData[i] = 0;
                GCRData[i + 1] = 1;
                i += 2;
            }
            else if (length > 6 && length <= 9){
                GCRData[i] = 0;
                GCRData[i + 1] = 0;
                GCRData[i + 2] = 1;
                i += 3;
            }
            else {
                switch (i) {
                case 18:
                    GCRData[i] = 0;
                    GCRData[i + 1] = 0;
                    i = 20;
                    break;
                case 19:
                    GCRData[i] = 0;
                    i = 20;
                    break;
                default:
                    return -1;//表示出错
                }
            }
        }

        // 将20位GCR编码转换为16位数据
        uint16_t decodedData = 0;
        for (int seg = 0; seg < 4; seg++) {
            uint8_t encodedByte = 0;
            for (int bit = 0; bit < 5; bit++) {
                encodedByte = (encodedByte << 1) | GCRData[seg * 5 + bit];
            }
            uint8_t data = GCRToData(encodedByte);
            if (data == 0xFF) return -1; // 无效编码
            decodedData = (decodedData << 4) | data;
        }

        // CRC检查（与原实现保持一致，简单校验）
        // uint8_t crc = (decodedData >> 4) ^ (decodedData >> 8) ^ (decodedData >> 12);
        // crc = ~crc;

        // if((crc & 0xff) == (decodedData & 0xff))
        {
            if(decodedData >> 4 == 0x0fff) {
                return 0; //默认速度0
            }
            float RPM{0};
            uint8_t shiftLeftBits = decodedData >> 13;
            RPM = static_cast<float>(((decodedData >> 4) & 0b111111111) << shiftLeftBits);
            if (RPM == 0.0f) {
                return -1;
            }
            RPM = (1000000 / RPM * 60 + 50) / static_cast<float>(MOTOR_POLE_PAIR);
            return RPM;
        }
        return -1;
    }

    // 对接收到的rxBuffer按照给定端口的pins进行分离并解码，结果保存到pinRPMs
    void DecodeReceivedPort(size_t portQueueIndex) {
        if (portQueueIndex >= gpioPortsQueue.size()) return;
        DSHOT_GPIO_PortPins &entry = gpioPortsQueue[portQueueIndex];
        // 对每个pin从rxBuffer中抽取位序列
        for (auto &pin : entry.pins) {
            uint16_t pinMask = BSP_DHOSTPinList[pin.first]; // GPIO引脚位掩码
            static uint8_t bitBuffer[DSHOT_RX_LENGTH];
            for (size_t i = 0; i < DSHOT_RX_LENGTH; ++i) {
                bitBuffer[i] = ( (rxBuffer[i] & pinMask) ? 1 : 0 );
            }
            float rpm = CalaVelForBuffer(bitBuffer, DSHOT_RX_LENGTH);
            if(rpm > 0) {
                RPMs[pin.first] = rpm;
            }
        }
    }

    /*** 接收与解码函数END ***/


public:
    DSHOT_Port(const DSHOT_Port&) = delete;
    DSHOT_Port& operator=(const DSHOT_Port&) = delete;

    static DSHOT_Port& GetInstance() {
        static DSHOT_Port instance;
        return instance;
    }

    void AddPin(GPIO_TypeDef *port, uint16_t ID) {
        for(auto &data : gpioPortsQueue) {
            if(data.port == port) {
                data.pins[ID] = 0;
                return;
            }
        }
        if(!gpioPortsQueue.full()) {
            gpioPortsQueue.push_back(DSHOT_GPIO_PortPins(port, ID));
        }
        else {
            // 如果队列已满，可以选择覆盖最旧的元素或抛出异常
            gpioPortsQueue[0] = DSHOT_GPIO_PortPins(port, ID);
        }
    }

    void SetDataPack(GPIO_TypeDef *port, uint16_t ID, uint16_t DshotDataPack) {
        for(auto &data : gpioPortsQueue) {
            if(data.port == port && data.pins.count(ID)) {
                data.pins[ID] = DshotDataPack;
                return;
            }
        }
    }

    bool InvertState() const{
        return isInverted;
    }

    void TransmitMessage() {
        if (currentIndex >= gpioPortsQueue.size()) {
            return; // 队列已遍历完毕
        }

        if(isInverted) {
            uint16_t pinMusk = 0;
            for(auto &pin : gpioPortsQueue[currentIndex].pins) {
                pinMusk |= BSP_DHOSTPinList[pin.first];
            }
            BSP_DSHOTs::GetInstance().SetPinsOutput(gpioPortsQueue[currentIndex].port, pinMusk);
        }

        GenerateMessage(gpioPortsQueue[currentIndex].pins);
        BSP_DSHOTs::GetInstance().TransmitMessage(outPutData, gpioPortsQueue[currentIndex].port, DSHOT_PACK_LENGTH * 3);
        currentIndex++;
    }

    void ReceiveMessage() {
        uint16_t pinMusk = 0;
        for(auto &pin : gpioPortsQueue[currentIndex-1].pins) {
            pinMusk |= BSP_DHOSTPinList[pin.first];
        }
        BSP_DSHOTs::GetInstance().SetPinsInput(gpioPortsQueue[currentIndex-1].port, pinMusk);
        BSP_DSHOTs::GetInstance().RecceiveMessage((uint32_t*)rxBuffer, gpioPortsQueue[currentIndex-1].port, DSHOT_RX_LENGTH);
    }

    /*** new add ***/
    void Deccode() {
        DecodeReceivedPort(currentIndex-1);
    }

    float getRPM(uint16_t ID) {
        if(ID == 0 || ID > DSHOT_PIN_NUMBER) {
            return -1.0f; // 无效ID
        }
        return RPMs[ID];
    }
    /*** end ***/

    void Handle() override {
        currentIndex = 0; // 每次处理时从头开始
        TransmitMessage();
    }
};



template <uint16_t ID>
class DSHOT_PIN {
public:
    DSHOT_PIN(const DSHOT_PIN &) = delete;
    DSHOT_PIN &operator=(const DSHOT_PIN &) = delete;
    static DSHOT_PIN &GetInstance() {
        static DSHOT_PIN instance;
        return instance;
    }

    float GetRPM() {
        return DSHOT_Port::GetInstance().getRPM(ID);
    }

    void SetIntThrottle(int integerThrottle) {
        if(!isMotorEnabled) {
            targetThrottle = 0;
            GenerateDshotPack();
            return;
        }

        targetThrottle = integerThrottle;
        GenerateDshotPack();
    }

    void SetTargetThrottle(float throttle) {
        if(!isMotorEnabled) {
            targetThrottle = 0;
            GenerateDshotPack();
            return;
        }
        // range of throttle 3D(-1，1), normal(0，1)
        if(is3DModeEnabled) {
            if (fabsf(throttle) < 1e-6f) {
                targetThrottle = 0;
            }
            else {
                if(throttle < 0) {
                    targetThrottle = DSHOT_MIN_THROTTLE + static_cast<int>(roundf(fabsf(throttle) * (DSHOT_3D_BACKWARD_MAX_THROTTLE - DSHOT_MIN_THROTTLE)));
                    targetThrottle = targetThrottle < DSHOT_MIN_THROTTLE ? DSHOT_MIN_THROTTLE : targetThrottle;
                    targetThrottle = targetThrottle > DSHOT_3D_BACKWARD_MAX_THROTTLE ? DSHOT_3D_BACKWARD_MAX_THROTTLE : targetThrottle;
                }
                else {
                    targetThrottle = DSHOT_3D_FORWARD_MIN_THROTTLE + static_cast<int>(roundf(throttle * (DSHOT_MAX_THROTTLE - DSHOT_3D_FORWARD_MIN_THROTTLE)));
                    targetThrottle = targetThrottle < DSHOT_3D_FORWARD_MIN_THROTTLE ? DSHOT_3D_FORWARD_MIN_THROTTLE : targetThrottle;
                    targetThrottle = targetThrottle > DSHOT_MAX_THROTTLE ? DSHOT_MAX_THROTTLE : targetThrottle;
                }
            }
        }
        else {
            int integerThrottle = static_cast<int>(roundf(throttle * DSHOT_RANGE));
            targetThrottle = integerThrottle > DSHOT_RANGE ? DSHOT_MAX_THROTTLE : (integerThrottle <= 0 ? 0 : integerThrottle + DSHOT_MIN_THROTTLE); // 1-47保留为特殊指令
        }

        GenerateDshotPack();
    }

    void Set3DMode(bool isEnable) {
        is3DModeEnabled = isEnable;
    }

    void SetTelemetry(bool isEnable) {
        telemetryRequest = isEnable;
    }

    void EnableMotor() {
        isMotorEnabled = true;
    }

    void DisableMotor() {
        isMotorEnabled = false;
        targetThrottle = 0;
        GenerateDshotPack();
    }

private:
    uint16_t targetThrottle = 0;
    bool is3DModeEnabled = true;
    bool isInverted = true;
    bool telemetryRequest = true;
    bool isMotorEnabled = false; // 是否允许电机工作

    DSHOT_PIN() {
        BSP_DSHOT<ID>::GetInstance();
        DSHOT_Port::GetInstance().AddPin(BSP_DHOSTPortList[ID], ID);
        isInverted = DSHOT_Port::GetInstance().InvertState();//同一个GPIO PORT下不同的PIN必须保持一致
    }

    void GenerateDshotPack() {
        // 构建完整DShot数据包（11位油门 + 1位遥测 + 4位CRC）
        uint16_t packet{0};
        packet |= (targetThrottle << 5);
        packet |= (telemetryRequest ? 1<<4 : 0<<4);

        // 计算CRC并合并到数据包
        uint8_t crc = (packet >> 4) ^ (packet >> 8) ^ (packet >> 12);  // 取前12位计算
        if(isInverted) {
            crc = ~crc;
        }
        packet |= crc & 0x0F;

        DSHOT_Port::GetInstance().SetDataPack(BSP_DHOSTPortList[ID], ID, packet);
    }
};

inline void DMA_XferCpltCallback(DMA_HandleTypeDef *hdma) {
    if (hdma->Instance == DSHOT_TRANSMIT_DMA_HANDLE.Instance) {
        if(DSHOT_Port::GetInstance().InvertState()) {
            DSHOT_Port::GetInstance().ReceiveMessage();
        }
        else {
            DSHOT_Port::GetInstance().TransmitMessage();
        }
    }
    else if (hdma->Instance == DSHOT_RECEIVE_DMA_HANDLE.Instance) {
        if(DSHOT_Port::GetInstance().InvertState()) {
            DSHOT_Port::GetInstance().Deccode();
            DSHOT_Port::GetInstance().TransmitMessage();
        }
    }
}

#endif
