#pragma once

#include <aidl/android/hardware/ir/BnConsumerIr.h>

namespace aidl {
namespace android {
namespace hardware {
namespace ir {

class ConsumerIr : public BnConsumerIr {
  public:
    // Função para dizer ao Android quais frequências suportamos (ex: 30-60kHz)
    ::ndk::ScopedAStatus getCarrierFreqs(std::vector<ConsumerIrFreqRange>* _aidl_return) override;
    
    // Função principal: Recebe a ordem do Android para disparar o IR
    ::ndk::ScopedAStatus transmit(int32_t carrierFreqHz, const std::vector<int32_t>& pattern) override;
};

}  // namespace ir
}  // namespace hardware
}  // namespace android
}  // namespace aidl